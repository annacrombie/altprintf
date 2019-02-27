# AltPrintf
[![Build Status](https://travis-ci.org/zashoku/alt_printf.svg?branch=master)](https://travis-ci.org/zashoku/alt_printf)

AltPrintf is a alternative printf

## Motivation

The vanilla ruby Kernel#sprintf works fine for formatting most strings

For example:
```ruby
favorite_animals = %w[cat dog elephant]

favorite_animals.each_with_index do |animal, i|
  puts sprintf("%-8s #%d", animal, i)
end
```

Produces a nicely formatted list:

```
cat      #0
dog      #1
elephant #2
```

But it won't work as expected if you use strings containing wide characters

```ruby
favorite_animals = %w[cat dog キリン 野鳥 elephant]

favorite_animals.each_with_index do |animal, i|
  puts sprintf("%-8s #%d", animal, i)
end
```

```
cat      #0
dog      #1
キリン      #2
野鳥       #3
elephant #4
```

This is happening because strings like `キリン` and `帽子` have a length of 3
and 2 respectively, but are "wide" so they really take up 6 and 4 screen
columns.

The solution?  There are many libraries that can provide the width of a string,
however native ruby libraries are quite slow, therefore `alt_printf` is
implemented as a native extension.  Since I went to all the trouble to
re-implement `sprintf`, I thought I would also add some new functionality!

```ruby
AltPrintf.printf(
  "%~|(yes|no)<bool>? %(%s)<time>t",
  {bool: true, time: Time.now}
)
#=> "yes 1537537300"
```

## Usage

`require 'alt_printf'`

## Format Spec

## General Formatter

### Arguments
- [x] `<val>` - access the value of the given hash with key `:val` and store it as `value`
- [ ] `(arg)` - the string within `()` is taken as a `string_argument`
- [x] `~`     - the following character is taken as a `character_argument`.  There can only be one character argument so the last one parsed is the one that is used.
- [x] `-`     - left-align output within given width
- [x] ` `     - pad output with spaces (this is the default for strings)
- [x] `0`     - pad output with zeroes (this is the default for numeric types)
- [x] `0-9`   - starting with a `1-9` the integer is read as `field_width`
- [ ] `.0-9`  - the integer after the `.` is read as `precision`
- [ ] `!0-9`  - the integer after the `!` is read as `absolute_width`

### Specifiers
- [x] `*`     - print `value` occurences of `character_argument`
- [ ] `?`     - given a `string_argument` like `true:false`, if `value` is true, print `true`, else print `false`.  If a `character_argument` is given, it is used as the separator in the `string_argument`.  The default separator is `:`
- [x] `s`     - print `value` as a string
- [ ] `t`     - call `strftime` on `value`, passing `string_argument`
- [x] `d`     - format `value` as an integer
- [ ] `D`     - format `value` as a *duration*
- [ ] `z`     - format `value` as a *filesize*
- [x] `%`     - a literal `%`
- [x] `=`     - right-align remainder of string

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
