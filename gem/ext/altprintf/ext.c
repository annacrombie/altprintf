#ifdef _FORTIFY_SOURCE
#undef _FORTIFY_SOURCE
#endif

#ifdef RUBY_EXTCONF_H
#undef RUBY_EXTCONF_H
#endif

#ifdef DEBUG
#define LOG(...) printf("%s:%d [\e[35m%s\e[0m] ", __FILE__, __LINE__, __func__); printf(__VA_ARGS__);
#else
#define LOG(msg, ...)
#endif

#include "extconf.h"
#include <stdio.h>
#include <locale.h>
#include <ruby.h>
#include <ruby/encoding.h>
#include <altprintf.h>

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

VALUE get_entry(struct apf_fmte *f, size_t argc, size_t *argi, VALUE *argv, VALUE *hash)
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
	struct apf_fmte *f, *head;
	char *final;
	int loop = 1;
	long *tmpi;
	char *tmpc;
	char *tmps;
	double *tmpd;
	void *tmp;
	VALUE entry;

	head = f = apf_parse(&fmt);

	while (loop) {
		if (apf_errno != apf_err_none)
			rb_raise(rb_eArgError, "malformed format string");

		LOG("scanned type: %d\n", f->type);

		if (f->type != apf_argt_end && f->type != apf_argt_raw)
			entry = get_entry(f, argc, argi, argv, hash);
		else
			entry = Qnil;

		switch (f->type) {
		case apf_argt_string:
			Check_Type(entry, T_STRING);

			tmp = rbstocs(&entry);
			goto match;
		case apf_argt_tern:
			tmpi = malloc(sizeof(long));

			*tmpi = (entry == Qfalse || entry == Qnil) ? 0 : 1;

			tmp = tmpi;
			goto match;
		case apf_argt_mul:
		case apf_argt_align:
		case apf_argt_int:
			Check_Type(entry, T_FIXNUM);

			tmpi = malloc(sizeof(long));
			*tmpi = FIX2LONG(entry);
			tmp = tmpi;
			goto match;
		case apf_argt_char:
			Check_Type(entry, T_STRING);

			tmpc = malloc(sizeof(char));
			tmps = StringValueCStr(entry);
			*tmpc = tmps[0];
			tmp = tmpc;
			goto match;
		case apf_argt_double:
			Check_Type(entry, T_FLOAT);

			tmpd = malloc(sizeof(double));
			*tmpd = RFLOAT_VALUE(entry);
			tmp = tmpd;
match:
			f->value = tmp;
			break;
		case apf_argt_raw:
			break;
		case apf_argt_end:
			LOG("EOS (end of string)\n");
			loop = 0;
			break;
		case apf_argt_none:
			LOG("error! shouldn' t be none\n");
			break;
		}

		LOG("pushing fmt\n");
#ifdef DEBUG
		fmte_inspect(f);
#endif
		apf_fmte_push(head, f);
		if (loop)
			f = apf_parse(&fmt);
	}

	LOG("got all fmt elements\n");
	final = apf_assemble(head);
	LOG("assembled fmt: %s", final);
	apf_fmte_destroy(head);
	return final;
}

VALUE rb_altprintf(long passes, size_t argc, VALUE *argv, VALUE self)
{
	VALUE fmt, args, hash, final;
	char *wfmt;
	char *formatted;
	size_t argi;
	int free_wfmt;

	apf_errno = apf_err_none;
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

/*
 * call-seq:
 *	fmt(format_string [, arguments...]) -> String
 *
 * Formats a `format_string` with `arguments`.  `format_string` may be any
 * vaild altprintf format string .
 * Additionally, a hash may be specified as the last argument, or as keyword
 * arguments and the values of the hash may be directly accessed in the format
 * string using < and >.
 *
 * For example:
 *	fmt("hello %<name>", name: "John") #=> "hello John"
 *
 * Note that the keys within <> are always assumed to be symbols, so the
 * following would not work
 *	fmt("hello %<name>", { "name" => "John" }) #=> "hello John"
 */
VALUE rb_altprintf_single_pass(size_t argc, VALUE *argv, VALUE self)
{
	return rb_altprintf(1, argc, argv, self);
}



/*
 * call-seq:
 *   fmtm(passes, format_string [, arguments...]) -> String
 *
 * Same as #fmt, but takes an additional argument, passes, which specifies the
 * number of passes to go over the format string.
 *
 * For example:
 *	fmtm(0, "%%%%%%%%") #=> "%%%%%%%%"
 *	fmtm(1, "%%%%%%%%") #=> "%%%%"
 *	fmtm(2, "%%%%%%%%") #=> "%%"
 *	fmtm(3, "%%%%%%%%") #=> "%"
 *	fmtm(4, "%%%%%%%%") #=> ""
 */
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

	/*
	 * The base module for *Altprintf*.  For documentation on the syntax of
	 * the format string, see https://github.com/annacrombie/altprintf
	 */
	mod = rb_define_module("Altprintf");

	len = strlen(ALTPRINTF_VERSION);
	ver = rb_external_str_new_with_enc(ALTPRINTF_VERSION, len, enc);

	/*
	 * The version of libaltprintf that this extension was compiled against.
	 */
	rb_define_const(mod, "LIB_VERSION", ver);

	rb_define_module_function(mod, "fmt", rb_altprintf_single_pass, -1);
	rb_define_module_function(mod, "fmtm", rb_altprintf_multi_pass, -1);
}
