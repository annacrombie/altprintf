#include "cli.h"

struct lconv *locale_info;

wchar_t *format(wchar_t *fmt, int argc, int *argi, char **argv) {
	void *tmp;
	struct fmte *f, *head;
	size_t len;
	int loop = 1;
	wchar_t *final;

	head = f = parsef(&fmt);

	while (loop) {
		LOG("scanned type: %d\n", f->type);

		if (!(f->type == FRaw || f->type == FEnd) && (*argi) >= argc) {
			LOG("out of values\n");
			goto process_next_fmt;
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
			LOG("got int %ld, from string \"%s\"\n", *tmpl, argv[(*argi)]);
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
		case FRaw:
			break;
		case FEnd:
			LOG("EOS (end of string)\n");
			loop = 0;
			break;
		case FNone:
			LOG("error! shouldn' t be none\n");
			break;
		}

process_next_fmt:
		LOG("pushing fmt\n");
#ifdef DEBUG
		fmte_inspect(f);
#endif
		fmte_push(head, f);
		if (loop) f = parsef(&fmt);
	}

	LOG("got all fmt elements\n");
	final = assemble_fmt(head);
	fmte_destroy(head);
	return final;
}

int main(int argc, char **argv) {
	const char *mbfmt;
	wchar_t *fmt, *str;
	size_t len;
	int argi, oargi;

	setlocale(LC_ALL, "");
	locale_info = localeconv();

	if (argc < 2) {
		printf(
			"altprintf v%s\nusage: %s format [arg1 [arg2 [...]]]\n",
			ALTPRINTF_VERSION,
			argv[0]
		);
		return 1;
	}

	mbfmt = argv[1];
	len = mbsrtowcs(NULL, &mbfmt, 0, NULL);
	fmt = calloc(len + 1, sizeof(wchar_t));
	mbsrtowcs(fmt, &mbfmt, len, NULL);

	argi = 2;
	oargi = 0;
	while ((argc == 2 || argi < argc) && argi != oargi) {
		oargi = argi;

		str = format(fmt, argc, &argi, argv);
		free(fmt);
		fmt = str;

		if (argc == 2) break;
	}

	LOG("final output: '%ls'\n", str);
	printf("%ls", str);
	free(str);

	return 0;
}
