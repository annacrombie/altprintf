#include <ruby.h>
#include <ruby/encoding.h>
#include "extconf.h"
#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include "../src/list.h"
#include "../src/altprintf.h"
#include "../src/log.h"

#define MODNAME "AltPrintf"

#define FS_A_HASHSTART '{'
#define FS_A_HASHEND   '}'

struct list_elem *rb_altprintf_make_list(wchar_t *fmt, VALUE *argv, VALUE *hash) {
  struct list_elem *le_cur;
  struct list_elem *le_start;
  struct list_elem *le_prev;
  /* create a dummy element as the head */
  le_start = le_prev = list_elem_create();

  long int *tmp_int;
  wint_t *tmp_char;
  double *tmp_double;
  wchar_t *tmp_str;
  char *sstr;

  int mode  = 0;
  long argc = rb_array_len(*argv);
  int arg_i = 0;

  wchar_t *end = &fmt[wcslen(fmt)];

  for (;fmt<end;fmt++) {
    LOG("checking char '%lc', lvl: '%d'\n", (wint_t)(*fmt), mode);
    if (mode == 0) {
      switch(*fmt) {
        case FS_START: mode = 1;
                  break;
      }
    } else {
      switch (*fmt) {
        case FS_A_CHARARG:
          fmt++;
          break;
        case FS_A_STRINGSTART:
          while (fmt < end && *fmt != FS_A_STRINGEND) fmt++;
          break;
        case FS_T_STRING:
                  if (arg_i >= argc) goto no_more_args;
		  VALUE entry = rb_ary_entry(*argv, arg_i);
                  long slen = rb_str_strlen(entry);
		  char *sstr = StringValueCStr(entry);
                  tmp_str = calloc(slen, sizeof(wchar_t));
                  mbstowcs(tmp_str, sstr, slen);
                  le_cur = list_elem_ini(tmp_str, String);
                  goto match;
        case FS_T_MUL:
        case FS_T_TERN:
        case FS_T_ALIGN:
        case FS_T_INT:
                  if (arg_i >= argc) goto no_more_args;
                  tmp_int = malloc(sizeof(long int));
                  *tmp_int = FIX2LONG(rb_ary_entry(*argv, arg_i));
                  LOG("got int %ld\n", *tmp_int);
                  le_cur = list_elem_ini(tmp_int, Int);
                  goto match;
        case FS_T_CHAR:
                  if (arg_i >= argc) goto no_more_args;
                  tmp_char = malloc(sizeof(wint_t));
		  entry = rb_ary_entry(*argv, arg_i);
		  sstr = StringValueCStr(entry);
                  *tmp_char = btowc(sstr[0]);
                  le_cur = list_elem_ini(tmp_char, Char);
                  goto match;
        case FS_T_DOUBLE:
                  if (arg_i >= argc) goto no_more_args;
                  tmp_double = malloc(sizeof(double));
                  *tmp_double = RFLOAT_VALUE(rb_ary_entry(*argv, arg_i));
                  le_cur = list_elem_ini(tmp_double, Double);
                  goto match;
        match: le_prev->next = le_cur;
               le_prev = le_cur;
               mode = 0;
               arg_i++;
               break;
        case FS_START: mode = 0; break;
      }
    }
  }

no_more_args:
  if (le_start->next == NULL) return le_start;

  /* set cur to the 2nd element and destroy the first one */
  le_cur = le_start->next;
  le_start->next = NULL;
  list_elem_destroy(le_start);

  return le_cur;
}


VALUE rb_alt_printf(size_t argc, VALUE *argv, VALUE self) {
  VALUE fmt, args, hash;
  struct list_elem *le;
  rb_scan_args(argc, argv, "1*:", &fmt, &args, &hash);

  char *sstr = StringValueCStr(fmt);
  long slen = rb_str_strlen(fmt);
  wchar_t* wfmt = calloc(slen, sizeof(wchar_t));
  mbstowcs(wfmt, sstr, slen);

  le = rb_altprintf_make_list(wfmt, &args, &hash);

  wchar_t *str = altsprintf(wfmt, le);
  slen = wcslen(str);
  char *sfinal = calloc(slen, sizeof(wchar_t));
  wcstombs(sfinal, str, slen);
  VALUE final = rb_str_new_cstr(sfinal);
  rb_str_export_locale(final);

  return final;
}

void Init_alt_printf()
{
  VALUE mod = rb_define_module(MODNAME);
  rb_define_module_function(mod, "sprintf", rb_alt_printf, -1);
}
