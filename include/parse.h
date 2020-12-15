#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>

#include "apf.h"
#include "err.h"

struct apf_template apf_parse(uint8_t *buf, uint32_t blen, const char *fmt,
	struct apf_err_ctx *err);
#endif

