#include <ruby.h>
#include <ruby/encoding.h>
#include "extconf.h"
#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include "../src/list.h"
#include "../src/altprintf.h"
#include "../src/log.h"

#define MODNAME "AltPrintf"

#define FS_A_HASHSTART '{'
#define FS_A_HASHEND   '}'

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

struct list_elem *rb_altprintf_make_list(const wchar_t *fmt, VALUE *argv, VALUE *hash) {
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
	int arg_i = 0;
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
			case FS_A_HASHSTART:
				tmp_str = fmt + 1;

				use_hash = 0;
				while (fmt < end && *fmt != FS_A_HASHEND) {
					fmt++;
					use_hash++;
				}

				len = wcsnrtombs(NULL, &fmt, use_hash, 0, NULL);
				cstr = calloc(len + 1, sizeof(char));
				wcsnrtombs(cstr, &fmt, use_hash, 0, NULL);

				symbol = rb_check_symbol_cstr(cstr, len, enc);
				//rb_raise(rb_eRuntimeError, "Error code %+"PRIsVALUE" %d %c", sym, tag_len, *tag_start);
				entry = rb_hash_lookup2(*hash, symbol, Qnil);
				free(cstr);
				use_hash = 1;

				break;
			case FS_A_STRINGSTART:
				while (fmt < end && *fmt != FS_A_STRINGEND) fmt++;
				break;
			case FS_T_STRING:
				if (!use_hash) {
					if (arg_i >= argc) goto no_more_args;
					entry = rb_ary_entry(*argv, arg_i);
				}

				tmp_str = rbstowcs(entry);
				le_cur = list_elem_ini(tmp_str, String);
				goto match;
			case FS_T_MUL:
			case FS_T_TERN:
			case FS_T_ALIGN:
			case FS_T_INT:
				if (!use_hash) {
					if (arg_i >= argc) goto no_more_args;
					entry = rb_ary_entry(*argv, arg_i);
				}

				tmp_int = malloc(sizeof(long int));
				*tmp_int = FIX2LONG(entry);
				LOG("got int %ld\n", *tmp_int);
				le_cur = list_elem_ini(tmp_int, Int);
				goto match;
			case FS_T_CHAR:
				if (!use_hash) {
					if (arg_i >= argc) goto no_more_args;
					entry = rb_ary_entry(*argv, arg_i);
				}

				tmp_char = malloc(sizeof(wint_t));
				tmp_str = rbstowcs(entry);
				*tmp_char = btowc(tmp_str[0]);
				le_cur = list_elem_ini(tmp_char, Char);
				goto match;
			case FS_T_DOUBLE:
				if (!use_hash) {
					if (arg_i >= argc) goto no_more_args;
					entry = rb_ary_entry(*argv, arg_i);
				}

				tmp_double = malloc(sizeof(double));
				*tmp_double = RFLOAT_VALUE(entry);
				le_cur = list_elem_ini(tmp_double, Double);
				goto match;
			match: le_prev->next = le_cur;
				le_prev = le_cur;
				mode = 0;
				arg_i++;
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

VALUE rb_alt_printf(size_t argc, VALUE *argv, VALUE self) {
	VALUE fmt, args, hash, final;
	wchar_t *wfmt;
	wchar_t *formatted;
	struct list_elem *ap;

	rb_scan_args(argc, argv, "1*:", &fmt, &args, &hash);

	wfmt = rbstowcs(fmt);

	ap = rb_altprintf_make_list(wfmt, &args, &hash);

	formatted = altsprintf(wfmt, ap);
	LOG("formatted result: '%ls'\n", formatted);

	free(wfmt);
	list_elem_destroy(ap);

	final = wcstorbs(formatted);

	free(formatted);

	return final;
}

void Init_alt_printf()
{
	enc = rb_enc_find("UTF-8");
	VALUE mod = rb_define_module(MODNAME);
	rb_define_module_function(mod, "sprintf", rb_alt_printf, -1);
}
