# altprintf
[![builds.sr.ht status](https://builds.sr.ht/~lattis.svg?search=altprintf)](https://builds.sr.ht/~lattis?search=altprintf)

altprintf is a string template language.  It was designed with a very specific
goal in mind: allowing users to customize a program's functionality through the
use of format strings.

## Features

+ Cool format specifiers, like conditionals.
+ UTF-8 character width sensitive alignment
+ Nesting expressions
+ Robust
  - the library makes no memory allocations
  - an advanced error reporting system
  - fuzz testing based on the grammar in the docs
+ Template strings are compiled into a custom binary format
  - this saves resources when reusing the same template

Note that altprintf is not designed to replace printf.  It is designed with the
specific use case of providing user-specified templates to customize output.

## Building

You need `meson`.  If you want man pages, you also need `scdoc`.

Example:

```sh
meson setup build
meson compile -C build
meson install -C build # for system-wide installation
```

## Docs

See the man pages
