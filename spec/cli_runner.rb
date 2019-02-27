module CliRunner
  def exec(*args)
    cmd = "#{@exec} #{args.map(&:to_s).map(&:shellescape).join(' ')}"
    puts cmd
    `#{cmd}`
  end
end
