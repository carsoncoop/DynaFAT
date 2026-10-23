#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>


//==============================================================================



void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //May not need a spec
    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();



    // SmoothedValue Preparation Lambda
    auto prepareSmoothed = [&](juce::SmoothedValue<float>& s, const juce::String& paramId,
        float timeMs = 0.01f, std::function<float(float)> transform = {})
    {
        s.reset (sampleRate, timeMs);
        if (auto* p = state.getRawParameterValue(paramId))
        {
            const float v = p->load();
            s.setTargetValue(transform ? transform(v) : v);
        }
    };

    const auto numProcessingChannels = std::max(getTotalNumInputChannels(), getTotalNumOutputChannels());

    //Envelop Follower Preparation--------------------------------------------------------------------------------------
    prepareSmoothed(smoothedEnvAttack, "envAttack");
    prepareSmoothed(smoothedEnvRelease, "envRelease");
    prepareSmoothed(smoothedEnvDynamix, "envDynamix");

    preEnvelopeFollower.prepare(spec, smoothedEnvAttack.getCurrentValue(),
        smoothedEnvRelease.getCurrentValue(), smoothedEnvDynamix.getNextValue());
    postEnvelopeFollower.prepare(spec, smoothedEnvAttack.getCurrentValue(),
        smoothedEnvRelease.getCurrentValue(), smoothedEnvDynamix.getNextValue());


    compressor.prepare(spec,
        state.getRawParameterValue("compAttack")->load(),
        state.getRawParameterValue("compRelease")->load(),
        state.getRawParameterValue("compThreshHigh")->load(),
        state.getRawParameterValue("compThreshLow")->load(),
        state.getRawParameterValue("compressorPasses")->load(),
        state.getRawParameterValue("compRatio")->load()
        );

    distortion.prepare(getSampleRate(), juce::Decibels::decibelsToGain(state.getRawParameterValue("drive")->load()),
        juce::Decibels::decibelsToGain(state.getRawParameterValue("thresh")->load()),
        state.getRawParameterValue("mix")->load(),
        juce::Decibels::decibelsToGain(state.getRawParameterValue("output")->load()),
        juce::roundToInt(state.getRawParameterValue("distortionPasses")->load()));



    //General preparation-----------------------------------------------------------------------------------------------
    prepareSmoothed(smoothedMasterGain, "masterGain");

    //Filter preparation------------------------------------------------------------------------------------------------
    /*filter.prepare();
    smoothedCutoff.reset(sampleRate, 0.01f);
    smoothedReso.reset(sampleRate, 0.01f);
    smoothedCutoff.setTargetValue(state.getRawParameterValue("cutoff")->load());
    smoothedReso.setTargetValue(state.getRawParameterValue("resonance")->load());*/


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
    smoothedEnvDynamix.setTargetValue(state.getRawParameterValue("envDynamix")->load());

    preEnvelopeFollower.setEnvAttack(smoothedEnvAttack.getNextValue());
    preEnvelopeFollower.setEnvRelease(smoothedEnvRelease.getNextValue());
    postEnvelopeFollower.setEnvAttack(smoothedEnvAttack.getNextValue());
    postEnvelopeFollower.setEnvRelease(smoothedEnvRelease.getNextValue());

    //Distortion--------------------------------------------------------------------------------------------------------
    distortion.setDrive(juce::Decibels::decibelsToGain(state.getRawParameterValue("drive")->load()));
    distortion.setThresh(juce::Decibels::decibelsToGain(state.getRawParameterValue("thresh")->load()));
    distortion.setOutput(juce::Decibels::decibelsToGain(state.getRawParameterValue("output")->load()));
    distortion.setMix(state.getRawParameterValue("mix")->load());
    distortion.setNumPasses(juce::roundToInt(state.getRawParameterValue("distortionPasses")->load()));

    //Compressor--------------------------------------------------------------------------------------------------------
    compressor.setEnvAttack(state.getRawParameterValue("compAttack")->load());
    compressor.setEnvRelease(state.getRawParameterValue("compRelease")->load());
    compressor.setThreshHigh(state.getRawParameterValue("compThreshHigh")->load());
    compressor.setThreshLow(state.getRawParameterValue("compThreshLow")->load());
    compressor.setRatio(state.getRawParameterValue("compRatio")->load());
    compressor.setNumPasses(juce::roundToInt(state.getRawParameterValue("compressorPasses")->load()));

    //General-----------------------------------------------------------------------------------------------------------
    smoothedMasterGain.setTargetValue(state.getRawParameterValue("masterGain")->load());

    /*//Filter------------------------------------------------------------------------------------------------------------
    smoothedCutoff.setTargetValue(state.getRawParameterValue("cutoff")->load());
    smoothedReso.setTargetValue(state.getRawParameterValue("resonance")->load());
    filter.setResonance(smoothedReso.getNextValue());
    filter.setCutoff(smoothedCutoff.getNextValue());*/

    /*if (filter.getActivation() && filter.getFilterOrder() == Pre) {
        smoothedCutoff.skip(buffer.getNumSamples());
        filter.setCutoff(smoothedCutoff.getCurrentValue());
        smoothedReso.skip(buffer.getNumSamples());
        filter.setResonance(smoothedReso.getCurrentValue());
        filter.myProcess(juce::dsp::AudioBlock<float>(buffer));
    }*/

    //Distortion processing
    distortion.distProcess(buffer);

    //Compressor processing
    if (compressor.getActivation()) {
        for (int s = 0; s < buffer.getNumSamples(); s++) {
            postEnvelopeFollower.setEnvDynamix(smoothedEnvDynamix.getNextValue());
            for (int c = 0; c < totalNumInputChannels; c++) {
                buffer.setSample(c, s, compressor.compProcessSample(buffer.getSample(c, s), c));
            }
        }
    }

    //Master gain
    for (int s = 0; s < buffer.getNumSamples(); s++) {
        const float masterGain = juce::Decibels::decibelsToGain(smoothedMasterGain.getNextValue());
        for (int c = 0; c < totalNumInputChannels; c++) {
            buffer.setSample(c, s, buffer.getSample(c, s) * masterGain);
        }
    }

    distortion.setDistortionType (static_cast<DistortionType> (
    juce::jlimit (0, 3, (int) std::round (state.getRawParameterValue ("algButton")->load()))));

    /*
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        // compute master gain for this sample (smoothed in dB then converted to linear)
        const float masterGain = juce::Decibels::decibelsToGain(smoothedMasterGain.getNextValue());
        //Assign parameters to class variables
        distortion.setDrive(smoothedDrive.getNextValue());
        distortion.setThresh(smoothedThresh.getNextValue());
        distortion.setMix(smoothedMix.getNextValue());
        distortion.setOutput(smoothedOutput.getNextValue());

        postEnvelopeFollower.setEnvDynamix(smoothedEnvDynamix.getNextValue());

        compressor.setThreshHigh(smoothedCompThreshHigh.getNextValue());
        compressor.setThreshLow(smoothedCompThreshLow.getNextValue());
        compressor.setRatio(smoothedCompRatio.getNextValue());

        //Channel processing
        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            auto* input = buffer.getWritePointer(channel);
            if (postEnvelopeFollower.getActivation()) {preEnvelopeFollower.followEnv(input[sample], channel);}

            if (routing == DistFirst) {

                input[sample] = distortion.process(input[sample]);
                if (compressor.getActivation()) { input[sample] = compressor.compProcessSample(input[sample], channel);}

            }

            else if (routing == CompFirst) {

                if (compressor.getActivation()) {input[sample] = compressor.compProcessSample(input[sample], channel);}
                input[sample] = distortion.process(input[sample]);

            }

            //Post processing envelope

            //*Gain match goes here*
            if (postEnvelopeFollower.getActivation() == true) {

                postEnvelopeFollower.followEnv(input[sample], channel);

                //Use correction gain in envelope follower
                const float postEnvSignal = input[sample] *
                    postEnvelopeFollower.computeCorrectionGain(preEnvelopeFollower.getEnvPerChannel()[channel], channel);

                //Use Dynamix for envelope follower
                input[sample] = postEnvelopeFollower.useDynamix(input[sample],
                    postEnvSignal);

            }
            input[sample] *= masterGain;
        }
    }
    */

    // if (filter.getActivation() && filter.getFilterOrder() == Post) {
    //     smoothedCutoff.skip(buffer.getNumSamples());
    //     filter.setCutoff(smoothedCutoff.getCurrentValue());
    //     smoothedReso.skip(buffer.getNumSamples());
    //     filter.setResonance(smoothedReso.getCurrentValue());
    //     filter.myProcess(juce::dsp::AudioBlock<float>(buffer));
    // }

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
    //
    // as intermediaries to make it easy to save and load complex data.

    if (auto xml = state.copyState().createXml()) {
        copyXmlToBinary (*xml, destData);
    }
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    if (auto xml = getXmlFromBinary (data, sizeInBytes)) {
        if (xml->hasTagName (state.state.getType())) {
            state.replaceState (juce::ValueTree::fromXml (*xml));
        }
    }
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}


juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto addFloatParam = [&layout](const juce::String& id,
                                  const juce::String& name,
                                  const juce::NormalisableRange<float>& range,
                                  float defaultValue,
                                  const juce::String& suffix = {})
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { id },
            name,
            range,
            defaultValue,
            juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction([suffix](float value, int) {
                    return juce::String(value, 2) + suffix;
                })));
    };

    auto addIntParam = [&layout](const juce::String& id,
                              const juce::String& name,
                              const juce::NormalisableRange<int>& range,
                              int defaultValue,
                              const juce::String& suffix = {})
    {
        layout.add(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID { id, 1 },
            name,
            range.start,
            range.end,
            defaultValue,
            juce::AudioParameterIntAttributes().withLabel(suffix)));
    };

    auto addBoolParam = [&](const juce::String& id, const juce::String& name, bool def) {
        layout.add(std::make_unique<juce::AudioParameterBool>(id, name, def));
    };



    auto addChoiceParam = [&](const juce::String& id, const juce::String& name,
        const juce::StringArray& choices, int defaultIndex) {
        layout.add(std::make_unique<juce::AudioParameterChoice>(id, name, choices, defaultIndex));
    };

    juce::NormalisableRange<float> cutoffRange(20.0f, 20000.0f);
    cutoffRange.setSkewForCentre(1000.0f);

    addFloatParam("drive", "drive", juce::NormalisableRange<float>(0.0f, 36.0f), 0.0f, " dB");
    addFloatParam("thresh", "thresh", juce::NormalisableRange<float>(-36.0f, 0.0f), 0.0f, " dB");
    addFloatParam("output", "output", juce::NormalisableRange<float>(-36.0f, 36.0f), 0.0f, " dB");
    addFloatParam("mix", "mix", juce::NormalisableRange<float>(0.0f, 100.0f), 100.0f, "%");
    //addFloatParam("cutoff", "cutoff", cutoffRange, 20000.0f, "Hz");
    //addFloatParam("resonance", "resonance", juce::NormalisableRange<float>(0.01f, 6.0f), 0.7f, "");
    addFloatParam("envAttack", "envAttack", juce::NormalisableRange<float>(1.0f, 200.0f), 5.0f, " ms");
    addFloatParam("envRelease", "envRelease", juce::NormalisableRange<float>(1.0f, 200.0f), 5.0f, " ms");
    addFloatParam("envDynamix", "envDynamix", juce::NormalisableRange<float>(0.0f, 100.0f), 100.0f, " %");
    addFloatParam("compThreshHigh", "compThreshHigh", juce::NormalisableRange<float>(-36.0f, 0.0f), -6.0f, " dB");
    addFloatParam("compThreshLow", "compThreshLow", juce::NormalisableRange<float>(-36.0f, 0.0f), -12.0f, " dB");
    addFloatParam("compRatio", "compRatio", juce::NormalisableRange<float>(1.0f, 10.0f), 3.0f, ":1");
    addFloatParam("compAttack", "compAttack", juce::NormalisableRange<float>(1.0f, 200.0f), 15.0f, " ms");
    addFloatParam("compRelease", "compRelease", juce::NormalisableRange<float>(2.0f, 200.0f), 15.0f, " ms");
    addFloatParam("masterGain", "masterGain", juce::NormalisableRange<float>(-36.0f, 36.0f), 0.0f, " dB");

    addIntParam("distortionPasses", "distortionPasses", juce::NormalisableRange<int>(1, 4), 1, "");
    addIntParam("compressorPasses", "compressorPasses", juce::NormalisableRange<int>(1, 4), 1, "");

    addChoiceParam("algButton", "algButton", {"Soft Clip", "Hard Clip", "Foldback", "Downsample"}, 0);
    addChoiceParam("routingButton", "routingButton", {"Distortion -> Compression", "Compression -> Distortion"}, 0);
    addBoolParam("compressorButton", "compressorButton", true);
    addBoolParam("envelopeButton", "envelopeButton", true);

    return layout;
}
