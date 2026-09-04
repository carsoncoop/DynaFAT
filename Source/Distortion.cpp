#include "Distortion.h"

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
    switch (distortionAlg) {
        case SoftClip:
            return thresh * std::tanh(inputSample / thresh);
            break;
        case HardClip:
            return juce::jlimit(-thresh,thresh, inputSample);
        case Foldback:
            return std::abs(std::abs(fmod((inputSample - thresh), (4.0f * thresh))) - 2.0f * thresh) - thresh;
        case Downsample:
            break;
            //Downsample case must be handled in the process block because it requires mulitple samples
    }
    return 0.0f;
}
