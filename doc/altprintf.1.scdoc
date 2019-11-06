altprintf(1)

# NAME

altprintf - command line frontend for *altprintf*(7)

# SYNOPSIS

*altprintf* *format string* [_value_ [_value_ [_value_ ...]]]

# DESCRIPTION

*altprintf* is a command line program that works as a frontend to
*altprintf*(7).  Command line arguments are converted into appropriate types,
and formatting proceeds as described in *altprintf*(7).

# MULTIPASS

The CLI does provide one additional piece of functionality:  If too many
_values_ are given for the format string, it will attempt to format the
formatted string using those arguments.

That was a bit confusing, so here is an example:

```
$ altprintf "%%(%s %s)?" true false
%(true false)?
$ altprintf "%%(%s %s)?" true false 0
false
$ altprintf "%%(%s %s)?" true false 1
true
```

In the given format string, only two _values_ are needed, since *%* takes no
_value_.  Likewise, only two _values_ are supplied, namely "true" and "false".
Thus there is only one pass, and the output is "%(true false)?".

In the second and third examples, nothing changes except an additional _value_
is supplied.  This causes an additional pass which uses the result from the
first pass,  "%(true false)?", as its *format string*.

# SEE ALSO

*altprintf*(7)

# AUTHORS

Maintained by Stone Tickle <lattis@mochiro.moe>
