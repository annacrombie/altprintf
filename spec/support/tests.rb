module Tests
  class << self
    attr_accessor :tests

    def load(file)
      self.tests = {}
      eval(File.read(file))
    end

    def group(g, ts)
      tests.merge!(g => ts)
    end

    def for(kind, &block)
      (tests || {}).select { |c, _| kind.is_a?(c) }.each(&block)
    end
  end
end
