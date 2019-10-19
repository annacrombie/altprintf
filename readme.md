# altprintf
[![builds.sr.ht status](https://builds.sr.ht/~lattis.svg?search=altprintf)](https://builds.sr.ht/~lattis?search=altprintf)

altprintf is a powerful printf-like template language.

## Why use it?

+ If you need to format/align strings with wide characters
+ If you want build a tui that is customisable with format strings
+ If you want fancy output for your cli

## Why not use it?

+ You need debugging output from your program
  - Good old printf is your friend
+ Altprintf can't do what you need
  - Submit a patch!
+ You need extremely fast formatting
  - altprintf is not as fast as plain old printf, but I want to get it as close
    as possible.  I doubt it will ever beat printf because its code is split
    into a library that can be shared among many implementations, and it does
    things like determining display width of utf8 characters.

## Building

altprintf uses meson.
```sh
$ meson build/
$ ninja -C build/
```

## Installation

```sh
$ sudo ninja -C build/ install
```
