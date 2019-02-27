#include "tokens.h"
#include "altprintf.h"
#include "log.h"

enum align {
  Left,
  Right
};

struct format {
  wchar_t *stringarg_start;
  wchar_t *stringarg_end;
  wint_t chararg;
  wint_t padchar;
  enum align align;
  long int prec;
  long int width;
  struct list_elem *le;
};

void default_format(struct format *f) {
  f->stringarg_start = NULL;
  f->stringarg_end = NULL;
  f->chararg = L':';
  f->padchar = L' ';
  f->align = Right;
  f->prec = 3;
  f->width = -1;
  f->le = NULL;
}

void format(struct strbuf *sb, struct format *f, void (*to_s)(struct strbuf *, void *))
{
  struct strbuf *tmp = strbuf_new();
  to_s(tmp, f->le->data);

  int width = wcswidth(tmp->start, tmp->len);
  int pad = f->width - width;
  LOG("padding: %d\n", pad);

  if (f->align == Right) strbuf_pad(sb, f->padchar, pad);
  strbuf_append_strbuf(sb, tmp);
  if (f->align == Left) strbuf_pad(sb, f->padchar, pad);

  strbuf_destroy(tmp);
}

wchar_t *altsprintf(wchar_t *fmt, struct list_elem *le) {
  int lvl = 0;
  struct strbuf *sb = strbuf_new();
  wchar_t *end = &fmt[wcslen(fmt)];
  wchar_t *jump;

  void (*append_func)(struct strbuf *, void *);
  struct format f;

  for (;fmt<end;fmt++) {
    LOG("checking char '%lc', lvl: '%d'\n", (wint_t)(*fmt), lvl);
    switch (lvl) {
      case 0:
        if (*fmt == FS_START) {
          default_format(&f);
          lvl = 1;
        } else {
          strbuf_append(sb, *fmt);
        }; break;
      case 1:
        switch(*fmt) {
          /* special arguments */
          case FS_A_STRINGSTART:
            f.stringarg_start = fmt;
            lvl = 2;
            break;
          case FS_A_CHARARG:
            f.chararg = *(fmt+1);
            break;

          /* standard arguments */
          case FS_A_LALIGN:
            f.align = Left;
            break;
          case FS_A_SPAD:
            f.padchar = FS_A_SPAD;
            break;
          case 0:
            f.padchar = '0';
            break;
          case '1': case '2': case '3': case '4': case '5':
          case '6': case '7': case '8': case '9':
            f.width = wcstol(fmt, &jump, 10);
            fmt = (jump-1);
            break;

          /* types */
          case FS_T_STRING:
            append_func = strbuf_append_str;
            goto match;
          case FS_T_INT:
            append_func = strbuf_append_int;
            goto match;
          case FS_T_CHAR:
            append_func = strbuf_append_char;
            goto match;
          case FS_T_DOUBLE:
            append_func = strbuf_append_double;
          match:
            if (le != NULL && le->type != Null) {
              f.le = le;
              format(sb, &f, append_func);
              le = le->next;
            }
            lvl = 0;
            break;
          case FS_START:
            strbuf_append(sb, FS_START);
            lvl = 0;
            break;
        }; break;
      case 2:
        if (*fmt == FS_A_STRINGEND) {
          lvl = 2;
        }; break;
    }
  }

  wchar_t *str;
  str = strbuf_cstr(sb);

  strbuf_destroy(sb);

  return str;
}
