#include "posix.h"

#include <assert.h>
#include <string.h> // mem{set,move,cpy}

#include "cswidth.h"
#include "fmt.h"
#include "log.h"

struct apf_interp_ctx {
	const struct apf_template *apft;
	struct apf_err_ctx *err;
	char *buf;
	uint32_t bufi, blen;
	void *usr_ctx;
	apf_template_cb cb;
};

union apf_id {
	struct { uint8_t *id; uint8_t len; } sym;
	uint8_t num;
	bool exp;
};

static uint32_t
format_float(char *buf, uint32_t blen, float val, uint8_t prec)
{
	uint64_t ival;
	uint16_t ilen = prec ? 1 : 0, ilen_cpy;
	uint8_t prec_cpy = prec;
	bool neg = false;

	if (val < 0) {
		neg = true;
		val *= -1;
	}

	if (prec) {
		for (; prec_cpy; --prec_cpy) {
			val *= 10;
		}
	}

	ival = val + 0.5;
	for (; ival; ival /= 10) {
		++ilen;
	}

	if (neg) {
		++ilen;
	}

	ilen_cpy = ilen;
	if (ilen > 255 || ilen >= blen) {
		return 0;
	}

	if (neg) {
		*buf = '-';
		buf = buf + 1;
		--ilen;
	}

	for (ival = val + 0.5; ival; ival /= 10) {
		--ilen;
		buf[ilen] = (ival % 10) + '0';

		if (prec && ++prec_cpy == prec) {
			--ilen;
			buf[ilen] = '.';
		}
	}

	return ilen_cpy;
}

static uint32_t
format_int_plain(char *buf, uint32_t blen, int32_t val)
{
	if (!val) {
		assert(blen);
		buf[0] = '0';
		return 1;
	}

	uint32_t ilen = 0, ret = 0;
	uint8_t neg = val < 0;
	int32_t tmp;

	if (neg) {
		val *= -1;
	}
	tmp = val;

	for (; tmp; tmp /= 10) {
		++ilen;
	}

	if (neg) {
		++ilen;
	}

	if (ilen >= blen) {
		return 0;
	}

	ret = ilen;

	if (neg) {
		buf[0] = '-';
	}
	for (tmp = val; tmp; tmp /= 10) {
		--ilen;
		buf[ilen] = (tmp % 10) + '0';
	}

	return ret;
}

static uint32_t
format_int_bin(char *buf, uint32_t blen, uint32_t val)
{
	if (!val) {
		assert(blen);
		buf[0] = '0';
		return 1;
	}

	uint8_t bufi = 0;
	bool started = false;

	int32_t i;

	for (i = 31; i >= 0; --i) {
		if (!started) {
			if (val & (1 << i)) {
				started = true;
			} else {
				continue;
			}
		}

		if (bufi >= blen) {
			return 0;
		}
		buf[bufi] = val & (1 << i) ? '1' : '0';
		++bufi;
	}

	return bufi;
}


static uint32_t
format_int_hex(char *buf, uint32_t blen, uint32_t val)
{
	uint8_t bufi = 0;
	bool started = false;
	uint32_t v;
	int32_t i;

	for (i = 0; i < 8; ++i) {
		v = val >> 28;

		if (!started) {
			if (v) {
				started = true;
			} else {
				goto cont;
			}
		}

		if (bufi >= blen) {
			return 0;
		}
		if (v < 10) {
			buf[bufi] = v + '0';
		} else {
			buf[bufi] = v - 10 + 'a';
		}
		++bufi;
cont:
		val <<= 4;
	}

	return bufi;
}

static uint32_t
format_int(char *buf, uint32_t blen, int32_t val, enum apf_transform trans)
{
	switch (trans) {
	case apf_trans_binary:
		return format_int_bin(buf, blen, val);
	case apf_trans_hex:
		return format_int_hex(buf, blen, val);
	case apf_trans_none:
		return format_int_plain(buf, blen, val);
	default:
		assert(false);
		return 0;
	}
}

static bool
pad_insert_str(struct apf_interp_ctx *ctx, char fill, uint8_t algn, uint16_t width,
	struct apf_data *arg)
{
	uint16_t pad = 0;

	if (width > arg->width) {
		pad = width - arg->width;
	} else {
		ctx->bufi += arg->size;
		return true;
	}

	if (arg->size + pad >= ctx->blen) {
		ctx->err->err = apf_err_buf_full;
		return false;
	}

	switch (algn) {
	case apff_align_l:
		memset(&ctx->buf[ctx->bufi + arg->size], fill, pad);
		break;
	case apff_align_r:
		memmove(&ctx->buf[ctx->bufi + pad], arg->dat.str, arg->size);
		memset(&ctx->buf[ctx->bufi], fill, pad);
		break;
	}

	ctx->bufi += pad + arg->size;

	return true;
}

static bool
parse_subexp(struct apf_interp_ctx *ctx,
	uint8_t *start, uint32_t len, struct apf_data *arg)
{
	struct apf_template partial = { .len = len, .elem = start };

	arg->size = apf_fmt(&ctx->buf[ctx->bufi], ctx->blen - ctx->bufi,
		&partial, ctx->usr_ctx, ctx->cb, ctx->err);
	arg->dat.str = &ctx->buf[ctx->bufi];

	if (ctx->err->err) {
		return false;
	}

	return true;
}

static bool
format_data_basic(struct apf_interp_ctx *ctx, uint32_t *i, uint32_t j,
	union apf_id *id)
{
	struct apf_data arg = { 0 };
	struct {
		uint16_t width;
		char fill;
		uint8_t algn, prec, transform;
	} args = { 0 };

