require_relative 'spec_helper'

RSpec.describe('altprintf') do
  Backends.each do |backend|
    context backend.class.name do
      {
        ['test'] => 'test',
        ['test %s', 'string'] => 'test string',
        ['%.3f', 1.5] => '1.500',
        ['%d', 80] => '80',
        ['%c', 'c'] => 'c',
        ['%~#*', 10] => '#' * 10,
        ['%(a:b)?', 1] => 'a',
        ['a%~ =b', 10] => 'a        b',
        ['片%s', '仮名'] => '片仮名'
      }.each do |args, result|
        it "can format #{args}" do
          expect(backend.format(*args)).to eq(result)
        end
      end
    end
  end
end
