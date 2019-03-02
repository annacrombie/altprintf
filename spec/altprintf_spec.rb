require_relative 'spec_helper'

RSpec.describe('altprintf') do
  it 'can echo' do
    expect(altprintf('test', 80)).to eq('test')
  end

  it 'can insert a string' do
    expect(altprintf('test %s', 80, 'string')).to eq('test string')
  end

  it 'can format doubles' do
    expect(altprintf('%.3f', 80, 1.5)).to eq('1.500')
  end

  it 'can format ints' do
    expect(altprintf('%d', 80, 80)).to eq('80')
  end

  it 'can format chars' do
    expect(altprintf('%c', 80, 'c')).to eq('c')
  end

  it 'can format *' do
    expect(altprintf('%~#*', 80, 10)).to eq('#' * 10)
  end

  it 'can format ?' do
    expect(altprintf('%(a:b)?', 80, 1)).to eq('a')
  end
end
