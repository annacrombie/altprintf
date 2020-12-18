#ifndef CSWIDTH_H
#define CSWIDTH_H

#include <stdint.h>
#include <stdbool.h>

bool cswidth(const char *utf8, uint32_t n, uint32_t *width);
#endif
