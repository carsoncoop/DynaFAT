#pragma once
#include <juce_dsp/juce_dsp.h>

enum FilterType {
    Lowpass,
    Highpass,
    Bandpass,
};

enum FilterOrder {
    Pre,
    Post
};

class Filter {
    FilterType filterType = Lowpass;
    FilterOrder filterOrder = Pre;

    float cutoff = 0.0f;
    float resonance = 0.0f;

    bool activated = false;

public:
    void prepare(float sampleRate_, float cutoff_, float resonance_);

    void setActivation(const bool status) {activated = status;}
    bool getActivation() const {return activated;}

    //attack/release for envelope measurements
    void setEnvAttack(float envAttackMs_);
    void setEnvRelease(float envReleaseMs_);

    void followEnv (float inputSample, int channelIndex);

    // Compute the correction gain (returns linear multiplier)
    [[nodiscard]] float computeCorrectionGain (float preSampleEnv, int channelIndex) const;
};
};
