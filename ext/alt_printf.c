#include <ruby.h>
#include <ruby/encoding.h>
#include "extconf.h"

#define MODNAME "AltPrintf"

VALUE rb_altprintf(size_t argc, VALUE *argv, VALUE self) {
  VALUE fmt, args;
  rb_scan_args(argc, argv, "*:", &fmt, &args);
}

void Init_altprintf()
{
  VALUE mod = rb_define_module(MODNAME);
  rb_define_module_function(mod, "sprintf", rb_altprintf, 2);
}
