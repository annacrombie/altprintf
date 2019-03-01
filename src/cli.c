#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include "list.h"
#include "altprintf.h"
#include "log.h"
//#include <mcheck.h>
struct lconv *locale_info;

struct list_elem *argv_make_list(wchar_t *fmt, int argc, char **argv) {
  struct list_elem *le_cur;
  struct list_elem *le_start;
  struct list_elem *le_prev;
  /* create a dummy element as the head */
  le_start = le_prev = list_elem_create();

  long int *tmp_int;
  wint_t *tmp_char;
  double *tmp_double;
  wchar_t *tmp_str;

  int mode  = 0;
  int arg_i = 0;

  wchar_t *end = &fmt[wcslen(fmt)];

  for (;fmt<end;fmt++) {
    if (mode == 0) {
      switch(*fmt) {
        case FS_START: mode = 1;
                  break;
      }
    } else {
      switch (*fmt) {
        case FS_T_STRING:
                  if (arg_i >= argc) goto no_more_args;
                  int slen = strlen(argv[arg_i]) + 1;
                  tmp_str = calloc(slen, sizeof(wchar_t));
                  mbstowcs(tmp_str, argv[arg_i], slen);
                  le_cur = list_elem_ini(tmp_str, String);
                  goto match;
        case FS_T_INT:
                  if (arg_i >= argc) goto no_more_args;
                  tmp_int = malloc(sizeof(long int));
                  *tmp_int = atol(argv[arg_i]);
                  le_cur = list_elem_ini(tmp_int, Int);
                  goto match;
        case FS_T_CHAR:
                  if (arg_i >= argc) goto no_more_args;
                  tmp_char = malloc(sizeof(wint_t));
                  *tmp_char = btowc(*argv[arg_i]);
                  le_cur = list_elem_ini(tmp_char, Char);
                  goto match;
        case FS_T_DOUBLE:
                  if (arg_i >= argc) goto no_more_args;
                  tmp_double = malloc(sizeof(double));
                  *tmp_double = strtod(argv[arg_i], NULL);
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

/*
void abortfunc(enum mcheck_status ms) {
  //printf();
}
*/

int main(int argc, char **argv) {
  int width;
  char *locale = setlocale(LC_ALL, "");
  LOG("locale: %s\n", locale);
  locale_info = localeconv();

  struct list_elem *ap;
  if (argc < 2) {
    printf("usage: %s format [width [arg1 [arg2 [...]]]]]\n", argv[0]);
    return 1;
  }

  wchar_t *fmt = calloc(strlen(argv[1]) + 1, sizeof(wchar_t));
  mbstowcs(fmt, argv[1], strlen(argv[1]));

  width = argc < 3 ? 80 : atoi(argv[2]);

  if (argc > 3) {
    ap = argv_make_list(fmt, argc - 2, &argv[3]);
  } else {
    ap = list_elem_create();
  }

  LOG("list element contents:\n");
#ifdef DEBUG
  list_elem_inspect_all(ap);
#endif

  wchar_t *str = altsprintf(fmt, ap, width);

  LOG("final output: '%ls'\n", str);
  printf("%ls", str);

  free(fmt);
  free(str);

  list_elem_destroy(ap);
  return 0;
}
