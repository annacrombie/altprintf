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
  - Submit an issue!
+ You need extremely fast formatting
  - altprintf is about 1/2 the speed of native printf, but I'm looking to
    improve this.  A big reason for this is because the code is split into
    several components, a cli front-end, a parser, and the core peice where the
    output is generated.  Writing it this way opens the door to many different
    front-ends all using the same core, e.g. a ruby gem, another c program, a
    cli, etc..
