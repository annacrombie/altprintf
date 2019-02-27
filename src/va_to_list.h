#ifndef VA_TO_LIST_H
#define VA_TO_LIST_H
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "tokens.h"

struct list_elem *va_make_list(char *, ...);

#endif
