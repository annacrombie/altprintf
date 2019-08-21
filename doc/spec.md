## Format Spec

## General Formatter

### Arguments
- `<arg>` - the string within `<>` is taken as a `string_argument`
- `(arg)` - the string within `()` is taken as a `string_argument`
- `~` - the following character is taken as a `character_argument`.
- `-` - left-align output within given width
- `^` - center output within given width
- ` ` - pad output with spaces (this is the default for strings)
- `0` - pad output with zeroes (this is the default for numeric types)
- `1-9` - the integer is read as `field_width`
- `.` - the next integer `.` is read as `precision`

### Specifiers
- `*` - print `value` occurences of `character_argument`
- `?` - given a `string_argument` like `true:false`, if `value` is true,
  print `true`, else print `false`.  If a `character_argument` is given, it is
  used as the separator in the `string_argument`.  The default separator is `:`
- `s` - print `value` as a string
- `d` - format `value` as an integer
- `%` - a literal `%`
- `=` - right-align remainder of string using `value` as the total desired width
