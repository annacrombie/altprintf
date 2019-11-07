# altprintf
[![builds.sr.ht status](https://builds.sr.ht/~lattis.svg?search=altprintf)](https://builds.sr.ht/~lattis?search=altprintf)

altprintf is a powerful printf-like template language.

## Features

+ Some cool format specifiers, like conditionals.
+ Advanced alignment
+ You can use it in your favorite programming language

## Why not use it?

+ You just need to print some strings
+ You need extremely fast formatting
  - altprintf is not as fast as plain old printf, but I want to get it as close
    as possible.  I doubt it will ever beat printf though, because of its more
    modular design

## Building

You need `meson` and `ninja`.  If you want man pages, you also need `scdoc`.

```sh
$ meson build/
$ ninja -C build/
```

## Installation

```sh
$ sudo ninja -C build/ install
```

## Docs

See the man pages
