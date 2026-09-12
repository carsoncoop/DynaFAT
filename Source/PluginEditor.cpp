#include "PluginProcessor.h"
#include "PluginEditor.h"

void linkMinMaxSliders (juce::Slider& minSlider, juce::Slider& maxSlider){
    minSlider.onValueChange = [&minSlider, &maxSlider] {
        if (minSlider.getValue() > maxSlider.getValue())
            maxSlider.setValue (minSlider.getValue());
    };

    maxSlider.onValueChange = [&minSlider, &maxSlider] {
        if (maxSlider.getValue() < minSlider.getValue())
            minSlider.setValue (maxSlider.getValue());
    };
}
//==============================================================================
void AudioPluginAudioProcessorEditor::timerCallback()
{
    visual.pushBuffer(processorRef.visualizerBuffer);
}
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
    algButtonAttachment(processorRef.getState(), "algButton", algButton),
    satSliderAttachment(processorRef.getState(), "drive", satSlider),
    threshSliderAttachment(processorRef.getState(), "thresh", threshSlider),
    outputSliderAttachment(processorRef.getState(), "output", outputSlider),
    mixSliderAttachment(processorRef.getState(), "mix", mixSlider),
    /*cutoffSliderAttachment(processorRef.getState(), "cutoff", cutoffSlider),
    resoSliderAttachment(processorRef.getState(), "resonance", resoSlider),*/
    compressorButtonAttachment(processorRef.getState(), "compressorButton", compressorButton),
    compThreshHighSliderAttachment(processorRef.getState(), "compThreshHigh", compThreshHighSlider),
    compThreshLowSliderAttachment(processorRef.getState(), "compThreshLow", compThreshLowSlider),
    compRatioSliderAttachment(processorRef.getState(), "compRatio", compRatioSlider),
    compAttackSliderAttachment(processorRef.getState(), "compAttack", compAttackSlider),
    compReleaseSliderAttachment(processorRef.getState(), "compRelease", compReleaseSlider),
    envelopeButtonAttachment(processorRef.getState(), "envelopeButton", envelopeButton),
    envAttackSliderAttachment(processorRef.getState(), "envAttack", envAttackSlider),
    envReleaseSliderAttachment(processorRef.getState(), "envRelease", envReleaseSlider)
{
    auto setupLabel = [this] (juce::Label& label, const juce::Font& font = juce::Font(), const juce::String& text = {}) {
        label.setJustificationType(juce::Justification::centred);
        if (font.getHeight() > 0.0f)
            label.setFont(font);
        if (text.isNotEmpty())
            label.setText(text, juce::dontSendNotification);
        addAndMakeVisible(label);
    };
    auto setupKnob = [this, &setupLabel] (juce::Slider& slider, juce::Label& label,
                                         bool popupDisplay = true, bool menuEnabled = true,
                                         juce::Component* popupOwner = nullptr,
                                         const juce::Font& font = juce::Font(juce::FontOptions { 14.0f }))
    {
        juce::Component* owner = popupOwner ? popupOwner : this;
        slider.setPopupDisplayEnabled(popupDisplay, false, owner);
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setPopupMenuEnabled(menuEnabled);
        setupLabel(label, font);
        addAndMakeVisible(slider);
    };

    //Headers-----------------------------------------------------------------------------------------------------------
    setupLabel(distortionHeader);
    setupLabel(compressionHeader);
    setupLabel(envelopeHeader);

    visual.setBufferSize(64);
    visual.setSamplesPerBlock(64);
    visual.setRepaintRate(60);
    visual.setColours(juce::Colours::black, juce::Colours::blueviolet);
    startTimerHz(60);
    addAndMakeVisible(visual);

    //Distortion Parameters---------------------------------------------------------------------------------------------

    setupKnob(satSlider, satLabel);
    setupKnob(threshSlider, threshLabel);
    setupKnob(outputSlider, outputLabel);
    setupKnob(mixSlider, mixLabel);
    addAndMakeVisible(algButton);

    //Initialize algButton text
    const juce::StringArray algChoices { "Soft Clip", "Hard Clip", "Foldback", "Downsample"};
    int idx = static_cast<int>(std::round(processorRef.getState().getRawParameterValue("algButton")->load()));
    algButton.setButtonText(algChoices[idx]);

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

    //Compressor Parameters---------------------------------------------------------------------------------------------
    setupKnob(compThreshHighSlider, compThreshHighLabel);
    setupKnob(compThreshLowSlider, compThreshLowLabel);
    setupKnob(compRatioSlider, compRatioLabel);
    setupKnob(compAttackSlider, compAttackLabel);
    setupKnob(compReleaseSlider, compReleaseLabel);
    addAndMakeVisible(compressorButton);

    //Initialize compressorButton toggle
    compressorButton.setToggleState(processorRef.getState().getRawParameterValue("compressorButton")->load() > 0.5f, juce::dontSendNotification);
    compressorButton.onClick = [this] {
        if (compressorButton.getToggleState()) {
            processorRef.getCompressor().setActivation(true);
        }
        else {
            processorRef.getCompressor().setActivation(false);
        }
    };

    linkMinMaxSliders(compThreshLowSlider, compThreshHighSlider);


    //Envelope Follower Parameters--------------------------------------------------------------------------------------
    setupKnob(envAttackSlider, envAttackLabel);
    setupKnob(envReleaseSlider, envReleaseLabel);
    addAndMakeVisible(envelopeButton);

    //Initialize envelopeButton toggle
    envelopeButton.setToggleState(processorRef.getState().getRawParameterValue("envelopeButton")->load() > 0.5f, juce::dontSendNotification);
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



    setSize (600, 700);
    //Filter stuff below------------------------------------------------------------------------------------------------

    /*offButton.setToggleState(false, juce::dontSendNotification);
    preButton.setRadioGroupId(1);
    postButton.setRadioGroupId(1);
    offButton.setRadioGroupId(1);*/

    /*preButton.onClick = [this] {
        if (preButton.getToggleState()) {
            processorRef.getFilter().setFilterOrder(Pre);
        }
    };

    postButton.onClick = [this] {
        if (postButton.getToggleState()) {
            processorRef.getFilter().setFilterOrder(Post);
        }
    };
    offButton.onClick = [this] {
        if (offButton.getToggleState()) {
            processorRef.getFilter().setActivation(false);
        }
    };*/

    /*filterTypeButton.setButtonText("Lowpass");
    filterTypeButton.onClick = [this]() {
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
                    filterTypeButton.setButtonText("Lowpass");
                    processorRef.getFilter().setFilterType(juce::dsp::StateVariableTPTFilterType::lowpass);
                }
                else if (result == 2) {
                    filterTypeButton.setButtonText("Highpass");
                    processorRef.getFilter().setFilterType(juce::dsp::StateVariableTPTFilterType::highpass);
                }
                else if (result == 3) {
                    filterTypeButton.setButtonText("Bandpass");
                    processorRef.getFilter().setFilterType(juce::dsp::StateVariableTPTFilterType::bandpass);
                }
        });
    };*/

    /*preLabel.setJustificationType(juce::Justification::centred);
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
    addAndMakeVisible(resoSlider);*/


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
    compressionHeader.setBounds(getWidth() / 5 * 4 - 62,-40, 150 ,150);
    envelopeHeader.setBounds(getWidth() / 2 - 75, getHeight() / 5 * 3, 150, 150);

    //Distortion Drawings-----------------------------------------------------------------------------------------------
    algButton.setBounds(getWidth() / 5 - 28, getHeight() / 5 - 80, 80, 30);

    satSlider.setBounds(getWidth() / 5 - 35, getHeight() / 15 * 3 - 40, 90, 90);
    threshSlider.setBounds(getWidth() / 5 - 35, getHeight() / 15 * 5 - 40, 90, 90);
    outputSlider.setBounds(getWidth() / 5 - 35, getHeight() / 15 * 7 - 40, 90, 90);
    mixSlider.setBounds(getWidth() / 5 - 35, getHeight() / 15 * 9 - 40, 90, 90);

    satLabel.setBounds(satSlider.getX(), satSlider.getBottom() - 10, satSlider.getWidth(), 18);
    threshLabel.setBounds(threshSlider.getX(), threshSlider.getBottom() - 10, threshSlider.getWidth(), 18);
    outputLabel.setBounds(outputSlider.getX(), outputSlider.getBottom() - 10, outputSlider.getWidth(), 18);
    mixLabel.setBounds(mixSlider.getX(), mixSlider.getBottom() - 10, mixSlider.getWidth(), 18);

    //Compressor Drawings-----------------------------------------------------------------------------------------------
    compressorButton.setBounds(getWidth() / 5 * 4 - 30, getHeight() / 5 - 65, 60, 30);

    compThreshHighSlider.setBounds(getWidth() / 5 * 4 + 35, getHeight() / 15 * 3 - 40, 60, 60);
    compThreshLowSlider.setBounds(getWidth() / 5 * 4 - 35, getHeight() / 15 * 3 - 40, 60, 60);
    compRatioSlider.setBounds(getWidth() / 5 * 4 - 35, getHeight() / 15 * 5 - 40, 90, 90);
    compAttackSlider.setBounds(getWidth() / 5 * 4 - 35, getHeight() / 15 * 7 - 40, 90, 90);
    compReleaseSlider.setBounds(getWidth() / 5 * 4 - 35, getHeight() / 15 * 9 - 40, 90, 90);

    compThreshHighLabel.setBounds(compThreshHighSlider.getX(), compThreshHighSlider.getBottom() - 10, compThreshHighSlider.getWidth(), 18);
    compThreshLowLabel.setBounds(compThreshLowSlider.getX(), compThreshLowSlider.getBottom() - 10, compThreshLowSlider.getWidth(), 18);
    compRatioLabel.setBounds(compRatioSlider.getX(), compRatioSlider.getBottom() - 10, compRatioSlider.getWidth(), 18);
    compAttackLabel.setBounds(compAttackSlider.getX(), compAttackSlider.getBottom() - 10, compAttackSlider.getWidth(), 18);
    compReleaseLabel.setBounds(compReleaseSlider.getX(), compReleaseSlider.getBottom() - 10, compReleaseSlider.getWidth(), 18);

    //Envelope Follower Drawings----------------------------------------------------------------------------------------

    envelopeButton.setBounds(getWidth() / 2 - 15, getHeight() / 5 * 4 - 25, 30, 30);

    envAttackSlider.setBounds(200, getHeight() / 5 * 4, 80, 80);
    envReleaseSlider.setBounds(320, getHeight() / 5 * 4, 80, 80);

    envAttackLabel.setBounds(envAttackSlider.getX(), envAttackSlider.getBottom() - 10, envAttackSlider.getWidth(), 18);
    envReleaseLabel.setBounds(envReleaseSlider.getX(), envReleaseSlider.getBottom() - 10, envReleaseSlider.getWidth(), 18);

    /*//Filter Drawings---------------------------------------------------------------------------------------------------
     filterTypeButton.setBounds(getWidth() / 5 * 4 - 40, getHeight() / 5 - 70, 80, 30);
     preButton.setBounds(getWidth() / 2 + 20, 20 ,20, 20);
     postButton.setBounds(getWidth() / 2 + 20, 40 ,20, 20);
     offButton.setBounds(getWidth() / 2 + 20, 60 ,20, 20);
     cutoffLabel.setBounds(getWidth() / 5 * 4 - 25, getHeight() / 5 - 20, 50, 50);
     resoLabel.setBounds(getWidth() / 5 * 4 - 25, getHeight() / 5 * 2 - 20, 50, 50);
     preLabel.setBounds(getWidth() / 2 - 5, 20, 30, 20);
     postLabel.setBounds(getWidth() / 2 - 5, 40, 30, 20);
     offLabel.setBounds(getWidth() / 2 - 5, 60, 30, 20);
     cutoffSlider.setBounds(getWidth() / 5 * 4 - 45, getHeight() / 5 - 40, 90, 90);
     resoSlider.setBounds(getWidth() / 5 * 4 - 45, getHeight() / 5 * 2 - 40, 90, 90);*/

    /*addAndMakeVisible(filterTypeButton);
    addAndMakeVisible(preButton);
    addAndMakeVisible(postButton);
    addAndMakeVisible(offButton);*/
}
