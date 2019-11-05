altprintf(7)

# NAME

altprintf - a printf-like template language

# SYNOPSIS

*format string*: (.\*|*format element*)\*

*format element*: %(*format arguments*)\**format function*

# DESCRIPTION

altprintf is a template language inspired by *printf*(3).  It is intended to
format strings for display on a terminal or similar, but may be shoehorned into
other applications.

# FORMAT STRING

A document written in this template language is referred to as a format string.
This format string is composed of raw text and *format elements*, denoted by a
%.  Raw text remains unchanged in the output.

## FORMAT ELEMENT

A *format element* always starts with a %.  After that, any number of arguments
may be specified, and in any order.  If the same arguments are repeated, the
value of the last argument is taken.  The final character of every format
element determines what *format function* will be called to format the
associated _value_.

## FORMAT ARGUMENTS

	*(*_text_*)* - the _text_ within *(* and *)* is set to the *paren arg*

		default: unset

	*<*_text_*>* - the _text_ within *<* and *>* is set to the *angle arg*.

		Note: no *format function* uses this argument.

		default: unset

	*~*_c_ - the _character_ after *~* is set to the *char arg*

		default: ' '

	*\_*_c_ - the _character_ after *\_* is set to the *pad char*

		default: ' '

	*.*[*0-9*]\* - set the *precision*

		default: unset

	\[*0-9*]+ - set the *field width*

		default: 0

	The following two flags manipulate one argument: *alignment*.

		*-* - left-align the output within the *field width*

		*^* - center-align the output within the *field width*

		the default alignment is right-align.


Any of the *format arguments* may be specified for any *format function*.

## FORMAT FUNCTIONS

	*%* - percent
		output: %

		arguments: *pad char*, *field width*, *alignment*

		example: "%%%%" => "%%"

	*s* - string

		output: _value_ as a string

		arguments: *pad char*, *precision*, *field width*, *alignment*

		If *precision* is given, it determines the maximum amount of
		characters to output

		example: "%10s", "hello" => "     hello"

	*c* - character

		output: _value_ as a character

		arguments: *pad char*, *field width*, *alignment*

		example: "%\_-^5c", '!' => "--!--"

	*d* - integer

		output: _value_ as an integer

		arguments: *pad char*, *field width*, *alignment*

		example: "%3d%%", 75 => " 75%"

	*f* - floating point

		output: The double _value_ is rounded and converted to decimal
		notation in the style [-]ddd.ddd, where the number of digits
		after the decimal-point character is equal to the *precision*.

		arguments: *pad char*, *precision*, *field width*, *alignment*

		example: "pi: %.3f", 3.14159 => pi: 3.142

	*\** - multiply

		output: repeat *char arg* _value_ times

		arguments: *char arg*, *pad char*, *field width*, *alignment*

		example: "%~@\*", 10 => "@@@@@@@@@@"

	*?* - ternary

		output: split *paren arg* into two parts at the first occurrence
		of *char arg*.  If _value_ is true, print the first half,
		otherwise print the second half.

		arguments: *paren arg*, *char arg*, *pad char*, *field width*,
		*alignment*

		example: "%(yes no)?", 0 => "no"

	*=* - align

		output: none, after all other formats have been processed, add
		padding at this spot to make the total string equal to _value_
		width.  If _value_ is smaller than the total width, the string
		will be truncated to equal _value_ width.  Multiple align
		formats will be processed in the order they are parsed (from
		left to right).

		arguments: none

		example: "left%=right", 30 => "left                     right"

# NOTE ABOUT WIDTH

All widths refer to column widths.  For ascii, this is the same as the number
of characters in the string. e.g. the width of "hello" is 5.  However, many CJK
characters are double width, e.g. one character is two columns wide.
*altprintf* uses a variant of *wcswidth*(3) to determine a characters width.

# SEE ALSO

*altprintf*(1), *altprintf*(3)

# AUTHORS

Maintained by Stone Tickle <lattis@mochiro.moe>
