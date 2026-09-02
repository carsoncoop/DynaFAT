#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>


//==============================================================================

// float AudioPluginAudioProcessor::computeCompressionGain(float x_dB, float threshDown, float ratioDown, float threshUp, float ratioUp) {
//
// }

void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    //Distortion Preparation--------------------------------------------------------------------------------------------
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.setCutoffFrequency(20000.0f);
    filter.setResonance(0.7f);

    smoothedDrive.reset(sampleRate, 0.01f);
    smoothedThresh.reset(sampleRate, 0.01f);
    smoothedOutput.reset(sampleRate, 0.01f);
    smoothedMix.reset(sampleRate, 0.01f);
    smoothedCutoff.reset(sampleRate, 0.01f);
    smoothedReso.reset(sampleRate, 0.01f);
    smoothedDrive.setTargetValue(state.getRawParameterValue("drive")->load());
    smoothedThresh.setTargetValue(state.getRawParameterValue("thresh")->load());
    smoothedOutput.setTargetValue(state.getRawParameterValue("output")->load());
    smoothedMix.setTargetValue(state.getRawParameterValue("mix")->load());
    smoothedCutoff.setTargetValue(state.getRawParameterValue("cutoff")->load());
    smoothedReso.setTargetValue(state.getRawParameterValue("resonance")->load());

    //Compression Preparation-------------------------------------------------------------------------------------------
    lowCrossoverWide.prepare(spec);
    highCrossoverWide.prepare(spec);
    lowCrossoverNarrow.prepare(spec);
    highCrossoverNarrow.prepare(spec);
    lowCrossoverWide.setType(juce::dsp::LinkwitzRileyFilter<float>::Type::lowpass);
    highCrossoverWide.setType(juce::dsp::LinkwitzRileyFilter<float>::Type::highpass);
    lowCrossoverNarrow.setType(juce::dsp::LinkwitzRileyFilter<float>::Type::lowpass);
    highCrossoverNarrow.setType(juce::dsp::LinkwitzRileyFilter<float>::Type::highpass);
    lowCrossoverWide.setCutoffFrequency(120.0f);
    highCrossoverWide.setCutoffFrequency(120.0f);
    lowCrossoverNarrow.setCutoffFrequency(2500.0f);
    highCrossoverNarrow.setCutoffFrequency(2500.0f);

    smoothedLowLowerThresh.reset(sampleRate, 0.01f);
    smoothedLowLowerThresh.setCurrentAndTargetValue(state.getRawParameterValue("lowLowerThresh")->load());
    smoothedLowUpperThresh.reset(sampleRate, 0.01f);
    smoothedLowUpperThresh.setCurrentAndTargetValue(state.getRawParameterValue("lowUpperThresh")->load());
    smoothedLowLowerRatio.reset(sampleRate, 0.01f);
    smoothedLowLowerRatio.setCurrentAndTargetValue(state.getRawParameterValue("lowLowerRatio")->load());
    smoothedLowUpperRatio.reset(sampleRate, 0.01f);
    smoothedLowUpperRatio.setCurrentAndTargetValue(state.getRawParameterValue("lowUpperRatio")->load());
    smoothedMidLowerThresh.reset(sampleRate, 0.01f);
    smoothedMidLowerThresh.setCurrentAndTargetValue(state.getRawParameterValue("midLowerThresh")->load());
    smoothedMidUpperThresh.reset(sampleRate, 0.01f);
    smoothedMidUpperThresh.setCurrentAndTargetValue(state.getRawParameterValue("midUpperThresh")->load());
    smoothedMidLowerRatio.reset(sampleRate, 0.01f);
    smoothedMidLowerRatio.setCurrentAndTargetValue(state.getRawParameterValue("midLowerRatio")->load());
    smoothedMidUpperRatio.reset(sampleRate, 0.01f);
    smoothedMidUpperRatio.setCurrentAndTargetValue(state.getRawParameterValue("midUpperRatio")->load());
    smoothedHighLowerThresh.reset(sampleRate, 0.01f);
    smoothedHighLowerThresh.setCurrentAndTargetValue(state.getRawParameterValue("highLowerThresh")->load());
    smoothedHighUpperThresh.reset(sampleRate, 0.01f);
    smoothedHighUpperThresh.setCurrentAndTargetValue(state.getRawParameterValue("highUpperThresh")->load());
    smoothedHighLowerRatio.reset(sampleRate, 0.01f);
    smoothedHighLowerRatio.setCurrentAndTargetValue(state.getRawParameterValue("highLowerRatio")->load());
    smoothedHighUpperRatio.reset(sampleRate, 0.01f);
    smoothedHighUpperRatio.setCurrentAndTargetValue(state.getRawParameterValue("highUpperRatio")->load());
    smoothedLowInputGain.reset(sampleRate, 0.01f);
    smoothedLowInputGain.setCurrentAndTargetValue(state.getRawParameterValue("lowInputGain")->load());
    smoothedLowOutputGain.reset(sampleRate, 0.01f);
    smoothedLowOutputGain.setCurrentAndTargetValue(state.getRawParameterValue("lowOutputGain")->load());
    smoothedMidInputGain.reset(sampleRate, 0.01f);
    smoothedMidInputGain.setCurrentAndTargetValue(state.getRawParameterValue("midInputGain")->load());
    smoothedMidOutputGain.reset(sampleRate, 0.01f);
    smoothedMidOutputGain.setCurrentAndTargetValue(state.getRawParameterValue("midOutputGain")->load());
    smoothedHighInputGain.reset(sampleRate, 0.01f);
    smoothedHighInputGain.setCurrentAndTargetValue(state.getRawParameterValue("highInputGain")->load());
    smoothedHighOutputGain.reset(sampleRate, 0.01f);
    smoothedHighOutputGain.setCurrentAndTargetValue(state.getRawParameterValue("highOutputGain")->load());
    smoothedCompMasterGain.reset(sampleRate, 0.01f);
    smoothedCompMasterGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("compMasterGain")->load()));
    smoothedAttack.reset(sampleRate, 0.01f);
    smoothedAttack.setCurrentAndTargetValue(state.getRawParameterValue("attack")->load());
    smoothedRelease.reset(sampleRate, 0.01f);
    smoothedRelease.setCurrentAndTargetValue(state.getRawParameterValue("release")->load());

    //Visualizer--------------------------------------------------------------------------------------------------------
    visualizerBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}


