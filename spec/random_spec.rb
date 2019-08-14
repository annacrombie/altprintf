require_relative 'spec_helper'

RSpec.describe 'random formats' do
  it do
    r = Runner.new('benign_formats')
    api = APIs::RubyExtension.new

    r.run(100.times) do |id|
      fmt, args = FmtGenerator.fmt_with_args

      [$stdout, $stderr].each do |o|
        o.printf("---\ntest #%d\nfmt: %s\nargs: %s\n", id, fmt, args)
      end

      api.fmt([fmt] + args)
    end.then { |res| expect(res[:ok]).to eq(100) }
  end
end
