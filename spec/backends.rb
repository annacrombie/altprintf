module Backends
  class<<self
    def each
      [Cli, RubyExtension].each { |b| yield(b.new) }
    end
  end

  class Backend end

  class Cli < Backend
    def format(*args)
      cmd = "target/release/altprintf #{args.map(&:to_s).map(&:shellescape).join(' ')}"
      `#{cmd}`
    end
  end

  class RubyExtension < Backend
    def initialize
      require_relative '../ext/alt_printf'
    end

    def format(*args)
      ::AltPrintf.sprintf(*args)
    end
  end
end
