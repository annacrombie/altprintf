#include "altprintf.h"
#include "syntax.h"
#include "strbuf.h"
#include "log.h"

#define BUFNUM 25

#define CHECKNULL(p) do {                                \
		if (p == NULL) {                         \
			apf_errno = apf_err_missing_argument; \
			return;                          \
		}                                        \
} while (0)

enum apf_err apf_errno;

static void fmt_mul(struct strbuf *sb, struct apf_fmte *f)
{
	CHECKNULL(f->value);

	long *i;

	i = f->value;
	if (f->parenarg_start == NULL)
		strbuf_pad(sb, f->chararg, *i);
	else
		for (int j = 0; j < *i; j++)
			strbuf_append_str(sb, f->parenarg_start, -f->parenarg_len);
}

static void fmt_tern(struct strbuf *sb, struct apf_fmte *f)
{
	CHECKNULL(f->parenarg_start);
	CHECKNULL(f->value);

	long int *b = f->value;
	int first_half = 1;
	char sep = f->chararg;
	const char *p = f->parenarg_start;
	for (; p <= f->parenarg_end; p++) {
		LOG("*p: %lc, first half? %d, bool: %ld, sep: %lc\n",
		    (char)*p, first_half, *b, (char)sep);
		if (*p == sep)
			first_half = 0;
		else if (*b && first_half)
			strbuf_append_char(sb, p);
		else if (!*b && !first_half)
			strbuf_append_char(sb, p);
	}
}

static char process_escape_seq(char seq)
{
	switch (seq) {
	case FS_ESC_NL:
		return '\n';
	case FS_ESC_ESC:
		return '\e';
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	default:
		return seq;
	}
}

static void fmt_raw(struct strbuf *sb, struct apf_fmte *f)
{
	CHECKNULL(f->parenarg_start);

	const char *p;
	char c;
	long i;

	p = f->parenarg_start;
	i = f->parenarg_len;

	for (; i > 0; i--) {
		LOG("p is '%lc' (%ld)\n", (char)*p, (long)*p);
		c = (i > 1 && *p == FS_ESC) ? process_escape_seq(*++p) : *p;

		strbuf_append(sb, c);
		p++;
		if (*p == EOS)
			break;
	}
}

static void fmt_string(struct strbuf *sb, struct apf_fmte *f)
{
	CHECKNULL(f->value);

	int prec = f->prec == -1 ? 100000000 : f->prec;
	strbuf_append_str(sb, f->value, prec);
}

static void fmt_char(struct strbuf *sb, struct apf_fmte *f)
{
	CHECKNULL(f->value);
	strbuf_append_char(sb, f->value);
}

static void fmt_int(struct strbuf *sb, struct apf_fmte *f)
{
	CHECKNULL(f->value);
	strbuf_append_int(sb, f->value);
}

static void fmt_double(struct strbuf *sb, struct apf_fmte *f)
{
	CHECKNULL(f->value);
	int prec = f->prec == -1 ? 3 : f->prec;
	strbuf_append_double(sb, f->value, prec);
}

static void
fmt(struct strbuf *sb, struct apf_fmte *f, void (*fmtr)(struct strbuf *, struct apf_fmte *))
{
	struct strbuf *tmp = strbuf_new();

	fmtr(tmp, f);

	if (tmp->len == 0) {
		strbuf_destroy(tmp);
		return;
	}

	int pad = f->pad > 0 ? f->pad - strbuf_width(tmp) : 0;

	if (pad > 0) {
		LOG("padding: %d\n", pad);
		switch (f->align) {
		case apf_algn_right:
			strbuf_append_strbuf(sb, tmp);
			strbuf_pad(sb, f->padchar, pad);
			break;
		case apf_algn_left:
			strbuf_pad(sb, f->padchar, pad);
			strbuf_append_strbuf(sb, tmp);
			break;
		case apf_algn_center:
			strbuf_pad(sb, f->padchar, pad / 2);
			strbuf_append_strbuf(sb, tmp);
			strbuf_pad(sb, f->padchar, pad / 2 + pad % 2);
			break;
		}
	} else {
		strbuf_append_strbuf(sb, tmp);
	}

	strbuf_destroy(tmp);
}

char *apf_assemble(struct apf_fmte *head)
{
	struct apf_fmte *f = head;
	struct strbuf *bufs[BUFNUM];
	struct apf_fmte *splits[BUFNUM];
	size_t buf = 0, i;
	size_t w, tw, rw;
	char *final;

	void (*fmtr)(struct strbuf *, struct apf_fmte *) = NULL;
	int loop = 1;

	bufs[buf] = strbuf_new();

	LOG("assembling elements\n");
	while (loop) {
		switch (f->type) {
		case apf_argt_mul:
			fmtr = fmt_mul;
			break;
		case apf_argt_tern:
			fmtr = fmt_tern;
			break;
		case apf_argt_int:
			fmtr = fmt_int;
			break;
		case apf_argt_char:
			fmtr = fmt_char;
			break;
		case apf_argt_double:
			fmtr = fmt_double;
			break;
		case apf_argt_string:
			fmtr = fmt_string;
			break;
		case apf_argt_raw:
			fmtr = fmt_raw;
			break;

		case apf_argt_align:
			buf++;
			splits[buf] = f;
			bufs[buf] = strbuf_new();
			fmtr = NULL;
			break;
		case apf_argt_end:
			loop = 0;
			fmtr = NULL;
			break;
		case apf_argt_none:
			fmtr = NULL;
			break;
		}

		if (fmtr != NULL)
			fmt(bufs[buf], f, fmtr);
		f = f->next;
	}

	// Assemble splits
	LOG("assembling %d splits\n", buf);
	if (buf > 0)
		tw = strbuf_width(bufs[0]);
	for (i = 1; i <= buf; i++) {
		rw = *(long*)splits[i]->value;

		if (tw > rw) {
			LOG("trimming first half to width\n");
			strbuf_destroy(bufs[i]);
			bufs[i] = bufs[0];
			bufs[0] = strbuf_new();
			strbuf_appendw_strbuf(bufs[0], bufs[i], rw);
		} else {
			if (rw > strbuf_width(bufs[i]) + tw) {
				w = rw - (strbuf_width(bufs[i]) + tw);
				LOG("padding %d\n", w);
				strbuf_pad(bufs[0], splits[i]->chararg, w);
				strbuf_append_strbuf(bufs[0], bufs[i]);
			} else {
				LOG("%d %d %d\n", w, rw, tw);
				strbuf_appendw_strbuf(bufs[0], bufs[i], rw - tw);
			}
		}

		tw = rw;
	}

	final = strbuf_cstr(bufs[0]);
	for (i = 0; i <= buf; i++) strbuf_destroy(bufs[i]);
	return final;
}
