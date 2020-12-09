#ifndef LOG_H
#define LOG_H
#ifndef NDEBUG
#include <stdio.h>

#define L(...) do { \
		fprintf(stderr, "%s:%d [\033[35m%s\033[0m] ", __FILE__, __LINE__, __func__); \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
} while (0)
#else
#define L(...)
#endif
#endif
