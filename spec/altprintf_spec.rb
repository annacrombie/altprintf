require_relative 'spec_helper'

RSpec.describe('altprintf') do
  it 'can echo' do
    expect(altprintf('test', 80)).to eq('test')
  end

  it 'can insert a string' do
    expect(altprintf('test %s', 80, 'string')).to eq('test string')
  end
end
