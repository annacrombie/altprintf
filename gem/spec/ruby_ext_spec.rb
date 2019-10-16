require_relative 'spec_helper'

RSpec.describe APIs::RubyExtension do
  context 'fmtm' do
    context 'invalid passes arg' do
      it 'raises an type error' do
        expect { subject.fmtm(['nan', '%%']) }.to raise_exception(TypeError)
      end
    end
  end

  it 'should take the last argument as a hash if no kwargs are given' do
    d = { "mail"=>"0", "weechat_messages"=>"0", "input_method"=>"英",
          "space"=>nil }.transform_keys(&:to_sym)

    s = '〒%<mail>s メ%<weechat_messages>s 入%<input_method>s'

    Altprintf.fmtm(2, s, d)
  end

  context 'an invalid key is used' do
    it 'should raise a key error' do
      expect do
        subject.fmtm([2, '%<wrong>s', right: 'right'])
      end.to raise_exception(KeyError)
    end
  end
end