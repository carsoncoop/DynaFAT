#pragma once

#include <juce_audio_utils/gui/juce_AudioVisualiserComponent.h>
#include "PluginProcessor.h"


//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, juce::Timer
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;


    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    Square firstObj;
    AudioPluginAudioProcessor& processorRef;

    juce::Label distortionHeader{"distortionHeader", "DISTORTION"};
    juce::Label compressionHeader{"compressionHeader", "COMPRESSION"};

    //Distortion
    juce::Slider satSlider;
    juce::Slider threshSlider;
    juce::Slider outputSlider;
    juce::Slider mixSlider;
    juce::Slider cutoffSlider;
    juce::Slider resoSlider;
    juce::Label satLabel{"Drive Label", "Drive (+dB)"};
    juce::Label threshLabel{"Thresh Label", "Thresh (+dB)"};
    juce::Label outputLabel{"Output Label", "Output (+dB)"};
    juce::Label mixLabel{"Mix Label", "Mix (%)"};
    juce::Label cutoffLabel{"Cutoff Label", "Cutoff (Hz)"};
    juce::Label resoLabel{"Resonance Label", "Res/Q"};
    juce::Label preLabel{"Pre Label", "Pre"};
    juce::Label postLabel{"Post Label", "Post"};
    juce::Label offLabel{"Off Label", "Off"};
    juce::AudioProcessorValueTreeState::SliderAttachment satSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment threshSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment outputSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment mixSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment cutoffSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment resoSliderAttachment;
    juce::AudioVisualiserComponent visual {1};
    juce::TextButton algButton;
    juce::TextButton filterButton;
    juce::ToggleButton preButton;
    juce::ToggleButton postButton;
    juce::ToggleButton offButton;


    //Compression
    juce::Component ottPanel;
    juce::Label ottTitle{"ottTitle", "OTT"};
    juce::ToggleButton compressorButton;
    juce::Slider lowLowerThreshSlider, lowUpperThreshSlider, midLowerThreshSlider, midUpperThreshSlider, highLowerThreshSlider, highUpperThreshSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment lowLowerThreshSliderAttachment, lowUpperThreshSliderAttachment, midLowerThreshSliderAttachment, midUpperThreshSliderAttachment, highLowerThreshSliderAttachment, highUpperThreshSliderAttachment;
    juce::Slider lowLowerRatioSlider, lowUpperRatioSlider, midLowerRatioSlider, midUpperRatioSlider, highLowerRatioSlider, highUpperRatioSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment lowLowerRatioSliderAttachment, lowUpperRatioSliderAttachment, midLowerRatioSliderAttachment, midUpperRatioSliderAttachment, highLowerRatioSliderAttachment, highUpperRatioSliderAttachment;
    juce::Slider lowInputGainSlider, lowOutputGainSlider, midInputGainSlider, midOutputGainSlider, highInputGainSlider, highOutputGainSlider, compMasterGainSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment lowInputGainSliderAttachment, lowOutputGainSliderAttachment, midInputGainSliderAttachment, midOutputGainSliderAttachment, highInputGainSliderAttachment, highOutputGainSliderAttachment, compMasterGainSliderAttachment;
    juce::Slider attackSlider, releaseSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment attackSliderAttachment, releaseSliderAttachment;

    juce::Label lowLowerThreshLabel{"lowLowerThreshLabel", "Low Low T"};
    juce::Label lowUpperThreshLabel{"lowUpperThreshLabel", "Low High T"};
    juce::Label midLowerThreshLabel{"midLowerThreshLabel", "Mid Low T"};
    juce::Label midUpperThreshLabel{"midUpperThreshLabel", "Mid High T"};
    juce::Label highLowerThreshLabel{"highLowerThreshLabel", "High Low T"};
    juce::Label highUpperThreshLabel{"highUpperThreshLabel", "High High T"};
    juce::Label lowLowerRatioLabel{"lowLowerRatioLabel", "Low Low R"};
    juce::Label lowUpperRatioLabel{"lowUpperRatioLabel", "Low High R"};
    juce::Label midLowerRatioLabel{"midLowerRatioLabel", "Mid Low R"};
    juce::Label midUpperRatioLabel{"midUpperRatioLabel", "Mid High R"};
    juce::Label highLowerRatioLabel{"highLowerRatioLabel", "High Low R"};
    juce::Label highUpperRatioLabel{"highUpperRatioLabel", "High High R"};
    juce::Label lowInputGainLabel{"lowInputGainLabel", "Low Input"};
    juce::Label lowOutputGainLabel{"lowOutputGainLabel", "Low Output"};
    juce::Label midInputGainLabel{"midInputGainLabel", "Mid Input"};
    juce::Label midOutputGainLabel{"midOutputGainLabel", "Mid Output"};
    juce::Label highInputGainLabel{"highInputGainLabel", "High Input"};
    juce::Label highOutputGainLabel{"highOutputGainLabel", "High Output"};
    juce::Label compMasterGainLabel{"compMasterGainLabel", "Master"};
    juce::Label attackLabel{"attackLabel", "Attack"};
    juce::Label releaseLabel{"releaseLabel", "Release"};
};

