#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include "list.h"
#include "altprintf.h"
#include "log.h"
//#include <mcheck.h>
struct lconv *locale_info;

struct list_elem *argv_make_list(wchar_t *fmt, int argc, int *argi, char **argv) {
	struct list_elem *le_cur;
	struct list_elem *le_start;
	struct list_elem *le_prev;
	/* create a dummy element as the head */
	le_start = le_prev = list_elem_create();

	long int *tmp_int;
	wint_t *tmp_char;
	double *tmp_double;
	wchar_t *tmp_str;

	int mode = 0;

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
				if ((*argi) >= argc) goto no_more_args;
				int slen = strlen(argv[(*argi)]) + 1;
				tmp_str = calloc(slen, sizeof(wchar_t));
				mbstowcs(tmp_str, argv[(*argi)], slen);
				le_cur = list_elem_ini(tmp_str, String);
				goto match;
			case FS_T_MUL:
			case FS_T_TERN:
			case FS_T_ALIGN:
			case FS_T_INT:
				if ((*argi) >= argc) goto no_more_args;
				tmp_int = malloc(sizeof(long int));
				*tmp_int = atol(argv[(*argi)]);
				LOG("got int %ld, %s\n", *tmp_int, argv[(*argi)]);
				le_cur = list_elem_ini(tmp_int, Int);
				goto match;
			case FS_T_CHAR:
				if ((*argi) >= argc) goto no_more_args;
				tmp_char = malloc(sizeof(wint_t));
				*tmp_char = btowc(*argv[(*argi)]);
				le_cur = list_elem_ini(tmp_char, Char);
				goto match;
			case FS_T_DOUBLE:
				if ((*argi) >= argc) goto no_more_args;
				tmp_double = malloc(sizeof(double));
				*tmp_double = strtod(argv[(*argi)], NULL);
				le_cur = list_elem_ini(tmp_double, Double);
				goto match;
			match: le_prev->next = le_cur;
				le_prev = le_cur;
				mode = 0;
				(*argi)++;
				break;
			case FS_START:
				mode = 0;
				break;
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
	setlocale(LC_ALL, "");
	locale_info = localeconv();
	struct list_elem *ap;

	if (argc < 2) {
		printf("usage: %s format [arg1 [arg2 [...]]]\n", argv[0]);
		return 1;
	}

	wchar_t *fmt = calloc(strlen(argv[1]) + 1, sizeof(wchar_t));
	wchar_t *str;
	mbstowcs(fmt, argv[1], strlen(argv[1]));

	int argi = 2;
	int oargi = 0;
	while (argi <= argc) {
		if (argi == oargi) {
			LOG("failed to use up all the arguments\n");
			break;
		} else {
			oargi = argi;
		}
		LOG("argi: %d, argc: %d\n", argi, argc);

		if (argc > 2) {
			ap = argv_make_list(fmt, argc, &argi, argv);
		} else {
			ap = list_elem_create();
		}

		LOG("list element contents:\n");
#ifdef DEBUG
		list_elem_inspect_all(ap);
#endif

		str = altsprintf(fmt, ap);
		list_elem_destroy(ap);

		LOG("pass result: '%ls'\n", str);
		free(fmt);
		fmt = str;
	}

	LOG("final output: '%ls'\n", str);
	printf("%ls", str);
	free(str);

	return 0;
}
