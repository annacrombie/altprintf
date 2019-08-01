#!/usr/bin/env ruby
require 'mkmf'

find_header('altprintf.h', File.join(__dir__, '../src/'))
$objs = Dir.glob('../target/release/*.o') + ['alt_printf.o']

create_header
create_makefile('alt_printf')
