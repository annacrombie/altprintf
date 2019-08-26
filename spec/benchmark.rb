require 'bundler/setup'
require 'benchmark/ips'
require_relative 'support/altprintf'

fmt_string = "hello %-20s | %3d | %.50f\n"
args = ['Enots Elkcit', 42, Math::PI]

kwfmt_string = "hello %<name>-20s | %3<meaning>d | %<pi>.50f\n"
kwargs = { meaning: 42, pi: Math::PI, name: 'Enots Elkcit' }

Benchmark.ips do |bm|
  bm.report('sprintf') { sprintf(fmt_string, *args) }
  bm.report('altprintf') { AltPrintf.fmt(fmt_string, *args) }
  bm.report('sprintf - kwargs') { sprintf(kwfmt_string, **kwargs) }
  bm.report('altprintf - kwargs') { AltPrintf.fmt(kwfmt_string, **kwargs) }
  bm.compare!
end
