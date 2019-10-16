#include <stdio.h>
#include <locale.h>
#include <string.h>
#include "altprintf/log.h"
#include "altprintf/parsef.h"
#include "altprintf/altprintf.h"

struct lconv *locale_info;
enum altprintf_err apf_err;

char *format(char *fmt, int argc, int *argi, char **argv)
{
	void *tmp;
	struct fmte *f, *head;
	size_t len;
	int loop = 1;
	char *final;

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
			tmp = calloc(len, sizeof(char));
			strcpy(tmp, argv[(*argi)]);
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
			tmp = malloc(sizeof(char));
			char *tmpc = tmp;
			*tmpc = *argv[(*argi)];
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
		if (loop)
			f = parsef(&fmt);
	}

	LOG("got all fmt elements\n");
	final = assemble_fmt(head);
	fmte_destroy(head);
	return final;
}

int main(int argc, char **argv)
{
	char *fmt, *str;
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

	fmt = calloc(strlen(argv[1]) + 1, sizeof(char));
	strcpy(fmt, argv[1]);
	str = NULL;

	argi = 2;
	oargi = 0;
	while ((argc == 2 || argi < argc) && argi != oargi) {
		oargi = argi;

		str = format(fmt, argc, &argi, argv);
		free(fmt);
		fmt = str;

		if (argc == 2)
			break;
	}

	LOG("final output: '%s'\n", str);
	printf("%s", str);
	free(str);

	return apf_err;
}
