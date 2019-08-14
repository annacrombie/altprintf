class Runner
  TIMEOUT = 100

  def initialize(name, fail_fast: nil)
    @fail_fast = fail_fast || !!ENV['FAIL_FAST']
    @fd =
      File.join(__dir__, '../../log/').then do |d|
        { out: "#{name}.log", err: "#{name}_err.log" }
          .map { |k, v| [k, File.join(d, v)] }
          .each { |(_, f)| File.write(f, '') }
          .to_h
      end

    reset_res
  end

  def reset_res
    @res = { ok: 0, err: 0, crash: 0, timeout: 0 }
  end

  def run(times, &block)
    total = times.count

    times.each do |i|
      print(status(i, total))
      result = forkblock(i, &block)

      if @fail_fast && result != :ok
        puts "failing fast. last result: #{result}"
        break
      end

      @res[result] += 1
    end

    puts

    @res.tap { reset_res }
  end

  private

  def status(i, total)
    sprintf(
      "\r%d/%d, [%s]\e[K",
      i + 1,
      total,
      @res.map { |k, v| "#{k}:#{v}" }.join('|')
    )
  end

  def forkblock(id)
    pid =
      fork do
        $stdout.reopen(@fd[:out], 'a')
        $stderr.reopen(@fd[:err], 'a')

        yield(id)
      end

    q = Queue.new
    thr = Thread.new { Process.wait(pid); q.push($?.exitstatus) }

    slept = 0
    while slept < TIMEOUT && thr.alive?
      sleep 0.01
      slept += 1
    end

    if thr.alive?
      Process.kill(9, pid)
      thr.exit
      :timeout
    else
      case q.pop
      when 0
        :ok
      when nil
        :crash
      else
        :err
      end
    end
  end
end
