#include "Compressor.h"
#include <cmath>
#include <algorithm>

void Compressor::prepare(const float sampleRate_, const unsigned int numChannels_,
    const float envAttackMs_, const float envReleaseMs_,
    const float ratio_, const float thresh_dB_high_, const float thresh_dB_low_) {

    sampleRate = sampleRate_;
    numChannels = std::max(2u, numChannels_);

    envPerChannel.assign(numChannels, 1e-6f);

    envAttackCoeff = 1.0f - std::exp(-1.0f / (envAttackMs_ * 0.001f * sampleRate));
    envReleaseCoeff = 1.0f - std::exp(-1.0f / (envReleaseMs_ * 0.001f * sampleRate));

    ratio = ratio_;
    thresh_dB_high = thresh_dB_high_;
    thresh_dB_low = thresh_dB_low_;
}

void Compressor::setEnvAttack(const float envAttackMs_) {
    envAttackCoeff = 1.0f - std::exp(-1.0f / (envAttackMs_ * 0.001f * sampleRate));
}

void Compressor::setEnvRelease(const float envReleaseMs_) {
    envReleaseCoeff = 1.0f - std::exp(-1.0f / (envReleaseMs_ * 0.001f * sampleRate));
}

void Compressor::setRatio(const float ratio_) {
    ratio = ratio_;
}

void Compressor::setThreshHigh(const float thresh_dB_high_) {
    thresh_dB_high = thresh_dB_high_;
}

void Compressor::setThreshLow(const float thresh_dB_low_) {
    thresh_dB_low = thresh_dB_low_;
}

void Compressor::followEnv (const float inputSample, const int channelIndex) {

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

float Compressor::computeGainChange(const int channelIndex) const {

    constexpr float eps = 1e-6f;
    const float envLinear = std::max(envPerChannel[channelIndex], eps);
    const float env_dB = juce::Decibels::gainToDecibels(envLinear);

    //knee == 0.0f is a hard knee. Trying soft knee
    //const float knee = 2.0f;
    //const float halfKnee = knee / 2.0f;

    float gainChange_dB = 0.0f;

    //Downward Compression
    if (env_dB > thresh_dB_high) {
        const float overshoot_dB = env_dB - thresh_dB_high;
        gainChange_dB = -(overshoot_dB * (1.0f - 1.0f / ratio));
    }

    //Upward Compression
    else if (env_dB < thresh_dB_low) {
        const float undershoot_dB = thresh_dB_low - env_dB;
        gainChange_dB = (undershoot_dB * (1.0f - 1.0f / ratio));
    }

    //Return linear gain
    return juce::Decibels::decibelsToGain(gainChange_dB);
}

