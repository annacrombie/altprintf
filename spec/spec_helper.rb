require 'bundler/setup'
require_relative 'support/apis'
require_relative 'support/generator'
require_relative 'support/tests'

$LOAD_PATH.unshift(File.expand_path(File.join(__dir__, '../gem/lib')))
Tests.load('spec/specs.rb')

RSpec.configure do |config|
  config.example_status_persistence_file_path = '.rspec_status'

  config.disable_monkey_patching!

  config.expect_with :rspec do |c|
    c.syntax = :expect
  end

  config.include APIs, Tests
end
