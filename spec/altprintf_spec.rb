require_relative 'spec_helper'

Examples.load(File.join(__dir__, 'examples/basic.rb'))

RSpec.describe('altprintf') do
  APIs.each do |api|
    context api.class do
      Examples.for(api) do |group, tests|
        context group do
          tests.each do |args, result|
            pretty = args[1..].map(&:inspect).join(', ')

            it "can #{args[0]}(#{pretty}) => \"#{result}\"" do
              expect(api.send(args[0], args[1..])).to eq(result)
            end
          end
        end
      end
    end
  end
end
