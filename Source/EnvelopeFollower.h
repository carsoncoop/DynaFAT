//Use followEnv on pre- and post-sample. Then, processedSample *= computeCorrectionGain(...)
#pragma once
#include <vector>
#include <juce_dsp/juce_dsp.h>

class EnvelopeFollower {
    float sampleRate = 44100.0f;
    unsigned int numChannels = 2;

    float envAttackCoeff = 0.0f;
    float envReleaseCoeff = 0.0f;
    std::vector<float> envPerChannel;

    bool activated = true;


public:
    void prepare(float sampleRate_, unsigned int numChannels_, float envAttackMs_, float envReleaseMs_);

    std::vector<float>& getEnvPerChannel() { return envPerChannel; }

    void setActivation(const bool status) {activated = status;}
    bool getActivation() const {return activated;}

    //attack/release for envelope measurements
    void setEnvAttack(float envAttackMs_);
    void setEnvRelease(float envReleaseMs_);

    void followEnv (float inputSample, int channelIndex);

    // Compute the correction gain (returns linear multiplier)
    [[nodiscard]] float computeCorrectionGain (float preSampleEnv, int channelIndex) const;
};
