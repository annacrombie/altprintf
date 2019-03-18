require_relative 'spec_helper'

RSpec.describe('altprintf') do
  it 'can echo' do
    expect(altprintf('test')).to eq('test')
  end

  it 'can insert a string' do
    expect(altprintf('test %s', 'string')).to eq('test string')
  end

  it 'can format doubles' do
    expect(altprintf('%.3f', 1.5)).to eq('1.500')
  end

  it 'can format ints' do
    expect(altprintf('%d', 80)).to eq('80')
  end

  it 'can format chars' do
    expect(altprintf('%c', 'c')).to eq('c')
  end

  it 'can format *' do
    expect(altprintf('%~#*', 10)).to eq('#' * 10)
  end

  it 'can format ?' do
    expect(altprintf('%(a:b)?', 1)).to eq('a')
  end

  it 'can format =' do
    expect(altprintf('a%~ =b', 10)).to eq('a        b')
  end
end
