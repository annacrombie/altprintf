#ifndef FMTE_H_
#define FMTE_H_
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "enums.h"
#include "log.h"

struct fmte {
	wchar_t *parenarg_start;
	wchar_t *parenarg_end;
	size_t parenarg_len;

	wchar_t *anglearg_start;
	wchar_t *anglearg_end;
	size_t anglearg_len;

	wint_t chararg;

	wint_t padchar;

	enum arg_type type;
	enum align align;

	long prec;
	long pad;

	void *value;

	struct fmte *next;
};

struct fmte *fmte_ini();
void fmte_inspect(struct fmte *);
void fmte_push(struct fmte *, struct fmte *);
void fmte_destroy(struct fmte *);
#endif
