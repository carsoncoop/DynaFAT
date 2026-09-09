#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "EnvelopeFollower.h"
#include "Distortion.h"
#include "Compressor.h"
#include "Filter.h"
#include <vector>


class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getState() {return state;}

    juce::AudioBuffer<float> visualizerBuffer;
    //Distortion
    juce::SmoothedValue<float> smoothedDrive;
    juce::SmoothedValue<float> smoothedThresh;
    juce::SmoothedValue<float> smoothedOutput;
    juce::SmoothedValue<float> smoothedMix;

    //Filter
    juce::SmoothedValue<float> smoothedCutoff;
    juce::SmoothedValue<float> smoothedReso;

    //Envelope Follower
    juce::SmoothedValue<float> smoothedEnvAttack;
    juce::SmoothedValue<float> smoothedEnvRelease;

    //Compressor controls (loaded for use, not yet connected to DSP)
    juce::SmoothedValue<float> smoothedCompThresh;
    juce::SmoothedValue<float> smoothedCompRatio;
    juce::SmoothedValue<float> smoothedCompAttack;
    juce::SmoothedValue<float> smoothedCompRelease;

    Distortion& getDistortion() {return distortion;}
    EnvelopeFollower& getPreEnvelopeFollower() {return preEnvelopeFollower;}
    EnvelopeFollower& getPostEnvelopeFollower() {return postEnvelopeFollower;}
    Compressor& getCompressor() {return compressor;}
    Filter& getFilter() {return filter;}

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)

    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    juce::AudioProcessorValueTreeState state;

    EnvelopeFollower preEnvelopeFollower, postEnvelopeFollower;
    Distortion distortion;
    Compressor compressor;
    Filter filter;

    //Band splitting (mids are created later)
    juce::dsp::LinkwitzRileyFilter<float> lowCrossoverWide;
    juce::dsp::LinkwitzRileyFilter<float> highCrossoverWide;
    juce::dsp::LinkwitzRileyFilter<float> lowCrossoverNarrow;
    juce::dsp::LinkwitzRileyFilter<float> highCrossoverNarrow;

};
