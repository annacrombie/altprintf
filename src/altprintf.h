#ifndef ALTPRINTF_H
#define ALTPRINTF_H
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <wchar.h>
#include "strbuf.h"
#include "list.h"
#include "tokens.h"

wchar_t *altsprintf(wchar_t *fmt, struct list_elem *le);
#endif
