$LOAD_PATH.unshift(File.expand_path(File.join(__dir__, '../gem/lib')))

require 'bundler/setup'
require 'alt_printf'
Dir[File.join(__dir__, 'support/*.rb')].each { |f| require(f) }

RSpec.configure do |config|
  config.example_status_persistence_file_path = '.rspec_status'

  config.disable_monkey_patching!

  config.expect_with :rspec do |c|
    c.syntax = :expect
  end

  config.include APIs
end
