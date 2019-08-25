#include "fmt.h"

#define BUFNUM 25

#define CHECKNULL(p) if (p == NULL) { apf_err = apfe_missing_argument; return; }

enum altprintf_err apf_err;

void fmt_mul(struct strbuf *sb, struct fmte *f) {
	CHECKNULL(f->value);

	long *i;

	i = f->value;
	if (f->parenarg_start == NULL) {
		strbuf_pad(sb, f->chararg, *i);
	} else {
		for (int j=0; j<*i; j++) {
			strbuf_append_str(sb, f->parenarg_start, -f->parenarg_len);
		}
	}
}

void fmt_tern(struct strbuf *sb, struct fmte *f) {
	CHECKNULL(f->parenarg_start);
	CHECKNULL(f->value);

	long int *b = f->value;
	int first_half = 1;
	wchar_t sep = f->chararg;
	wchar_t *p = f->parenarg_start;
	for (;p<=f->parenarg_end;p++) {
		LOG("*p: %lc, first half? %d, bool: %ld, sep: %lc\n", (wint_t)*p, first_half, *b, (wint_t)sep);
		if (*p == sep) first_half = 0;
		else if (*b && first_half) strbuf_append_char(sb, p);
		else if (!*b && !first_half) strbuf_append_char(sb, p);
	}
}

wchar_t process_escape_seq(wchar_t seq) {
	switch (seq) {
	case FS_ESC_NL:
		return L'\n';
	case FS_ESC_ESC:
		return L'\e';
	case 'a':
		return L'\a';
	case 'b':
		return L'\b';
	case 'f':
		return L'\f';
	case 'r':
		return L'\r';
	case 't':
		return L'\t';
	case 'v':
		return L'\v';
	default:
		return seq;
	}
}

void fmt_raw(struct strbuf *sb, struct fmte *f) {
	CHECKNULL(f->parenarg_start);

	wchar_t *p;
	wchar_t c;
	long i;

	p = f->parenarg_start;
	i = f->parenarg_len;

	for (; i > 0; i--) {
		LOG("p is '%lc' (%ld)\n", (wint_t)*p, (long)*p);
		c = (i > 1 && *p == FS_ESC) ? process_escape_seq(*++p) : *p;

		strbuf_append(sb, c);
		p++;
		if (*p == EOS) break;
	}
}

void fmt_string(struct strbuf *sb, struct fmte *f) {
	CHECKNULL(f->value);

	int prec = f->prec == -1 ? 100000000 : f->prec;
	strbuf_append_str(sb, f->value, prec);
}

void fmt_char(struct strbuf *sb, struct fmte *f) {
	CHECKNULL(f->value);
	strbuf_append_char(sb, f->value);
}

void fmt_int(struct strbuf *sb, struct fmte *f) {
	CHECKNULL(f->value);
	strbuf_append_int(sb, f->value);
}

void fmt_double(struct strbuf *sb, struct fmte *f) {
	CHECKNULL(f->value);
	int prec = f->prec == -1 ? 3 : f->prec;
	strbuf_append_double(sb, f->value, prec);
}

void fmt(struct strbuf *sb, struct fmte *f, void (*fmtr)(struct strbuf *, struct fmte *)) {
	struct strbuf *tmp = strbuf_new();
	fmtr(tmp, f);

	if (tmp->len == 0) {
		strbuf_destroy(tmp);
		return;
	};

	int pad = f->pad - tmp->width;

	if (pad > 0) {
		LOG("padding: %d\n", pad);
		switch(f->align) {
		case Right:
			strbuf_append_strbuf(sb, tmp);
			strbuf_pad(sb, f->padchar, pad);
			break;
		case Left:
			strbuf_pad(sb, f->padchar, pad);
			strbuf_append_strbuf(sb, tmp);
			break;
		case Center:
			strbuf_pad(sb, f->padchar, pad/2);
			strbuf_append_strbuf(sb, tmp);
			strbuf_pad(sb, f->padchar, pad/2 + pad%2);
			break;
		}
	} else {
		strbuf_append_strbuf(sb, tmp);
	}

	strbuf_destroy(tmp);
}

wchar_t *assemble_fmt(struct fmte *head) {
	struct fmte *f = head;
	struct strbuf *bufs[BUFNUM];
	struct fmte *splits[BUFNUM];
	size_t buf = 0, i;
	size_t w, tw, rw;
	wchar_t *final;
	void (*fmtr)(struct strbuf *, struct fmte *) = NULL;
	int loop = 1;

	bufs[buf] = strbuf_new();

	LOG("assembling elements\n");
	while (loop) {
		switch (f->type) {
		case FMul:
			fmtr = fmt_mul;
			break;
		case FTern:
			fmtr = fmt_tern;
			break;
		case FInt:
			fmtr = fmt_int;
			break;
		case FChar:
			fmtr = fmt_char;
			break;
		case FDouble:
			fmtr = fmt_double;
			break;
		case FString:
			fmtr = fmt_string;
			break;
		case FRaw:
			fmtr = fmt_raw;
			break;

		case FAlign:
			buf++;
			splits[buf] = f;
			bufs[buf] = strbuf_new();
			fmtr = NULL;
			break;
		case FEnd:
			loop = 0;
			fmtr = NULL;
			break;
		case FNone:
			fmtr = NULL;
			break;
		}

		if (fmtr != NULL) fmt(bufs[buf], f, fmtr);
		f = f->next;
	}

	// Assemble splits
	LOG("assembling %d splits\n", buf);
	tw = bufs[0]->width;
	for (i = 1; i <= buf; i++) {
		rw = *(long *)splits[i]->value;

		if (tw > rw) {
			LOG("trimming first half to width\n");
			strbuf_destroy(bufs[i]);
			bufs[i] = bufs[0];
			bufs[0] = strbuf_new();
			strbuf_appendw_strbuf(bufs[0], bufs[i], rw);
		} else {
			if (rw > bufs[i]->width + tw) {
				w = rw - (bufs[i]->width + tw);
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
