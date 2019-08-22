#ifdef _FORTIFY_SOURCE
#undef _FORTIFY_SOURCE
#endif

#include "extconf.h"
#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include <ruby.h>
#include <ruby/encoding.h>
#include "altprintf.h"
#include "log.h"

#define MODNAME "AltPrintf"

rb_encoding *enc;

wchar_t *rbstowcs(VALUE str) {
	const char *cstr;
	wchar_t *wstr;
	size_t len;

	cstr = StringValueCStr(str);

	len = mbsrtowcs(NULL, &cstr, 0, NULL);
	wstr = calloc(len + 1, sizeof(wchar_t));
	len = mbsrtowcs(wstr, &cstr, len, NULL);

	LOG("rbs to wcs, len: %d, cstr: '%s'\n", len, cstr);
	LOG("wide string: '%ls'\n", wstr);

	return wstr;
}

VALUE wcstorbs(const wchar_t *wstr) {
	size_t len;
	char *cstr;
	VALUE str;

	LOG("converting wcs to rbs\n");
	LOG("wcs: '%ls'\n\n", wstr);

	len = wcsrtombs(NULL, &wstr, 0, NULL);
	LOG("len: %d\n", len);
	cstr = calloc(len, sizeof(wchar_t));
	wcsrtombs(cstr, &wstr, len, NULL);
	LOG("cstr: '%s'\n", cstr);

	LOG("wcs to rbs, len: %d, wcs: %ls, mbs: '%s'\n", len, wstr, cstr);

	if (len == (size_t)-1) return Qnil;

	str = rb_external_str_new_with_enc(cstr, len, enc);
	free(cstr);

	return str;
}

VALUE get_entry(struct fmte *f, VALUE *argv, long *argi, VALUE *hash) {
	VALUE sym, entry;
	size_t len;
	const wchar_t *tmpw;
	char *cstr;

	LOG("getting entry\n");

	if (f->anglearg_len == 0) {
		LOG("getting from argv[%ld]\n", *argi);
		entry = rb_ary_entry(*argv, *argi);
		(*argi)++;
		return entry;
	}

	LOG("getting from hash\n");

	//fmte_inspect(f);

	tmpw = f->anglearg_start;
	len = wcsnrtombs(NULL, &tmpw, f->anglearg_len, 0, NULL);
	LOG("allocating %d, maxlen %d\n", len, f->anglearg_len);

	cstr = calloc(len + 1, sizeof(char));
	wcsnrtombs(cstr, &tmpw, f->anglearg_len, len, NULL);
	LOG("wrote cstr %s\n", cstr);

	LOG("symbol | cstr: '%s', len %d\n", cstr, len);

	sym = rb_check_symbol_cstr(cstr, len, enc);
	entry = rb_hash_lookup2(*hash, sym, Qnil);
	if (entry == Qnil) rb_raise(rb_eKeyError, "missing key :%s", cstr);
	free(cstr);

	return entry;
}

wchar_t *rb_apformat(wchar_t *fmt, VALUE *argv, long *argi, VALUE *hash) {
	struct fmte *f, *head;
	wchar_t *final;
	int loop = 1;
	long *tmpi;
	wint_t *tmpc;
	wchar_t *tmps;
	double *tmpd;
	void *tmp;
	VALUE entry;

	head = f = parsef(&fmt);

	while (loop) {
		LOG("scanned type: %d\n", f->type);

		if (f->type != FEnd && f->type != FRaw) {
			entry = get_entry(f, argv, argi, hash);
		} else {
			entry = Qnil;
		}

		switch (f->type) {
		case FString:
			Check_Type(entry, T_STRING);

			tmp = rbstowcs(entry);
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

			tmpi = malloc(sizeof(long int));
			*tmpi = FIX2LONG(entry);
			tmp = tmpi;
			goto match;
		case FChar:
			Check_Type(entry, T_STRING);

			tmpc = malloc(sizeof(wint_t));
			tmps = rbstowcs(entry);
			*tmpc = btowc(tmps[0]);
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
		if (loop) f = parsef(&fmt);
	}

	LOG("got all fmt elements\n");
	final = assemble_fmt(head);
	fmte_destroy(head);
	return final;
}

VALUE rb_alt_printf(long passes, size_t argc, VALUE *argv, VALUE self) {
	VALUE fmt, args, hash, final;
	wchar_t *wfmt;
	wchar_t *formatted;
	long argi;

	rb_scan_args(argc, argv, "1*:", &fmt, &args, &hash);

	if (hash == Qnil && RB_TYPE_P(argv[argc - 1], T_HASH)) {
		hash = argv[argc - 1];
		argc--;
	}

	if (passes == 0) return fmt;

	wfmt = rbstowcs(fmt);
	formatted = NULL;

	argi = 0;
	for (; passes > 0; passes--) {
		LOG("wfmt: %ls\n", wfmt);

		formatted = rb_apformat(wfmt, &args, &argi, &hash);

		LOG("formatted result: '%ls'\n", formatted);

		free(wfmt);
		wfmt = formatted;
	}


	LOG("final: '%ls'\n", formatted);
	final = wcstorbs(formatted);

	free(formatted);

	return final;
}

VALUE rb_alt_printf_single_pass(size_t argc, VALUE *argv, VALUE self) {
	return rb_alt_printf(1, argc, argv, self);
}

VALUE rb_alt_printf_multi_pass(size_t argc, VALUE *argv, VALUE self) {
	long passes;


	if (RB_TYPE_P(argv[0], T_FIXNUM)) {
		passes = FIX2LONG(argv[0]);
	} else {
		rb_raise(rb_eArgError, "integer expected");
		return Qnil;
	}

	LOG("passes: %ld\n", passes);
	return rb_alt_printf(passes, argc - 1, &argv[1], self);
}

void Init_alt_printf() {
	VALUE mod, ver;
	size_t len;

	enc = rb_enc_find("UTF-8");
	mod = rb_define_module(MODNAME);

	len = strlen(ALTPRINTF_VERSION);
	ver = rb_external_str_new_with_enc(ALTPRINTF_VERSION, len, enc);
	rb_define_const(mod, "LIB_VERSION", ver);

	rb_define_module_function(mod, "fmt", rb_alt_printf_single_pass, -1);
	rb_define_module_function(mod, "fmtm", rb_alt_printf_multi_pass, -1);
}
