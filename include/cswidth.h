#ifndef CSWIDTH_H
#define CSWIDTH_H

#include <stdint.h>

uint8_t rune_length(const char *u8);
int8_t cwidth(const char* u8);
uint32_t cswidth(const char *utf8, uint32_t n);
#endif
