#include "parsef.h"

#define EOS L'\0'

struct fmte *parsef(wchar_t **fmt) {
	wchar_t *w_c;
	wchar_t **w_a = &w_c;
	struct fmte *f = ini_format();
	long *l_a = &f->pad;
	wchar_t raw[1] = { FS_START };

	LOG("processing %ld (%lc)\n", **fmt, (wint_t)**fmt);

	if (**fmt != FS_START) {
		if (**fmt == EOS) {
			f->type = FEnd;
			goto return_format;
		}

		LOG("building raw format\n");
		f->type = FRaw;

		f->parenarg_start = *fmt;
		LOG("scanning long arg\n");
		get_longarg(fmt, &f->parenarg_end, FS_START, &f->parenarg_len);

		f->parenarg_end++;
		f->parenarg_len++;
		(*fmt)--;

		goto return_format;
	} else {
		LOG("building format\n");
		(*fmt)++;
	}

	for (;**fmt != L'\0';(*fmt)++) {
		LOG("scanned char '%lc'\n", (wint_t)(**fmt));
		switch (**fmt) {
		case FS_A_CHARARG:
			(*fmt)++;
			f->chararg = **fmt;
			break;
		case FS_A_ANGLEARG_S:
			f->anglearg_start = *fmt + 1;
			get_longarg(fmt, &f->anglearg_end, FS_A_ANGLEARG_E, &f->anglearg_len);
			break;
		case FS_A_PARENARG_S:
			f->parenarg_start = *fmt + 1;
			get_longarg(fmt, &f->parenarg_end, FS_A_PARENARG_E, &f->parenarg_len);
			break;
		case FS_A_LALIGN:
			f->align = Left;
			break;
		case FS_A_SPAD:
			f->padchar = FS_A_SPAD;
			break;
		case '0':
			f->padchar = '0';
			break;
		case '.':
			l_a = &f->prec;
			break;
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
			LOG("l_a: %p %ld\n", l_a, *l_a);
			*l_a = wcstol(*fmt, w_a, 10);
			*fmt = *w_a - 1;
			break;
		// Psuedo-type
		case FS_START:
			f->type = FRaw;
			f->parenarg_start = f->parenarg_end = raw;
			f->parenarg_len = 1;
			goto return_format;
		// Types
		case FS_T_STRING:
			f->type = FString;
			goto return_format;
		case FS_T_MUL:
			f->type = FMul;
			goto return_format;
		case FS_T_TERN:
			f->type = FTern;
			goto return_format;
		case FS_T_ALIGN:
			f->type = FAlign;
			goto return_format;
		case FS_T_INT:
			f->type = FInt;
			goto return_format;
		case FS_T_CHAR:
			f->type = FChar;
			goto return_format;
		case FS_T_DOUBLE:
			f->type = FDouble;
			goto return_format;
		default:
			f->type = FRaw;
		}
	}

	f->type = FEnd;
return_format:
	(*fmt)++;
	return f;
}

void get_longarg(wchar_t **s, wchar_t **e, wchar_t stop, size_t *size) {
	*size = 0;

	while (**s != EOS && **s != stop) {
		LOG("checking (%lc)\n", (wint_t)**s);
		(*s)++;
		(*size)++;
	}

	(*size)--;
	if (*size > 0) *e = *s - 1;
}
