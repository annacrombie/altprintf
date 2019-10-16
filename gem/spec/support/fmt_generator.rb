module FmtGenerator
  CHARS = [%w[a z], %w[A Z]].map { |(a, z)| (a..z).take(26) }.flatten +
    ('0'..'10').to_a +
    %w[{ $ % ! & # @ = ` ~ ^ < * + - } " | : _ > ; ' \\ / . , \ ]

  module_function

  def fmt_with_args
    args = []

    fmt =
      (rand(5) + 2).times.map do
        spec, arg = random_spec
        args << arg

        "%#{random_fmt_args}#{spec} #{random_unicode}"
      end.join

    [fmt, args.compact]
  end

  def random_fmt_args
    [
      -> { ?~ + random_char },
      -> { ?0 },
      -> { ?\s },
      -> { "(#{random_chars})" },
      -> { ?- },
      -> { ?^ },
      -> { ?. },
      -> { random_int },
    ].yield_self { |e| e.sample(rand(e.length - 1) + 1) }.map(&:call).join
  end

  def random_spec
    {
      ?s => -> { random_chars },
      ?c => -> { random_char },
      ?f => -> { random_float },
      ?d => -> { random_int },
      ?? => -> { [true, false].sample },
      ?= => -> { random_int },
      ?% => -> { nil }
    }.to_a.sample.yield_self { |(s, p)| [s, p.call] }
  end

  def random_char
    CHARS.sample
  end

  def random_chars
    (rand(12) + 1).times.map { random_char }.join
  end

  def random_unicode
    12.times
      .map { begin; rand(0xffff).chr('UTF-8'); rescue RangeError; end }
      .select { |e| e != ?% && /[[:print:]]/.match?(e) }.join
  end

  def random_float
    rand * rand(100)
  end

  def random_int
    rand(100)
  end
end
