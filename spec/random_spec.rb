require_relative 'spec_helper'
require 'alt_printf'

LOG_DIR = File.join(__dir__, '../log/')

RSpec.describe 'random formats' do
  it do
    cnt = { ok: 0, err: 0, crash: 0, timeout: 0 }
    tests = 100

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
          $stdout.reopen(File.join(LOG_DIR, 'random_spec.log'))
          $stderr.reopen(File.join(LOG_DIR, 'random_spec.err.log'))
          p fmt
          p args

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

    expect(cnt[:ok]).to eq(tests)
  end
end
