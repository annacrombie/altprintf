#include "posix.h"

#include <assert.h>
#include <stdlib.h>  // strtol
#include <string.h>  // memcpy

#include "apf.h"
#include "args.h"
#include "common.h"
#include "debug.h"

struct apf_parse_ctx {
	struct apf_template *apft;
	apf_parse_sym_cb sym_cb;
	struct apf_err_ctx *err;
	void *usr_ctx;
	uint16_t cap, argi;
};

static bool parse_template_until(struct apf_parse_ctx *ctx, const char *fmt,
	const char *stop, const char **endptr);

#define DIGIT(character) (character >= '0' && character <= '9')

static bool
chr_in_str(char chr, const char *str)
{
	if (!str) {
		return chr == 0;
	} else {
		for (; *str; ++str) {
			if (chr == *str) {
				return true;
			}
		}
	}

	return false;
}

static bool
parse_escape(struct apf_parse_ctx *ctx, const char *fmt, char *ret)
{
	switch (*(fmt + 1)) {
	case 'e':
		*ret = '\033';
		break;
	case 't':
		*ret = '\t';
		break;
	case 'n':
		*ret = '\n';
		break;
	case 0:
		ctx->err->err = apf_err_incomplete_escape;
		ctx->err->err_pos = fmt;
		return false;
	default:
		*ret = *(fmt + 1);
		break;
	}

	return true;
}

static uint8_t
parse_algn(char c)
{
	switch (c) {
	case '<':
		return apff_align_l;
	case '>':
		return apff_align_r;
	default:
		return 1;
	}
}

static uint8_t
parse_transform(char c)
{
	switch (c) {
	case 'b':
		return apf_trans_binary;
	case 'x':
		return apf_trans_hex;
	default:
		return apf_trans_none;
	}
}

static bool
set_id_hdr(struct apf_parse_ctx *ctx, uint32_t elemi, uint32_t id_len, bool id_exp,
	uint8_t *id_start)
{
	uint16_t id_hdr;
	uint8_t type;

	if (id_exp) {
		id_hdr = id_len;
		type = apft_id_lit;
	} else if (id_len) {
		if (ctx->sym_cb) {
			id_hdr = ctx->sym_cb(ctx->err, ctx->usr_ctx, (char *)id_start, id_len);
			if (ctx->err->err) {
				return false;
			}

			memset(id_start, 0, id_len);
			ctx->apft->len -= id_len;
			type = apft_id_num;
		} else {
			id_hdr = id_len;
			type = apft_id_sym;
		}
	} else {
		id_hdr = ctx->argi;
		type = apft_id_num;
		++ctx->argi;
	}

	if (id_hdr >= 16384) {
		ctx->err->err = apf_err_id_too_long;
		return false;
	}

	id_hdr <<= 2;
	id_hdr |= type;

	memcpy(&ctx->apft->elem[elemi + 1], &id_hdr, 2);
	return true;
}

static bool
parse_subexp(struct apf_parse_ctx *ctx, const char *fmt, uint16_t *i, uint32_t *subexp_len, char *stopchr)
{
	const char *endptr = NULL;

	*i += 1;

	uint16_t start_len = ctx->apft->len;
	if (!parse_template_until(ctx, &fmt[*i], stopchr, &endptr)) {
		return false;
	}

	*subexp_len = ctx->apft->len - start_len;
	assert(endptr);
	*i += endptr - &fmt[*i];

	return true;
}

static bool
strtol_check(struct apf_parse_ctx *ctx, uint16_t *i, const char *start)
{
	char *endptr;

	long res = strtol(start, &endptr, 10);

	if (res > apff_max_width) {
		ctx->err->err = apf_err_num_overflow;
		ctx->err->err_pos = start;
		return false;
	}

	ctx->apft->elem[ctx->apft->len] = res;
	assert(endptr);
	*i += endptr - start;
	return true;
}

