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

    float drive = 0.0f;
    float thresh = 0.0f;
    float mix = 100.0f;
    float output = 0.0f;

    DistortionType distortionAlg = SoftClip;

public:
    DistortionType getDistortionType() const;
    float getDrive() const;
    float getMix() const;
    void setDrive(float drive_);
    void setThresh(float thresh_);
    void setMix(float mix_);
    void setOutput(float output_);
    void setDistortionType(DistortionType type);
    void prepare(float sampleRate_, float drive_, float thresh_, float mix_, float output_);
    float process(float inputSample) const;
};
