#include "altprintf.h"
#include "fmte.h"
#include "syntax.h"
#include "log.h"

char *altprintf_pct = "%";

static void get_longarg(const char **s, const char **e, char stop, size_t *size)
{
	*size = 0;

	while (**s != EOS && **s != stop) {
		LOG("checking (%c)\n", **s);
		(*s)++;
		(*size)++;
	}

	(*size)--;
	if (*size > 0)
		*e = *s - 1;
}

struct apf_fmte *apf_parse(const char **fmt)
{
	char *w_c;
	char **w_a = &w_c;
	struct apf_fmte *f = apf_fmte_ini();
	long *l_a = &f->pad;

	LOG("processing %d (%c)\n", **fmt, **fmt);

	if (**fmt != FS_START) {
		if (**fmt == EOS) {
			f->type = apf_argt_end;
			goto return_format;
		}

		LOG("building raw format\n");
		f->type = apf_argt_raw;

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

	for (; **fmt != '\0'; (*fmt)++) {
		LOG("scanned char '%c'\n", **fmt);
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
			f->align = apf_algn_left;
			break;
		case FS_A_CALIGN:
			f->align = apf_algn_center;
			break;
		case FS_A_PAD:
			(*fmt)++;
			f->padchar = **fmt;
			break;
		case FS_A_PREC:
			l_a = &f->prec;
			break;
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9': case '0':
			LOG("l_a: %p %ld\n", l_a, *l_a);
			*l_a = strtol(*fmt, w_a, 10);
			LOG("read num: %ld\n", *l_a);
			*fmt = *w_a - 1;
			break;
		// Psuedo-type
		case FS_START:
			f->type = apf_argt_raw;
			f->parenarg_start = &altprintf_pct[0];
			f->parenarg_end = &altprintf_pct[1];
			f->parenarg_len = 1;
			goto return_format;
		// Types
		case FS_T_STRING:
			f->type = apf_argt_string;
			goto return_format;
		case FS_T_MUL:
			f->type = apf_argt_mul;
			goto return_format;
		case FS_T_TERN:
			f->type = apf_argt_tern;
			goto return_format;
		case FS_T_ALIGN:
			f->type = apf_argt_align;
			goto return_format;
		case FS_T_INT:
			f->type = apf_argt_int;
			goto return_format;
		case FS_T_CHAR:
			f->type = apf_argt_char;
			goto return_format;
		case FS_T_DOUBLE:
			f->type = apf_argt_double;
			goto return_format;
		default:
			apf_errno = apf_err_invalid_token;
			f->type = apf_argt_raw;
		}
	}

	f->type = apf_argt_end;
return_format:
	(*fmt)++;
	return f;
}
