#include "posix.h"

#include <assert.h>
#include <stdlib.h>  // strtol

#include "parse.h"

struct apf_parse_ctx {
	struct apf_template *apft;
	struct apf_err_ctx *err;
	uint16_t cap, argi;
};

static bool parse_template_until(struct apf_parse_ctx *ctx, const char *fmt,
	const char *stop, const char **endptr);

#define DIGIT(character) (character >= '0' && character <= '9')

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

static uint8_t
make_id_byte(struct apf_parse_ctx *ctx, uint32_t id_len, bool id_exp)
{
	uint8_t byte;

	if (id_exp) {
		assert(id_len < 64);
		byte = apft_id_lit | (id_len << 2);
	} else if (id_len) {
		assert(id_len < 64);
		byte = apft_id_sym | (id_len << 2);
	} else {
		assert(ctx->argi < 64);
		byte = apft_id_num | (ctx->argi << 2);
		++ctx->argi;
	}

	return byte;
}

static bool
parse_subexp(struct apf_parse_ctx *ctx, const char *fmt, uint16_t *i, uint16_t *subexp_len, char *stopchr)
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

	if (res >= apff_max_width) {
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
	uint16_t id_len = 0;
	uint16_t elemi = ctx->apft->len;
	uint8_t elem = apft_dat;
	uint8_t tmp;
	bool id_exp = false;

	ctx->apft->len += 2;

	if (ctx->apft->len >= ctx->cap) {
		goto full_error;
	}

	*i = 1;

	if (fmt[*i] == '=') {
		id_exp = true;

		if (!parse_subexp(ctx, fmt, i, &id_len, ":}")) {
			return false;
		}

		if (fmt[*i] == '}') {
			goto finished;
		}

		*i += 1;
	} else {
		for (; fmt[*i]; ++(*i)) {
			if (fmt[*i] == ':') {
				*i += 1;
				break;
			} else if (fmt[*i] == '?') {
				goto parse_conditional;
			} else if (fmt[*i] == '}') {
				goto finished;
			} else if (ctx->apft->len >= ctx->cap) {
				goto full_error;
			}

			ctx->apft->elem[ctx->apft->len] = fmt[*i];
			++id_len;
			++ctx->apft->len;
		}
	}

	if ((tmp = parse_algn(fmt[*i + 1])) != 1) {
		ctx->apft->elem[ctx->apft->len] = fmt[*i];

		elem |= tmp;
		elem |= apff_align_chr;
		*i += 2;

		if (ctx->apft->len >= ctx->cap) {
			goto full_error;
		}
		++ctx->apft->len;
	} else if ((tmp = parse_algn(fmt[*i])) != 1) {
		elem |= tmp;
		*i += 1;
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
	if (id_len >= 64) {
		ctx->err->err = apf_err_id_too_long;
		return false;
	}

	ctx->apft->elem[elemi + 0] = elem;
	ctx->apft->elem[elemi + 1] = make_id_byte(ctx, id_len, id_exp);
	return true;
parse_conditional:
	elem |= apff_conditional;

	uint16_t s1_len = 0, s2_len = 0, cond_argi = ctx->apft->len;
	if (ctx->apft->len + 2 >= ctx->cap) {
		goto full_error;
	}
	ctx->apft->len += 2;

	if (fmt[*i] != ':') {
		if (!parse_subexp(ctx, fmt, i, &s1_len, ":}")) {
			return false;
		}
	}

	if (fmt[*i] == ':') {
		if (!parse_subexp(ctx, fmt, i, &s2_len, "}")) {
			return false;
		}
	}

	if (s1_len > 255 || s2_len > 255) {
		ctx->err->err = apf_err_branch_too_long;
		return false;
	}

	ctx->apft->elem[elemi + 0] = elem;
	ctx->apft->elem[elemi + 1] = make_id_byte(ctx, id_len, id_exp);
	ctx->apft->elem[cond_argi + 0] = s1_len; //s1_len > 0 ? s1_len - 1 : 0;
	ctx->apft->elem[cond_argi + 1] = s2_len; //s2_len > 0 ? s2_len - 1 : 0;

	return true;
full_error:
	ctx->err->err = apf_err_too_many_elements;
	return false;
}

static bool
chr_not_in_str(char chr, const char *str)
{
	if (!str) {
		return chr != 0;
	} else {
		for (; *str; ++str) {
			if (chr == *str) {
				return false;
			}
		}
	}

	return true;
}

static bool
parse_template_until(struct apf_parse_ctx *ctx, const char *fmt, const char *stop, const char **endptr)
{
	struct apf_str {
		const char *start;
		uint32_t len;
	} raw = { 0 };
	uint16_t consumed;
	uint16_t elemi = ctx->apft->len;
	char c;

	for (; *fmt && chr_not_in_str(*fmt, stop); ++fmt) {
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
apf_parse(uint8_t *buf, uint32_t cap, const char *fmt, struct apf_err_ctx *err)
{

	struct apf_template apft = { .elem = buf };

	struct apf_parse_ctx ctx = {
		.apft = &apft,
		.err = err,
		.cap = cap
	};

	err->ctx = fmt;

	const char *endptr;
	parse_template_until(&ctx, fmt, NULL, &endptr);

	return apft;
}
