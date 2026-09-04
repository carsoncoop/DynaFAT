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
    lowLowerThreshSliderAttachment(processorRef.getState(), "lowLowerThresh", lowLowerThreshSlider),
    lowUpperThreshSliderAttachment(processorRef.getState(), "lowUpperThresh", lowUpperThreshSlider),
    midLowerThreshSliderAttachment(processorRef.getState(), "midLowerThresh", midLowerThreshSlider),
    midUpperThreshSliderAttachment(processorRef.getState(), "midUpperThresh", midUpperThreshSlider),
    highLowerThreshSliderAttachment(processorRef.getState(), "highLowerThresh", highLowerThreshSlider),
    highUpperThreshSliderAttachment(processorRef.getState(), "highUpperThresh", highUpperThreshSlider),
    lowLowerRatioSliderAttachment(processorRef.getState(), "lowLowerRatio", lowLowerRatioSlider),
    lowUpperRatioSliderAttachment(processorRef.getState(), "lowUpperRatio", lowUpperRatioSlider),
    midLowerRatioSliderAttachment(processorRef.getState(), "midLowerRatio", midLowerRatioSlider),
    midUpperRatioSliderAttachment(processorRef.getState(), "midUpperRatio", midUpperRatioSlider),
    highLowerRatioSliderAttachment(processorRef.getState(), "highLowerRatio", highLowerRatioSlider),
    highUpperRatioSliderAttachment(processorRef.getState(), "highUpperRatio", highUpperRatioSlider),
    lowInputGainSliderAttachment(processorRef.getState(), "lowInputGain", lowInputGainSlider),
    lowOutputGainSliderAttachment(processorRef.getState(), "lowOutputGain", lowOutputGainSlider),
    midInputGainSliderAttachment(processorRef.getState(), "midInputGain", midInputGainSlider),
    midOutputGainSliderAttachment(processorRef.getState(), "midOutputGain", midOutputGainSlider),
    highInputGainSliderAttachment(processorRef.getState(), "highInputGain", highInputGainSlider),
    highOutputGainSliderAttachment(processorRef.getState(), "highOutputGain", highOutputGainSlider),
    compMasterGainSliderAttachment(processorRef.getState(), "compMasterGain", compMasterGainSlider),
    attackSliderAttachment(processorRef.getState(), "attack", attackSlider),
    releaseSliderAttachment(processorRef.getState(), "release", releaseSlider),
    envAttackSliderAttachment(processorRef.getState(), "envAttack", envAttackSlider),
    envReleaseSliderAttachment(processorRef.getState(), "envRelease", envReleaseSlider),
    gainMatchAttackSliderAttachment(processorRef.getState(), "gainMatchAttack", gainMatchAttackSlider),
    releaseMatchReleaseSliderAttachment(processorRef.getState(), "releaseMatchRelease", releaseMatchReleaseSlider)
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
                    processorRef.distortionAlg = SoftClip;
                    threshLabel.setText("Thresh (+dB)", juce::dontSendNotification);
                    satLabel.setText("Drive (+dB)", juce::dontSendNotification);
                }
                else if (result == 2) {
                    algButton.setButtonText("Hard Clip");
                    processorRef.distortionAlg = HardClip;
                    threshLabel.setText("Thresh (+dB)", juce::dontSendNotification);
                    satLabel.setText("Drive (+dB)", juce::dontSendNotification);
                }
                else if (result == 3) {
                    algButton.setButtonText("Foldback");
                    processorRef.distortionAlg = Foldback;
                    threshLabel.setText("Thresh (+dB)", juce::dontSendNotification);
                    satLabel.setText("Drive (+dB)", juce::dontSendNotification);
                }
                else if (result == 4) {
                    algButton.setButtonText("Downsample");
                    processorRef.distortionAlg = Downsample;
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
    addAndMakeVisible(distortionHeader);
    addAndMakeVisible(compressionHeader);

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

    compressorButton.setToggleState(false, juce::dontSendNotification);

    compressorButton.onClick = [this] {
        if (compressorButton.getToggleState()) {
            processorRef.compressorToggle = CompOn;
        }
        else {
            processorRef.compressorToggle = CompOff;
        }
    };

    lowLowerThreshSlider.setPopupDisplayEnabled(true, false, this);
    lowLowerThreshSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lowLowerThreshSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    lowLowerThreshSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(lowLowerThreshSlider);

    lowUpperThreshSlider.setPopupDisplayEnabled(true, false, this);
    lowUpperThreshSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lowUpperThreshSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    lowUpperThreshSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(lowUpperThreshSlider);

    midLowerThreshSlider.setPopupDisplayEnabled(true, false, this);
    midLowerThreshSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    midLowerThreshSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midLowerThreshSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(midLowerThreshSlider);

    midUpperThreshSlider.setPopupDisplayEnabled(true, false, this);
    midUpperThreshSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    midUpperThreshSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midUpperThreshSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(midUpperThreshSlider);

    highLowerThreshSlider.setPopupDisplayEnabled(true, false, this);
    highLowerThreshSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    highLowerThreshSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    highLowerThreshSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(highLowerThreshSlider);

    highUpperThreshSlider.setPopupDisplayEnabled(true, false, this);
    highUpperThreshSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    highUpperThreshSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    highUpperThreshSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(highUpperThreshSlider);

    lowLowerRatioSlider.setPopupDisplayEnabled(true, false, this);
    lowLowerRatioSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lowLowerRatioSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    lowLowerRatioSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(lowLowerRatioSlider);

    lowUpperRatioSlider.setPopupDisplayEnabled(true, false, this);
    lowUpperRatioSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lowUpperRatioSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    lowUpperRatioSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(lowUpperRatioSlider);

    midLowerRatioSlider.setPopupDisplayEnabled(true, false, this);
    midLowerRatioSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    midLowerRatioSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midLowerRatioSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(midLowerRatioSlider);

    midUpperRatioSlider.setPopupDisplayEnabled(true, false, this);
    midUpperRatioSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    midUpperRatioSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midUpperRatioSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(midUpperRatioSlider);

    highLowerRatioSlider.setPopupDisplayEnabled(true, false, this);
    highLowerRatioSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    highLowerRatioSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    highLowerRatioSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(highLowerRatioSlider);

    highUpperRatioSlider.setPopupDisplayEnabled(true, false, this);
    highUpperRatioSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    highUpperRatioSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    highUpperRatioSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(highUpperRatioSlider);

    attackSlider.setPopupDisplayEnabled(true, false, this);
    attackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    attackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    attackSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(attackSlider);

    releaseSlider.setPopupDisplayEnabled(true, false, this);
    releaseSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    releaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    releaseSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(releaseSlider);

    compMasterGainSlider.setPopupDisplayEnabled(true, false, this);
    compMasterGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    compMasterGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    compMasterGainSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(compMasterGainSlider);

    lowInputGainSlider.setPopupDisplayEnabled(true, false, this);
    lowInputGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lowInputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    lowInputGainSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(lowInputGainSlider);

    lowOutputGainSlider.setPopupDisplayEnabled(true, false, this);
    lowOutputGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lowOutputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    lowOutputGainSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(lowOutputGainSlider);

    midInputGainSlider.setPopupDisplayEnabled(true, false, this);
    midInputGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    midInputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midInputGainSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(midInputGainSlider);

    midOutputGainSlider.setPopupDisplayEnabled(true, false, this);
    midOutputGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    midOutputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midOutputGainSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(midOutputGainSlider);

    highInputGainSlider.setPopupDisplayEnabled(true, false, this);
    highInputGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    highInputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    highInputGainSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(highInputGainSlider);

    highOutputGainSlider.setPopupDisplayEnabled(true, false, this);
    highOutputGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    highOutputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    highOutputGainSlider.setPopupMenuEnabled(true);
    addAndMakeVisible(highOutputGainSlider);

    const auto addCompressorLabel = [] (juce::Label& label) {
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font (juce::FontOptions { 10.0f }));
    };
    addCompressorLabel(lowLowerThreshLabel); addAndMakeVisible(lowLowerThreshLabel);
    addCompressorLabel(lowUpperThreshLabel); addAndMakeVisible(lowUpperThreshLabel);
    addCompressorLabel(midLowerThreshLabel); addAndMakeVisible(midLowerThreshLabel);
    addCompressorLabel(midUpperThreshLabel); addAndMakeVisible(midUpperThreshLabel);
    addCompressorLabel(highLowerThreshLabel); addAndMakeVisible(highLowerThreshLabel);
    addCompressorLabel(highUpperThreshLabel); addAndMakeVisible(highUpperThreshLabel);
    addCompressorLabel(lowLowerRatioLabel); addAndMakeVisible(lowLowerRatioLabel);
    addCompressorLabel(lowUpperRatioLabel); addAndMakeVisible(lowUpperRatioLabel);
    addCompressorLabel(midLowerRatioLabel); addAndMakeVisible(midLowerRatioLabel);
    addCompressorLabel(midUpperRatioLabel); addAndMakeVisible(midUpperRatioLabel);
    addCompressorLabel(highLowerRatioLabel); addAndMakeVisible(highLowerRatioLabel);
    addCompressorLabel(highUpperRatioLabel); addAndMakeVisible(highUpperRatioLabel);
    addCompressorLabel(lowInputGainLabel); addAndMakeVisible(lowInputGainLabel);
    addCompressorLabel(lowOutputGainLabel); addAndMakeVisible(lowOutputGainLabel);
    addCompressorLabel(midInputGainLabel); addAndMakeVisible(midInputGainLabel);
    addCompressorLabel(midOutputGainLabel); addAndMakeVisible(midOutputGainLabel);
    addCompressorLabel(highInputGainLabel); addAndMakeVisible(highInputGainLabel);
    addCompressorLabel(highOutputGainLabel); addAndMakeVisible(highOutputGainLabel);
    addCompressorLabel(compMasterGainLabel); addAndMakeVisible(compMasterGainLabel);
    addCompressorLabel(attackLabel); addAndMakeVisible(attackLabel);
    addCompressorLabel(releaseLabel); addAndMakeVisible(releaseLabel);

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

    addEnvKnob(gainMatchAttackSlider, gainMatchAttackLabel);
    addAndMakeVisible(gainMatchAttackSlider);
    addAndMakeVisible(gainMatchAttackLabel);

    addEnvKnob(releaseMatchReleaseSlider, releaseMatchReleaseLabel);
    addAndMakeVisible(releaseMatchReleaseSlider);
    addAndMakeVisible(releaseMatchReleaseLabel);

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

    //Distortion Drawings-----------------------------------------------------------------------------------------------
    satLabel.setBounds(getWidth() / 5 - 15, getHeight() / 5 - 20, 50, 50);
    threshLabel.setBounds(getWidth() / 5 - 15, getHeight() / 5 * 2 - 20, 50, 50);
    outputLabel.setBounds(getWidth() / 5 - 15, getHeight() / 5 * 3 - 20, 50, 50);
    mixLabel.setBounds(getWidth() / 5 - 15, getHeight() / 5 * 4 - 20, 50, 50);
    //cutoffLabel.setBounds(getWidth() / 5 * 4 - 25, getHeight() / 5 - 20, 50, 50);
    //resoLabel.setBounds(getWidth() / 5 * 4 - 25, getHeight() / 5 * 2 - 20, 50, 50);
    //preLabel.setBounds(getWidth() / 2 - 5, 20, 30, 20);
    //postLabel.setBounds(getWidth() / 2 - 5, 40, 30, 20);
    //offLabel.setBounds(getWidth() / 2 - 5, 60, 30, 20);

    satSlider.setBounds(getWidth() / 5 - 35, getHeight() / 5 - 40, 90, 90);
    threshSlider.setBounds(getWidth() / 5 - 35, getHeight() / 5 * 2 - 40, 90, 90);
    outputSlider.setBounds(getWidth() / 5 - 35, getHeight() / 5 * 3 - 40, 90, 90);
    mixSlider.setBounds(getWidth() / 5 - 35, getHeight() / 5 * 4 - 40, 90, 90);
    //cutoffSlider.setBounds(getWidth() / 5 * 4 - 45, getHeight() / 5 - 40, 90, 90);
    //resoSlider.setBounds(getWidth() / 5 * 4 - 45, getHeight() / 5 * 2 - 40, 90, 90);

    algButton.setBounds(getWidth() / 5 - 28, getHeight() / 5 - 70, 80, 30);
    //filterButton.setBounds(getWidth() / 5 * 4 - 40, getHeight() / 5 - 70, 80, 30);
    //preButton.setBounds(getWidth() / 2 + 20, 20 ,20, 20);
    //postButton.setBounds(getWidth() / 2 + 20, 40 ,20, 20);
    //offButton.setBounds(getWidth() / 2 + 20, 60 ,20, 20);

    //Compressor Drawings-----------------------------------------------------------------------------------------------
    const auto rightPanelX = getWidth() - 268;
    const auto rightPanelY = 48;
    const auto sliderW = 62;
    const auto sliderH = 50;
    const auto bandGap = 126;
    const auto colGap = 72;

    compressorButton.setBounds(rightPanelX + 100, rightPanelY, 110, 30);

    const auto bandLayout = [&](juce::Slider& lowerT,
                               juce::Slider& upperT,
                               juce::Slider& lowerR,
                               juce::Slider& upperR,
                               juce::Slider& inputGain,
                               juce::Slider& outputGain,
                               juce::Label& lowerTLabel,
                               juce::Label& upperTLabel,
                               juce::Label& lowerRLabel,
                               juce::Label& upperRLabel,
                               juce::Label& inputGainLabel,
                               juce::Label& outputGainLabel,
                               int bandIndex)
    {
        const auto baseX = rightPanelX;
        const auto yBase = rightPanelY + 34 + bandIndex * bandGap;
        const auto row1Y = yBase;
        const auto row2Y = yBase + 46;
        const auto row3Y = yBase + 92;

        lowerT.setBounds(baseX + colGap, row1Y, sliderW, sliderH);
        upperT.setBounds(baseX + colGap * 2, row1Y, sliderW, sliderH);
        lowerR.setBounds(baseX + colGap, row2Y, sliderW, sliderH);
        upperR.setBounds(baseX + colGap * 2, row2Y, sliderW, sliderH);
        inputGain.setBounds(baseX + colGap, row3Y, sliderW, sliderH);
        outputGain.setBounds(baseX + colGap * 2, row3Y, sliderW, sliderH);

        lowerTLabel.setBounds(lowerT.getX(), lowerT.getBottom() - 8, lowerT.getWidth(), 18);
        upperTLabel.setBounds(upperT.getX(), upperT.getBottom() - 8, upperT.getWidth(), 18);
        lowerRLabel.setBounds(lowerR.getX(), lowerR.getBottom() - 8, lowerR.getWidth(), 18);
        upperRLabel.setBounds(upperR.getX(), upperR.getBottom() - 8, upperR.getWidth(), 18);
        inputGainLabel.setBounds(inputGain.getX(), inputGain.getBottom() - 8, inputGain.getWidth(), 18);
        outputGainLabel.setBounds(outputGain.getX(), outputGain.getBottom() - 8, outputGain.getWidth(), 18);
    };

    bandLayout(lowLowerThreshSlider, lowUpperThreshSlider, lowLowerRatioSlider, lowUpperRatioSlider,
               lowInputGainSlider, lowOutputGainSlider,
               lowLowerThreshLabel, lowUpperThreshLabel, lowLowerRatioLabel, lowUpperRatioLabel,
               lowInputGainLabel, lowOutputGainLabel, 0);
    bandLayout(midLowerThreshSlider, midUpperThreshSlider, midLowerRatioSlider, midUpperRatioSlider,
               midInputGainSlider, midOutputGainSlider,
               midLowerThreshLabel, midUpperThreshLabel, midLowerRatioLabel, midUpperRatioLabel,
               midInputGainLabel, midOutputGainLabel, 1);
    bandLayout(highLowerThreshSlider, highUpperThreshSlider, highLowerRatioSlider, highUpperRatioSlider,
               highInputGainSlider, highOutputGainSlider,
               highLowerThreshLabel, highUpperThreshLabel, highLowerRatioLabel, highUpperRatioLabel,
               highInputGainLabel, highOutputGainLabel, 2);

    const auto sideX = rightPanelX + 10;
    const auto sideY = rightPanelY + 34 + 3 * bandGap + 20;
    attackSlider.setBounds(sideX, sideY, sliderW, sliderH);
    releaseSlider.setBounds(sideX + colGap, sideY, sliderW, sliderH);
    compMasterGainSlider.setBounds(sideX + colGap * 2, sideY, sliderW, sliderH);

    attackLabel.setBounds(attackSlider.getX(), attackSlider.getBottom() - 8, attackSlider.getWidth(), 18);
    releaseLabel.setBounds(releaseSlider.getX(), releaseSlider.getBottom() - 8, releaseSlider.getWidth(), 18);
    compMasterGainLabel.setBounds(compMasterGainSlider.getX(), compMasterGainSlider.getBottom() - 8, compMasterGainSlider.getWidth(), 18);

    const auto bottomKnobSize = 68;
    const auto bottomY = getHeight() - 120;
    const auto bottomXStart = 32;
    const auto bottomGap = 130;

    envAttackSlider.setBounds(bottomXStart, bottomY, bottomKnobSize, bottomKnobSize);
    envReleaseSlider.setBounds(bottomXStart + bottomGap, bottomY, bottomKnobSize, bottomKnobSize);
    gainMatchAttackSlider.setBounds(bottomXStart + bottomGap * 2, bottomY, bottomKnobSize, bottomKnobSize);
    releaseMatchReleaseSlider.setBounds(bottomXStart + bottomGap * 3, bottomY, bottomKnobSize, bottomKnobSize);

    envAttackLabel.setBounds(envAttackSlider.getX(), envAttackSlider.getBottom() - 8, envAttackSlider.getWidth(), 18);
    envReleaseLabel.setBounds(envReleaseSlider.getX(), envReleaseSlider.getBottom() - 8, envReleaseSlider.getWidth(), 18);
    gainMatchAttackLabel.setBounds(gainMatchAttackSlider.getX(), gainMatchAttackSlider.getBottom() - 8, gainMatchAttackSlider.getWidth(), 18);
    releaseMatchReleaseLabel.setBounds(releaseMatchReleaseSlider.getX(), releaseMatchReleaseSlider.getBottom() - 8, releaseMatchReleaseSlider.getWidth(), 18);
}
