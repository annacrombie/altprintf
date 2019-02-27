require_relative 'spec_helper'

RSpec.describe('altprintf') do
  before(:all) do
    @exec = './altprintf'
  end

  it 'formats text' do
    exec('test')
  end
end
