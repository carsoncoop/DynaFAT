#include "Filter.h"

void Filter::prepare() {
    filterOrder = Pre;
    activated = true;
    filter.setCutoffFrequency(120.0f);
    filter.setResonance(0.07f);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

void Filter::myProcess(const juce::dsp::AudioBlock<float>& inputBlock) {
    juce::dsp::AudioBlock<float> block(inputBlock);
    const juce::dsp::ProcessContextReplacing context(block);
    filter.process(context);
}