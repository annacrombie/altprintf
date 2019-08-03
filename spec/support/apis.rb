module APIs
  class<<self
    def each
      (constants - [:Generic]).sort.each { |b| yield(const_get(b).new) }
    end
  end

  class Generic end

  class Cli < Generic
    EXEC = 'target/release/altprintf'

    def format(*args)
      `#{EXEC} #{args.map(&:to_s).map(&:shellescape).join(' ')}`
    end
  end

  class RubyExtension < Generic
    def initialize
      require_relative '../../ext/alt_printf'
    end

    def format(*args)
      AltPrintf.sprintf(*args)
    end
  end
end
