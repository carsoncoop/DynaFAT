#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

enum DistortionType {
    SoftClip,
    HardClip,
    Foldback,
    Downsample
};

class Distortion {
    float sampleRate = 44100.0f;

    juce::SmoothedValue<float> smoothedDrive;
    juce::SmoothedValue<float> smoothedThresh;
    juce::SmoothedValue<float> smoothedOutput;
    juce::SmoothedValue<float> smoothedMix;

    int passes = 1;

    DistortionType distortionAlg = SoftClip;

public:
    Distortion();

    [[nodiscard]] DistortionType getDistortionType() const;
    void setDrive(float drive_);
    void setThresh(float thresh_);
    void setMix(float mix_);
    void setOutput(float output_);
    void setNumPasses(int passes_);
    void setDistortionType(DistortionType type);
    void prepare(double sampleRate, float drive, float thresh, float mix, float output, int numPasses);
    void distProcess(juce::AudioBuffer<float>& buffer);
};


//Previous downsample implementation:
/*
 *const float dryInput = input[sample];
if (counter == 0) {
    hold = input[sample];
}
input[sample] = hold;
counter += 0.5;
if (counter >= static_cast<int>(juce::Decibels::gainToDecibels(distortion.getDrive()))) {
    counter = 0;
}
input[sample] *= distortion.getOutput();
float mix = distortion.getMix();
input[sample] = dryInput * (1.0f - mix / 100) + (mix / 100) * input[sample];
*/