void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //Distortion Starts Here--------------------------------------------------------------------------------------------

    //Takes linear gain value, and converts it to decibel representation
    smoothedDrive.setTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("drive")->load()));
    smoothedThresh.setTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("thresh")->load()));
    smoothedOutput.setTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("output")->load()));
    smoothedMix.setTargetValue(state.getRawParameterValue("mix")->load());
    smoothedCutoff.setTargetValue(state.getRawParameterValue("cutoff")->load());
    smoothedReso.setTargetValue(state.getRawParameterValue("resonance")->load());

    //Make copy of dry buffer for mix knob later
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.setSize(totalNumInputChannels, buffer.getNumSamples(), false, false, true);
    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        dryBuffer.copyFrom(channel, 0, buffer.getReadPointer(channel), buffer.getNumSamples());
    }

    /*//Filter
    if (filterType == Lowpass) {
        filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    }
    else if (filterType == Highpass) {
        filter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    }
    else if (filterType == Bandpass) {
        filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    }

    if (filterOrder == Pre) {
        smoothedCutoff.skip(buffer.getNumSamples());//Uses entire buffer
        filter.setCutoffFrequency(smoothedCutoff.getCurrentValue());
        smoothedReso.skip(buffer.getNumSamples());//Uses entire buffer
        filter.setResonance(smoothedReso.getCurrentValue());
        juce::dsp::AudioBlock<float> block(buffer); //(kinda) makes a copy of the buffer
        juce::dsp::ProcessContextReplacing context(block);
        filter.process(context);
    }*/

    //Used for downsampling/sample & hold
    float hold = 0;
    float counter = 0;

    //Sample processing
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        //Assign parameters
        const float drive = smoothedDrive.getNextValue();
        const float thresh = smoothedThresh.getNextValue();

        //Channel processing
        for (int channel = 0; channel < totalNumInputChannels; ++channel){
            auto* input = buffer.getWritePointer(channel); //essentially an array of floats (raw pointer technically)

            //Input drive
            if (distortionAlg != Downsample) {
                input[sample] *= drive;
            }

            //Distortion
            if (distortionAlg == SoftClip) {
                input[sample] = thresh * std::tanh(input[sample] / thresh);
            }
            else if (distortionAlg == HardClip) {
                input[sample] = juce::jlimit(-thresh,thresh, input[sample]);
            }
            else if (distortionAlg == Foldback) {
                input[sample] = std::abs(std::abs(fmod((input[sample] - thresh), (4.0f * thresh))) - 2.0f * thresh) - thresh;
            }
            else if (distortionAlg == Downsample) {
                if (counter == 0) {
                    hold = input[sample];
                }
                input[sample] = hold;
                counter += 0.5;
                if (counter >= static_cast<int>(juce::Decibels::gainToDecibels(drive))) {
                    counter = 0;
                }
            }
        }
    }
    /*if (filterOrder == Post) {
        smoothedCutoff.skip(buffer.getNumSamples());//Uses entire buffer
        filter.setCutoffFrequency(smoothedCutoff.getCurrentValue());
        smoothedReso.skip(buffer.getNumSamples());//Uses entire buffer
        filter.setResonance(smoothedReso.getCurrentValue());
        juce::dsp::AudioBlock<float> block(buffer); //(kinda) makes a copy of the buffer
        juce::dsp::ProcessContextReplacing context(block);
        filter.process(context);
    }*/

    //Mix sample processing
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        //Assign parameters
        const float output = smoothedOutput.getNextValue();
        const float mix = smoothedMix.getNextValue();
        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            auto* dryInput = dryBuffer.getReadPointer(channel);
            auto* wetInput = buffer.getWritePointer(channel);
            wetInput[sample] = dryInput[sample] * (1.0f - mix / 100) + (mix / 100) * wetInput[sample];
            wetInput[sample] *= output;
        }
    }
    //Compression Starts Here-------------------------------------------------------------------------------------------
    smoothedLowLowerThresh.setCurrentAndTargetValue(state.getRawParameterValue("lowLowerThresh")->load());
    smoothedLowUpperThresh.setCurrentAndTargetValue(state.getRawParameterValue("lowUpperThresh")->load());
    smoothedMidLowerThresh.setCurrentAndTargetValue(state.getRawParameterValue("midLowerThresh")->load());
    smoothedMidUpperThresh.setCurrentAndTargetValue(state.getRawParameterValue("midUpperThresh")->load());
    smoothedHighLowerThresh.setCurrentAndTargetValue(state.getRawParameterValue("highLowerThresh")->load());
    smoothedHighUpperThresh.setCurrentAndTargetValue(state.getRawParameterValue("highUpperThresh")->load());
    smoothedLowInputGain.setCurrentAndTargetValue(state.getRawParameterValue("lowInputGain")->load());
    smoothedLowOutputGain.setCurrentAndTargetValue(state.getRawParameterValue("lowOutputGain")->load());
    smoothedMidInputGain.setCurrentAndTargetValue(state.getRawParameterValue("midInputGain")->load());
    smoothedMidOutputGain.setCurrentAndTargetValue(state.getRawParameterValue("midOutputGain")->load());
    smoothedHighInputGain.setCurrentAndTargetValue(state.getRawParameterValue("highInputGain")->load());
    smoothedHighOutputGain.setCurrentAndTargetValue(state.getRawParameterValue("highOutputGain")->load());
    smoothedCompMasterGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("compMasterGain")->load()));
    smoothedAttack.setCurrentAndTargetValue(state.getRawParameterValue("attack")->load());
    smoothedRelease.setCurrentAndTargetValue(state.getRawParameterValue("release")->load());
    smoothedLowLowerRatio.setCurrentAndTargetValue(state.getRawParameterValue("lowLowerRatio")->load());
    smoothedLowUpperRatio.setCurrentAndTargetValue(state.getRawParameterValue("lowUpperRatio")->load());
    smoothedMidLowerRatio.setCurrentAndTargetValue(state.getRawParameterValue("midLowerRatio")->load());
    smoothedMidUpperRatio.setCurrentAndTargetValue(state.getRawParameterValue("midUpperRatio")->load());
    smoothedHighLowerRatio.setCurrentAndTargetValue(state.getRawParameterValue("highLowerRatio")->load());
    smoothedHighUpperRatio.setCurrentAndTargetValue(state.getRawParameterValue("highUpperRatio")->load());

    if (compressorToggle == CompOn){
        const float lowLowerThresholdDb = smoothedLowLowerThresh.getNextValue();
        const float lowUpperThresholdDb = smoothedLowUpperThresh.getNextValue();
        const float midLowerThresholdDb = smoothedMidLowerThresh.getNextValue();
        const float midUpperThresholdDb = smoothedMidUpperThresh.getNextValue();
        const float highLowerThresholdDb = smoothedHighLowerThresh.getNextValue();
        const float highUpperThresholdDb = smoothedHighUpperThresh.getNextValue();
        const float lowLowerRatio = smoothedLowLowerRatio.getNextValue();
        const float lowUpperRatio = smoothedLowUpperRatio.getNextValue();
        const float midLowerRatio = smoothedMidLowerRatio.getNextValue();
        const float midUpperRatio = smoothedMidUpperRatio.getNextValue();
        const float highLowerRatio = smoothedHighLowerRatio.getNextValue();
        const float highUpperRatio = smoothedHighUpperRatio.getNextValue();
        const float lowInputGain = smoothedLowInputGain.getNextValue();
        const float lowOutputGain = smoothedLowOutputGain.getNextValue();
        const float midInputGain = smoothedMidInputGain.getNextValue();
        const float midOutputGain = smoothedMidOutputGain.getNextValue();
        const float highInputGain = smoothedHighInputGain.getNextValue();
        const float highOutputGain = smoothedHighOutputGain.getNextValue();
        const float masterGain = smoothedCompMasterGain.getNextValue();
        const float attackMs = smoothedAttack.getNextValue();
        const float releaseMs = smoothedRelease.getNextValue();
        const auto sampleRate = static_cast<float>(getSampleRate());

        const auto dbToLinear = [](float db) {
            return std::pow(10.0f, db * 0.05f);
        };

        const auto calculateBandGain = [&](float envelope, float lowerThresholdDb, float upperThresholdDb,
                                          float lowerRatio, float upperRatio) {
            const float lowerThreshold = dbToLinear(lowerThresholdDb);
            const float upperThreshold = dbToLinear(upperThresholdDb);
            const float safeEnvelope = std::max(envelope, 1.0e-6f);

            const float lowerDelta = lowerThreshold / safeEnvelope;
            const float upperDelta = upperThreshold / safeEnvelope;
            const float lowerMult = std::pow(lowerDelta, 1.0f / std::max(1.0f, lowerRatio));
            const float upperMult = std::pow(upperDelta, 1.0f / std::max(1.0f, upperRatio));
            return juce::jlimit(0.0f, 32.0f, upperMult * lowerMult);
        };

        const auto processBand = [&](juce::AudioBuffer<float>& bandBuffer,
                                    float lowerThresholdDb, float upperThresholdDb,
                                    float lowerRatio, float upperRatio,
                                    float inputGainDb, float outputGainDb,
                                    float attackScaleMs, float releaseScaleMs,
                                    std::vector<float>& envelope) {
            const float attackSamples = std::max(1.0f, attackScaleMs * 0.001f * sampleRate);
            const float releaseSamples = std::max(1.0f, releaseScaleMs * 0.001f * sampleRate);
            const float attackCoeff = 1.0f / (attackSamples + 1.0f);
            const float releaseCoeff = 1.0f / (releaseSamples + 1.0f);
            const float inputGain = dbToLinear(inputGainDb);
            const float outputGain = dbToLinear(outputGainDb);

            for (int channel = 0; channel < totalNumInputChannels; ++channel)
            {
                auto* band = bandBuffer.getWritePointer(channel);
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    const float inputSample = band[sample] * inputGain;
                    const float sampleSquared = inputSample * inputSample;
                    float& bandEnvelope = envelope[channel];

                    if (sampleSquared > bandEnvelope)
                        bandEnvelope = (sampleSquared + bandEnvelope * attackSamples) * attackCoeff;
                    else
                        bandEnvelope = (sampleSquared + bandEnvelope * releaseSamples) * releaseCoeff;

                    const float gain = calculateBandGain(bandEnvelope, lowerThresholdDb, upperThresholdDb,
                                                         lowerRatio, upperRatio);
                    band[sample] = inputSample * gain * outputGain;
                }
            }
        };

        juce::AudioBuffer<float> lowBuffer(totalNumInputChannels, buffer.getNumSamples());
        juce::AudioBuffer<float> midBuffer(totalNumInputChannels, buffer.getNumSamples());
        juce::AudioBuffer<float> highBuffer(totalNumInputChannels, buffer.getNumSamples());

        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            lowBuffer.copyFrom(channel, 0, buffer.getReadPointer(channel), buffer.getNumSamples());
            midBuffer.copyFrom(channel, 0, buffer.getReadPointer(channel), buffer.getNumSamples());
            highBuffer.copyFrom(channel, 0, buffer.getReadPointer(channel), buffer.getNumSamples());
        }

        juce::dsp::AudioBlock<float> lowBlock(lowBuffer);
        juce::dsp::ProcessContextReplacing<float> lowContext(lowBlock);
        lowCrossoverWide.process(lowContext);

        juce::dsp::AudioBlock<float> midBlock(midBuffer);
        juce::dsp::ProcessContextReplacing<float> midContext(midBlock);
        highCrossoverWide.process(midContext);
        lowCrossoverNarrow.process(midContext);

        juce::dsp::AudioBlock<float> highBlock(highBuffer);
        juce::dsp::ProcessContextReplacing<float> highContext(highBlock);
        highCrossoverNarrow.process(highContext);

        std::vector<float> lowEnvelope(totalNumInputChannels, 0.0f);
        std::vector<float> midEnvelope(totalNumInputChannels, 0.0f);
        std::vector<float> highEnvelope(totalNumInputChannels, 0.0f);

        processBand(lowBuffer, lowLowerThresholdDb, lowUpperThresholdDb, lowLowerRatio, lowUpperRatio,
                    lowInputGain, lowOutputGain, attackMs * 0.85f, releaseMs * 1.2f, lowEnvelope);
        processBand(midBuffer, midLowerThresholdDb, midUpperThresholdDb, midLowerRatio, midUpperRatio,
                    midInputGain, midOutputGain, attackMs, releaseMs, midEnvelope);
        processBand(highBuffer, highLowerThresholdDb, highUpperThresholdDb, highLowerRatio, highUpperRatio,
                    highInputGain, highOutputGain, attackMs * 1.15f, releaseMs * 0.9f, highEnvelope);

        buffer.clear();
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* output = buffer.getWritePointer(channel);
            auto* low = lowBuffer.getWritePointer(channel);
            auto* mid = midBuffer.getWritePointer(channel);
            auto* high = highBuffer.getWritePointer(channel);

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                output[sample] = low[sample] + mid[sample] + high[sample];
        }

        buffer.applyGain(masterGain);
    }
    visualizerBuffer.makeCopyOf(buffer);//Make copy of buffer to pass into visualizer
}

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), state(*this, nullptr, "parameters", createParameters())
{
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to have a built-in UI or not)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}


juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameters() {
    juce::NormalisableRange cutoffRange(20.0f, 20000.0f);
    cutoffRange.setSkewForCentre(1000.0f);

    return{
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"drive"},
            "drive",juce::NormalisableRange(0.0f, 36.0f),
            0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
                return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"thresh"},
            "thresh", juce::NormalisableRange(-36.0f, 0.0f),
            0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
                return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"output"},
            "output", juce::NormalisableRange(-36.0f, 36.0f),
            0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
                return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"mix"},
        "mix", juce::NormalisableRange(0.0f, 100.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + "%";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"cutoff"},
        "cutoff", cutoffRange,
        20000.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
        return juce::String(value, 2) + "Hz";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"resonance"},
        "resonance", juce::NormalisableRange(0.01f, 6.0f),
        0.7f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2);
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowLowerThresh"},
        "lowLowerThresh", juce::NormalisableRange(-40.00f, 0.0f),
        -40.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowUpperThresh"},
        "lowUpperThresh", juce::NormalisableRange(-40.00f, 0.0f),
        -33.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowLowerRatio"},
        "lowLowerRatio", juce::NormalisableRange(1.0f, 20.0f),
        4.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2);
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowUpperRatio"},
        "lowUpperRatio", juce::NormalisableRange(1.0f, 100.0f),
        66.7f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2);
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midLowerThresh"},
        "midLowerThresh", juce::NormalisableRange(-40.00f, 0.0f),
        -40.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midUpperThresh"},
        "midUpperThresh", juce::NormalisableRange(-40.00f, 0.0f),
        -33.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midLowerRatio"},
        "midLowerRatio", juce::NormalisableRange(1.0f, 20.0f),
        4.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2);
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midUpperRatio"},
        "midUpperRatio", juce::NormalisableRange(1.0f, 100.0f),
        66.7f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2);
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highLowerThresh"},
        "highLowerThresh", juce::NormalisableRange(-40.00f, 0.0f),
        -40.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highUpperThresh"},
        "highUpperThresh", juce::NormalisableRange(-40.00f, 0.0f),
        -33.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highLowerRatio"},
        "highLowerRatio", juce::NormalisableRange(1.0f, 20.0f),
        4.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2);
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highUpperRatio"},
        "highUpperRatio", juce::NormalisableRange(1.0f, 100.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2);
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"attack"},
        "attack", juce::NormalisableRange(0.1f, 500.0f),
        25.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " ms";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"release"},
        "release", juce::NormalisableRange(0.1f, 500.0f),
        150.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " ms";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowInputGain"},
        "lowInputGain", juce::NormalisableRange(-24.0f, 24.0f),
        5.2f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowOutputGain"},
        "lowOutputGain", juce::NormalisableRange(-24.0f, 24.0f),
        10.4f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midInputGain"},
        "midInputGain", juce::NormalisableRange(-24.0f, 24.0f),
        5.2f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midOutputGain"},
        "midOutputGain", juce::NormalisableRange(-24.0f, 24.0f),
        5.7f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highInputGain"},
        "highInputGain", juce::NormalisableRange(-24.0f, 24.0f),
        5.2f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highOutputGain"},
        "highOutputGain", juce::NormalisableRange(-24.0f, 24.0f),
        10.4f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        })),

        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"compMasterGain"},
        "compMasterGain", juce::NormalisableRange(-24.0f, 24.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float value, int){
            return juce::String(value, 2) + " dB";
        }))
    };
}
