#include "posix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "apf.h"
#include "debug.h"

#define OUT_BLEN 4096
#define ELEM_BLEN 4096

struct arg_ctx {
	const char **argv;
	uint32_t argc;
};

enum sym {
	sym_start = 100,
	sym_time,
	sym_info,
	sym_end
};

static const struct { const char *sym; uint16_t id; } sym_tbl[] = {
	"time", sym_time,
	"info", sym_info,
	NULL,
};

uint16_t
parse_sym_cb(struct apf_err_ctx *err, void *_ctx, const char *sym, uint16_t len)
{
	uint16_t i;

	for (i = 0; sym_tbl[i].sym; ++i) {
		if (strlen(sym_tbl[i].sym) == len
		    && strncmp(sym, sym_tbl[i].sym, len) == 0) {
			return sym_tbl[i].id;
		}
	}

	err->err = apf_err_missing_sym;
	return 0;
}

static struct apf_arg
fmt_sym_cb(struct apf_err_ctx *err, void *_ctx, const char *sym, uint16_t len)
{
	static char str[45] = "sym:";
	memset(&str[4], 0, 45 - 4);
	memcpy(&str[4], sym, len > 40 ? 40 : len);
	return apf_tag(str);
}

static struct apf_arg
fmt_cb(struct apf_err_ctx *err, void *_ctx, uint16_t id)
{
	struct arg_ctx *ctx = _ctx;
	const char *arg;
	char *endptr;
	int32_t ival;
	float fval;

	if (id > sym_start && id < sym_end) {
		switch ((enum sym)id) {
		case sym_time:
		{
			static char timestr[30] = { 0 };
			struct timespec ts = { 0 };
			clock_gettime(CLOCK_REALTIME, &ts);
			struct tm *tm = localtime(&ts.tv_sec);
			strftime(timestr, 29, "%H:%M:%S", tm);
			arg = timestr;
		}
		break;
		case sym_info:
			arg = "jej";
			break;
		default:
			break;
		}
	} else if (id >= ctx->argc) {
		fprintf(stderr, "trying to access arg with id: %d\n", id);
		err->err = apf_err_arg_missing;
		return (struct apf_arg) { 0 };
	} else {
		arg = ctx->argv[id];
	}

	ival = strtol(arg, &endptr, 10);
	if (!*endptr) {
		return apf_tag(ival);
	}

	fval = strtod(arg, &endptr);
	if (!*endptr) {
		return apf_tag(fval);
	}

	return apf_tag(arg);
}

static int
print_err(struct apf_err_ctx *err)
{
	char errbuf[OUT_BLEN] = { 0 };
	apf_strerr(errbuf, OUT_BLEN, err);
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

#ifndef NDEBUG
	apf_verbose = true;
#endif

	uint8_t elems[ELEM_BLEN];
	char buf[OUT_BLEN] = { 0 };

	struct apf_err_ctx err = { 0 };

	struct apf_template apft = apf_parse(elems, ELEM_BLEN, argv[1], NULL,
		NULL, &err);

	if (err.err) {
		return print_err(&err);
	}

	apf_fmt(buf, OUT_BLEN, &apft, &(struct arg_ctx){ &argv[2], argc - 2, },
		fmt_cb, fmt_sym_cb, &err);

	if (err.err) {
		return print_err(&err);
	}

	fputs(buf, stdout);
}
