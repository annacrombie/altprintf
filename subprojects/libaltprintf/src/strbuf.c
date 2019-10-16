#define STRBUF_INI_SIZE 5
#define STRBUF_GROW_STEP 100
#define TMPLEN 50
#define MAXPREC 25

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "altprintf/log.h"
#include "altprintf/strbuf.h"
#include "altprintf/cwidth.h"

extern struct lconv *locale_info;

struct strbuf *strbuf_new(void)
{
	struct strbuf *sb = malloc(sizeof(struct strbuf));

	if (NULL == sb) {
		LOG("can't alloc memory for new strbuf\n");
		exit(1);
	}

	sb->start = sb->end = calloc(STRBUF_INI_SIZE, sizeof(char));

	if (NULL == sb->start) {
		LOG("can't alloc memory for new strbuf string\n");
		exit(1);
	}

	sb->len = 0;
	sb->cap = STRBUF_INI_SIZE;
	sb->width = 0;
	return sb;
}

size_t strbuf_width(struct strbuf *sb)
{
	if (sb->width == 0)
		sb->width = cswidth(sb->start, sb->len);

	return sb->width;
}

void strbuf_destroy(struct strbuf *sb)
{
	free(sb->start);
	free(sb);
}

void strbuf_append(struct strbuf *sb, char c)
{
	char *ns;

	if (sb->cap < sb->len + 2) {
		ns = calloc(sb->cap + STRBUF_GROW_STEP, sizeof(char));

		if (ns == NULL) {
			LOG("can't increase size of strbuf to %d\n", sb->cap + STRBUF_GROW_STEP);
			exit(1);
		}

		strcpy(ns, sb->start);
		free(sb->start);

		sb->start = ns;
		sb->cap += STRBUF_GROW_STEP;
	}

	sb->start[sb->len] = c;
	sb->start[sb->len + 1] = '\0';
	LOG("string so far: %s\n", sb->start);
	sb->end = &sb->start[sb->len];
	sb->len++;
}

void strbuf_append_strbuf(struct strbuf *sb, void *sbuf)
{
	char *pos;
	struct strbuf *frm = sbuf;

	LOG("frm->start: %p | frm->end: %p\n", frm->start, frm->end);

	for (pos = frm->start; pos <= frm->end; pos++)
		strbuf_append(sb, *pos);
}

void strbuf_appendw_strbuf(struct strbuf *sb, void *sbuf, long w)
{
	char *pos;
	long ws = 0;
	struct strbuf *frm = sbuf;

	LOG("frm->start: %p | frm->end: %p\n", frm->start, frm->end);

	for (pos = frm->start; pos <= frm->end; pos++) {
		ws += cwidth(pos);
		LOG("new width would be: %ld, requested width: %ld\n", ws, w);
		if (ws > w)
			break;
		strbuf_append(sb, *pos);
	}
}

void strbuf_append_char(struct strbuf *sb, void *chr)
{
	char *c = chr;

	strbuf_append(sb, *c);
}

void strbuf_append_str(struct strbuf *sb, void *str, int maxwidth)
{
	char *s = str;
	char *end = &s[strlen(s)];
	int width = 0;
	int maxlen = -1;

	if (maxwidth < 0)
		maxlen = maxwidth * -1;

	for (; s < end; s++) {
		if (maxlen >= 0) {
			width++;
			if (width > maxlen)
				return;
		} else {
			width += cwidth(s);
			if (width > maxwidth)
				return;
		}

		strbuf_append(sb, *s);
	}
}

void strbuf_append_int(struct strbuf *sb, void *in)
{
	long int *i = in;
	char wcs[TMPLEN];
	long len = snprintf(wcs, TMPLEN - 1, "%ld", *i);

	if (len < 0)
		strbuf_append_str(sb, "error adding int", 16);
	else
		strbuf_append_str(sb, wcs, -1 * len);
}

void strbuf_append_double(struct strbuf *sb, void *dub, int prec)
{
	double *d = dub;
	char wcs[TMPLEN];
	char format[TMPLEN];
	int rprec = prec;

	if (rprec > MAXPREC)
		rprec = MAXPREC;

	snprintf(format, TMPLEN - 1, "%%.%df", rprec);

	LOG("format: %s\n", format);

	long len = snprintf(wcs, TMPLEN - 1, format, *d);
	if (len < 0) {
		strbuf_append_str(sb, "error adding float", 18);
	} else {
		LOG("inserting double: len: %ld\n", len);
		strbuf_append_str(sb, wcs, len);

		if (rprec < prec)
			strbuf_pad(sb, '0', rprec - prec);
	}
}

void strbuf_pad(struct strbuf *sb, char pc, int amnt)
{
	for (; amnt > 0; amnt--) strbuf_append(sb, pc);
}

char *strbuf_cstr(struct strbuf *sb)
{
	char *cstr;

	cstr = calloc(sb->len, sizeof(char));

	strncpy(cstr, sb->start, sb->len);

	return cstr;
}
