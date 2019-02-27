#ifndef STRBUF_H
#define STRBUF_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <wchar.h>

struct strbuf *strbuf_new();
void strbuf_destroy(struct strbuf *sb);
void strbuf_append(struct strbuf *sb, wchar_t);
void strbuf_append_char(struct strbuf *sb, void  *);
void strbuf_append_str(struct strbuf *, void *);
void strbuf_append_int(struct strbuf *sb, void *);
void strbuf_append_double(struct strbuf *, void *);
void strbuf_append_strbuf(struct strbuf *, void *);
void strbuf_pad(struct strbuf *, wchar_t, int);
wchar_t *strbuf_cstr(struct strbuf *);

struct strbuf {
  wchar_t *start;
  wchar_t *end;
  size_t len;
  size_t cap;
};
#endif
