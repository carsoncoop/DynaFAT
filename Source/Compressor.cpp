#include "Compressor.h"
#include <cmath>
#include <algorithm>

void Compressor::prepare(const float sampleRate_, const unsigned int numChannels_,
    const float envAttackMs_, const float envReleaseMs_,
    const float ratio_, const float thresh_dB_) {

    sampleRate = sampleRate_;
    numChannels = numChannels_;

    //Creates a vector of envelopes per each channel
    envPerChannel.assign(numChannels, 1e-6f);

    envAttackCoeff = 1.0f - std::exp(-1.0f / (envAttackMs_ * 0.001f * sampleRate));
    envReleaseCoeff = 1.0f - std::exp(-1.0f / (envReleaseMs_ * 0.001f * sampleRate));

    ratio = ratio_;
    thresh_dB = thresh_dB_;
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

void Compressor::setThresh(const float thresh_dB_) {
    thresh_dB = thresh_dB_;
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
    const float env_dB = juce::Decibels::gainToDecibels(envPerChannel[channelIndex]);

    //*Compressor gain math (in dB)*


    //return linear gain
    return 0.0f;
}

