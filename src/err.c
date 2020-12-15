#include "posix.h"

#include "err.h"

#include <stdio.h>

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
	[apf_err_too_many_elements] = "too many elements",
	[apf_err_branch_too_long] = "branch is too long",
	[apf_err_num_overflow] = "number too big",
};

void
apf_strerr(char *buf, uint32_t blen, struct apf_err_ctx *ctx)
{
	uint32_t bufi = 0;

	bufi += snprintf(buf, blen, "%s\n", apf_err_str[ctx->err]);

	if (!ctx) {
		return;
	}

	if (blen <= bufi) {
		return;
	}
	bufi += snprintf(&buf[bufi], blen - bufi, "%s\n", ctx->ctx);

	uint32_t i;
	for (i = 0; i < ctx->err_pos - ctx->ctx; ++i) {
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
