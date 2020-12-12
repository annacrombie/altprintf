#ifndef DEBUG_H
#define DEBUG_H
#ifndef NDEBUG
#include <stdio.h>
#include <stdbool.h>

#include "apf.h"

extern bool apf_verbose;

void apf_dbg_print_parse_tree(struct apf_template *apft, uint32_t depth, uint32_t orig_i);

#define L(...) if (apf_verbose) fprintf(stderr, __VA_ARGS__);
#define PRINT_PARSE_TREE(apft, err) if (!err->err && apf_verbose) apf_dbg_print_parse_tree(apft, 0, 0)
#else
#define L(...)
#endif
#endif
