# Ideas

Kind of like a to do list, but you don't have to do everything on it.

## Misc

## Features

### Don't consume flag

Add a flag that can be used by the api implementation to determine wether or not
to "consume" the argument (i.e. advance argi).  This could be used to reuse the
same argument multiple times.

e.g. (if the flag was `&`)

```sh
$ altprintf "%%(%&~#*\n)*" 5
#####
#####
#####
#####
#####
```
