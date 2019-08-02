# AltPrintf

AltPrintf is a alternative printf

## Format Spec

## General Formatter

### Arguments
- [ ] `<val>` - access the value of the given hash with key `:val` and store it as `value`
- [x] `(arg)` - the string within `()` is taken as a `string_argument`
- [x] `~`     - the following character is taken as a `character_argument`.  There can only be one character argument so the last one parsed is the one that is used.
- [x] `-`     - left-align output within given width
- [x] ` `     - pad output with spaces (this is the default for strings)
- [x] `0`     - pad output with zeroes (this is the default for numeric types)
- [x] `0-9`   - starting with a `1-9` the integer is read as `field_width`
- [x] `.0-9`  - the integer after the `.` is read as `precision`

### Specifiers
- [x] `*`     - print `value` occurences of `character_argument`
- [x] `?`     - given a `string_argument` like `true:false`, if `value` is true,
  print `true`, else print `false`.  If a `character_argument` is given, it is
  used as the separator in the `string_argument`.  The default separator is `:`
- [x] `s`     - print `value` as a string
- [x] `d`     - format `value` as an integer
- [ ] `D`     - format `value` as a *duration*
- [ ] `z`     - format `value` as a *filesize*
- [x] `%`     - a literal `%`
- [x] `=`     - right-align remainder of string using `value` as the total
  desired width

## Duration Formatter

### Arguments
- [ ] `-`     - left-align output within given width
- [ ] ` `     - pad output with spaces (this is the default for strings)
- [ ] `0`     - pad output with zeroes (this is the default for numeric types)
- [ ] `0-9`   - starting with a `1-9` the integer is read as `field_width`
- [ ] `?`     - if the unit is greater than the total duration, omit it from the output
- [ ] `:`     - suffix the output with `:`

### Specifiers
- [ ] `%`     - a literal `%`
- [ ] `S`     - total seconds
- [ ] `M`     - total minutes
- [ ] `H`     - total hours
- [ ] `D`     - total days
- [ ] `W`     - total weeks
- [ ] `Y`     - total years
- [ ] `s`     - seconds
- [ ] `m`     - minutes
- [ ] `h`     - hours
- [ ] `d`     - days
- [ ] `w`     - weeks
- [ ] `y`     - years

## Filesize Formatter

### Arguments
- [ ] `-`     - left-align output within given width
- [ ] ` `     - pad output with spaces (this is the default for strings)
- [ ] `0`     - pad output with zeroes (this is the default for numeric types)
- [ ] `0-9`   - starting with a `1-9` the integer is read as `field_width`
- [ ] `.0-9`  - the integer after the `.` is read as `precision`
- [ ] `&`     - infix a space between the number and the unit label
- [ ] `:`     - suffix the output with `:`

### Specifiers
- [ ] `%`     - a literal `%`
- [ ] `S`     - total seconds
- [ ] `M`     - total minutes
- [ ] `H`     - total hours
- [ ] `D`     - total days
- [ ] `W`     - total weeks
- [ ] `Y`     - total years
- [ ] `s`     - seconds
- [ ] `m`     - minutes
- [ ] `h`     - hours
- [ ] `d`     - days
- [ ] `w`     - weeks
- [ ] `y`     - years
