#ifndef ERR_H
#define ERR_H

#include <stdint.h>

enum apf_err {
	apf_err_ok,
	apf_err_arg,
	apf_err_arg_missing,
	apf_err_buf_full,
	apf_err_id_too_long,
	apf_err_incomplete_escape,
	apf_err_invalid_fmt,
	apf_err_invalid_spec,
	apf_err_unterminated_subexp,
	apf_err_invalid_transform,
	apf_err_too_many_elements,
	apf_err_branch_too_long,
	apf_err_num_overflow,
};

struct apf_err_ctx {
	const char *ctx, *err_pos;
	enum apf_err err;
	uint8_t err_hl;
};

void apf_strerr(char *buf, uint32_t blen, struct apf_err_ctx *ctx);
#endif
