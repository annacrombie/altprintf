require_relative 'spec_helper'
require 'alt_printf'

LOGS = File.join(__dir__, '../log/').then do |d|
  {
    out: 'random_spec.log',
    err: 'random_spec_err.log'
  }.map { |k, v| [k, File.join(d, v)] }.to_h
end

RSpec.describe 'random formats' do
  it do
    cnt = { ok: 0, err: 0, crash: 0, timeout: 0 }
    tests = 100

    LOGS.each { |_, f| File.write(f, '') }

    tests.times do |i|
      fmt, args = Generator.fmt_with_args

      printf(
        "\r%14.14s %d/%d, [%s]\e[K",
        fmt,
        i + 1,
        tests,
        cnt.map { |k, v| "#{k}:#{v}" }.join('|')
      )

      pid =
        fork do
          $stdout.reopen(LOGS[:out], 'a')
          $stderr.reopen(LOGS[:err], 'a')
          [$stdout, $stderr].each do |o|
            o.printf(
              "random_spec test %d\nfmt: %s\n args: %s\n-----\n",
              i,
              fmt,
              args
            )
            o.flush
          end

          AltPrintf.fmt(fmt, *args)
        end

      q = Queue.new
      thr = Thread.new { Process.wait(pid); q.push($?.exitstatus) }

      slept = 0
      while slept < 100 && thr.alive?
        sleep 0.01
        slept += 1
      end

      if thr.alive?
        Process.kill(9, pid)
        thr.exit
        cnt[:timeout] += 1
      else
        cnt[
          case q.pop
          when 0
            :ok
          when nil
            :crash
          else
            :err
          end
        ] += 1
      end

      break if ENV['FAIL_FAST'] &&
        %i[crash err timeout].detect { |k| cnt[k].positive? }
          .tap { |k| k.nil? ? false : puts("\nfailing-fast: #{k}") }
    end

    puts

    expect(cnt[:ok]).to eq(tests)
  end
end
