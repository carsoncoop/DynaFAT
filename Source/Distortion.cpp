#include "Distortion.h"

Distortion::Distortion(){}


DistortionType Distortion::getDistortionType() const {
    return distortionAlg;
}

void Distortion::setDrive(const float drive_) {
    smoothedDrive.setTargetValue(drive_);
}

void Distortion::setThresh(const float thresh_) {
    smoothedThresh.setTargetValue(thresh_);
}

void Distortion::setMix(const float mix_) {
    smoothedMix.setTargetValue(mix_);
}

void Distortion::setOutput(const float output_) {
    smoothedOutput.setTargetValue(output_);
}

void Distortion::setNumPasses(int passes_) {
    passes = passes_;
}

void Distortion::setDistortionType(DistortionType type) {
    distortionAlg = type;
}

void Distortion::prepare (double sampleRate, const float drive, const float thresh, const float mix, const float output, const int numPasses)
{
    constexpr double rampSeconds = 0.02;

    smoothedDrive.reset(sampleRate, rampSeconds);
    smoothedThresh.reset(sampleRate, rampSeconds);
    smoothedOutput.reset(sampleRate, rampSeconds);
    smoothedMix.reset(sampleRate, rampSeconds);

    // Jump straight to the starting values, no ramp
    smoothedDrive.setCurrentAndTargetValue(drive);
    smoothedThresh.setCurrentAndTargetValue(thresh);
    smoothedOutput.setCurrentAndTargetValue(output);
    smoothedMix.setCurrentAndTargetValue(mix);

    passes = numPasses;
}

void Distortion::distProcess (juce::AudioBuffer<float>& buffer){

        for (int s = 0; s < buffer.getNumSamples(); s++){

            const float drive = smoothedDrive.getNextValue();
            const float thresh = smoothedThresh.getNextValue();
            const float mix = smoothedMix.getNextValue();
            const float output = smoothedOutput.getNextValue();

            for (int c = 0; c < buffer.getNumChannels(); c++) {
                for (int p = 0; p < passes; p++) {
                float signal = buffer.getSample(c, s);
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
                        //TODO
                        break;
                }
                signal *= output;
                signal = buffer.getSample(c, s) * (1.0f - mix / 100.0f) + mix / 100.0f * signal;
                buffer.setSample(c, s, signal);
            }
        }
    }
}
