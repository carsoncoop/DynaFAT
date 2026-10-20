#include "Distortion.h"

Distortion::Distortion(){}

Distortion::Distortion(float sampleRate_, float drive_, float thresh_, float mix_, float output_, int passes_) {
    sampleRate = sampleRate_;
    drive = drive_;
    thresh = thresh_;
    mix = mix_;
    output = output_;
    passes = passes_;
}


DistortionType Distortion::getDistortionType() const {
    return distortionAlg;
}

float Distortion::getMix() const {
    return mix;
}

float Distortion::getOutput() const {
    return output;
}

float Distortion::getDrive() const {
    return drive;
}

int Distortion::getNumPasses() const {
    return passes;
}

void Distortion::setDrive(const float drive_) {
    drive = drive_;
}

void Distortion::setThresh(const float thresh_) {
    thresh = thresh_;
}

void Distortion::setMix(const float mix_) {
    mix = mix_;
}

void Distortion::setOutput(const float output_) {
    output = output_;
}

void Distortion::setNumPasses(int passes_) {
    passes = passes_;
}

void Distortion::setDistortionType(DistortionType type) {
    distortionAlg = type;
}

void Distortion::prepare(const double sampleRate_, const float drive_, const float thresh_, const float mix_, const float output_, const int passes_) {
    sampleRate = sampleRate_;
    drive = drive_;
    thresh = thresh_;
    mix = mix_;
    output = output_;
    passes = passes_;
    distortionAlg = SoftClip;
}

float Distortion::process (const float inputSample) const
{
    float signal = inputSample;

    for (int i = 0; i < passes; i++){
        switch (distortionAlg)
        {
            case SoftClip:
                signal = thresh * std::tanh ((signal * drive) / thresh);
                break;

            case HardClip:
                signal = juce::jlimit (-thresh, thresh, signal * drive);
                break;

            case Foldback:
                signal = std::abs (std::abs (std::fmod ((signal * drive) - thresh, 4.0f * thresh)) - 2.0f * thresh) - thresh;
                break;

            case Downsample:
                break; // handled in processBlock
        }
    }

    signal *= output;
    const float wet = mix / 100.0f;
    return inputSample * (1.0f - wet) + wet * signal;
}
