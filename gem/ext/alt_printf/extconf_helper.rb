require 'mkmf'

module ExtconfHelper
  BASE_DIR = File.join(__dir__, '../../../')

  module_function

  def dev_header
    find_header('altprintf.h', File.join(BASE_DIR, 'src'))
  end

  def dev_objs(folder = 'release')
    $objs = Dir[File.join(BASE_DIR, "target/#{folder}/*.o")] + ['alt_printf.o']
  end

  def setup(mode = 'release')
    puts "extconf setting up #{mode}"
    case mode
    when 'release'
      # do nothing
    when 'dev'
      dev_header
      dev_objs('release')
    when 'debug'
      dev_header
      $defs.push("-DDEBUG")
      dev_objs('debug')
    else
      raise(ArgumentError, "invalid mode #{mode}")
    end

    create_header
    create_makefile('alt_printf')
  end
end
