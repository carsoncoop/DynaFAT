#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
void AudioPluginAudioProcessorEditor::timerCallback()
{
    visual.pushBuffer(processorRef.visualizerBuffer);
}
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
    satSliderAttachment(processorRef.getState(), "drive", satSlider),
    threshSliderAttachment(processorRef.getState(), "thresh", threshSlider),
    outputSliderAttachment(processorRef.getState(), "output", outputSlider),
    mixSliderAttachment(processorRef.getState(), "mix", mixSlider),
    cutoffSliderAttachment(processorRef.getState(), "cutoff", cutoffSlider),
    resoSliderAttachment(processorRef.getState(), "resonance", resoSlider),
    compThreshSliderAttachment(processorRef.getState(), "compThresh", compThreshSlider),
    compRatioSliderAttachment(processorRef.getState(), "compRatio", compRatioSlider),
    compAttackSliderAttachment(processorRef.getState(), "compAttack", compAttackSlider),
    compReleaseSliderAttachment(processorRef.getState(), "compRelease", compReleaseSlider),
    envAttackSliderAttachment(processorRef.getState(), "envAttack", envAttackSlider),
    envReleaseSliderAttachment(processorRef.getState(), "envRelease", envReleaseSlider)
{
    //Distortion Parameters---------------------------------------------------------------------------------------------
    filterButton.setButtonText("Lowpass");
    filterButton.onClick = [this]() {
        juce::PopupMenu filterMenu;
        filterMenu.addItem(1, "Lowpass");
        filterMenu.addItem(2, "Highpass");
        filterMenu.addItem(3, "Bandpass");
        filterMenu.addItem(4, "OFF");

        filterMenu.showMenuAsync(juce::PopupMenu::Options(),
            [this] (int result) {
                if (result == 0) {//If no selection is made
                }
                else if (result == 1) {
                    filterButton.setButtonText("Lowpass");
                    processorRef.filterType = Lowpass;
                }
                else if (result == 2) {
                    filterButton.setButtonText("Highpass");
                    processorRef.filterType = Highpass;
                }
                else if (result == 3) {
                    filterButton.setButtonText("Bandpass");
                    processorRef.filterType = Bandpass;
                }
        });
    };

    algButton.setButtonText("Soft Clip");
    algButton.onClick = [this]() {
        juce::PopupMenu algMenu;
        algMenu.addItem(1, "Soft Clip");
        algMenu.addItem(2, "Hard Clip");
        algMenu.addItem(3, "Foldback");
        algMenu.addItem(4, "Downsample");

        algMenu.showMenuAsync(juce::PopupMenu::Options(),
            [this] (int result) {
                if (result == 0) {//If no selection is made

                }
                else if (result == 1) {
                    algButton.setButtonText("Soft Clip");
                    processorRef.getDistortion().setDistortionType(SoftClip);
                    threshLabel.setText("Thresh (+dB)", juce::dontSendNotification);
                    satLabel.setText("Drive (+dB)", juce::dontSendNotification);
                }
                else if (result == 2) {
                    algButton.setButtonText("Hard Clip");
                    processorRef.getDistortion().setDistortionType(HardClip);
                    threshLabel.setText("Thresh (+dB)", juce::dontSendNotification);
                    satLabel.setText("Drive (+dB)", juce::dontSendNotification);
                }
                else if (result == 3) {
                    algButton.setButtonText("Foldback");
                    processorRef.getDistortion().setDistortionType(Foldback);
                    threshLabel.setText("Thresh (+dB)", juce::dontSendNotification);
                    satLabel.setText("Drive (+dB)", juce::dontSendNotification);
                }
                else if (result == 4) {
                    algButton.setButtonText("Downsample");
                    processorRef.getDistortion().setDistortionType(Downsample);
                    threshLabel.setText("(NA)", juce::dontSendNotification);
                    satLabel.setText("Crush", juce::dontSendNotification);
                }
        });
    };

    //Handles filterOrder buttons' exclusivity
    offButton.setToggleState(true, juce::dontSendNotification);
    preButton.setRadioGroupId(1);
    postButton.setRadioGroupId(1);
    offButton.setRadioGroupId(1);

    preButton.onClick = [this] {
        if (preButton.getToggleState()) {
            processorRef.filterOrder = Pre;
        }
    };

    postButton.onClick = [this] {
        if (postButton.getToggleState()) {
            processorRef.filterOrder = Post;
        }
    };
    offButton.onClick = [this] {
        if (offButton.getToggleState()) {
            processorRef.filterOrder = Off;
        }
    };
    distortionHeader.setJustificationType(juce::Justification::centred);
    compressionHeader.setJustificationType(juce::Justification::centred);
    envelopeHeader.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(distortionHeader);
    addAndMakeVisible(compressionHeader);
    addAndMakeVisible(envelopeHeader);

    addAndMakeVisible(algButton);
    addAndMakeVisible(filterButton);
    addAndMakeVisible(preButton);
    addAndMakeVisible(postButton);
    addAndMakeVisible(offButton);

    visual.setBufferSize(64);
    visual.setSamplesPerBlock(64);
    visual.setRepaintRate(60);
    visual.setColours(juce::Colours::black, juce::Colours::blueviolet);
    startTimerHz(60);
    addAndMakeVisible(visual);


    preLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(preLabel);
    postLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(postLabel);
    offLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(offLabel);

    cutoffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cutoffLabel);
    cutoffSlider.setPopupDisplayEnabled(true, false, this);
    cutoffSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    cutoffSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(cutoffSlider);

    resoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(resoLabel);
    resoSlider.setPopupDisplayEnabled(true, false, this);
    resoSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    resoSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(resoSlider);

    satLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(satLabel);
    satSlider.setPopupDisplayEnabled(true, false, this);
    satSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    satSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(satSlider);

    threshLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(threshLabel);
    threshSlider.setPopupDisplayEnabled(true, false, this);
    threshSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    threshSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(threshSlider);

    outputLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outputLabel);
    outputSlider.setPopupDisplayEnabled(true, false, this);
    outputSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    outputSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(outputSlider);

    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);
    mixSlider.setPopupDisplayEnabled(true, false, this);
    mixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mixSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(mixSlider);

    //Compressor Parameters---------------------------------------------------------------------------------------------
    addAndMakeVisible(compressorButton);
    compressorButton.setToggleState(true, juce::dontSendNotification);
    compressorButton.onClick = [this] {
        if (compressorButton.getToggleState()) {
            processorRef.getPreEnvelopeFollower().setActivation(true);
        }
        else {
            processorRef.getPreEnvelopeFollower().setActivation(false);
        }
    };

    compThreshLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(compThreshLabel);
    compThreshSlider.setPopupDisplayEnabled(true, false, this);
    compThreshSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    compThreshSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(compThreshSlider);

    compRatioLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(compRatioLabel);
    compRatioSlider.setPopupDisplayEnabled(true, false, this);
    compRatioSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    compRatioSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(compRatioSlider);

    compAttackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(compAttackLabel);
    compAttackSlider.setPopupDisplayEnabled(true, false, this);
    compAttackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    compAttackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(compAttackSlider);

    compReleaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(compReleaseLabel);
    compReleaseSlider.setPopupDisplayEnabled(true, false, this);
    compReleaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    compReleaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(compReleaseSlider);

    //Envelope Follower Parameters--------------------------------------------------------------------------------------
    addAndMakeVisible(envelopeButton);
    envelopeButton.setToggleState(true, juce::dontSendNotification);
    envelopeButton.onClick = [this] {
        if (envelopeButton.getToggleState()) {
            processorRef.getPreEnvelopeFollower().setActivation(true);
            processorRef.getPostEnvelopeFollower().setActivation(true);
        }
        else {
            processorRef.getPreEnvelopeFollower().setActivation(false);
            processorRef.getPostEnvelopeFollower().setActivation(false);
        }
    };

    const auto addEnvKnob = [] (juce::Slider& slider, juce::Label& label) {
        slider.setPopupDisplayEnabled(true, false, nullptr);
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setPopupMenuEnabled(true);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font (juce::FontOptions { 10.0f }));
    };

    addEnvKnob(envAttackSlider, envAttackLabel);
    addAndMakeVisible(envAttackSlider);
    addAndMakeVisible(envAttackLabel);

    addEnvKnob(envReleaseSlider, envReleaseLabel);
    addAndMakeVisible(envReleaseSlider);
    addAndMakeVisible(envReleaseLabel);

    setSize (600, 700);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void AudioPluginAudioProcessorEditor::resized()
{
    visual.setBounds(getWidth() / 2 - 75,20, 150 ,150);
    distortionHeader.setBounds(getWidth() / 5 - 62,-40, 150 ,150);
    compressionHeader.setBounds(getWidth() / 5 * 4 - 75,-40, 150 ,150);
    envelopeHeader.setBounds(getWidth() / 2 - 75, getHeight() / 5 * 3, 150, 150);

    //Distortion Drawings-----------------------------------------------------------------------------------------------
    satLabel.setBounds(getWidth() / 5 - 15, getHeight() / 15 * 3 - 20, 50, 50);
    threshLabel.setBounds(getWidth() / 5 - 15, getHeight() / 15 * 5 - 20, 50, 50);
    outputLabel.setBounds(getWidth() / 5 - 15, getHeight() / 15 * 7 - 20, 50, 50);
    mixLabel.setBounds(getWidth() / 5 - 15, getHeight() / 15 * 9 - 20, 50, 50);
    //cutoffLabel.setBounds(getWidth() / 5 * 4 - 25, getHeight() / 5 - 20, 50, 50);
    //resoLabel.setBounds(getWidth() / 5 * 4 - 25, getHeight() / 5 * 2 - 20, 50, 50);
    //preLabel.setBounds(getWidth() / 2 - 5, 20, 30, 20);
    //postLabel.setBounds(getWidth() / 2 - 5, 40, 30, 20);
    //offLabel.setBounds(getWidth() / 2 - 5, 60, 30, 20);

    satSlider.setBounds(getWidth() / 5 - 35, getHeight() / 15 * 3 - 40, 90, 90);
    threshSlider.setBounds(getWidth() / 5 - 35, getHeight() / 15 * 5 - 40, 90, 90);
    outputSlider.setBounds(getWidth() / 5 - 35, getHeight() / 15 * 7 - 40, 90, 90);
    mixSlider.setBounds(getWidth() / 5 - 35, getHeight() / 15 * 9 - 40, 90, 90);
    //cutoffSlider.setBounds(getWidth() / 5 * 4 - 45, getHeight() / 5 - 40, 90, 90);
    //resoSlider.setBounds(getWidth() / 5 * 4 - 45, getHeight() / 5 * 2 - 40, 90, 90);

    algButton.setBounds(getWidth() / 5 - 28, getHeight() / 5 - 80, 80, 30);
    //filterButton.setBounds(getWidth() / 5 * 4 - 40, getHeight() / 5 - 70, 80, 30);
    //preButton.setBounds(getWidth() / 2 + 20, 20 ,20, 20);
    //postButton.setBounds(getWidth() / 2 + 20, 40 ,20, 20);
    //offButton.setBounds(getWidth() / 2 + 20, 60 ,20, 20);
    //Compressor Drawings-----------------------------------------------------------------------------------------------
    compressorButton.setBounds(getWidth() / 5 * 4 - 10, getHeight() / 5 - 65, 60, 30);

    compThreshSlider.setBounds(getWidth() / 5 * 4 - 35, getHeight() / 15 * 3 - 40, 90, 90);
    compRatioSlider.setBounds(getWidth() / 5 * 4 - 35, getHeight() / 15 * 5 - 40, 90, 90);
    compAttackSlider.setBounds(getWidth() / 5 * 4 - 35, getHeight() / 15 * 7 - 40, 90, 90);
    compReleaseSlider.setBounds(getWidth() / 5 * 4 - 35, getHeight() / 15 * 9 - 40, 90, 90);

    compThreshLabel.setBounds(compThreshSlider.getX(), compThreshSlider.getBottom() - 8, compThreshSlider.getWidth(), 18);
    compRatioLabel.setBounds(compRatioSlider.getX(), compRatioSlider.getBottom() - 8, compRatioSlider.getWidth(), 18);
    compAttackLabel.setBounds(compAttackSlider.getX(), compAttackSlider.getBottom() - 8, compAttackSlider.getWidth(), 18);
    compReleaseLabel.setBounds(compReleaseSlider.getX(), compReleaseSlider.getBottom() - 8, compReleaseSlider.getWidth(), 18);

    //Envelope Follower Drawings----------------------------------------------------------------------------------------

    envelopeButton.setBounds(getWidth() / 2 - 15, getHeight() / 5 * 4 - 25, 30, 30);

    envAttackSlider.setBounds(200, getHeight() / 5 * 4, 80, 80);
    envReleaseSlider.setBounds(320, getHeight() / 5 * 4, 80, 80);

    envAttackLabel.setBounds(envAttackSlider.getX(), envAttackSlider.getBottom() - 8, envAttackSlider.getWidth(), 18);
    envReleaseLabel.setBounds(envReleaseSlider.getX(), envReleaseSlider.getBottom() - 8, envReleaseSlider.getWidth(), 18);
}
