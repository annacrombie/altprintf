#include <stdlib.h>
#include <stdio.h>
#include "altprintf.h"
#include "log.h"

struct apf_fmte *apf_fmte_ini(void)
{
	struct apf_fmte *f = malloc(sizeof(struct apf_fmte));

	f->parenarg_start = NULL;
	f->parenarg_end = NULL;
	f->parenarg_len = 0;

	f->anglearg_start = NULL;
	f->anglearg_end = NULL;
	f->anglearg_len = 0;

	f->chararg = ' ';
	f->padchar = ' ';
	f->type = apf_argt_none;
	f->align = apf_algn_left;
	f->prec = -1;
	f->pad = 0;
	f->value = NULL;

	f->next = NULL;

	return f;
}

void apf_fmte_push(struct apf_fmte *a, struct apf_fmte *b)
{
	if (a == b)
		return;     // refuse to create an infinite loop

	while (a->next != NULL) a = a->next;

	a->next = b;
}

void apf_fmte_destroy(struct apf_fmte *f)
{
	struct apf_fmte *j;

	while (f != NULL) {
		j = f->next;
		free(f->value);
		free(f);
		f = j;
	}
}

void apf_fmte_inspect(struct apf_fmte *f)
{
	char *parenarg = calloc(f->parenarg_len + 1, sizeof(char));
	char *anglearg = calloc(f->anglearg_len + 1, sizeof(char));
	size_t i;

	for (i = 0; i < f->parenarg_len; i++) parenarg[i] = f->parenarg_start[i];
	for (i = 0; i < f->anglearg_len; i++) anglearg[i] = f->anglearg_start[i];

	printf(
		"Format@%p {\n\
	parenarg_start: %p,\n\
	parenarg_end: %p,\n\
	parenarg_len: %ld,\n\
	(parenarg): %s,\n\
	anglearg_start: %p,\n\
	anglearg_end: %p,\n\
	anglearg_len: %ld,\n\
	(anglearg): %s,\n\
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
