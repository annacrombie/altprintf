#include "posix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmt.h"
#include "parse.h"

struct my_template_ctx {
	uint32_t argc, argi;
	const char **argv;
};

static bool
my_template_cb(struct apf_err_ctx *err, void *_ctx, struct apf_data *ret)
{
	struct my_template_ctx *ctx = _ctx;
	const char *arg;

	if (ctx->argi >= ctx->argc) {
		err->err = apf_err_arg_missing;
		return false;
	}
	arg = ctx->argv[ctx->argi];
	++ctx->argi;

	char *endptr = NULL;

	uint32_t ival = strtol(arg, &endptr, 10);
	if (!*endptr) {
		ret->type = apf_data_int;
		ret->dat.i = ival;
		return true;
	}

	float val = strtod(arg, &endptr);
	if (!*endptr) {
		ret->type = apf_data_float;
		ret->dat.flt = val;
		return true;
	}

	ret->type = apf_data_string;
	ret->size = strlen(arg);
	ret->dat.str = arg;

	return true;
}

#define BUF_LEN 81
#define MAX_ELEMS 256

static int
print_err(struct apf_err_ctx *err)
{
	char errbuf[BUF_LEN] = { 0 };
	apf_strerr(errbuf, BUF_LEN, err);
	fprintf(stderr, "\033[31merror\033[0m: %s", errbuf);
	return 1;
}

int
main(int argc, const char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: apf <fmt> [arg [arg [...]]]\n");
		return 1;
	}

	/* printf("'%s', width: %d\n", argv[1], cswidth(argv[1], strlen(argv[1]))); */

	uint8_t elems[MAX_ELEMS];
	struct apf_err_ctx err = { 0 };
	struct apf_template apft = apf_parse(elems, MAX_ELEMS, argv[1], &err);

	if (err.err) {
		return print_err(&err);
	}

	struct my_template_ctx ctx = {
		.argc = argc - 2,
		.argv = argc > 2 ? &argv[2] : NULL,
	};

	char buf[BUF_LEN] = { 0 };
	apf_fmt(buf, BUF_LEN, &apft, &ctx, my_template_cb, &err);

	if (err.err) {
		return print_err(&err);
	}

	fputs(buf, stdout);
}
