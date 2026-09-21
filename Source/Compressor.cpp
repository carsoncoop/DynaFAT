#include "Compressor.h"
#include <cmath>
#include <algorithm>
#include "juce_audio_formats/format/juce_BufferingAudioFormatReader.h"

void Compressor::prepare(const juce::dsp::ProcessSpec& spec,
                         const float envAttackMs_, const float envReleaseMs_,
                         const float ratio_, const float thresh_dB_high_, const float thresh_dB_low_) {

    sampleRate = spec.sampleRate;
    numChannels = spec.numChannels;

    envPerChannel.assign(numChannels, 1e-6f);
    smoothedGainDbPerChannel.assign(numChannels, 0.0f);

    envAttackCoeff = 1.0f - std::exp(-1.0f / (envAttackMs_ * 0.001f * sampleRate));
    envReleaseCoeff = 1.0f - std::exp(-1.0f / (envReleaseMs_ * 0.001f * sampleRate));

    ratio = ratio_;
    thresh_dB_high = thresh_dB_high_;
    thresh_dB_low = thresh_dB_low_;

    //Multiband Preparation
    lowCrossoverWide.prepare(spec);
    highCrossoverWide.prepare(spec);
    lowCrossoverNarrow.prepare(spec);
    highCrossoverNarrow.prepare(spec);
    lowCrossoverWide.setType(juce::dsp::LinkwitzRileyFilter<float>::Type::lowpass);
    highCrossoverWide.setType(juce::dsp::LinkwitzRileyFilter<float>::Type::highpass);
    lowCrossoverNarrow.setType(juce::dsp::LinkwitzRileyFilter<float>::Type::lowpass);
    highCrossoverNarrow.setType(juce::dsp::LinkwitzRileyFilter<float>::Type::highpass);
    //Can make frequency ranges adjustable later
    lowCrossoverWide.setCutoffFrequency(120.0f);
    highCrossoverWide.setCutoffFrequency(120.0f);
    lowCrossoverNarrow.setCutoffFrequency(2500.0f);
    highCrossoverNarrow.setCutoffFrequency(2500.0f);

    //Will have to do some stuff to isolate the middle band
    crossoverVec = {lowCrossoverWide, lowCrossoverNarrow, highCrossoverNarrow};
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

    for (auto& band: crossoverVec) {
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

}

float Compressor::computeGainChange(const int channelIndex) {

    constexpr float eps = 1e-6f;
    const float envLinear = std::max(envPerChannel[channelIndex], eps);
    const float env_dB = juce::Decibels::gainToDecibels(envLinear);

    float desiredGain_dB = 0.0f;

    //Downward Compression
    if (env_dB > thresh_dB_high) {
        const float overshoot_dB = env_dB - thresh_dB_high;
        desiredGain_dB = -(overshoot_dB * (1.0f - 1.0f / ratio));
    }

    //Upward Compression
    else if (env_dB < thresh_dB_low) {
        const float undershoot_dB = thresh_dB_low - env_dB;
        desiredGain_dB = (undershoot_dB * (1.0f - 1.0f / ratio));
    }

    return juce::Decibels::decibelsToGain(desiredGain_dB);

    // Smooth the gain in dB. Not sure if it is necessary for now.
    float& currentGainDb = smoothedGainDbPerChannel[channelIndex];
    const float coeff = (desiredGain_dB < currentGainDb) ? envAttackCoeff : envReleaseCoeff;
    currentGainDb += coeff * (desiredGain_dB - currentGainDb);

    //Return linear gain (to multiply input sample)
    return juce::Decibels::decibelsToGain(currentGainDb);
}

