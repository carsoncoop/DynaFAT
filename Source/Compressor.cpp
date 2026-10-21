#include "Compressor.h"
#include <algorithm>

void Compressor::prepare(const juce::dsp::ProcessSpec& spec,
                         const float envAttackMs, const float envReleaseMs,
                         const float thresh_dB_high, const float thresh_dB_low,
                         const int passes_, const float ratio) {

    constexpr double rampSeconds = 0.02;

    smoothedCompThreshHigh.reset(sampleRate, rampSeconds);
    smoothedCompThreshLow.reset(sampleRate, rampSeconds);
    smoothedCompRatio.reset(sampleRate, rampSeconds);
    smoothedCompAttack.reset(sampleRate, rampSeconds);
    smoothedCompRelease.reset(sampleRate, rampSeconds);

    smoothedCompThreshHigh.setCurrentAndTargetValue(thresh_dB_high);
    smoothedCompThreshLow.setCurrentAndTargetValue(thresh_dB_low);
    smoothedCompRatio.setCurrentAndTargetValue(ratio);
    smoothedCompAttack.setCurrentAndTargetValue(envAttackMs);
    smoothedCompRelease.setCurrentAndTargetValue(envReleaseMs);

    passes = passes_;

    sampleRate = spec.sampleRate;
    numChannels = spec.numChannels;

    //Linkwitz-Riley
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

    bandBuffers[0].setSize(numChannels, spec.maximumBlockSize);
    bandBuffers[1].setSize(numChannels, spec.maximumBlockSize);
    bandBuffers[2].setSize(numChannels, spec.maximumBlockSize);
}

void Compressor::setEnvAttack(const float envAttackMs) {
    smoothedCompAttack.setTargetValue(envAttackMs);
}

void Compressor::setEnvRelease(const float envReleaseMs) {
    smoothedCompRelease.setTargetValue(envReleaseMs);
}

void Compressor::setRatio(const float ratio) {
    smoothedCompRatio.setTargetValue(ratio);
}

void Compressor::setThreshHigh(const float thresh_dB_high) {
    smoothedCompThreshHigh.setTargetValue(thresh_dB_high);
}

void Compressor::setThreshLow(const float thresh_dB_low) {
    smoothedCompThreshLow.setTargetValue(thresh_dB_low);
}

void Compressor::setNumPasses(const int passes_) {
    passes = passes_;
}

void Compressor::followEnv (const float inputSample, const int channelIndex, const int bandIndex) {
    float envAttackMs = smoothedCompAttack.getNextValue();
    float envReleaseMs = smoothedCompRelease.getNextValue();
    float envAttackCoeff = 1.0f - std::exp(-1.0f / (envAttackMs * 0.001f * sampleRate));
    float envReleaseCoeff = 1.0f - std::exp(-1.0f / (envReleaseMs * 0.001f * sampleRate));

    const float rectified = std::abs(inputSample);
    float env = envChannelByBand[channelIndex * 8 + bandIndex];
    if (rectified > env) {
        env += (rectified - env) * envAttackCoeff;
    }
    else {
        env += (rectified - env) * envReleaseCoeff;
    }
    envChannelByBand[channelIndex * 8 + bandIndex] = env;
}

float Compressor::computeGainChange(const int channelIndex, const int bandIndex) {
    const float thresh_dB_high = smoothedCompThreshHigh.getNextValue();
    const float thresh_dB_low = smoothedCompThreshLow.getNextValue();
    const float ratio = smoothedCompRatio.getNextValue();

    constexpr float eps = 1e-6f;
    const float envLinear = std::max(envChannelByBand[channelIndex * 8 + bandIndex], eps);
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

    /*
    // Smooth the gain in dB before converting back to linear.
    float& currentGainDb = smoothedGainDbChannelByBand[channelIndex];
    const float coeff = (desiredGain_dB < currentGainDb) ? envAttackCoeff : envReleaseCoeff;
    currentGainDb += coeff * (desiredGain_dB - currentGainDb);

    //Return linear gain (to multiply input sample)
    return juce::Decibels::decibelsToGain(currentGainDb);
    */

}

float Compressor::compProcessSample(float inputSample, int channelIndex) {

    float signal = inputSample;

    for (int i = 0; i < passes; i++) {

        //Split into 3 bands per sample per channel
        float lowSample = lowCrossoverWide.processSample(channelIndex, signal);
        float upperRemainder = highCrossoverWide.processSample(channelIndex, signal);

        float midSample = lowCrossoverNarrow.processSample(channelIndex, upperRemainder);
        float highSample = highCrossoverNarrow.processSample(channelIndex, upperRemainder);

        //Process each band compression per sample per channel

        followEnv(lowSample, channelIndex, lowBand);
        const float lowGain = computeGainChange(channelIndex, lowBand);
        const float processedLow = lowSample * lowGain;

        followEnv(midSample, channelIndex, midBand);
        const float midGain = computeGainChange(channelIndex, midBand);
        const float processedMid = midSample * midGain;

        followEnv(highSample, channelIndex, highBand);
        const float highGain = computeGainChange(channelIndex, highBand);
        const float processedHigh = highSample * highGain;

        signal = processedLow + processedMid + processedHigh;
    }
    return signal;

}