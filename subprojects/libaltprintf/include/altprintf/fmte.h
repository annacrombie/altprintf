#ifndef FMTE_H_
#define FMTE_H_
#include <stdlib.h>
#include "altprintf/enums.h"

struct fmte {
	char *parenarg_start;
	char *parenarg_end;
	size_t parenarg_len;

	char *anglearg_start;
	char *anglearg_end;
	size_t anglearg_len;

	char chararg;

	char padchar;

	enum arg_type type;
	enum align align;

	long prec;
	long pad;

	void *value;

	struct fmte *next;
};

struct fmte *fmte_ini(void);
void fmte_inspect(struct fmte *);
void fmte_push(struct fmte *, struct fmte *);
void fmte_destroy(struct fmte *);
#endif
