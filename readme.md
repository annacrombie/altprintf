# altprintf
[![builds.sr.ht status](https://builds.sr.ht/~lattis.svg?search=altprintf)](https://builds.sr.ht/~lattis?search=altprintf)

altprintf is a powerful template language.

## Features

+ Cool format specifiers, like conditionals.
+ Utf-8 character width sensitive alignment
+ Nesting expressions
+ Robust; the library makes no calls to malloc, and has an advanced error
  reporting system

Note that altprintf is not designed to replace printf.  It is designed with the
specific use case of providing user-specified templates to customize output.

## Building

You need `meson`.  If you want man pages, you also need `scdoc`.

Example:

```sh
meson build
ninja -C build
ninja -C build install
```

## Docs

See the man pages
