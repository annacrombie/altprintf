module Tests
  TESTS = {
    APIs::Generic => {
      ['test'] => 'test',
      ['test %s', 'string'] => 'test string',
      ['%.3f', 1.5] => '1.500',
      ['%d', 80] => '80',
      ['%c', 'c'] => 'c',
      ['%~#*', 10] => '#' * 10,
      ['%(a:b)?', 1] => 'a',
      ['a%~ =b', 10] => 'a        b',
      ['片%s', '仮名'] => '片仮名'
    },
    APIs::RubyExtension => {
      ['%<age>d years old', { age: 25 }] => '25 years old'
    }
  }

  class << self
    def for(kind, &block)
      TESTS.select { |c, _| kind.is_a?(c) }.each(&block)
    end
  end
end
