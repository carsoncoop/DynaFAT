#include "EnvelopeFollower.h"
#include <cmath>
#include <algorithm>

void EnvelopeFollower::prepare(const float sampleRate_, const unsigned int numChannels_,
    const float envAttackMs_, const float envReleaseMs_) {

    sampleRate = sampleRate_;
    numChannels = std::max(2u, numChannels_);

    envPerChannel.assign(numChannels, 1e-6f);

    envAttackCoeff = 1.0f - std::exp(-1.0f / (envAttackMs_ * 0.001f * sampleRate));
    envReleaseCoeff = 1.0f - std::exp(-1.0f / (envReleaseMs_ * 0.001f * sampleRate));
}

void EnvelopeFollower::setEnvAttack(const float envAttackMs_) {
    envAttackCoeff = 1.0f - std::exp(-1.0f / (envAttackMs_ * 0.001f * sampleRate));
}

void EnvelopeFollower::setEnvRelease(const float envReleaseMs_) {
    envReleaseCoeff = 1.0f - std::exp(-1.0f / (envReleaseMs_ * 0.001f * sampleRate));
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

float EnvelopeFollower::computeCorrectionGain(const float preSampleEnv, const int channelIndex) const {//Post sampleEnv doesn't need to be passed in because the postEnvelopeFollower is where we call this

    // Avoid divide-by-zero and clamp tiny values
    constexpr float eps = 1e-6f;
    const float post = std::max(envPerChannel[channelIndex], eps);
    const float pre = std::max(preSampleEnv, eps);

    // Raw gain (linear) that would map post -> pre
    const float rawGainLinear = pre / post;

    return rawGainLinear;
    //This implementation seems to work great without smoothing on the gain. Should test to be sure.
    //It would be cool if I could find out how to compute gain correction AFTER this point to emulate the same perceived loudness of the post-processed signal, with the dynamics preserver on.
}

