#include "cli.h"

struct lconv *locale_info;

void format(wchar_t *fmt, int argc, int *argi, char **argv) {
	void *tmp;
	struct fmte *f;
	size_t len;

	while (1) {
		f = parsef(&fmt);
		LOG("scanned type: %d\n", f->type);

		if (!(f->type == FRaw || f->type == FEnd) && (*argi) >= argc) {
			LOG("out of values\n");
		}

		switch (f->type) {
		case FString:
			len = strlen(argv[(*argi)]) + 1;
			tmp = calloc(len, sizeof(wchar_t));
			mbstowcs(tmp, argv[(*argi)], len);
			goto match;
		case FMul:
		case FTern:
		case FAlign:
		case FInt:
			tmp = malloc(sizeof(long int));
			long *tmpl = tmp;
			*tmpl = atol(argv[(*argi)]);
			LOG("got int %ld, %s\n", *tmpl, argv[(*argi)]);
			goto match;
		case FChar:
			tmp = malloc(sizeof(wint_t));
			wint_t *tmpc = tmp;
			*tmpc = btowc(*argv[(*argi)]);
			goto match;
		case FDouble:
			tmp = malloc(sizeof(double));
			double *tmpd = tmp;
			*tmpd = strtod(argv[(*argi)], NULL);
			goto match;
		match:
			f->value = tmp;
			(*argi)++;
			break;
		case FEnd:
			LOG("EOS (end of string)\n");
			free(f);
			return;
		case FRaw:
			break;
		case FNone:
			LOG("error! shouldn' t be none\n");
			break;
		}

		inspect_format(f);
		free(f->value);
		free(f);
	}
}

int main(int argc, char **argv) {
	setlocale(LC_ALL, "");
	locale_info = localeconv();

	if (argc < 2) {
		printf("usage: %s format [arg1 [arg2 [...]]]\n", argv[0]);
		return 1;
	}

	const char *mbfmt = argv[1];

	size_t len = mbsrtowcs(NULL, &mbfmt, 0, NULL);
	wchar_t *fmt = calloc(len + 1, sizeof(wchar_t));
	mbsrtowcs(fmt, &mbfmt, len, NULL);

	int argi = 2;
	format(fmt, argc, &argi, argv);
	free(fmt);
	//struct fmte *f = parsef(&fmt);
	//inspect_format(f);

	//wchar_t *str;

	/*
	int argi = 2;
	int oargi = 0;

	while (argc == 2 || argi < argc) {
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

		if (argc == 2) break;
	}

	LOG("final output: '%ls'\n", str);
	printf("%ls", str);
	free(str);
	*/

	return 0;
}
