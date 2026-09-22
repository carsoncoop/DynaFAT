//Use followEnv on pre- and post-sample. Then, processedSample *= computeCorrectionGain(...)
#pragma once
#include <array>
#include <vector>
#include <juce_dsp/juce_dsp.h>

class Compressor {
    double sampleRate = 44100.0f;
    unsigned int numChannels = 2;

    float envAttackCoeff = 0.0f;
    float envReleaseCoeff = 0.0f;

    //2d arrays flattened to 1d. To access elements: myArray[(channelNum * 8) + bandNum]
    //Arrays are initialized to the maximum possible size (8 channels, and 3 bands). Usually only 1 or 2 channels are used.
    std::array<float, 8 * 3> envChannelByBand = {};
    std::array<float, 8 * 3> smoothedGainDbChannelByBand = {};

    float ratio = 1;
    float thresh_dB_high = 1;
    float thresh_dB_low = 1;

    bool activated = true;

    //Band splitting (middle band is created later)
    juce::dsp::LinkwitzRileyFilter<float> lowCrossoverWide;
    juce::dsp::LinkwitzRileyFilter<float> highCrossoverWide;
    juce::dsp::LinkwitzRileyFilter<float> lowCrossoverNarrow;
    juce::dsp::LinkwitzRileyFilter<float> highCrossoverNarrow;

    enum BandIndex {
        lowBand = 0,
        midBand = 1,
        highBand = 2
    };

public:
    void prepare(const juce::dsp::ProcessSpec& spec, float envAttackMs_, float envReleaseMs_,
        float ratio_, float thresh_dB_high_, float thresh_dB_low_);

    std::array<float, 8 * 3> getEnvPerChannel() const { return envChannelByBand; }

    void setActivation(const bool status) {activated = status;}
    bool getActivation() const {return activated;}

    //attack/release for envelope measurements
    void setEnvAttack(float envAttackMs_);
    void setEnvRelease(float envReleaseMs_);

    //ratio & thresh
    void setRatio(float ratio_);
    void setThreshHigh(float thresh_dB_high_);
    void setThreshLow(float thresh_dB_low_);

    void followEnv (float inputSample, int channelIndex, int bandIndx);

    // Compute the correction gain (returns linear multiplier)
    //Do math in dB
    [[nodiscard]] float computeGainChange (int channelIndex, int bandIndex);

    float processSample(float inputSample, int channelIndex);
};
