require_relative 'spec_helper'

RSpec.describe 'fuzz test' do
  let(:times) { (ENV['FUZZ_COUNT'] || 100).to_i.times }

  it 'valid formats' do
    r = Runner.new('valid_formats')
    api = APIs::RubyExtension.new

    r.run(times) do |id|
      fmt, args = FmtGenerator.fmt_with_args

      [$stdout, $stderr].each do |o|
        o.printf("---\ntest #%d\nfmt: %s\nargs: %s\n", id, fmt, args)
      end

      api.fmt([fmt] + args)
    end.yield_self { |res| expect(res[:ok]).to eq(times.count) }
  end

  it 'invalid formats (some errors rescued)' do
    r = Runner.new('invalid_formats')
    api = APIs::RubyExtension.new

    r.run(times) do |id|
      fmt, args = FmtGenerator.fmt_with_args
      fmt = fmt.chars.shuffle.join

      [$stdout, $stderr].each do |o|
        o.printf("---\ntest #%d\nfmt: %s\nargs: %s\n", id, fmt, args)
      end

      begin
        api.fmt([fmt] + args)
      rescue ArgumentError, TypeError, KeyError => e
        puts "rescued #{e}"
      end
    end.yield_self { |res| expect(res[:ok]).to eq(times.count) }
  end
end
