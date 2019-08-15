#ifndef ALTPRINTF_H
#define ALTPRINTF_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <wchar.h>
#include "strbuf.h"
#include "list.h"
#include "syntax.h"

wchar_t *altsprintf(wchar_t *fmt, struct list_elem *le);

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
  size_t stringarg_len;
  wint_t chararg;
  wint_t padchar;
  enum align align;
  struct width width;
  struct list_elem *le;
};

#endif
