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
    routingButtonAttachment(processorRef.getState(), "routingButton", routingButton),
    algButtonAttachment(processorRef.getState(), "algButton", algButton),
    satSliderAttachment(processorRef.getState(), "drive", satSlider),
    threshSliderAttachment(processorRef.getState(), "thresh", threshSlider),
    outputSliderAttachment(processorRef.getState(), "output", outputSlider),
    mixSliderAttachment(processorRef.getState(), "mix", mixSlider),
    distortionPassesSliderAttachment(processorRef.getState(), "distortionPasses", distortionPassesSlider),
    /*cutoffSliderAttachment(processorRef.getState(), "cutoff", cutoffSlider),
    resoSliderAttachment(processorRef.getState(), "resonance", resoSlider),*/
    compressorButtonAttachment(processorRef.getState(), "compressorButton", compressorButton),
    compThreshHighSliderAttachment(processorRef.getState(), "compThreshHigh", compThreshHighSlider),
    compThreshLowSliderAttachment(processorRef.getState(), "compThreshLow", compThreshLowSlider),
    compRatioSliderAttachment(processorRef.getState(), "compRatio", compRatioSlider),
    compAttackSliderAttachment(processorRef.getState(), "compAttack", compAttackSlider),
    compReleaseSliderAttachment(processorRef.getState(), "compRelease", compReleaseSlider),
    compressorPassesSliderAttachment(processorRef.getState(), "compressorPasses", compressorPassesSlider),
    envelopeButtonAttachment(processorRef.getState(), "envelopeButton", envelopeButton),
    envAttackSliderAttachment(processorRef.getState(), "envAttack", envAttackSlider),
    envReleaseSliderAttachment(processorRef.getState(), "envRelease", envReleaseSlider),
    envDynamixSliderAttachment(processorRef.getState(), "envDynamix", envDynamixSlider),
    masterGainSliderAttachment(processorRef.getState(), "masterGain", masterGainSlider)
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

    const auto background = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    visual.setBufferSize(90);
    visual.setSamplesPerBlock(90);
    visual.setRepaintRate(144);
    visual.setColours(background, juce::Colours::deepskyblue);
    startTimerHz(60);
    addAndMakeVisible(visual);

    //Distortion Parameters---------------------------------------------------------------------------------------------

    setupKnob(satSlider, satLabel);
    setupKnob(threshSlider, threshLabel);
    setupKnob(outputSlider, outputLabel);
    setupKnob(mixSlider, mixLabel);
    addAndMakeVisible(algButton);
    setupKnob(distortionPassesSlider, distortionPassesLabel);
    distortionPassesSlider.setSliderStyle (juce::Slider::IncDecButtons);
    distortionPassesSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 60, 24);


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
                    threshLabel.setText("Thresh", juce::dontSendNotification);
                    satLabel.setText("Drive", juce::dontSendNotification);
                }
                else if (result == 2) {
                    algButton.setButtonText("Hard Clip");
                    processorRef.getDistortion().setDistortionType(HardClip);
                    threshLabel.setText("Thresh", juce::dontSendNotification);
                    satLabel.setText("Drive", juce::dontSendNotification);
                }
                else if (result == 3) {
                    algButton.setButtonText("Foldback");
                    processorRef.getDistortion().setDistortionType(Foldback);
                    threshLabel.setText("Thresh", juce::dontSendNotification);
                    satLabel.setText("Drive", juce::dontSendNotification);
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

    compressorPassesSlider.setEnabled(false); //Temporary line to disable compressorPassesSlider while debugging


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

    setupKnob(compressorPassesSlider, compressorPassesLabel);
    compressorPassesSlider.setSliderStyle (juce::Slider::IncDecButtons);
    compressorPassesSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 60, 24);

    //Envelope Follower Parameters--------------------------------------------------------------------------------------
    setupKnob(envAttackSlider, envAttackLabel);
    setupKnob(envReleaseSlider, envReleaseLabel);
    setupKnob(envDynamixSlider, envDynamixLabel);
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

    //General Parameters------------------------------------------------------------------------------------------------
    setupKnob(masterGainSlider, masterGainLabel);

    //Initialize routingButton text
    const juce::StringArray routingChoices { "Distortion -> Compression", "Compression -> Distortion"};
    int idx1 = static_cast<int>(std::round(processorRef.getState().getRawParameterValue("routingButton")->load()));
    routingButton.setButtonText(routingChoices[idx1]);

    routingButton.onClick = [this]() {
        juce::PopupMenu routingMenu;
        routingMenu.addItem(1, "Distortion -> Compression");
        routingMenu.addItem(2, "Compression -> Distortion");

        routingMenu.showMenuAsync(juce::PopupMenu::Options(),
            [this] (int result) {
                if (result == 0) {//If no selection is made

                }
                else if (result == 1) {
                    routingButton.setButtonText("Distortion -> Compression");
                    processorRef.setRouting(DistFirst);
                }
                else if (result == 2) {
                    routingButton.setButtonText("Compression -> Distortion");
                    processorRef.setRouting(CompFirst);
                }
        });
    };

    addAndMakeVisible(routingButton);



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

    const auto midX = static_cast<float> (getWidth() / 2);
    const auto lineY = static_cast<float> (getHeight() / 5 * 3 + 62);

    g.setColour (juce::Colours::darkgrey);
    g.drawLine (midX, 0.0f, midX, lineY, 1.0f);                                   // vertical divider
    g.drawLine (0.0f, lineY, static_cast<float> (getWidth()), lineY, 1.0f);       // horizontal divider

    const auto frame = visual.getBounds().expanded (4).toFloat();   // 4px padding around the waveform
    g.setColour (juce::Colours::darkgrey);
    g.drawRoundedRectangle (frame, 4.0f, 1.0f);

}

