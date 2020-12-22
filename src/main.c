#include "posix.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apf.h"
#include "debug.h"

#define MAX_SYM_ARGS 32

struct arg_ctx {
	char *const *argv;
	uint32_t argc;

	struct {
		char *sym;
		char *val;
	} sym[MAX_SYM_ARGS];
	uint32_t symc;
};

struct opts {
	char *fmt;
	struct arg_ctx arg_ctx;
	uint32_t elem_blen, out_blen;
	bool compile_only,
	     compiled_c_out,
	     read_compiled;
};

static const struct opts defopts = {
	.elem_blen = 4096,
	.out_blen = 4096
};

#define ERR_BLEN 255
static int
print_err(struct apf_err_ctx *err)
{
	char errbuf[ERR_BLEN] = { 0 };
	apf_strerr(errbuf, ERR_BLEN, err);
	fprintf(stderr, "\033[31merror\033[0m: %s", errbuf);
	return 1;
}

static struct apf_arg
coerce_string(const char *str)
{
	char *endptr;

	struct apf_arg arg = { 0 };

	arg.i32 = strtol(str, &endptr, 10);
	if (!*endptr) {
		arg.tag = apfat_int32_t;
		return arg;
	}

	arg.flt = strtod(str, &endptr);
	if (!*endptr) {
		arg.tag = apfat_float;
		return arg;
	}

	arg.tag = apfat_str;
	arg.str = str;
	return arg;
}

static struct apf_arg
fmt_sym_cb(struct apf_err_ctx *err, void *_ctx, const char *sym, uint16_t len)
{
	struct arg_ctx *ctx = _ctx;

	uint32_t i;
	for (i = 0; i < ctx->symc; ++i) {
		if (strlen(ctx->sym[i].sym) == len
		    && strncmp(ctx->sym[i].sym, sym, len) == 0) {
			return coerce_string(ctx->sym[i].val);
		}
	}

	err->err = apf_err_arg_missing;
	return apf_arg_null;
}

static struct apf_arg
fmt_cb(struct apf_err_ctx *err, void *_ctx, uint16_t id)
{
	struct arg_ctx *ctx = _ctx;
	const char *arg;

	if (id >= ctx->argc) {
		err->err = apf_err_arg_missing;
		return apf_arg_null;
	} else {
		arg = ctx->argv[id];
	}

	return coerce_string(arg);
}

static void
print_usage(void)
{
	printf("usage: apf [opts] <fmt> [arg [arg [...]]]\n"
		"       apf -r [opts] <file> [arg [arg [...]]]\n\n"
		"opts:\n"
		"-n <name>=<val> - set named argument\n"
		"-c              - compile only\n"
		"-C              - write compiled output as a C byte array\n"
		"-r              - read compiled format from file\n"
		"-e <num>        - set length of elem buffer\n"
		"                  default: %d\n"
		"-o <num>        - set length of output buffer\n"
		"                  default: %d\n"
#ifndef NDEBUG
		"-v              - enable debugging output\n"
#endif
		"-h              - show this message\n"
		"\n\n"
		"for more information see altprintf(7)\n",
		defopts.elem_blen,
		defopts.out_blen
		);
}

static const char *optstr = "n:rcCo:e:h"
#ifndef NDEBUG
			    "v"
#endif
;

static bool
check_parse_int(const char *strint, uint32_t *res)
{
	char *endptr;
	long parsed = strtol(strint, &endptr, 10);

	if (!*endptr || parsed >= UINT32_MAX) {
		return false;
	}

	*res = parsed;
	return true;
}

