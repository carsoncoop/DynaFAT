//Use followEnv on pre- and post-sample. Then, processedSample *= computeCorrectionGain(...)
#pragma once
#include <vector>
#include <juce_dsp/juce_dsp.h>

class Compressor {
    float sampleRate = 44100.0f;
    unsigned int numChannels = 2;

    float envAttackCoeff = 0.0f;
    float envReleaseCoeff = 0.0f;
    std::vector<float> envPerChannel;
    // per-channel smoothed gain in dB (0.0f = no change)
    std::vector<float> smoothedGainDbPerChannel;

    float ratio = 1;
    float thresh_dB_high = 1;
    float thresh_dB_low = 1;

    bool activated = true;


public:
    void prepare(float sampleRate_, unsigned int numChannels_, float envAttackMs_, float envReleaseMs_,
        float ratio_, float thresh_dB_high_, float thresh_dB_low_);

    std::vector<float>& getEnvPerChannel() { return envPerChannel; }

    void setActivation(const bool status) {activated = status;}
    bool getActivation() const {return activated;}

    //attack/release for envelope measurements
    void setEnvAttack(float envAttackMs_);
    void setEnvRelease(float envReleaseMs_);

    //ratio & thresh
    void setRatio(float ratio_);
    void setThreshHigh(float thresh_dB_high_);
    void setThreshLow(float thresh_dB_low_);

    void followEnv (float inputSample, int channelIndex);

    // Compute the correction gain (returns linear multiplier)
    //Do math in dB
    [[nodiscard]] float computeGainChange (int channelIndex);
};
