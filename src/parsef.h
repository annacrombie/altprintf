#ifndef PARSEF_H_
#define PARSEF_H_
#include "fmte.h"
#include "syntax.h"
#include "log.h"

void get_longarg(wchar_t **, wchar_t **, wchar_t, size_t *);
struct fmte *parsef(wchar_t **);

#endif
