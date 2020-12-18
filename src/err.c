#include "posix.h"

#include <assert.h>
#include <stdio.h>  //snprintf
#include <string.h> // memcpy

#include "apf.h"
#include "common.h"
#include "cswidth.h"

static const char *apf_err_str[] = {
	[apf_err_ok] = "no error",
	[apf_err_arg] = "invalid argument",
	[apf_err_arg_missing] = "missing argument",
	[apf_err_buf_full] = "output buffer full",
	[apf_err_id_too_long] = "id too long",
	[apf_err_incomplete_escape] = "incomplete format",
	[apf_err_invalid_fmt] = "invalid format",
	[apf_err_invalid_spec] = "invalid specifier",
	[apf_err_unterminated_subexp] = "unterminated subexpression",
	[apf_err_invalid_transform] = "invalid transform",
	[apf_err_branch_too_long] = "branch is too long",
	[apf_err_num_overflow] = "number too big",
	[apf_err_missing_id_cb] = "missing id callback",
	[apf_err_missing_sym_cb] = "missing symbol callback",
	[apf_err_missing_sym] = "unknown symbol",
	[apf_err_invalid_unicode] = "invalid unicode",
};

static void
write_to_buf(char *buf, uint32_t *bufi, uint32_t blen, const char *src, uint32_t len)
{
	if (*bufi + len > blen) {
		return;
	}

	memcpy(&buf[*bufi], src, len);
	*bufi += len;
}

uint32_t
rebuild_fmt_str(char *buf, uint32_t blen, const uint8_t *elem,
	uint32_t elem_len, const uint8_t **err_pos)
{
	uint32_t i, len, bufi = 0;
	uint8_t dat_hdr;
	uint16_t id_hdr;
	uint16_t cond_hdr[2];

	for (i = 0; i < elem_len; ++i) {
		if (&elem[i] == *err_pos) {
			*err_pos = (uint8_t *)&buf[bufi];
		}

		switch (elem[i] & 0x1) {
		case apft_dat:
			write_to_buf(buf, &bufi, blen, "{", 1);

			dat_hdr = elem[i];
			memcpy(&id_hdr, &elem[i + 1], 2);

			i += 3;

			if (dat_hdr & apff_conditional) {
				switch (id_hdr & 0x3) {
				case apft_id_num:
					break;
				case apft_id_sym:
					len = id_hdr >> 2;
					write_to_buf(buf, &bufi, blen, (char *)&elem[i], len);
					i += len;
					break;
				default:
					assert(false);
					break;
				}

				memcpy(cond_hdr, &elem[i], 4);
				i += 4;

				write_to_buf(buf, &bufi, blen, "?", 1);

				if (cond_hdr[0]) {
					bufi += rebuild_fmt_str(&buf[bufi], blen - bufi, &elem[i], cond_hdr[0], err_pos);
					i += cond_hdr[0];
				}

				write_to_buf(buf, &bufi, blen, ":", 1);

				if (cond_hdr[1]) {
					bufi += rebuild_fmt_str(&buf[bufi], blen - bufi, &elem[i], cond_hdr[1], err_pos);
					i += cond_hdr[1];
				}
			} else {
				switch (id_hdr & 0x3) {
				case apft_id_num:
					break;
				case apft_id_sym:
					len = id_hdr >> 2;
					write_to_buf(buf, &bufi, blen, (char *)&elem[i], len);
					i += len;
					break;
				case apft_id_lit:
					len = id_hdr >> 2;
					write_to_buf(buf, &bufi, blen, "=", 1);
					bufi += rebuild_fmt_str(&buf[bufi], blen - bufi, &elem[i], len, err_pos);
					i += len;
					break;
				default:
					assert(false);
					break;
				}

				write_to_buf(buf, &bufi, blen, ":", 1);

				if (dat_hdr & apff_align_chr) {
					write_to_buf(buf, &bufi, blen, (char [2]){
						dat_hdr & apff_align ? '<' : '>',
						elem[i]
					}, 2);
					i += 1;
				}

				if (dat_hdr & apff_width) {
					bufi += snprintf(&buf[bufi], blen - bufi, "%d", elem[i]);
					i += 1;
				}

				if (dat_hdr & apff_prec) {
					bufi += snprintf(&buf[bufi], blen - bufi, ".%d", elem[i]);
					i += 1;
				}

				if (dat_hdr & apff_trans) {
					switch ((enum apf_transform)elem[i]) {
					case apf_trans_none:
						assert(false);
						break;
					case apf_trans_binary:
						write_to_buf(buf, &bufi, blen, "b", 1);
						break;
					case apf_trans_hex:
						write_to_buf(buf, &bufi, blen, "x", 1);
						break;
					}
					i += 1;
				}
			}

			write_to_buf(buf, &bufi, blen, "}", 1);
			i -= 1;

			break;
		case apft_raw:
			len = elem[i] >> 1;
			write_to_buf(buf, &bufi, blen, (char *)&elem[i + 1], len);
			i += len;
			break;
		}
	}

	return bufi;
}

void
apf_strerr(char *buf, uint32_t blen, struct apf_err_ctx *ctx)
{
	uint32_t bufi = 0, err_idx;
	char *fmt_str_start;
	const uint8_t *rebuilt_err_pos, *rebuilt;

	bufi += snprintf(buf, blen, "%s\n", apf_err_str[ctx->err]);
	fmt_str_start = &buf[bufi];

	switch (ctx->ctx_type) {
	case 0:
		if (blen <= bufi) {
			return;
		}
		bufi += snprintf(&buf[bufi], blen - bufi, "%s", ctx->ctx);
		err_idx = ctx->err_pos - ctx->ctx;
		break;
	case 1:
		rebuilt_err_pos = (uint8_t *)ctx->err_pos;
		rebuilt = (uint8_t *)&buf[bufi];
		bufi += rebuild_fmt_str(&buf[bufi], blen - bufi, (uint8_t *)ctx->ctx,
			ctx->ctx_len, &rebuilt_err_pos);

		err_idx = rebuilt_err_pos - rebuilt;
		break;
	default:
		assert(false);

	}

	if (!cswidth(fmt_str_start, err_idx, &err_idx)) {
		return;
	}

	if (bufi >= blen) {
		return;
	}
	buf[bufi] = '\n';
	++bufi;

	uint32_t i;
	for (i = 0; i < err_idx; ++i) {
		buf[bufi] = ' ';
		if (++bufi >= blen) {
			break;
		}
	}

	if (blen <= bufi) {
		return;
	}
	bufi += snprintf(&buf[bufi], blen - bufi, "^\n");
}