	/* collect arguments */
	if (ctx->apft->elem[*i] & apff_align_chr) {
		args.fill = ctx->apft->elem[j];
		++j;
	} else {
		args.fill = ' ';
	}
	if (ctx->apft->elem[*i] & apff_width) {
		if (ctx->apft->elem[j] == apff_max_width) {
			args.width = ctx->blen - ctx->bufi - 1;
		} else {
			args.width = ctx->apft->elem[j];
		}
		++j;
	}
	if (ctx->apft->elem[*i] & apff_prec) {
		args.prec = ctx->apft->elem[j];
		++j;
	}
	if (ctx->apft->elem[*i] & apff_trans) {
		args.transform = ctx->apft->elem[j];
		++j;
	} else {
		args.transform = apf_trans_none;
	}

	/* get argument */
	if (id->exp) {
		if (!parse_subexp(ctx, id->sym.id, id->sym.len, &arg)) {
			return false;
		}
	} else if (ctx->cb(ctx->err, ctx->usr_ctx, &arg)) {
		switch (arg.type) {
		case apf_data_int:
			arg.width = arg.size = format_int(&ctx->buf[ctx->bufi],
				ctx->blen - ctx->bufi, arg.dat.i, args.transform);
			if (!arg.size) {
				goto full_err;
			}

			arg.dat.str = &ctx->buf[ctx->bufi];
			break;
		case apf_data_float:
			arg.width = arg.size = format_float(&ctx->buf[ctx->bufi],
				ctx->blen - ctx->bufi, arg.dat.flt, args.prec);
			if (!arg.size) {
				goto full_err;
			}

			arg.dat.str = &ctx->buf[ctx->bufi];
			break;
		case apf_data_string:
			if (ctx->bufi + arg.size >= ctx->blen) {
				goto full_err;
			}
			memcpy(&ctx->buf[ctx->bufi], arg.dat.str, arg.size);
			arg.width = cswidth(arg.dat.str, arg.size);
			break;
		}
	} else {
		return false;
	}

	if (!pad_insert_str(ctx, args.fill,
		ctx->apft->elem[*i] & apff_align, args.width,
		&arg)) {
		return false;
	}

	*i = j - 1;

	return true;
full_err:
	ctx->err->err = apf_err_buf_full;
	return false;
}

static bool
format_data_conditional(struct apf_interp_ctx *ctx, uint32_t *i, uint32_t j)
{
	struct apf_data arg = { 0 };
	uint8_t arm[2] = { ctx->apft->elem[j], ctx->apft->elem[j + 1] };
	j += 2;

	bool b;

	if (ctx->cb(ctx->err, ctx->usr_ctx, &arg)) {
		switch (arg.type) {
		case apf_data_int:
			b = arg.dat.i != 0;
			break;
		case apf_data_string:
			b = arg.size != 0;
			break;
		case apf_data_float:
			b = arg.dat.flt != 0.0f;
			break;
		}
	} else {
		return false;
	}

	if (b && arm[0]) {
		if (!parse_subexp(ctx, &ctx->apft->elem[j], arm[0], &arg)) {
			return false;
		}
		ctx->bufi += arg.size;
	} else if (!b && arm[1]) {
		if (!parse_subexp(ctx, &ctx->apft->elem[j + arm[0]], arm[1], &arg)) {
			return false;
		}
		ctx->bufi += arg.size;
	}

	j += arm[0] + arm[1];

	*i = j - 1;
	return true;
}

static bool
format_data(struct apf_interp_ctx *ctx, uint32_t *i)
{
	uint32_t j;
	union apf_id id = { 0 };

	j = *i + 1;
	switch ((enum apf_type)ctx->apft->elem[j] & 0x3) {
	case apft_id_num:
		id.num = ctx->apft->elem[j] >> 2;
		j += 1;
		break;
	case apft_id_sym:
		id.sym.len = ctx->apft->elem[j] >> 2;
		id.sym.id = &ctx->apft->elem[j + 1];
		j += 1 + id.sym.len;
		break;
	case apft_id_lit:
		id.sym.len = ctx->apft->elem[j] >> 2;
		id.sym.id = &ctx->apft->elem[j + 1];
		j += 1 + id.sym.len;
		break;
	}

	/* check for conditional */
	if (ctx->apft->elem[*i] & apff_conditional) {
		return format_data_conditional(ctx, i, j);
	} else {
		return format_data_basic(ctx, i, j, &id);
	}

}

uint32_t
apf_fmt(char *buf, uint32_t blen, const struct apf_template *apft,
	void *usr_ctx, apf_template_cb cb,
	struct apf_err_ctx *err)
{
	uint32_t i, len;
	struct apf_interp_ctx ctx = {
		.apft = apft, .err = err, .buf = buf, .blen = blen,
		.usr_ctx = usr_ctx, .cb = cb,
	};

	for (i = 0; i < apft->len; ++i) {
		switch (apft->elem[i] & 0x1) {
		case apft_dat:
			if (!format_data(&ctx, &i)) {
				return false;
			}
			break;
		case apft_raw:
			len = apft->elem[i] >> 1;

			if (len + ctx.bufi >= blen) {
				err->err = apf_err_buf_full;
				return 0;
			}

			memcpy(&buf[ctx.bufi], &apft->elem[i + 1], len);
			i += len;
			ctx.bufi += len;
			break;
		}
	}

	return ctx.bufi;
}
