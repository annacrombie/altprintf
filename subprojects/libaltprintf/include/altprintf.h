#ifndef ALTPRINTF_H_
#define ALTPRINTF_H_
#define ALTPRINTF_VERSION "0.3.3"
#include <stdlib.h>

enum apf_argt {
	apf_argt_mul,
	apf_argt_tern,
	apf_argt_align,
	apf_argt_int,
	apf_argt_char,
	apf_argt_double,
	apf_argt_string,
	apf_argt_raw,
	apf_argt_none,
	apf_argt_end
};
enum apf_align { apf_algn_left, apf_algn_right, apf_algn_center };

enum apf_err {
	apf_err_none,
	apf_err_invalid_token,
	apf_err_missing_argument
};

extern enum apf_err apf_errno;

struct apf_fmte {
	const char *parenarg_start;
	const char *parenarg_end;
	size_t parenarg_len;

	const char *anglearg_start;
	const char *anglearg_end;
	size_t anglearg_len;

	char chararg;

	char padchar;

	enum apf_argt type;
	enum apf_align align;

	long prec;
	long pad;

	void *value;

	struct apf_fmte *next;
};

struct apf_fmte *apf_fmte_ini(void);
void apf_fmte_inspect(struct apf_fmte *);
void apf_fmte_push(struct apf_fmte *, struct apf_fmte *);
void apf_fmte_destroy(struct apf_fmte *);
struct apf_fmte *apf_parse(const char **);
char *apf_assemble(struct apf_fmte *);
#endif
