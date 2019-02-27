require_relative 'spec_helper'

RSpec.describe('altprintf') do
  before(:all) do
    @exec = 'target/release/altprintf'
  end

  it 'can echo' do
    expect(exec('test')).to eq('test')
  end

  it 'can insert a string' do
    expect(exec('test %s', 'string')).to eq('test string')
  end
end
