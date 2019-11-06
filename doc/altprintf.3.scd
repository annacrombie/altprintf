altprintf(3)

# NAME

libaltprintf - C library implementation of *altprintf*(7)

# SYNOPSIS

*\#include <altprintf.h>*

*enum apf_err *_apf_errno_*;*++
*enum apf_argt;*

*struct apf_fmte;*++
*void apf_fmte_push(struct apf_fmte \**_head_*, struct apf_fmte \**_elem_*);*++
*void apf_fmte_destroy(struct apf_fmte \**_head_*);*

*struct apf_fmte \*apf_parse(const char \*\**_format_string_*);*++
*char \*apf_assemble(struct apf_fmte \**_head_*);*

# DESCRIPTION

The provided functions provide an extremely flexible way to use the *altprintf*
library.  This flexibility comes at the cost of a little bit of complexity.
There is no *printf*(3)-like interface where you call a function like
*altprintf(const char \**_fmt_*, *_varargs..._*)*.  You must make a series of
calls to incrementally parse the format string and then supply the resulting
*apf_fmte* a value which will be used in formatting later on.  Once the entire
string is parsed, then a function is called to process the formats and produce
an output string.

To put it more simply, these are the steps to using this library:

. Parse the initial format element _f_.  Keep a separate reference to it as
  _head_.  This is your handle to the whole chain of parsed formats
. Check the type of the _f_ and insert the appropriate value.
. Push _f_ onto _head_ (if _f_ and _head_ are the same, this will be a no-op)
. Parse the next format as _f_
. Repeat steps 2-4 until _f_ has a type of *apf_argt_end*, indicating that the
  entire *format string* has been parsed.
. Call *apf_assemble(*_head_*)*

# EXAMPLE

```
#include <string.h>
#include <stdio.h>
#include <altprintf.h>

char *format(const char *fmt, int argc, long *argv)
{
	void *tmp;
	char *final;
	int argi, loop;
	struct apf_fmte *f, *head;

	argi = 0;
	loop = 1;

	head = f = apf_parse(&fmt);

	while (loop) {
		if (!(f->type == apf_argt_raw || f->type == apf_argt_end)
		    && argi >= argc)
			goto process_next_fmt;

		switch (f->type) {
		case apf_argt_string:
		case apf_argt_char:
		case apf_argt_double:
		case apf_argt_mul:
		case apf_argt_tern:
		case apf_argt_align:
			// not implemented
			return NULL;

		case apf_argt_int:
			tmp = malloc(sizeof(long));
			memcpy(tmp, &argv[argi], sizeof(long));
			f->value = tmp;
			argi++;

			break;
		case apf_argt_end:
			loop = 0;
			break;
		case apf_argt_raw:
		case apf_argt_none:
			break;
		}

process_next_fmt:
		apf_fmte_push(head, f);
		if (loop)
			f = apf_parse(&fmt);
	}

	final = apf_assemble(head);
	apf_fmte_destroy(head);
	return final;
}

int main(void)
{
	long arr[] = { 1, 2, 3, 4 };

	char *formatted = format("%d, %d, %d, %d", 4, arr);

	printf("%s\n", formatted);

	free(formatted);

	return 0;
}
```

# SEE ALSO

*altprintf*(1), *altprintf*(7)

# AUTHORS

Maintained by Stone Tickle <lattis@mochiro.moe>
