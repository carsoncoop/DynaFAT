#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <functional>

struct ParamSpec {
    juce::String id;
    juce::String name;
    juce::NormalisableRange<float> range {0.0f, 1.0f};
    float defaultValue = 0.0f;
    std::function<juce::String(float,int)> stringFromValue;

    ParamSpec(const juce::String& i, const juce::String& n,
             const juce::NormalisableRange<float>& r, float d,
             std::function<juce::String(float,int)> fmt = {})
        : id(i), name(n), range(r), defaultValue(d), stringFromValue(fmt) {}
};

juce::AudioProcessorValueTreeState::ParameterLayout createParametersFromSpecs(const std::vector<ParamSpec>& specs);
