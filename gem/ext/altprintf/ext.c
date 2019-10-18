#ifdef _FORTIFY_SOURCE
#undef _FORTIFY_SOURCE
#endif

#ifdef RUBY_EXTCONF_H
#undef RUBY_EXTCONF_H
#endif

#include "extconf.h"
#include <stdio.h>
#include <locale.h>
#include <ruby.h>
#include <ruby/encoding.h>
#include <altprintf/altprintf.h>
#include <altprintf/log.h>

#define MODNAME "Altprintf"

rb_encoding *enc;

char *rbstocs(VALUE *rbstr)
{
	char *p, *tmp;
	long len;

	p = StringValuePtr(*rbstr);
	len = RSTRING_LEN(*rbstr);
	tmp = calloc(len + 1, sizeof(char));
	strncpy(tmp, p, len);
	tmp[len] = '\0';

	return tmp;
}

VALUE cstorbs(const char *cstr)
{
	size_t len;
	VALUE str;

	len = strlen(cstr);

	if (len == (size_t)-1)
		return Qnil;

	str = rb_external_str_new_with_enc(cstr, len, enc);

	return str;
}

VALUE get_entry(struct fmte *f, size_t argc, size_t *argi, VALUE *argv, VALUE *hash)
{
	VALUE sym, entry;

	LOG("getting entry\n");

	if (f->anglearg_len == 0) {
		LOG("getting from argv[%d] (argc: %d)\n", *argi, argc);
		if (*argi >= argc)
			rb_raise(rb_eArgError, "too few arguments");
		entry = rb_ary_entry(*argv, *argi);
		(*argi)++;
		return entry;
	}

	sym = rb_check_symbol_cstr(f->anglearg_start, f->anglearg_len, enc);
	entry = rb_hash_lookup2(*hash, sym, Qnil);
	if (entry == Qnil)
		rb_raise(rb_eKeyError, "missing key " PRIsVALUE, sym);

	return entry;
}

char *rb_apformat(const char *fmt, size_t argc, size_t *argi, VALUE *argv, VALUE *hash)
{
	struct fmte *f, *head;
	char *final;
	int loop = 1;
	long *tmpi;
	char *tmpc;
	char *tmps;
	double *tmpd;
	void *tmp;
	VALUE entry;

	head = f = parsef(&fmt);

	while (loop) {
		if (apf_err != apfe_none)
			rb_raise(rb_eArgError, "malformed format string");

		LOG("scanned type: %d\n", f->type);

		if (f->type != FEnd && f->type != FRaw)
			entry = get_entry(f, argc, argi, argv, hash);
		else
			entry = Qnil;

		switch (f->type) {
		case FString:
			Check_Type(entry, T_STRING);

			tmp = rbstocs(&entry);
			goto match;
		case FTern:
			tmpi = malloc(sizeof(long));

			*tmpi = (entry == Qfalse || entry == Qnil) ? 0 : 1;

			tmp = tmpi;
			goto match;
		case FMul:
		case FAlign:
		case FInt:
			Check_Type(entry, T_FIXNUM);

			tmpi = malloc(sizeof(long));
			*tmpi = FIX2LONG(entry);
			tmp = tmpi;
			goto match;
		case FChar:
			Check_Type(entry, T_STRING);

			tmpc = malloc(sizeof(char));
			tmps = StringValueCStr(entry);
			*tmpc = tmps[0];
			tmp = tmpc;
			goto match;
		case FDouble:
			Check_Type(entry, T_FLOAT);

			tmpd = malloc(sizeof(double));
			*tmpd = RFLOAT_VALUE(entry);
			tmp = tmpd;
match:
			f->value = tmp;
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
	LOG("assembled fmt: %s", final);
	fmte_destroy(head);
	return final;
}

VALUE rb_altprintf(long passes, size_t argc, VALUE *argv, VALUE self)
{
	VALUE fmt, args, hash, final;
	char *wfmt;
	char *formatted;
	size_t argi;
	int free_wfmt;

	apf_err = apfe_none;
	rb_scan_args(argc, argv, "1*:", &fmt, &args, &hash);
	argc--;

	if (hash == Qnil && RB_TYPE_P(argv[argc - 1], T_HASH)) {
		hash = argv[argc - 1];
		argc--;
	}

	if (passes == 0)
		return fmt;

	free_wfmt = 0;
	wfmt = StringValueCStr(fmt);
	formatted = NULL;

	argi = 0;
	for (; passes > 0; passes--) {
		LOG("wfmt: %s\n", wfmt);

		formatted = rb_apformat(wfmt, argc, &argi, &args, &hash);

		LOG("formatted result: '%s'\n", formatted);

		if (free_wfmt)
			free(wfmt);
		else
			free_wfmt = 1;
		wfmt = formatted;
	}


	LOG("final: '%s'\n", formatted);
	final = cstorbs(formatted);

	free(formatted);

	return final;
}

VALUE rb_altprintf_single_pass(size_t argc, VALUE *argv, VALUE self)
{
	return rb_altprintf(1, argc, argv, self);
}

VALUE rb_altprintf_multi_pass(size_t argc, VALUE *argv, VALUE self)
{
	long passes;

	Check_Type(argv[0], T_FIXNUM);
	passes = FIX2LONG(argv[0]);
	if (passes < 0)
		rb_raise(rb_eArgError, "expected positive number of passes");

	LOG("passes: %ld\n", passes);
	return rb_altprintf(passes, argc - 1, &argv[1], self);
}

void Init_altprintf()
{
	VALUE mod, ver;
	size_t len;

	enc = rb_enc_find("UTF-8");
	mod = rb_define_module(MODNAME);

	len = strlen(ALTPRINTF_VERSION);
	ver = rb_external_str_new_with_enc(ALTPRINTF_VERSION, len, enc);
	rb_define_const(mod, "LIB_VERSION", ver);

	rb_define_module_function(mod, "fmt", rb_altprintf_single_pass, -1);
	rb_define_module_function(mod, "fmtm", rb_altprintf_multi_pass, -1);
}
