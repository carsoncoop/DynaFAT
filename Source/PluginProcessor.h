#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

enum DistortionType {
    SoftClip,
    HardClip,
    Foldback,
    Downsample
};

enum FilterOrder {
    Pre,
    Post,
    Off
};

enum FilterType {
    Lowpass,
    Highpass,
    Bandpass,
};

enum CompressorToggle {
    CompOn,
    CompOff
};
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    DistortionType distortionAlg = SoftClip;
    FilterType filterType = Lowpass;
    FilterOrder filterOrder = Off;
    CompressorToggle compressorToggle = CompOn;
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

    juce::SmoothedValue<float> smoothedDrive;
    juce::SmoothedValue<float> smoothedThresh;
    juce::SmoothedValue<float> smoothedOutput;
    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedCutoff;
    juce::SmoothedValue<float> smoothedReso;

    juce::SmoothedValue<float> smoothedAttack;
    juce::SmoothedValue<float> smoothedRelease;
    juce::SmoothedValue<float> smoothedCompMasterGain;
    
    // Per-band compressor smoothing values
    juce::SmoothedValue<float> smoothedLowInputGain;
    juce::SmoothedValue<float> smoothedLowOutputGain;
    juce::SmoothedValue<float> smoothedLowLowerThresh;
    juce::SmoothedValue<float> smoothedLowUpperThresh;
    juce::SmoothedValue<float> smoothedLowLowerRatio;
    juce::SmoothedValue<float> smoothedLowUpperRatio;
    juce::SmoothedValue<float> smoothedMidInputGain;
    juce::SmoothedValue<float> smoothedMidOutputGain;
    juce::SmoothedValue<float> smoothedMidLowerThresh;
    juce::SmoothedValue<float> smoothedMidUpperThresh;
    juce::SmoothedValue<float> smoothedMidLowerRatio;
    juce::SmoothedValue<float> smoothedMidUpperRatio;
    juce::SmoothedValue<float> smoothedHighInputGain;
    juce::SmoothedValue<float> smoothedHighOutputGain;
    juce::SmoothedValue<float> smoothedHighLowerThresh;
    juce::SmoothedValue<float> smoothedHighUpperThresh;
    juce::SmoothedValue<float> smoothedHighLowerRatio;
    juce::SmoothedValue<float> smoothedHighUpperRatio;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)

    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    juce::AudioProcessorValueTreeState state;

    juce::dsp::StateVariableTPTFilter<float> filter;

    //Band splitting (mids are created later)
    juce::dsp::LinkwitzRileyFilter<float> lowCrossoverWide;
    juce::dsp::LinkwitzRileyFilter<float> highCrossoverWide;
    juce::dsp::LinkwitzRileyFilter<float> lowCrossoverNarrow;
    juce::dsp::LinkwitzRileyFilter<float> highCrossoverNarrow;


    //float computeCompressionGain(float x_dB, float threshDown, float ratioDown, float threshUp, float ratioUp);
};
