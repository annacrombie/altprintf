#include "syntax.h"
#include "altprintf.h"
#include "log.h"

void default_format(struct format *f) {
  struct width width = {.prec = -1, .pad = 0};
  f->stringarg_start = NULL;
  f->stringarg_end = NULL;
  f->chararg = L':';
  f->padchar = L' ';
  f->align = Right;
  f->width = width;
  f->le = NULL;
}

void format_mul(struct strbuf *sb, struct format *f)
{
  long int *i = f->le->data;
  strbuf_pad(sb, f->chararg, *i);
}
void format_tern(struct strbuf *sb, struct format *f)
{
  long int *b = f->le->data;
  int first_half = 1;
  wchar_t sep = f->chararg;
  wchar_t *p = f->stringarg_start;
  for (;p<=f->stringarg_end;p++) {
    LOG("*p: %lc, first half? %d, bool: %ld, sep: %lc\n", (wint_t)*p, first_half, *b, (wint_t)sep);
    if (*p == sep) first_half = 0;
    else if (*b && first_half) strbuf_append_char(sb, p);
    else if (!*b && !first_half) strbuf_append_char(sb, p);
  }
}

void format_string(struct strbuf *sb, struct format *f)
{
  int prec = f->width.prec == -1 ? 100000000 : f->width.prec;
  strbuf_append_str(sb, f->le->data, prec);
}
void format_char(struct strbuf *sb, struct format *f)
{
  strbuf_append_char(sb, f->le->data);
}
void format_int(struct strbuf *sb, struct format *f)
{
  strbuf_append_int(sb, f->le->data);
}
void format_double(struct strbuf *sb, struct format *f)
{
  int prec = f->width.prec == -1 ? 3 : f->width.prec;
  strbuf_append_double(sb, f->le->data, prec);
}

void format(struct strbuf *sb, struct format *f, void (*to_s)(struct strbuf *, struct format *))
{
  struct strbuf *tmp = strbuf_new();
  to_s(tmp, f);

  if (tmp->len == 0) return;

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
  int split = 0;
  struct strbuf *sbs[] = {strbuf_new(), strbuf_new()};
  struct strbuf *sb = sbs[0];
  wchar_t *end = &fmt[wcslen(fmt)];
  wchar_t *jump;

  void (*append_func)(struct strbuf *, struct format *);
  struct format f;
  long int *number_p = NULL;
  long int *width;
  wint_t split_pad;

  for (;fmt<end;fmt++) {
    LOG("checking char '%lc', lvl: '%d'\n", (wint_t)(*fmt), lvl);
    switch (lvl) {
      case 0:
        switch(*fmt) {
          case FS_START:
            default_format(&f);
            lvl = 1;
            break;
          case FS_ESC:
            lvl = 3;
            break;
          default:
            strbuf_append(sb, *fmt);
            break;
        }; break;
      case 1:
        switch(*fmt) {
          /* special arguments */
          case FS_A_STRINGSTART:
            f.stringarg_start = fmt + 1;
            lvl = 2;
            break;
          case FS_A_CHARARG:
            f.chararg = *(fmt+1);
            fmt += 1;
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
            fmt++;
          case '1': case '2': case '3': case '4': case '5':
          case '6': case '7': case '8': case '9':
            if (number_p == NULL) number_p = &f.width.pad;
            *number_p = wcstol(fmt, &jump, 10);
            fmt = (jump-1);
            break;

          /* align operator */
          case FS_T_ALIGN:
            if (le != NULL && le->type != Null) {
              width = le->data;
              le = le->next;
            } else {
              goto no_more_args;
            }

            split = 1;
            split_pad = f.chararg;
            sb = sbs[1];
            lvl = 0;
            break;

          /* types */
          case FS_T_STRING:
            append_func = format_string;
            goto match;
          case FS_T_TERN:
            append_func = format_tern;
            goto match;
          case FS_T_INT:
            append_func = format_int;
            goto match;
          case FS_T_MUL:
            append_func = format_mul;
            goto match;
          case FS_T_CHAR:
            append_func = format_char;
            goto match;
          case FS_T_DOUBLE:
            append_func = format_double;
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
          f.stringarg_end = fmt - 1;
          lvl = 1;
        }; break;
      case 3:
        switch(*fmt) {
          case FS_ESC_NL:
            strbuf_append(sb, '\n');
            break;
          case FS_ESC_ESC:
            strbuf_append(sb, '\e');
            break;
          default:
            strbuf_append(sb, *fmt);
            break;
        };
        lvl = 0;
        break;
    }
  }

  wchar_t *str;
no_more_args:
  if (split) {
    LOG("splitting string\n");
    lvl = *width - (sbs[0]->width + sbs[1]->width);
    if (lvl >= 0) {
      LOG("padding center\n");
      strbuf_pad(sbs[0], split_pad, lvl);
      strbuf_append_strbuf(sbs[0], sbs[1]);
    } else if (sbs[0]->width > *width) {
      LOG("the first half is longer than the requested with\n");
      strbuf_destroy(sbs[1]);
      sbs[1] = sbs[0];
      sbs[0] = strbuf_new();
      strbuf_appendw_strbuf(sbs[0], sbs[1], *width);
    } else {
      LOG("just shave some off the last half\n");
      strbuf_appendw_strbuf(sbs[0], sbs[1], *width - sbs[0]->width);
    }
  }

  str = strbuf_cstr(sbs[0]);

  strbuf_destroy(sbs[0]);
  strbuf_destroy(sbs[1]);

  return str;
}
