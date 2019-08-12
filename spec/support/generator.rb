module Generator
  CHARS = %w[a A].map { |s| (s..).take(26) }.flatten +
    ('0'..).take(10) +
    %w[{ $ % ! & # @ = ` ~ ^ < * + - } " | : _ > ; ' \\ / . , \ ]

  module_function

  def fmt_with_args
    args = []

    fmt =
      (rand(5) + 2).times.map do
        spec, arg = random_spec
        args << arg

        "%#{random_fmt_args}#{spec} "
      end.join

    [fmt, args]
  end

  def random_fmt_args
    [
      -> { '~' + random_char },
      -> { '0' },
      -> { ' ' },
      -> { "(#{random_chars})" },
      -> { random_int.then { |i| ["#{i}.#{i}", ".#{i}"].sample } },
      #-> { random_chars },
    ].then { |e| e.sample(rand(e.length - 1) + 1) }.map(&:call).join
  end

  def random_spec
    {
      's' => -> { random_chars },
      'c' => -> { random_char },
      'f' => -> { random_float },
      'd' => -> { random_int },
      '?' => -> { [true, false].sample },
      '=' => -> { random_int }
    }.to_a.sample.then { |(s, p)| [s, p.call] }
  end

  def random_char
    CHARS.sample
  end

  def random_chars
    (rand(12) + 1).times.map { random_char }.join
  end

  def random_float
    rand * rand(100)
  end

  def random_int
    rand(100)
  end
end
