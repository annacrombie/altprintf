#ifndef STRBUF_H
#define STRBUF_H
#include <stdlib.h>

struct strbuf *strbuf_new();
void strbuf_destroy(struct strbuf *sb);
void strbuf_append(struct strbuf *sb, wchar_t);
void strbuf_append_char(struct strbuf *sb, void  *);
void strbuf_append_str(struct strbuf *, void *, int);
void strbuf_append_int(struct strbuf *sb, void *);
void strbuf_append_double(struct strbuf *, void *, int);
void strbuf_append_strbuf(struct strbuf *, void *);
void strbuf_appendw_strbuf(struct strbuf *, void *, long);
void strbuf_pad(struct strbuf *, wchar_t, int);
wchar_t *strbuf_cstr(struct strbuf *);
size_t strbuf_width(struct strbuf *);

struct strbuf {
	wchar_t *start;
	wchar_t *end;
	size_t len;
	size_t width;
	size_t cap;
};
#endif