#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters.h"
#include <cmath>


//==============================================================================



void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();



    //Envelop Follower Preparation--------------------------------------------------------------------------------------
    smoothedEnvAttack.reset(sampleRate, 0.01f);
    smoothedEnvRelease.reset(sampleRate, 0.01f);
    smoothedEnvAttack.setTargetValue(state.getRawParameterValue("envAttack")->load());
    smoothedEnvRelease.setTargetValue(state.getRawParameterValue("envRelease")->load());
    preEnvelopeFollower.prepare(getSampleRate(), getTotalNumInputChannels(),
        smoothedEnvAttack.getCurrentValue(), smoothedEnvRelease.getCurrentValue());
    postEnvelopeFollower.prepare(getSampleRate(), getTotalNumInputChannels(),
        smoothedEnvAttack.getCurrentValue(), smoothedEnvRelease.getCurrentValue());

    //Compressor Preparation--------------------------------------------------------------------------------------------
    compressor.prepare(getSampleRate(), getTotalNumInputChannels(),
        smoothedCompThresh.getCurrentValue(), smoothedCompRatio.getCurrentValue(),
        smoothedCompAttack.getCurrentValue(), smoothedCompRelease.getCurrentValue());

    smoothedCompThresh.reset(sampleRate, 0.01f);
    smoothedCompRatio.reset(sampleRate, 0.01f);
    smoothedCompAttack.reset(sampleRate, 0.01f);
    smoothedCompRelease.reset(sampleRate, 0.01f);
    smoothedCompThresh.setTargetValue(state.getRawParameterValue("compThresh")->load());
    smoothedCompRatio.setTargetValue(state.getRawParameterValue("compRatio")->load());
    smoothedCompAttack.setTargetValue(state.getRawParameterValue("compAttack")->load());
    smoothedCompRelease.setTargetValue(state.getRawParameterValue("compRelease")->load());

    //Distortion Preparation--------------------------------------------------------------------------------------------
    distortion.prepare(getSampleRate(),
        smoothedDrive.getCurrentValue(), smoothedThresh.getCurrentValue(),
        smoothedMix.getCurrentValue(), smoothedOutput.getCurrentValue());

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

    //Multiband Preparation---------------------------------------------------------------------------------------------
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
    //Envelope Follower-------------------------------------------------------------------------------------------------
    smoothedEnvAttack.setTargetValue(state.getRawParameterValue("envAttack")->load());
    smoothedEnvRelease.setTargetValue(state.getRawParameterValue("envRelease")->load());
    preEnvelopeFollower.setEnvAttack(smoothedEnvAttack.getNextValue());
    preEnvelopeFollower.setEnvRelease(smoothedEnvRelease.getNextValue());
    postEnvelopeFollower.setEnvAttack(smoothedEnvAttack.getNextValue());
    postEnvelopeFollower.setEnvRelease(smoothedEnvRelease.getNextValue());

    //Distortion--------------------------------------------------------------------------------------------------------

    //Takes linear gain value, and converts it to decibel representation
    smoothedDrive.setTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("drive")->load()));
    smoothedThresh.setTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("thresh")->load()));
    smoothedOutput.setTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("output")->load()));
    smoothedMix.setTargetValue(state.getRawParameterValue("mix")->load());
    //smoothedCutoff.setTargetValue(state.getRawParameterValue("cutoff")->load());
    //smoothedReso.setTargetValue(state.getRawParameterValue("resonance")->load());

    //Compressor--------------------------------------------------------------------------------------------------------
    smoothedCompAttack.setTargetValue(state.getRawParameterValue("compAttack")->load());
    smoothedCompRelease.setTargetValue(state.getRawParameterValue("compRelease")->load());
    smoothedCompThresh.setTargetValue(state.getRawParameterValue("compThresh")->load());
    smoothedCompRatio.setTargetValue(state.getRawParameterValue("compRatio")->load());

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

    //Used for sample & hold
    float hold = 0;
    float counter = 0;
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        //Assign parameters to class variables
        distortion.setDrive(smoothedDrive.getNextValue());
        distortion.setThresh(smoothedThresh.getNextValue());
        distortion.setMix(smoothedMix.getNextValue());
        distortion.setOutput(smoothedOutput.getNextValue());

        preEnvelopeFollower.setEnvAttack(smoothedEnvAttack.getNextValue());
        preEnvelopeFollower.setEnvRelease(smoothedEnvRelease.getNextValue());

        compressor.setEnvAttack(smoothedCompAttack.getNextValue());
        compressor.setEnvRelease(smoothedCompRelease.getNextValue());
        compressor.setThresh(smoothedCompThresh.getNextValue());
        compressor.setRatio(smoothedCompRatio.getNextValue());

        //Channel processing
        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            auto* input = buffer.getWritePointer(channel);
            preEnvelopeFollower.followEnv(input[sample], channel);
            //Distortion
            if (distortion.getDistortionType() == Downsample) {
                float dryInput = input[sample];
                if (counter == 0) {
                    hold = input[sample];
                }
                input[sample] = hold;
                counter += 0.5;
                if (counter >= static_cast<int>(juce::Decibels::gainToDecibels(distortion.getDrive()))) {
                    counter = 0;
                }
                input[sample] *= distortion.getOutput();
                float mix = distortion.getMix();
                input[sample] = dryInput * (1.0f - mix / 100) + (mix / 100) * input[sample];
            }
            else {
                input[sample] = distortion.process(input[sample]);
            }
            //Compression
            compressor.followEnv(input[sample], channel);
            input[sample] *= compressor.computeGainChange(channel);


            //Post processing envelope
            postEnvelopeFollower.followEnv(input[sample], channel);
            //*Gain match goes here*
            if (postEnvelopeFollower.getActivation() == true) {
                input[sample] *= postEnvelopeFollower.computeCorrectionGain(preEnvelopeFollower.getEnvPerChannel()[channel], channel);
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

    visualizerBuffer.makeCopyOf(buffer);
}

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      ), state(*this, nullptr, "parameters", createParameters()), distortion() {
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
    juce::NormalisableRange<float> cutoffRange(20.0f, 20000.0f);
    cutoffRange.setSkewForCentre(1000.0f);

    std::vector<ParamSpec> specs{
        ParamSpec("drive", "drive", juce::NormalisableRange<float>(-36.0f, 36.0f), 0.0f,
            [](float value,int){ return juce::String(value, 2) + " dB"; }),
        ParamSpec("thresh", "thresh", juce::NormalisableRange<float>(-36.0f, 0.0f), 0.0f,
            [](float value,int){ return juce::String(value, 2) + " dB"; }),
        ParamSpec("output", "output", juce::NormalisableRange<float>(-36.0f, 36.0f), 0.0f,
            [](float value,int){ return juce::String(value, 2) + " dB"; }),
        ParamSpec("mix", "mix", juce::NormalisableRange<float>(0.0f, 100.0f), 100.0f,
            [](float value,int){ return juce::String(value, 2) + "%"; }),
        ParamSpec("cutoff", "cutoff", cutoffRange, 20000.0f,
            [](float value,int){ return juce::String(value, 2) + "Hz"; }),
        ParamSpec("resonance", "resonance", juce::NormalisableRange<float>(0.01f, 6.0f), 0.7f,
            [](float value,int){ return juce::String(value, 2); }),
        ParamSpec("envAttack", "envAttack", juce::NormalisableRange<float>(1.0f, 200.0f), 15.0f,
            [](float value,int){ return juce::String(value, 1) + " ms"; }),
        ParamSpec("envRelease", "envRelease", juce::NormalisableRange<float>(1.0f, 200.0f), 15.0f,
            [](float value,int){ return juce::String(value, 1) + " ms"; }),
        ParamSpec("compThresh", "compThresh", juce::NormalisableRange<float>(-36.0f, 0.0f), -6.0f,
            [](float value,int){ return juce::String(value, 2) + " dB"; }),
        ParamSpec("compRatio", "compRatio", juce::NormalisableRange<float>(1.0f, 10.0f), 3.0f,
            [](float value,int){ return juce::String(value, 2) + ":1"; }),
        ParamSpec("compAttack", "compAttack", juce::NormalisableRange<float>(1.0f, 200.0f), 15.0f,
            [](float value,int){ return juce::String(value, 1) + " ms"; }),
        ParamSpec("compRelease", "compRelease", juce::NormalisableRange<float>(1.0f, 200.0f), 15.0f,
            [](float value,int){ return juce::String(value, 1) + " ms"; })
    };

    return createParametersFromSpecs(specs);
}
