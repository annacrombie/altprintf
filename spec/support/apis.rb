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

    alias fmtm fmt
  end

  class RubyExtension < Generic
    def initialize
      require_relative '../../gem/alt_printf'
    end

    def parse_args(args)
      fmt = args.shift
      hash = args.last.is_a?(Hash) ? args.pop : {}
      [fmt, hash]
    end

    def fmt(args)
      fmt, hash = *parse_args(args)

      AltPrintf.fmt(fmt, *args, **hash)
    end

    def fmtm(args)
      fmt, hash = *parse_args(args)

      AltPrintf.fmtm(fmt, args[0], *args[1..], **hash)
    end
  end
end
