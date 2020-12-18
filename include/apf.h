#ifndef APF_H
#define APF_H

#include <stdbool.h>
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
	apf_err_branch_too_long,
	apf_err_num_overflow,
	apf_err_missing_id_cb,
	apf_err_missing_sym_cb,
	apf_err_missing_sym,
	apf_err_invalid_unicode,
};

struct apf_err_ctx {
	const char *ctx, *err_pos;
	uint32_t ctx_len;
	enum apf_err err;
	uint8_t ctx_type;
};

void apf_strerr(char *buf, uint32_t blen, struct apf_err_ctx *ctx);

struct apf_template {
	uint16_t len;
	uint8_t *elem;
};

typedef uint16_t ((*apf_parse_sym_cb)(struct apf_err_ctx *err, void *ctx,
				      const char *sym, uint16_t len));

struct apf_template apf_parse(uint8_t *buf, uint32_t blen, const char *fmt,
	void *usr_ctx, apf_parse_sym_cb cb, struct apf_err_ctx *err);

enum apf_arg_type {
	apfat_str,
	apfat_int32_t,
	apfat_float,
};

struct apf_arg {
	union {
		const char *str;
		float flt;
		int32_t i32;
	};
	uint8_t tag;
};

typedef struct apf_arg ((*apf_fmt_id_cb)(struct apf_err_ctx *err, void *ctx,
					 uint16_t id));
typedef struct apf_arg ((*apf_fmt_sym_cb)(struct apf_err_ctx *err, void *ctx,
					  const char *sym, uint16_t len));

uint32_t apf_fmt(char *buf, uint32_t blen, const struct apf_template *apft,
	void *usr_ctx, apf_fmt_id_cb id_cb, apf_fmt_sym_cb sym_cb, struct apf_err_ctx *err);

#define apf_tag(val) _Generic(val, \
	const char*: apf_tag_str, \
	char*: apf_tag_str, \
	float : apf_tag_float, \
	int32_t: apf_tag_int32_t)(val)

struct apf_arg apf_tag_float(float val);
struct apf_arg apf_tag_str(const char *val);
struct apf_arg apf_tag_int32_t(int32_t val);
#endif
