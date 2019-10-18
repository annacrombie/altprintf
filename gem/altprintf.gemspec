require 'date'
require_relative 'lib/altprintf/version'

Altprintf::SPEC = Gem::Specification.new do |s|
  s.name          = 'altprintf'
  s.version       = Altprintf::VERSION.join('.')
  s.date          = Date.today.strftime('%Y-%m-%d')
  s.summary       = 'A powerful printf-like template language'
  s.authors       = ['Stone Tickle']
  s.email         = 'lattis@mochiro.moe'
  s.homepage      = 'https://github.com/annacrombie/altprintf/tree/master/gem'
  s.license       = 'MIT'

  s.files         = Dir['{**/*}'] - ['Rakefile']

  s.platform = Gem::Platform::RUBY
  s.extensions = Dir["ext/**/extconf.rb"]
  s.require_paths = ['lib']

  s.required_ruby_version = '>= 2.5.5'

  s.add_development_dependency 'benchmark-ips', '~> 2.7'
  s.add_development_dependency 'rake-compiler', '~> 1.0'
  s.add_development_dependency 'rake', '~> 12.3'
  s.add_development_dependency 'rspec', '~> 3.9'
end unless Altprintf.const_defined?(:'SPEC')
