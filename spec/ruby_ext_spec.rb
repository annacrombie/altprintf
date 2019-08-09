require_relative 'spec_helper'
require 'alt_printf'

RSpec.describe 'ruby extension' do
  context 'fmtm' do
    context 'invalid passes arg' do
      it 'raises an argument error' do
        expect { AltPrintf.fmtm('nan', '%%') }.to raise_exception(ArgumentError)
      end
    end
  end
end
