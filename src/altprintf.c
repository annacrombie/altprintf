#include "syntax.h"
#include "altprintf.h"
#include "log.h"

enum align {
  Left,
  Right,
  Center
};

struct width {
  long int prec;
  long int pad;
};

struct format {
  wchar_t *stringarg_start;
  wchar_t *stringarg_end;
  wint_t chararg;
  wint_t padchar;
  enum align align;
  struct width width;
  struct list_elem *le;
};

void default_format(struct format *f) {
  struct width width = {.prec = 3, .pad = 0};
  f->stringarg_start = NULL;
  f->stringarg_end = NULL;
  f->chararg = L':';
  f->padchar = L' ';
  f->align = Right;
  f->width = width;
  f->le = NULL;
}

void format(struct strbuf *sb, struct format *f, void (*to_s)(struct strbuf *, void *))
{
  struct strbuf *tmp = strbuf_new();
  to_s(tmp, f->le->data);

  int pad = f->width.pad - tmp->width;

  if (pad > 0) {
    LOG("padding: %d\n", pad);
    switch(f->align) {
      case Right:
        strbuf_append_strbuf(sb, tmp);
        strbuf_pad(sb, f->padchar, pad);
        break;
      case Left:
        strbuf_pad(sb, f->padchar, pad);
        strbuf_append_strbuf(sb, tmp);
        break;
      case Center:
        strbuf_pad(sb, f->padchar, pad/2);
        strbuf_append_strbuf(sb, tmp);
        strbuf_pad(sb, f->padchar, pad/2 + pad%2);
        break;
    }
  } else {
    strbuf_append_strbuf(sb, tmp);
  }

  strbuf_destroy(tmp);
}

wchar_t *altsprintf(wchar_t *fmt, struct list_elem *le) {
  int lvl = 0;
  struct strbuf *sb = strbuf_new();
  wchar_t *end = &fmt[wcslen(fmt)];
  wchar_t *jump;

  void (*append_func)(struct strbuf *, void *);
  struct format f;
  long int *number_p = NULL;

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
          case '.':
            number_p = &f.width.prec;
          case '1': case '2': case '3': case '4': case '5':
          case '6': case '7': case '8': case '9':
            if (number_p == NULL) number_p = &f.width.pad;
            *number_p = wcstol(fmt, &jump, 10);
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
