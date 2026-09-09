#pragma once
#include <juce_dsp/juce_dsp.h>

enum FilterOrder {
    Pre,
    Post
};

class Filter {
    juce::dsp::StateVariableTPTFilter<float> filter;

    FilterOrder filterOrder = Pre;
    bool activated = true;

public:
    void prepare();

    [[nodiscard]] juce::dsp::StateVariableTPTFilter<float> getFilter() const {return filter;}

    void setActivation(const bool status) {activated = status;}
    [[nodiscard]] bool getActivation() const {return activated;}

    void setCutoff(const float cutoff_) {
        filter.setCutoffFrequency(cutoff_);
    }

    [[nodiscard]] float getCutoff() const {
        return filter.getCutoffFrequency();
    }

    void setResonance(const float resonance_) {
        filter.setResonance(resonance_);
    }
    [[nodiscard]] float getResonance() const {
        return filter.getResonance();
    }

    void setFilterType(juce::dsp::StateVariableTPTFilterType filterType_) {
        filter.setType(filterType_);
    }

    [[nodiscard]] juce::dsp::StateVariableTPTFilterType getFilterType() const {
        return filter.getType();
    }

    void setFilterOrder(const FilterOrder filterOrder_) {filterOrder = filterOrder_;}
    [[nodiscard]] FilterOrder getFilterOrder() const {return filterOrder;}

    void myProcess(const juce::dsp::AudioBlock<float>& inputBlock);

};

