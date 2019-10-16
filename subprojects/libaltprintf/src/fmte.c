#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "altprintf/enums.h"
#include "altprintf/log.h"
#include "altprintf/fmte.h"

struct fmte *fmte_ini()
{
	struct fmte *f = malloc(sizeof(struct fmte));

	f->parenarg_start = NULL;
	f->parenarg_end = NULL;
	f->parenarg_len = 0;

	f->anglearg_start = NULL;
	f->anglearg_end = NULL;
	f->anglearg_len = 0;

	f->chararg = L' ';
	f->padchar = L' ';
	f->type = FNone;
	f->align = Right;
	f->prec = -1;
	f->pad = 0;
	f->value = NULL;

	f->next = NULL;

	return f;
}

void fmte_push(struct fmte *a, struct fmte *b)
{
	if (a == b)
		return;     // refuse to create an infinite loop

	while (a->next != NULL) a = a->next;

	a->next = b;
}

void fmte_destroy(struct fmte *f)
{
	struct fmte *j;

	while (f != NULL) {
		j = f->next;
		free(f->value);
		free(f);
		f = j;
	}
}

void fmte_inspect(struct fmte *f)
{
	wchar_t *parenarg = calloc(f->parenarg_len + 1, sizeof(wchar_t));
	wchar_t *anglearg = calloc(f->anglearg_len + 1, sizeof(wchar_t));
	size_t i;

	for (i = 0; i < f->parenarg_len; i++) parenarg[i] = f->parenarg_start[i];
	for (i = 0; i < f->anglearg_len; i++) anglearg[i] = f->anglearg_start[i];

	printf(
		"Format@%p {\n\
	parenarg_start: %p,\n\
	parenarg_end: %p,\n\
	parenarg_len: %ld,\n\
	(parenarg): %ls,\n\
	anglearg_start: %p,\n\
	anglearg_end: %p,\n\
	anglearg_len: %ld,\n\
	(anglearg): %ls,\n\
	chararg: %lc,\n\
	padchar: %lc,\n\
	type: %d,\n\
	align: %d,\n\
	prec: %ld,\n\
	pad: %ld,\n\
	value: %p,\n\
	next: %p\n\
}\n",
		f,
		f->parenarg_start,
		f->parenarg_end,
		(unsigned long)f->parenarg_len,
		parenarg,
		f->anglearg_start,
		f->anglearg_end,
		(unsigned long)f->anglearg_len,
		anglearg,
		f->chararg,
		f->padchar,
		f->type,
		f->align,
		f->prec,
		f->pad,
		f->value,
		f->next);

	free(parenarg);
	free(anglearg);
}