static bool
parse_elem(struct apf_parse_ctx *ctx, const char *fmt, uint16_t *i)
{
	uint32_t id_len = 0;
	uint16_t elemi = ctx->apft->len;
	uint8_t elem = apft_dat;
	uint8_t tmp;
	uint8_t *id_start = NULL;
	bool id_exp = false;

	ctx->apft->len += apf_data_hdr;
	if (ctx->apft->len >= ctx->cap) {
		goto full_error;
	}

	*i = 1;

	if (fmt[*i] == '=') {
		id_exp = true;
		if (!parse_subexp(ctx, fmt, i, &id_len, ":}")) {
			return false;
		}
	} else {
		id_start = &ctx->apft->elem[ctx->apft->len];
		for (; fmt[*i]; ++(*i)) {
			if (chr_in_str(fmt[*i], ":?}")) {
				break;
			}

			if (ctx->apft->len >= ctx->cap) {
				goto full_error;
			}

			ctx->apft->elem[ctx->apft->len] = fmt[*i];
			++id_len;
			++ctx->apft->len;
		}
	}

	if (!set_id_hdr(ctx, elemi, id_len, id_exp, id_start)) {
		ctx->err->err_pos = &fmt[*i - id_len];
		return false;
	}

	if (fmt[*i] == ':') {
		*i += 1;
	} else if (fmt[*i] == '?') {
		goto parse_conditional;
	} else if (fmt[*i] == '}') {
		goto finished;
	}

	if ((tmp = parse_algn(fmt[*i])) != 1) {
		if (!fmt[*i + 1]) {
			ctx->err->err = apf_err_invalid_fmt;
			ctx->err->err_pos = &fmt[*i + 1];
			return false;
		}

		ctx->apft->elem[ctx->apft->len] = fmt[*i + 1];

		elem |= tmp;
		elem |= apff_align_chr;
		*i += 2;

		if (ctx->apft->len >= ctx->cap) {
			goto full_error;
		}
		++ctx->apft->len;
	}

	if (DIGIT(fmt[*i]) || fmt[*i] == 'w') {
		elem |= apff_width;

		if (ctx->apft->len >= ctx->cap) {
			goto full_error;
		}

		if (fmt[*i] == 'w') {
			ctx->apft->elem[ctx->apft->len] = apff_max_width;
			*i += 1;
		} else {
			if (!strtol_check(ctx, i, &fmt[*i])) {
				return false;
			}
		}
		++ctx->apft->len;
	}

	if (fmt[*i] == '.') {
		elem |= apff_prec;

		if (ctx->apft->len >= ctx->cap) {
			goto full_error;
		}

		if (DIGIT(fmt[*i + 1])) {
			*i += 1;
			if (!strtol_check(ctx, i, &fmt[*i])) {
				return false;
			}
		} else {
			ctx->apft->elem[ctx->apft->len] = 0;
			*i += 1;
		}

		++ctx->apft->len;
	}

	if ((tmp = parse_transform(fmt[*i])) != apf_trans_none) {
		elem |= apff_trans;
		*i += 1;

		if (ctx->apft->len >= ctx->cap) {
			goto full_error;
		}
		ctx->apft->elem[ctx->apft->len] = tmp;
		++ctx->apft->len;
	}

	if (fmt[*i] != '}') {
		ctx->err->err = apf_err_invalid_fmt;
		ctx->err->err_pos = &fmt[*i];

		return false;
	}
finished:
	ctx->apft->elem[elemi] = elem;
	return true;

parse_conditional:
	elem |= apff_conditional;
	ctx->apft->elem[elemi] = elem;

	uint32_t s1_len = 0, s2_len = 0, cond_argi = ctx->apft->len;
	if (ctx->apft->len + 2 >= ctx->cap) {
		goto full_error;
	}
	ctx->apft->len += apf_cond_hdr;

	uint16_t argi_start = ctx->argi, arg_max = ctx->argi;

	if (fmt[*i] != ':') {
		if (!parse_subexp(ctx, fmt, i, &s1_len, ":}")) {
			return false;
		}
		arg_max = ctx->argi;
	}

	if (fmt[*i] == ':') {
		ctx->argi = argi_start;
		if (!parse_subexp(ctx, fmt, i, &s2_len, "}")) {
			return false;
		}
		if (ctx->argi > arg_max) {
			arg_max = ctx->argi;
		}
	}

	ctx->argi = arg_max;

	if (s1_len > UINT16_MAX || s2_len > UINT16_MAX) {
		ctx->err->err = apf_err_branch_too_long;
		return false;
	}


	memcpy(&ctx->apft->elem[cond_argi + apf_cond_arm_1], &s1_len, 2);
	memcpy(&ctx->apft->elem[cond_argi + apf_cond_arm_2], &s2_len, 2);

	return true;
full_error:
	ctx->err->err = apf_err_too_many_elements;
	return false;
}

static bool
parse_template_until(struct apf_parse_ctx *ctx, const char *fmt,
	const char *stop, const char **endptr)
{
	struct apf_str {
		const char *start;
		uint32_t len;
	} raw = { 0 };
	uint16_t consumed;
	uint16_t elemi = ctx->apft->len;
	char c;

	for (; *fmt && !chr_in_str(*fmt, stop); ++fmt) {
		if (*fmt == '{') {
			if (raw.start) {
				ctx->apft->elem[elemi] = apft_raw | (raw.len << 1);
				raw.start = NULL;
				raw.len = 0;
			}

			if (!parse_elem(ctx, fmt, &consumed)) {
				return false;
			}
			fmt += consumed;

			elemi = ctx->apft->len;

			continue;
		}

		if (!raw.start) {
			raw.start = fmt;
			ctx->apft->len += 1;
		}

		if (*fmt == '\\') {
			if (!parse_escape(ctx, fmt, &c)) {
				return false;
			}
			++fmt;
		} else {
			c = *fmt;
		}

		if (ctx->apft->len > ctx->cap) {
			goto full_error;
		}
		ctx->apft->elem[ctx->apft->len] = c;
		++ctx->apft->len;
		++raw.len;

		if (raw.len >= 127) {
			assert(raw.len == 127);
			ctx->apft->elem[elemi] = apft_raw | (raw.len << 1);
			elemi = ctx->apft->len;
			raw.start = NULL;
			raw.len = 0;
		}
	}

	if (stop && *fmt == 0) {
		ctx->err->err_pos = fmt;
		ctx->err->err = apf_err_unterminated_subexp;
		return false;
	}

	if (raw.start) {
		ctx->apft->elem[elemi] = apft_raw | (raw.len << 1);
	}

	*endptr = fmt;

	return true;
full_error:
	ctx->err->err = apf_err_too_many_elements;
	return false;
}

struct apf_template
apf_parse(uint8_t *buf, uint32_t cap, const char *fmt, void *usr_ctx,
	apf_parse_sym_cb sym_cb, struct apf_err_ctx *err)
{
	struct apf_template apft = { .elem = buf };

	struct apf_parse_ctx ctx = {
		.apft = &apft,
		.sym_cb = sym_cb,
		.err = err,
		.cap = cap,
		.usr_ctx = usr_ctx,
	};

	err->ctx = fmt;

	const char *endptr;
	parse_template_until(&ctx, fmt, NULL, &endptr);

	PRINT_PARSE_TREE(&apft, err);

	return apft;
}
