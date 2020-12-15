#ifndef FMT_H
#define FMT_H

#include <stdbool.h>

#include "apf.h"
#include "err.h"

enum apf_data_type {
	apf_data_float,
	apf_data_string,
	apf_data_int,
};

struct apf_data {
	enum apf_data_type type;
	union {
		const char *str;
		float flt;
		uint32_t i;
	} dat;
	uint32_t size, width;
};

typedef bool ((*apf_template_cb)(struct apf_err_ctx *err, void *ctx,
				 struct apf_data *ret));

uint32_t apf_fmt(char *buf, uint32_t blen, const struct apf_template *apft,
	void *usr_ctx, apf_template_cb cb,
	struct apf_err_ctx *err);
#endif
