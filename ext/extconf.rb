#!/usr/bin/env ruby
require 'mkmf'

find_header('altprintf.h', File.join(__dir__, '../src/'))
$objs = %w[altprintf.o alt_printf.o strbuf.o list.o]

create_header
create_makefile('alt_printf')
