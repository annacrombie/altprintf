group APIs::Generic, {
  [:fmt, 'test'] => 'test',
  [:fmt, 'test %s', 'string'] => 'test string',
  [:fmt, '%.3f', 1.5] => '1.500',
  [:fmt, '%d', 80] => '80',
  [:fmt, '%c', 'c'] => 'c',
  [:fmt, '%~#*', 10] => '#' * 10,
  [:fmt, '%(a:b)?', 1] => 'a',
  [:fmt, 'a%~ =b', 10] => 'a        b',
  [:fmt, '片%s', '仮名'] => '片仮名',
  [:fmtm, 3, "%%(%%%s:%%%s)?", 'd', '~**', 1, 15] => '15',
  [:fmtm, 3, "%%(%%%s:%%%s)?", 'd', '~**', 0, 15] => '*' * 15,
}

group APIs::RubyExtension, {
  [:fmt, '%<age>d years old', { age: 25 }] => '25 years old',
  [:fmtm, 0, "%%"] => '%%',
}
