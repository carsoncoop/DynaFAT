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
    std::vector<float> envPerChannel;
    // per-channel smoothed gain in dB (0.0f = no change)
    std::vector<float> smoothedGainDbPerChannel;

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

    std::array<juce::AudioBuffer<float>, 3> bandBuffers;

public:
    void prepare(const juce::dsp::ProcessSpec& spec, float envAttackMs_, float envReleaseMs_,
        float ratio_, float thresh_dB_high_, float thresh_dB_low_);

    std::vector<float>& getEnvPerChannel() { return envPerChannel; }

    void setActivation(const bool status) {activated = status;}
    bool getActivation() const {return activated;}

    //attack/release for envelope measurements
    void setEnvAttack(float envAttackMs_);
    void setEnvRelease(float envReleaseMs_);

    //ratio & thresh
    void setRatio(float ratio_);
    void setThreshHigh(float thresh_dB_high_);
    void setThreshLow(float thresh_dB_low_);

    void followEnv (float inputSample, int channelIndex);

    // Compute the correction gain (returns linear multiplier)
    //Do math in dB
    [[nodiscard]] float computeGainChange (int channelIndex);

    //This is called each block, as whole buffers are separated at a time
    void separateBufferBands(const juce::AudioBuffer<float>& buffer);

    //Called at the end of process block, to combine 3 bands into buffer
    void sendBandsToBuffer(juce::AudioBuffer<float>& buffer);
};
