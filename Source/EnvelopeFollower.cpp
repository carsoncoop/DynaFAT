#include "EnvelopeFollower.h"
#include <cmath>
#include <algorithm>

void EnvelopeFollower::prepare(const float sampleRate_, const unsigned int numChannels_,
    const float envAttackMs_, const float envReleaseMs_,
    const float gainAttackMs_, const float gainReleaseMs_) {

    sampleRate = sampleRate_;
    numChannels = numChannels_;

    //Creates a vector of envelopes per each channel
    envPerChannel.assign(numChannels, 1e-6f);

    envAttackCoeff = 1.0f - std::exp(-1.0f / (envAttackMs_ * 0.001f * sampleRate));
    envReleaseCoeff = 1.0f - std::exp(-1.0f / (envReleaseMs_ * 0.001f * sampleRate));

    gainAttackCoeff = 1.0f - std::exp(-1.0f / (gainAttackMs_ * 0.001f * sampleRate));
    gainReleaseCoeff = 1.0f - std::exp(-1.0f / (gainReleaseMs_ * 0.001f * sampleRate));

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 128;
    spec.numChannels = 2;
    gainSmoother.setAttackTime(gainAttackMs_);
    gainSmoother.setReleaseTime(gainReleaseMs_);
    gainSmoother.prepare(spec);
    gainSmoother.reset();
}

void EnvelopeFollower::setEnvAttack(const float envAttackMs_) {
    envAttackCoeff = 1.0f - std::exp(-1.0f / (envAttackMs_ * 0.001f * sampleRate));
}

void EnvelopeFollower::setEnvRelease(const float envReleaseMs_) {
    envReleaseCoeff = 1.0f - std::exp(-1.0f / (envReleaseMs_ * 0.001f * sampleRate));
}

void EnvelopeFollower::setGainAttack(float gainAttackMs_) {
    gainAttackCoeff = 1.0f - std::exp(-1.0f / (gainAttackMs_ * 0.001f * sampleRate));
    gainSmoother.setAttackTime(gainAttackMs_);
}

void EnvelopeFollower::setGainRelease(float gainReleaseMs_) {
    gainReleaseCoeff = 1.0f - std::exp(-1.0f / (gainReleaseMs_ * 0.001f * sampleRate));
    gainSmoother.setReleaseTime(gainReleaseMs_);
}

void EnvelopeFollower::followEnv (const float inputSample, const int channelIndex) {
    const float rectified = std::abs(inputSample);
    float env = envPerChannel[channelIndex];
    if (rectified > env) {
        env += (rectified - env) * envAttackCoeff;
    }
    else {
        env += (rectified - env) * envReleaseCoeff;
    }
    envPerChannel[channelIndex] = env;
}

float EnvelopeFollower::computeCorrectionGain(const float preSampleEnv, const int channelIndex) {//Post sampleEnv doesn't need to be passed in because the postEnvelopeFollower is where we call this
    // Avoid divide-by-zero and clamp tiny values
    constexpr float eps = 1e-6f;
    const float post = std::max(envPerChannel[channelIndex], eps);
    const float pre = std::max(preSampleEnv, eps);

    // Raw gain (linear) that would map post -> pre
    const float rawGainLinear = pre / post;

    return rawGainLinear;
    //This implementation seems to work great without the buggy smoothing below.
    //It would be cool if I could find out how to compute gain correction AFTER this point to emulate the same perceived loudness of the post-processed signal, with the dynamics preserver on.

    // Work in dB for perceptual smoothing
    const float rawGainDb = juce::Decibels::gainToDecibels(rawGainLinear);

    // Feed the dB value into the ballistics smoother (channel 0)
    const float smoothedGainDb = gainSmoother.processSample(0, rawGainDb);

    // Clamp the smoothed gain in dB to avoid extreme boosts/cuts
    constexpr float maxGainDb = 24.0f; // +/- 24 dB cap
    const float clippedDb = std::clamp(smoothedGainDb, -maxGainDb, maxGainDb);

    // Convert back to linear gain
    const float smoothedLinear = juce::Decibels::decibelsToGain(clippedDb);

    return smoothedLinear;
    // Use this smoothedLinear to multiply the post-processed sample (not postEnv)
}

