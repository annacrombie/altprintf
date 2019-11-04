require 'mkmf'

module ExtconfHelper
  BASE_DIR = File.join(__dir__, '../../../')
  SRC_DIR = File.join(BASE_DIR, 'subprojects/libaltprintf/src/')
  BUILD_DIR = File.join(BASE_DIR, 'build')
  LIB_DIR = File.join(BUILD_DIR, 'subprojects/libaltprintf/')
  INC_DIR = File.join(BASE_DIR, 'subprojects/libaltprintf/include')

  module_function

  def dev_setup
    unless find_header('altprintf.h', INC_DIR)
      $stderr.puts("couldn't find header 'altprintf.h'")
      exit(1)
    end

    unless find_library('altprintf', 'apf_parse', LIB_DIR)
      $stderr.puts("you haven't built libaltprintf yet")
      exit(1)
    end
  end

  def setup(mode = 'release')
    puts "extconf setting up #{mode}"
    case mode
    when 'release'
      # do nothing
    when 'dev'
      dev_setup
    else
      raise(ArgumentError, "invalid mode #{mode}")
    end

    create_header
    create_makefile('altprintf/altprintf')
  end
end
