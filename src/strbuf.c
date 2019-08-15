#define _XOPEN_SOURCE
#include <locale.h>
#include <limits.h>
#include "strbuf.h"
#include "log.h"

#define STRBUF_INI_SIZE 5
#define STRBUF_GROW_STEP 100

extern struct lconv *locale_info;

struct strbuf *strbuf_new() {
	struct strbuf *sb = malloc(sizeof(struct strbuf));

	if (NULL == sb) {
		LOG("can't alloc memory for new strbuf", NULL);
		exit(1);
	}

	sb->start = sb->end = calloc(STRBUF_INI_SIZE, sizeof(wchar_t));

	if (NULL == sb->start) {
		LOG("can't alloc memory for new strbuf string", NULL);
		exit(1);
	}

	sb->len = 0;
	sb->cap = STRBUF_INI_SIZE;
	sb->width = 0;
	return sb;
}

void strbuf_destroy(struct strbuf *sb) {
	free(sb->start);
	free(sb);
}

void strbuf_append(struct strbuf *sb, wchar_t c)
{
	wchar_t *ns;

	if (sb->cap < sb->len + 2) {
		ns = calloc(sb->cap + STRBUF_GROW_STEP, sizeof(wchar_t));

		if (ns == NULL) {
			LOG("can't increase size of strbuf to %d\n", sb->cap + STRBUF_GROW_STEP);
			exit(1);
		}

		wcscpy(ns, sb->start);
		free(sb->start);

		sb->start = ns;
		sb->cap += STRBUF_GROW_STEP;
	}

	sb->start[sb->len] = c;
	sb->start[sb->len+1] = L'\0';
	LOG("string so far: %ls\n", sb->start);
	sb->end = &sb->start[sb->len];
	sb->len++;
	int w = wcwidth(c);
	if (w >= 0) sb->width += w;
}

void strbuf_append_strbuf(struct strbuf *sb, void *sbuf)
{
	wchar_t *pos;
	struct strbuf *frm = sbuf;
	LOG("frm->start: %p | frm->end: %p\n", frm->start, frm->end);

	for (pos = frm->start;pos<=frm->end;pos++) {
		strbuf_append(sb, *pos);
	}
}

void strbuf_appendw_strbuf(struct strbuf *sb, void *sbuf, long w)
{
	wchar_t *pos;
	long ws = 0;
	struct strbuf *frm = sbuf;
	LOG("frm->start: %p | frm->end: %p\n", frm->start, frm->end);

	for (pos = frm->start;pos<=frm->end;pos++) {
		ws += wcwidth(*pos);
		LOG("new width would be: %ld, requested width: %ld\n", ws, w);
		if (ws > w) break;
		strbuf_append(sb, *pos);
	}
}

void strbuf_append_char(struct strbuf *sb, void *chr)
{
	wint_t *c = chr;
	strbuf_append(sb, *c);
}

void strbuf_append_str(struct strbuf *sb, void *str, int maxwidth)
{
	wchar_t *s = str;
	wchar_t *end = &s[wcslen(s)];
	int width = 0;
	int maxlen = -1;

	if (maxwidth < 0) {
		maxlen = maxwidth * -1;
	}

	for (;s<end;s++) {
		if (maxlen >= 0) {
			width++;
			if (width > maxlen) return;
		} else {
			width += wcwidth(*s);
			if (width > maxwidth) return;
		}

		strbuf_append(sb, *s);
	}
}

void strbuf_append_int(struct strbuf *sb, void *in)
{
	long int *i = in;
	wchar_t wcs[50];
	swprintf(wcs, 50, L"%ld", *i);
	strbuf_append_str(sb, wcs, 50);
}

void strbuf_append_double(struct strbuf *sb, void *dub, int prec)
{
	double *d = dub;
	wchar_t wcs[50];
	wchar_t format[30];
	swprintf(format, 30, L"%%.%ldf", prec);
	LOG("format: %ls\n", format);
	swprintf(wcs, 50, format, *d);
	strbuf_append_str(sb, wcs, 50);
}

void strbuf_pad(struct strbuf *sb, wchar_t pc, int amnt)
{
	for (;amnt>0;amnt--) strbuf_append(sb, pc);
}

wchar_t *strbuf_cstr(struct strbuf *sb)
{
	wchar_t *str;
	str = calloc(sb->len + 1, sizeof(wchar_t));
	wcscpy(str, sb->start);

	return str;
}
