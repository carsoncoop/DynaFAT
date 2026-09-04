#include "Distortion.h"

DistortionType Distortion::getDistortionType() const {
    return distortionAlg;
}

float Distortion::getMix() const {
    return mix;
}

void Distortion::setDrive(const float drive_) {
    drive = drive_;
}

float Distortion::getDrive() const {
    return drive;
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

void Distortion::setDistortionType(DistortionType type) {
    distortionAlg = type;
}

void Distortion::prepare(const float sampleRate_, const float drive_, const float thresh_, const float mix_, const float output_) {
    sampleRate = sampleRate_;
    drive = drive_;
    thresh = thresh_;
    mix = mix_;
    output = output_;
    distortionAlg = SoftClip;
}

float Distortion::process(const float inputSample) const {
    float signal = 0.0f;
    switch (distortionAlg) {
        case SoftClip:
            signal = thresh * std::tanh((inputSample * drive) / thresh);
            signal = inputSample * (1.0f - mix / 100) + (mix / 100) * signal;
            signal *= output;
            return signal;
        case HardClip:
            signal = juce::jlimit(-thresh,thresh, (inputSample * drive));
            signal = inputSample * (1.0f - mix / 100) + (mix / 100) * signal;
            signal *= output;
            return signal;
        case Foldback:
            signal = std::abs(std::abs(fmod(((inputSample * drive) - thresh), (4.0f * thresh))) - 2.0f * thresh) - thresh;
            signal = inputSample * (1.0f - mix / 100) + (mix / 100) * signal;
            signal *= output;
            return signal;
        case Downsample:
            break;
            //Downsample case must be handled in the process block because it requires multiple samples
    }
    return 0.0f;
}
