#include <apf.h>
#include <stdio.h>

struct fmt_ctx { int argc; const char **argv; };

static struct apf_arg
arg_cb(struct apf_err_ctx *err, void *_ctx, uint16_t id)
{
	struct fmt_ctx *ctx = _ctx;

	if (id >= ctx->argc) {
		err->err = apf_err_arg_missing;
		return (struct apf_arg) { 0 };
	} else {
		return apf_tag(ctx->argv[id]);
	}
}

int
main(int argc, const char **argv)
{
	if (argc < 2) {
		return 1;
	}

	uint8_t elems[256];
	char buf[256] = { 0 };
	struct apf_err_ctx err = { 0 };
	struct apf_template apft;
	struct fmt_ctx fmt_ctx = { argc - 2, &argv[2] };

	apft = apf_parse(elems, 256, argv[1], NULL, NULL, &err);
	if (err.err) {
		return 1;
	}

	apf_fmt(buf, 256, &apft, &fmt_ctx, arg_cb, NULL, &err);
	if (err.err) {
		return 1;
	}

	printf("%s", buf);
}
