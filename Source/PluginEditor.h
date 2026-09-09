#pragma once

#include <juce_audio_utils/gui/juce_AudioVisualiserComponent.h>
#include "PluginProcessor.h"
#include "Distortion.h"
#include "EnvelopeFollower.h"
#include "Compressor.h"



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
    AudioPluginAudioProcessor& processorRef;

    juce::Label distortionHeader{"distortionHeader", "DISTORTION"};
    juce::Label compressionHeader{"compressionHeader", "COMPRESSION"};
    juce::Label envelopeHeader{"envelopeHeader", "DYNAMICS"};

    //Distortion
    juce::Slider satSlider;
    juce::Slider threshSlider;
    juce::Slider outputSlider;
    juce::Slider mixSlider;

    juce::Label satLabel{"Drive Label", "Drive (+dB)"};
    juce::Label threshLabel{"Thresh Label", "Thresh (+dB)"};
    juce::Label outputLabel{"Output Label", "Output (+dB)"};
    juce::Label mixLabel{"Mix Label", "Mix (%)"};
    juce::AudioProcessorValueTreeState::SliderAttachment satSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment threshSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment outputSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment mixSliderAttachment;

    juce::AudioVisualiserComponent visual {1};
    juce::TextButton algButton;

    //Compressor
    juce::ToggleButton compressorButton;
    juce::Slider compThreshSlider, compRatioSlider, compAttackSlider, compReleaseSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment
        compThreshSliderAttachment,
        compRatioSliderAttachment,
        compAttackSliderAttachment,
        compReleaseSliderAttachment;
    juce::Label compThreshLabel{"compThreshLabel", "Thresh (+dB)"};
    juce::Label compRatioLabel{"compRatioLabel", "Ratio"};
    juce::Label compAttackLabel{"compAttackLabel", "Attack"};
    juce::Label compReleaseLabel{"compReleaseLabel", "Release"};

    //Envelope Follower
    juce::ToggleButton envelopeButton;

    juce::Slider envAttackSlider, envReleaseSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment envAttackSliderAttachment, envReleaseSliderAttachment;

    juce::Label envAttackLabel{"envAttackLabel", "Attack"};
    juce::Label envReleaseLabel{"envReleaseLabel", "Release"};

    //Filter
    /*juce::TextButton filterTypeButton;
    juce::ToggleButton preButton;
    juce::ToggleButton postButton;
    juce::ToggleButton offButton;

    juce::AudioProcessorValueTreeState::SliderAttachment cutoffSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment resoSliderAttachment;

    juce::Slider cutoffSlider;
    juce::Slider resoSlider;

    juce::Label cutoffLabel{"Cutoff Label", "Cutoff (Hz)"};
    juce::Label resoLabel{"Resonance Label", "Res/Q"};
    juce::Label preLabel{"Pre Label", "Pre"};
    juce::Label postLabel{"Post Label", "Post"};
    juce::Label offLabel{"Off Label", "Off"};*/
};

