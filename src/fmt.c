#include "posix.h"

#include <assert.h>
#include <string.h> // mem{set,move,cpy}

#include "apf.h"
#include "args.h"
#include "common.h"
#include "cswidth.h"
#include "debug.h"

struct apf_interp_ctx {
	const struct apf_template *apft;
	struct apf_err_ctx *err;
	char *buf;
	uint32_t bufi, blen;
	void *usr_ctx;
	apf_fmt_id_cb id_cb;
	apf_fmt_sym_cb sym_cb;
};

struct apf_data {
	uint32_t size, width;
};

struct apf_id {
	union {
		struct { uint8_t *id; uint16_t len; } sym;
		uint16_t num;
	};
	enum apf_type type;
};

static uint32_t fmt(struct apf_interp_ctx *ctx);

static uint32_t
format_float(struct apf_err_ctx *err, char *buf, uint32_t blen, float val, uint8_t prec)
{
	uint64_t ival;
	uint16_t ilen = prec ? 1 : 0, ilen_cpy;
	uint8_t prec_cpy = prec;
	bool neg = false;

	if (val == 0.0f) {
		if (ilen + 1u + prec > blen) {
			goto full_err;
		}

		buf[0] = '0';
		if (prec) {
			buf[1] = '.';
			ilen = 2;
			for (; prec_cpy; --prec_cpy) {
				buf[ilen] = '0';
				++ilen;
			}

			return ilen;
		} else {
			return 1;
		}
	}

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
		goto full_err;
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
full_err:
	err->err = apf_err_buf_full;
	return 0;
}

static uint32_t
format_int_plain(struct apf_err_ctx *err, char *buf, uint32_t blen, int32_t val)
{
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
		goto full_err;
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

full_err:
	err->err = apf_err_buf_full;
	return 0;
}

static uint32_t
format_int_bin(struct apf_err_ctx *err, char *buf, uint32_t blen, uint32_t val)
{
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
			goto full_err;
		}
		buf[bufi] = val & (1 << i) ? '1' : '0';
		++bufi;
	}

	return bufi;
full_err:
	err->err = apf_err_buf_full;
	return 0;
}


static uint32_t
format_int_hex(struct apf_err_ctx *err, char *buf, uint32_t blen, uint32_t val)
{
	uint16_t bufi = 0;
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
			goto full_err;
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

full_err:
	err->err = apf_err_buf_full;
	return 0;
}

static uint32_t
format_int(struct apf_err_ctx *err, char *buf, uint32_t blen, int32_t val, enum apf_transform trans)
{
	if (!val) {
		assert(blen);
		buf[0] = '0';
		return 1;
	}

	switch (trans) {
	case apf_trans_binary:
		return format_int_bin(err, buf, blen, val);
	case apf_trans_hex:
		return format_int_hex(err, buf, blen, val);
	case apf_trans_none:
		return format_int_plain(err, buf, blen, val);
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
		memmove(&ctx->buf[ctx->bufi + pad], &ctx->buf[ctx->bufi], arg->size);
		memset(&ctx->buf[ctx->bufi], fill, pad);
		break;
	}

	ctx->bufi += pad + arg->size;

	return true;
}

static bool
fmt_subexp(struct apf_interp_ctx *ctx,
	uint8_t *start, uint32_t len, struct apf_data *arg)
{
	struct apf_template partial = { .len = len, .elem = start };

	struct apf_interp_ctx subctx = *ctx;
	subctx.buf = &ctx->buf[ctx->bufi];
	subctx.blen = ctx->blen - ctx->bufi;
	subctx.bufi = 0;
	subctx.apft = &partial;

	arg->size = fmt(&subctx);
	if (ctx->err->err) {
		return false;
	}
	if (!cswidth(&ctx->buf[ctx->bufi], arg->size, &arg->width)) {
		ctx->err->err = apf_err_invalid_unicode;
		return false;
	}

	return true;
}

static bool
format_data_basic(struct apf_interp_ctx *ctx, struct apf_id *id,
	uint32_t *i, uint32_t j)
{
	struct apf_data arg_info = { 0 };
	struct apf_arg arg = { 0 };
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
		args.width = ctx->apft->elem[j];
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

	/* L("{width:%d, fill:'%c', algn:%s, prec:%d, transform:%d}\n", */
	/* 	args.width, args.fill, */
	/* 	args.algn == apff_align_l ? "left" : "right", */
	/* 	args.prec, args.transform); */

	/* get argument */
	if (id->type == apft_id_lit) {
		if (!fmt_subexp(ctx, id->sym.id, id->sym.len, &arg_info)) {
			return false;
		}
	} else {
		if (id->type == apft_id_num) {
			arg = ctx->id_cb(ctx->err, ctx->usr_ctx, id->num);
		} else {
			arg = ctx->sym_cb(ctx->err, ctx->usr_ctx,
				(char *)id->sym.id, id->sym.len);
		}

		if (ctx->err->err) {
			return false;
		}

		switch (arg.tag) {
		case apfat_int32_t:
			arg_info.width = arg_info.size = format_int(ctx->err,
				&ctx->buf[ctx->bufi], ctx->blen - ctx->bufi,
				arg.i32, args.transform);
			if (ctx->err->err) {
				return false;
			}
			break;
		case apfat_float:
			arg_info.width = arg_info.size = format_float(ctx->err,
				&ctx->buf[ctx->bufi], ctx->blen - ctx->bufi,
				arg.flt, args.prec);
			if (ctx->err->err) {
				return false;
			}
			break;
		case apfat_str:
			arg_info.size = strlen(arg.str);
			if (ctx->bufi + arg_info.size >= ctx->blen) {
				goto full_err;
			}
			memcpy(&ctx->buf[ctx->bufi], arg.str, arg_info.size);

			if (!cswidth(&ctx->buf[ctx->bufi], arg_info.size, &arg_info.width)) {
				ctx->err->err = apf_err_invalid_unicode;
				return false;
			}
			break;
		default:
			assert(false);
		}
	}

	if (!pad_insert_str(ctx, args.fill,
		ctx->apft->elem[*i] & apff_align, args.width,
		&arg_info)) {
		return false;
	}

	*i = j - 1;

	return true;
full_err:
	ctx->err->err = apf_err_buf_full;
	return false;
}

