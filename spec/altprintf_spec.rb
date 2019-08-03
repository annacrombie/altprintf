require_relative 'spec_helper'

RSpec.describe('altprintf') do
  APIs.each do |api|
    context api do
      Tests.for(api) do |group, tests|
        context group do
          tests.each do |args, result|
            it "\"#{args[0]}\" (#{args[1..].join(', ')}) => \"#{result}\"" do
              expect(api.format(*args)).to eq(result)
            end
          end
        end
      end
    end
  end
end