static void
parse_opts(struct opts *opts, uint32_t argc, char *const *argv)
{
	signed char opt;
	char *ptr;

	while ((opt = getopt(argc, argv, optstr)) != -1) {
		switch (opt) {
#ifndef NDEBUG
		case 'v':
			apf_verbose = true;
			break;
#endif
		case 'n':
			if (opts->arg_ctx.symc >= MAX_SYM_ARGS) {
				fprintf(stderr, "too many named arguments\n");
				exit(1);
			} else if (!(ptr = strchr(optarg, '='))
				   || !*(ptr + 1)) {
				fprintf(stderr, "usage: -n name=val\n");
				exit(1);
			}

			opts->arg_ctx.sym[opts->arg_ctx.symc].sym = optarg;
			opts->arg_ctx.sym[opts->arg_ctx.symc].val = ptr + 1;
			*ptr = 0;

			++opts->arg_ctx.symc;
			break;
		case 'c':
			opts->compile_only = true;
			break;
		case 'C':
			opts->compiled_c_out = true;
			break;
		case 'r':
			opts->read_compiled = true;
			break;
		case 'e':
			if (!check_parse_int(optarg, &opts->elem_blen)) {
				exit(1);
			}
			break;
		case 'h':
			print_usage();
			exit(0);
			break;
		default:
			print_usage();
			exit(1);
			break;
		}
	}

	if ((uint32_t)optind >= argc) {
		print_usage();
		exit(1);
	}

	opts->fmt = argv[optind];
	opts->arg_ctx.argc = argc - optind - 1;
	opts->arg_ctx.argv = &argv[optind + 1];
}

static bool
fread_ensure(void *ptr, size_t size, FILE *f)
{
	return fread(ptr, 1, size, f) == size;
}

static bool
fwrite_ensure(void *ptr, size_t size, FILE *f)
{
	return fwrite(ptr, 1, size, f) == size;
}

static bool
read_compiled(struct apf_template *apft, uint8_t *elems, uint32_t blen, const char *path)
{
	FILE *f;

	if (strcmp(path, "-") == 0) {
		f = stdin;
	} else if (!(f = fopen(path, "r"))) {
		return false;
	}

	apft->elem = elems;

	if (!(fread_ensure(&apft->len, 2, f)
	      && fread_ensure(&apft->flags, 2, f))) {
		return false;
	}

	if (apft->len > blen) {
		fprintf(stderr, "elem buffer too small\n");
		return false;
	}

	return fread_ensure(apft->elem, apft->len, f);
}

static bool
write_bytes_as_hex(uint8_t *bytes, uint32_t len)
{
	char buf[16] = { 0 };

	uint32_t i, b;
	for (i = 0; i < len; ++i) {
		b = snprintf(buf, 15, "0x%02x, ", bytes[i]);
		if (!fwrite_ensure(buf, b, stdout)) {
			return false;
		}
	}

	return true;
}

static bool
write_compiled(struct apf_template *apft, bool c_format)
{
	if (c_format) {
		return fwrite_ensure("{ ", 2, stdout)
		       && write_bytes_as_hex((uint8_t *)&apft->len, 2)
		       && write_bytes_as_hex((uint8_t *)&apft->flags, 2)
		       && write_bytes_as_hex(apft->elem, apft->len)
		       && fwrite_ensure("};\n", 3, stdout);
	}

	return fwrite_ensure(&apft->len, 2, stdout)
	       && fwrite_ensure(&apft->flags, 2, stdout)
	       && fwrite_ensure(apft->elem, apft->len, stdout);
}

int
main(int argc, char *const *argv)
{
	struct opts opts = defopts;
	parse_opts(&opts, argc, argv);

	uint8_t *elems = calloc(opts.elem_blen + opts.out_blen, 1);
	char *buf = (char *)&elems[opts.elem_blen];
	struct apf_err_ctx err = { 0 };
	struct apf_template apft;

	if (opts.read_compiled) {
		if (!read_compiled(&apft, elems, opts.elem_blen, opts.fmt)) {
			fprintf(stderr, "failed to read compiled format\n");
			goto err_1;
		}
	} else {
		apft = apf_compile(elems, opts.elem_blen, opts.fmt, NULL, NULL, &err);
	}

	if (err.err) {
		print_err(&err);
		goto err_1;
	}

	if (opts.compile_only) {
		if (!write_compiled(&apft, opts.compiled_c_out)) {
			fprintf(stderr, "failed to write compiled format\n");
			goto err_1;
		}
		goto exit;
	}

	apf_fmt(buf, opts.out_blen, &apft, &opts.arg_ctx, fmt_cb, fmt_sym_cb, &err);

	if (err.err) {
		print_err(&err);
		goto err_1;
	}

	fputs(buf, stdout);

exit:
	free(elems);
	return 0;
err_1:
	free(elems);
	return 1;
}
