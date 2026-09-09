#include "Parameters.h"

juce::AudioProcessorValueTreeState::ParameterLayout createParametersFromSpecs(const std::vector<ParamSpec>& specs)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (const auto& s : specs) {
        juce::AudioParameterFloatAttributes attrs;
        if (s.stringFromValue)
            attrs = attrs.withStringFromValueFunction(s.stringFromValue);

        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{s.id}, s.name, s.range, s.defaultValue, attrs));
    }
    return layout;
}
