#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "syntax.h"

struct list_elem *va_make_list(char *fmt, ...) {
	struct list_elem *le;
	struct list_elem *ls;
	struct list_elem *ol;
	ls = ol = list_elem_create();

	int *tmp_int;
	char *tmp_char;
	double *tmp_double;

	va_list argp;
	int lvl = 0;

	va_start(argp, fmt);
	char *end = &fmt[strlen(fmt)];

	for (;fmt<end;fmt++) {
		if (lvl == 0) {
			switch(*fmt) {
				case FS_START: lvl = 1;
									break;
			}
		} else {
			switch (*fmt) {
			case FS_T_STRING:
				le = list_elem_ini(va_arg(argp, char *), String);
				goto match;
			case FS_T_INT:
				tmp_int = malloc(sizeof(int));
				*tmp_int = va_arg(argp, int);
				le = list_elem_ini(tmp_int, Int);
				le->heap = 1;
				goto match;
			case FS_T_CHAR:
				tmp_char = malloc(sizeof(char));
				*tmp_char = (char)va_arg(argp, int);
				le = list_elem_ini(tmp_char, Char);
				le->heap = 1;
				goto match;
			case FS_T_DOUBLE:
				tmp_double = malloc(sizeof(double));
				*tmp_double = va_arg(argp, double);
				le = list_elem_ini(tmp_double, Double);
				le->heap = 1;
				goto match;
			match: ol->next = le;
				ol = le;
				lvl = 0;
				break;
			case FS_START:
				lvl = 0;
				break;
			}
		}
	}

	va_end(argp);
	ol = ls->next;
	ls->next = NULL;
	list_elem_destroy(ls);

	return ol;
}

