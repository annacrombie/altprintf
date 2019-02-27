module CliRunner
  def exec(*args)
    cmd = "#{@exec} #{args.map(&:to_s).map(&:shellescape).join(' ')}"
    `#{cmd}`
  end
end
