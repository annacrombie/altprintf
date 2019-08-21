group APIs::RubyExtension, {
  [:fmtm, 3, "%%~:(%%%s:%%%s)?", 'd', '~**', false, 15] => '*' * 15,
  [:fmt, '%<age>d years old', { age: 25 }] => '25 years old',
  [:fmtm, 0, "%%"] => '%%',
}
