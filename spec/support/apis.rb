module APIs
  class<<self
    def each
      (constants - [:Generic]).sort.each { |b| yield(const_get(b).new) }
    end
  end

  class Generic end

  class Cli < Generic
    EXEC = 'target/release/altprintf'

    def fmt(args)
      `#{EXEC} #{args.map(&:to_s).map(&:shellescape).join(' ')}`
    end

    def fmtm(args)
      args.shift
      fmt(args)
    end
  end

  class RubyExtension < Generic
    def initialize
      require_relative '../../gem/alt_printf'
    end

    def fmt(args)
      hash = args.last.is_a?(Hash) ? args.pop : {}

      AltPrintf.fmt(*args, **hash)
    end

    def fmtm(args)
      hash = args.last.is_a?(Hash) ? args.pop : {}

      AltPrintf.fmtm(*args, **hash)
    end
  end
end
