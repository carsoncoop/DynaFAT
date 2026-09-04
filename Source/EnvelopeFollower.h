//Use followEnv on pre- and post-sample. Then, processedSample *= computeCorrectionGain(...)
#pragma once
#include <juce_dsp/juce_dsp.h>

class EnvelopeFollower {
    float sampleRate = 44100.0f;

    float envAttackCoeff = 0.0f;
    float envReleaseCoeff = 0.0f;
    float env = 0.0f;

    float gainAttackCoeff = 0.0f;
    float gainReleaseCoeff = 0.0f;

    // Ballistics filter to smooth the computed correction gain (operates on dB values)
    juce::dsp::BallisticsFilter<float> gainSmoother;

public:
    void prepare(float sampleRate_, float envAttackMs_, float envReleaseMs_, float gainAttackMs_, float gainReleaseMs_);

    //attack/release for envelope measurements
    void setEnvAttack(float envAttackMs_);
    void setEnvRelease(float envReleaseMs_);

    //attack/release for gain smoothing
    void setGainAttack(float gainAttackMs_);
    void setGainRelease(float gainReleaseMs_);

    float followEnv (float inputSample);

    // Compute the correction gain (returns linear multiplier)
    float computeCorrectionGain (float preSampleEnv, float postSampleEnv);
};
