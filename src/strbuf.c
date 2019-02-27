#define _XOPEN_SOURCE
#include <locale.h>
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
  int *i = in;
  if ((*i) < 0) { strbuf_append(sb, '-'); (*i) *= -1; }

  int div =  1000000000;
  while (div >= 1) {
    int num = ((*i) / div);
    if (num != 0) {
      strbuf_append(sb, (char)(num + 48));
    }
    (*i) %= div;
    div /= 10;
  }
}

void strbuf_append_double(struct strbuf *sb, void *dub)
{
  long int i;
  double *d = dub;
  i = (long int)floor(*d);
  strbuf_append_int(sb, &i);

  strbuf_append(sb, *locale_info->decimal_point);
  i = (long int)round((double)100 * (*d)) % 100;
  strbuf_append_int(sb, &i);
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
