require_relative 'spec_helper'

RSpec.describe('altprintf') do
  Backends.each do |backend|
    context backend.class.name do
      it 'can echo' do
        expect(backend.format('test')).to eq('test')
      end

      it 'can insert a string' do
        expect(backend.format('test %s', 'string')).to eq('test string')
      end

      it 'can format doubles' do
        expect(backend.format('%.3f', 1.5)).to eq('1.500')
      end

      it 'can format ints' do
        expect(backend.format('%d', 80)).to eq('80')
      end

      it 'can format chars' do
        expect(backend.format('%c', 'c')).to eq('c')
      end

      it 'can format *' do
        expect(backend.format('%~#*', 10)).to eq('#' * 10)
      end

      it 'can format ?' do
        expect(backend.format('%(a:b)?', 1)).to eq('a')
      end

      it 'can format =' do
        expect(backend.format('a%~ =b', 10)).to eq('a        b')
      end
    end
  end
end
