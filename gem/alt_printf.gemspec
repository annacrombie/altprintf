require 'date'
require_relative 'lib/alt_printf/version'

AltPrintf::SPEC = Gem::Specification.new do |s|
  s.name          = 'alt_printf'
  s.version       = AltPrintf::VERSION.join('.')
  s.date          = Date.today.strftime('%Y-%m-%d')
  s.summary       = 'A powerful printf-like template language'
  s.authors       = ['Stone Tickle']
  s.email         = 'lattis@mochiro.moe'
  s.homepage      = 'https://github.com/alt_printf/gem'
  s.license       = 'MIT'

  s.files         = Dir['{**/*}']

  s.platform = Gem::Platform::RUBY
  s.extensions = Dir["ext/**/extconf.rb"]
  s.require_paths = ['lib']

  s.required_ruby_version = '>= 2.6.3'

  s.add_development_dependency 'rake-compiler', '~> 1.0'
  s.add_development_dependency 'rake', '~> 12.3'
end unless AltPrintf.const_defined?(:'SPEC')
