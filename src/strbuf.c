#define _XOPEN_SOURCE
#include <locale.h>
#include <limits.h>
#include "strbuf.h"
#include "log.h"

#define STRBUF_INI_SIZE 5
#define STRBUF_GROW_STEP 5

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

  for (pos = frm->start;pos<=frm->end;pos++) strbuf_append(sb, *pos);
}

void strbuf_append_char(struct strbuf *sb, void *chr)
{
  wint_t *c = chr;
  strbuf_append(sb, *c);
}

void strbuf_append_str(struct strbuf *sb, void *str)
{
  wchar_t *s = str;
  wchar_t *end = &s[wcslen(s)];

  for (;s<end;s++) strbuf_append(sb, *s);
}

void strbuf_append_int(struct strbuf *sb, void *in)
{
  long int *i;
  wchar_t wcs[50];
  swprintf(wcs, 50, L"%d", *i);
  strbuf_append_str(sb, wcs);
}

void strbuf_append_double(struct strbuf *sb, void *dub)
{
  double *d = dub;
  wchar_t wcs[50];
  swprintf(wcs, 50, L"%f", *d);
  strbuf_append_str(sb, wcs);
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