void AudioPluginAudioProcessorEditor::resized()
{
    const int centerX = getWidth() / 2;
    const int distortionCenterX = getWidth() / 4;
    const int compressionCenterX = getWidth() - distortionCenterX;

    visual.setBounds(40,525, 120 ,120);
    distortionHeader.setBounds(distortionCenterX - 75,-40, 150 ,150);
    compressionHeader.setBounds(compressionCenterX - 75,-40, 150 ,150);
    envelopeHeader.setBounds(centerX - 75, getHeight() / 5 * 3, 150, 150);

    //Distortion Drawings-----------------------------------------------------------------------------------------------

    algButton.setBounds(distortionCenterX - 40, getHeight() / 5 - 80, 80, 30);

    satSlider.setBounds(distortionCenterX - 45, getHeight() / 15 * 3 - 40, 90, 90);
    threshSlider.setBounds(distortionCenterX - 45, getHeight() / 15 * 5 - 40, 90, 90);
    outputSlider.setBounds(distortionCenterX - 45, getHeight() / 15 * 7 - 40, 90, 90);
    mixSlider.setBounds(distortionCenterX - 45, getHeight() / 15 * 9 - 40, 90, 90);
    distortionPassesSlider.setBounds(distortionCenterX + 85, getHeight() / 15 * 9 - 10, 50, 50);

    satLabel.setBounds(satSlider.getX(), satSlider.getBottom() - 10, satSlider.getWidth(), 18);
    threshLabel.setBounds(threshSlider.getX(), threshSlider.getBottom() - 10, threshSlider.getWidth(), 18);
    outputLabel.setBounds(outputSlider.getX(), outputSlider.getBottom() - 10, outputSlider.getWidth(), 18);
    mixLabel.setBounds(mixSlider.getX(), mixSlider.getBottom() - 10, mixSlider.getWidth(), 18);
    distortionPassesLabel.setBounds(distortionPassesSlider.getX(), distortionPassesSlider.getBottom(), distortionPassesSlider.getWidth(), 18);

    //Compressor Drawings-----------------------------------------------------------------------------------------------
    compressorButton.setBounds(compressionCenterX - 12, getHeight() / 5 - 80, 60, 30);

    compThreshLowSlider.setBounds(compressionCenterX - 64, getHeight() / 15 * 3 - 40, 60, 60);
    compThreshHighSlider.setBounds(compressionCenterX + 4, getHeight() / 15 * 3 - 40, 60, 60);

    compRatioSlider.setBounds(compressionCenterX - 45, getHeight() / 15 * 5 - 40, 90, 90);
    compAttackSlider.setBounds(compressionCenterX - 45, getHeight() / 15 * 7 - 40, 90, 90);
    compReleaseSlider.setBounds(compressionCenterX - 45, getHeight() / 15 * 9 - 40, 90, 90);
    compressorPassesSlider.setBounds(compressionCenterX + 85, getHeight() / 15 * 9 - 10, 50, 50);

    compThreshHighLabel.setBounds(compThreshHighSlider.getX(), compThreshHighSlider.getBottom() - 10, compThreshHighSlider.getWidth(), 18);
    compThreshLowLabel.setBounds(compThreshLowSlider.getX(), compThreshLowSlider.getBottom() - 10, compThreshLowSlider.getWidth(), 18);
    compRatioLabel.setBounds(compRatioSlider.getX(), compRatioSlider.getBottom() - 10, compRatioSlider.getWidth(), 18);
    compAttackLabel.setBounds(compAttackSlider.getX(), compAttackSlider.getBottom() - 10, compAttackSlider.getWidth(), 18);
    compReleaseLabel.setBounds(compReleaseSlider.getX(), compReleaseSlider.getBottom() - 10, compReleaseSlider.getWidth(), 18);
    compressorPassesLabel.setBounds(compressorPassesSlider.getX(), compressorPassesSlider.getBottom(), compressorPassesSlider.getWidth(), 18);

    //Envelope Follower Drawings----------------------------------------------------------------------------------------

    envelopeButton.setBounds(centerX - 15, getHeight() - 55, 30, 30);   // was -12 (3px off-center)

    envDynamixSlider.setBounds(centerX - 40,  getHeight() / 5 * 4 - 60, 80, 80);
    envAttackSlider.setBounds(centerX - 100,  getHeight() / 5 * 4, 80, 80);
    envReleaseSlider.setBounds(centerX + 20,  getHeight() / 5 * 4, 80, 80);

    envAttackLabel.setBounds(envAttackSlider.getX(), envAttackSlider.getBottom() - 10, envAttackSlider.getWidth(), 18);
    envReleaseLabel.setBounds(envReleaseSlider.getX(), envReleaseSlider.getBottom() - 10, envReleaseSlider.getWidth(), 18);
    envDynamixLabel.setBounds(envDynamixSlider.getX(), envDynamixSlider.getBottom() - 10, envDynamixSlider.getWidth(), 18);

    //General Drawings--------------------------------------------------------------------------------------------------
    masterGainSlider.setBounds(getWidth() - 70, getHeight() - 70, 60, 60);
    masterGainLabel.setBounds(masterGainSlider.getX(), masterGainSlider.getBottom() - 10, masterGainSlider.getWidth(), 18);
    routingButton.setBounds(centerX - 40, 20, 80, 30);

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