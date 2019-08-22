# AltPrintf

AltPrintf is a gem that wraps `altprintf`.

It exposes the following module functions

+ `#fmt(format_string, *args, **kwargs)`
+ `#fmtm(passes, format_string, *args, **kwargs)`

In addition to the syntax of vanilla altprintf, the following additional
argument is accepted:

+ `<val>` - access the value of the given hash with key `:"#{val}"` rather than
  try to read the next argument.
