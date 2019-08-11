#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include <ruby.h>
#include <ruby/encoding.h>
#include "extconf.h"
#include "list.h"
#include "altprintf.h"
#include "log.h"

#define CHECKARG                              \
if (!use_hash) {                              \
	if (*argi >= argc) goto no_more_args; \
	entry = rb_ary_entry(*argv, *argi);   \
}

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

	len = wcsrtombs(NULL, &wstr, 0, NULL);
	cstr = calloc(len, sizeof(wchar_t));
	wcsrtombs(cstr, &wstr, len, NULL);

	LOG("wcs to rbs, len: %d, cstr: '%s'\n", len, cstr);

	str = rb_external_str_new_with_enc(cstr, len, enc);
	free(cstr);

	return str;
}

struct list_elem *rb_altprintf_make_list(const wchar_t *fmt, VALUE *argv, long *argi, VALUE *hash) {
	struct list_elem *le_cur;
	struct list_elem *le_start;
	struct list_elem *le_prev;
	/* create a dummy element as the head */
	le_start = le_prev = list_elem_create();

	VALUE entry, symbol;

	long int *tmp_int;
	wint_t *tmp_char;
	double *tmp_double;
	const wchar_t *tmp_str;

	char *cstr;
	size_t len;

	int mode	= 0;
	long argc = rb_array_len(*argv);
	int use_hash = 0;

	const wchar_t *end = &fmt[wcslen(fmt)];

	for (;fmt<end;fmt++) {
		LOG("checking char '%lc', lvl: '%d'\n", (wint_t)(*fmt), mode);
		if (mode == 0) {
			switch(*fmt) {
			case FS_START: mode = 1;
				break;
			}
		} else {
			switch (*fmt) {
			case FS_A_CHARARG:
				fmt++;
				break;
			case FS_A_RBHASHSTART:
				tmp_str = fmt + 1;

				use_hash = -1;
				while (fmt < end && *fmt != FS_A_RBHASHEND) {
					fmt++;
					use_hash++;
				}

				LOG("use_hash: %d\n", use_hash);
				len = wcsnrtombs(NULL, &tmp_str, use_hash, 0, NULL);

				cstr = calloc(len + 1, sizeof(char));
				wcsnrtombs(cstr, &tmp_str, use_hash, len, NULL);

				LOG("symbol | cstr: '%s', len %d\n", cstr, len);

				symbol = rb_check_symbol_cstr(cstr, len, enc);
				entry = rb_hash_lookup2(*hash, symbol, Qnil);

				if (entry == Qnil) {
					rb_raise(
						rb_eKeyError,
						"no such key :%s",
						cstr
					);
					free(cstr);
				}

				use_hash = 1;

				break;
			case FS_A_STRINGSTART:
				while (fmt < end && *fmt != FS_A_STRINGEND) fmt++;
				break;
			case FS_T_STRING:
				CHECKARG;

				tmp_str = rbstowcs(entry);
				le_cur = list_elem_ini(tmp_str, String);
				goto match;
			case FS_T_TERN:
				CHECKARG;
				tmp_int = malloc(sizeof(long int));
				if (entry == Qfalse || entry == Qnil) {
					*tmp_int = 0;
				} else {
					*tmp_int = 1;
				}
				LOG("got bool %ld\n", *tmp_int);
				le_cur = list_elem_ini(tmp_int, Int);

				goto match;
			case FS_T_MUL:
			case FS_T_ALIGN:
			case FS_T_INT:
				CHECKARG;

				tmp_int = malloc(sizeof(long int));
				*tmp_int = FIX2LONG(entry);
				LOG("got int %ld\n", *tmp_int);
				le_cur = list_elem_ini(tmp_int, Int);
				goto match;
			case FS_T_CHAR:
				CHECKARG;

				tmp_char = malloc(sizeof(wint_t));
				tmp_str = rbstowcs(entry);
				*tmp_char = btowc(tmp_str[0]);
				le_cur = list_elem_ini(tmp_char, Char);
				goto match;
			case FS_T_DOUBLE:
				CHECKARG;

				tmp_double = malloc(sizeof(double));
				*tmp_double = RFLOAT_VALUE(entry);
				le_cur = list_elem_ini(tmp_double, Double);
				goto match;
			match: le_prev->next = le_cur;
				le_prev = le_cur;
				mode = 0;
				(*argi)++;
				break;
			case FS_START:
				mode = 0;
				break;
			}
		}
	}

no_more_args:
	if (le_start->next == NULL) return le_start;

	/* set cur to the 2nd element and destroy the first one */
	le_cur = le_start->next;
	le_start->next = NULL;
	list_elem_destroy(le_start);

	return le_cur;
}

VALUE rb_alt_printf(long passes, size_t argc, VALUE *argv, VALUE self) {
	VALUE fmt, args, hash, final;
	wchar_t *wfmt;
	wchar_t *formatted;
	struct list_elem *ap;

	rb_scan_args(argc, argv, "1*:", &fmt, &args, &hash);

	if (hash == Qnil && RB_TYPE_P(argv[argc - 1], T_HASH)) {
		hash = argv[argc - 1];
		argc--;
	}

	if (passes == 0) return fmt;

	wfmt = rbstowcs(fmt);

	long argi = 0;
	while (passes > 0) {
		ap = rb_altprintf_make_list(wfmt, &args, &argi, &hash);

		//list_elem_inspect_all(ap);
		LOG("wfmt: %ls\n", wfmt);

		formatted = altsprintf(wfmt, ap);
		LOG("formatted result: '%ls'\n", formatted);

		free(wfmt);
		list_elem_destroy(ap);

		wfmt = formatted;
		passes--;
	}

	final = wcstorbs(formatted);

	free(formatted);

	return final;
}

VALUE rb_alt_printf_single_pass(size_t argc, VALUE *argv, VALUE self) {
	VALUE fmt, args, hash, final;

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

void Init_alt_printf()
{
	enc = rb_enc_find("UTF-8");
	VALUE mod = rb_define_module(MODNAME);
	rb_define_module_function(mod, "fmt", rb_alt_printf_single_pass, -1);
	rb_define_module_function(mod, "fmtm", rb_alt_printf_multi_pass, -1);
}