static bool
format_data_conditional(struct apf_interp_ctx *ctx, struct apf_id *id,
	uint32_t *i, uint32_t j)
{
	struct apf_data arg_info = { 0 };
	struct apf_arg arg = { 0 };
	uint16_t *arm = (uint16_t *)&ctx->apft->elem[j];
	j += apf_cond_hdr;

	bool b;

	if (id->type == apft_id_num) {
		arg = ctx->id_cb(ctx->err, ctx->usr_ctx, id->num);
	} else {
		arg = ctx->sym_cb(ctx->err, ctx->usr_ctx,
			(char *)id->sym.id, id->sym.len);
	}

	if (ctx->err->err) {
		return false;
	}

	switch (arg.tag) {
	case apfat_int32_t:
		b = arg.i32 != 0;
		break;
	case apfat_str:
		b = strlen(arg.str) != 0;
		break;
	case apfat_float:
		b = arg.flt != 0.0f;
		break;
	default:
		b = true;
	}

	if (b && arm[0]) {
		if (!fmt_subexp(ctx, &ctx->apft->elem[j], arm[0], &arg_info)) {
			return false;
		}
		ctx->bufi += arg_info.size;
	} else if (!b && arm[1]) {
		if (!fmt_subexp(ctx, &ctx->apft->elem[j + arm[0]], arm[1], &arg_info)) {
			return false;
		}
		ctx->bufi += arg_info.size;
	}

	j += arm[0] + arm[1];

	*i = j - 1;
	return true;
}

static bool
format_data(struct apf_interp_ctx *ctx, uint32_t *i)
{
	uint32_t j;
	struct apf_id id = { 0 };

	uint16_t *id_hdr = (uint16_t *)&ctx->apft->elem[*i + 1];
	j = *i + 3;

	/* L("%s:id:", ctx->apft->elem[*i] & apff_conditional ? "cond" : "basic"); */

	switch ((id.type = (*id_hdr & 0x3))) {
	case apft_id_num:
		id.num = *id_hdr >> 2;
		if (!ctx->id_cb) {
			ctx->err->err = apf_err_missing_id_cb;
			return false;
		}
		break;
	case apft_id_sym:
		id.sym.len = *id_hdr >> 2;
		id.sym.id = &ctx->apft->elem[j];
		if (!ctx->sym_cb) {
			ctx->err->err = apf_err_missing_sym_cb;
			return false;
		}
		j += id.sym.len;
		break;
	case apft_id_lit:
		id.sym.len = *id_hdr >> 2;
		id.sym.id = &ctx->apft->elem[j];
		j += id.sym.len;
		break;
	}

	if (ctx->apft->elem[*i] & apff_conditional) {
		return format_data_conditional(ctx, &id, i, j);
	} else {
		return format_data_basic(ctx, &id, i, j);
	}

}

static uint32_t
fmt(struct apf_interp_ctx *ctx)
{
	uint32_t i, len;

	for (i = 0; i < ctx->apft->len; ++i) {
		switch (ctx->apft->elem[i] & 0x1) {
		case apft_dat:
			if (!format_data(ctx, &i)) {
				if (!ctx->err->err_pos) {
					ctx->err->err_pos = (char *)&ctx->apft->elem[i];
				}
				return 0;
			}

			break;
		case apft_raw:
			len = ctx->apft->elem[i] >> 1;

			if (len + ctx->bufi >= ctx->blen) {
				ctx->err->err_pos = (char *)&ctx->apft->elem[i];
				ctx->err->err = apf_err_buf_full;
				return 0;
			}

			memcpy(&ctx->buf[ctx->bufi], &ctx->apft->elem[i + 1], len);
			i += len;
			ctx->bufi += len;
			break;
		}
	}

	return ctx->bufi;
}

uint32_t
apf_fmt(char *buf, uint32_t blen, const struct apf_template *apft,
	void *usr_ctx, apf_fmt_id_cb id_cb, apf_fmt_sym_cb sym_cb,
	struct apf_err_ctx *err)
{
	struct apf_interp_ctx ctx = {
		.apft = apft, .err = err, .buf = buf, .blen = blen,
		.usr_ctx = usr_ctx, .id_cb = id_cb, .sym_cb = sym_cb,
	};

	fmt(&ctx);

	if (err->err) {
		err->ctx_type = 1;
		err->ctx_len = apft->len;
		err->ctx = (char *)apft->elem;
	}

	ctx.buf[ctx.bufi] = 0;
	++ctx.bufi;

	return ctx.bufi;
}
