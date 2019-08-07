#!/usr/bin/env ruby
require 'mkmf'

base_dir = File.join(__dir__, '../../../')
find_header('altprintf.h', File.join(base_dir, 'src'))
$objs = Dir[File.join(base_dir, 'target/release/*.o')] + ['alt_printf.o']

create_header
create_makefile('alt_printf')
