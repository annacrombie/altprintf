#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <altprintf.h>

struct lconv *locale_info;
enum apf_err apf_errno;

char *format(const char *fmt, int argc, int *argi, char **argv)
{
	void *tmp;
	struct apf_fmte *f, *head;
	size_t len;
	int loop = 1;
	char *final;

	head = f = apf_parse(&fmt);

	while (loop) {
		if (!(f->type == apf_argt_raw || f->type == apf_argt_end) && (*argi) >= argc)
			goto process_next_fmt;

		switch (f->type) {
		case apf_argt_string:
			len = strlen(argv[(*argi)]) + 1;
			tmp = calloc(len, sizeof(char));
			strcpy(tmp, argv[(*argi)]);
			goto match;
		case apf_argt_mul:
		case apf_argt_tern:
		case apf_argt_align:
		case apf_argt_int:
			tmp = malloc(sizeof(long int));
			long *tmpl = tmp;
			*tmpl = atol(argv[(*argi)]);
			goto match;
		case apf_argt_char:
			tmp = malloc(sizeof(char));
			char *tmpc = tmp;
			*tmpc = *argv[(*argi)];
			goto match;
		case apf_argt_double:
			tmp = malloc(sizeof(double));
			double *tmpd = tmp;
			*tmpd = strtod(argv[(*argi)], NULL);
			goto match;
match:
			f->value = tmp;
			(*argi)++;
			break;
		case apf_argt_raw:
			break;
		case apf_argt_end:
			loop = 0;
			break;
		case apf_argt_none:
			break;
		}

process_next_fmt:
		apf_fmte_push(head, f);
		if (loop)
			f = apf_parse(&fmt);
	}

	final = apf_assemble(head);
	apf_fmte_destroy(head);
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
			"altprintf cli, compiled against libaltprintf v"
			ALTPRINTF_VERSION "\n"
			"Usage: %s format [arg1 [arg2 [...]]]\n",
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

	printf("%s", str);
	free(str);

	return apf_errno;
}
