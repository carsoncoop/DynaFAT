#include "EnvelopeFollower.h"
#include <cmath>
#include <algorithm>

void EnvelopeFollower::prepare(const float sampleRate_, const unsigned int numChannels_,
    const float envAttackMs_, const float envReleaseMs_,
    const float gainAttackMs_, const float gainReleaseMs_) {

    sampleRate = sampleRate_;
    numChannels = numChannels_;

    //Creates a vector of envelopes per each channel
    envPerChannel.assign(numChannels, 0.0f);

    envAttackCoeff = 1.0f - std::exp(-1.0f / (envAttackMs_ * 0.001f * sampleRate));
    envReleaseCoeff = 1.0f - std::exp(-1.0f / (envReleaseMs_ * 0.001f * sampleRate));
    envPerChannel = {0.0f, 0.0f};

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

float EnvelopeFollower::followEnv (const float inputSample, const int channelIndex) {
    const float rectified = std::abs(inputSample);
    float env = envPerChannel[channelIndex];
    if (rectified > env) {
        env += (rectified - env) * envAttackCoeff;
    }
    else {
        env += (rectified - env) * envReleaseCoeff;
    }
    envPerChannel[channelIndex] = env;
    return env;
}

float EnvelopeFollower::computeCorrectionGain(const float preSampleEnv, const int channelIndex) {//Post sampleEnv doesn't need to be passed in because the postEnvelopeFollower is where we call this
    // Avoid divide-by-zero and clamp tiny values
    const float eps = 1e-6f;
    const float post = std::max(envPerChannel[channelIndex], eps);
    const float pre = std::max(preSampleEnv, eps);

    // Raw gain (linear) that would map post -> pre
    const float rawGainLinear = pre / post;

    // Work in dB for perceptual smoothing
    const float rawGainDb = 20.0f * std::log10(rawGainLinear);

    // Feed the dB value into the ballistics smoother (channel 0)
    const float smoothedGainDb = gainSmoother.processSample(0, rawGainDb);

    // Clamp the smoothed gain in dB to avoid extreme boosts/cuts
    const float maxGainDb = 24.0f; // +/- 24 dB cap
    const float clippedDb = std::clamp(smoothedGainDb, -maxGainDb, maxGainDb);

    // Convert back to linear gain
    const float smoothedLinear = std::pow(10.0f, clippedDb * (1.0f / 20.0f));

    return smoothedLinear;
    // Use this smoothedLinear to multiply the post-processed sample (not postEnv)
}

