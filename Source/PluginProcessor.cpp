/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MBDistProcessor::MBDistProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
         apvts(*this, nullptr, "Parameters", createLayout())
#endif
{
#ifdef OSC
    oscSender = std::make_unique<juce::OSCSender>();
    oscSender->connect(oscIP, oscPortOut);

    for (auto& param : this->getParameters())
    {
        // try static cast to AudioParameterWithID
        try {
            juce::AudioProcessorParameterWithID* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(param);
            if (p != nullptr)
            {
                apvts.addParameterListener (p->getParameterID(), this);
            }
        } catch (...) {
            // do nothing
        }
    }
#else
    
#endif
}

MBDistProcessor::~MBDistProcessor()
{
    
#ifdef OSC
    for (auto& param : this->getParameters())
    {
        // try static cast to AudioParameterWithID
        try {
            juce::AudioProcessorParameterWithID* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(param);
            if (p != nullptr)
            {
                apvts.removeParameterListener (p->getParameterID(), this);
            }
        } catch (...) {
            // do nothing
        }
    }
#endif
}

#ifdef OSC
void MBDistProcessor::parameterChanged (const String& parameterID, float newValue) {
    juce::OSCMessage msg("/parameterChanged");
    msg.addString(parameterID);
    msg.addFloat32(newValue);
    oscSender->send(msg);
}
#endif

const juce::StringArray MBDistProcessor::bandEffects = { "Distortion", "Fuzz", "Overdrive" };

// Create layout function
juce::AudioProcessorValueTreeState::ParameterLayout MBDistProcessor::createLayout()
{
    // Create a vector to hold the parameters
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "Bypass",
        "Bypass",
        false
    ));

    for(int i=0; i<NUM_BANDS; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            "Band" + std::to_string(i + 1),
            "Band " + std::to_string(i + 1) + " Effect",
            bandEffects,
            (int)(i  / 3)
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "Band" + std::to_string(i + 1) + "Gain",
            "Band " + std::to_string(i + 1) + " Gain",
            juce::NormalisableRange<float>(0.0f, 10.0f, 2.0f),
            4.0f
        ));

        //"Band" + std::to_string(i + 1) + "Tone"
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "Band" + std::to_string(i + 1) + "Tone",
            "Band " + std::to_string(i + 1) + " Tone",
            juce::NormalisableRange<float>(0.0f, 10.0f, 2.0f),
            4.0f
        ));
    }

    // Volume
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "OutputVolume",
        "Output Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String MBDistProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MBDistProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MBDistProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MBDistProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MBDistProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MBDistProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MBDistProcessor::getCurrentProgram()
{
    return 0;
}

void MBDistProcessor::setCurrentProgram (int /*index*/)
{
}

const juce::String MBDistProcessor::getProgramName (int /*index*/)
{
    return {};
}

void MBDistProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/)
{
}

//==============================================================================
void MBDistProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
#ifdef OSC
    for (auto& param : this->getParameters())
    {
        // try static cast to AudioParameterWithID
        try {
            juce::AudioProcessorParameterWithID* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(param);
            if (p != nullptr)
            {
                // void MBDistProcessor::parameterChanged (const String& parameterID, float newValue) {
                auto value = apvts.getRawParameterValue(p->getParameterID());
                parameterChanged(p->getParameterID(), value->load());
            }
        } catch (...) {
            // do nothing
        }
    }
#else
    this->_sampleRate = sampleRate;
    inferenceEngine.prepareToPlay(samplesPerBlock);
    monoBuffer.setSize(1, samplesPerBlock);
#endif
}

void MBDistProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MBDistProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
#endif

void MBDistProcessor::refreshLatents()
{
    for (int i = 0; i < NUM_BANDS; ++i)
    {
        std::atomic<float>* bandParam = apvts.getRawParameterValue("Band" + std::to_string(i + 1));
        std::atomic<float>* gainParam = apvts.getRawParameterValue("Band" + std::to_string(i + 1) + "Gain");
        std::atomic<float>* toneParam = apvts.getRawParameterValue("Band" + std::to_string(i + 1) + "Tone");

        // latents[i]
        latentDataFrame.getLatent((int)bandParam->load(),
                                  (int)gainParam->load(),
                                  (int)toneParam->load(),
                                  latents[i]);

    }
}

void MBDistProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    bool bypass = false;
    {
        std::atomic<float>* bypassParam = apvts.getRawParameterValue("Bypass");
        float bypassValue = bypassParam->load();
        bypass = (bypassValue >= 0.5f);
    }
    // buffer.clear();
    if (!bypass) {
        
        inferenceEngine.runInference(buffer, monoBuffer);

        // Copy monoBuffer channel 0 to all output channels
        for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
            auto* writePtr = buffer.getWritePointer(channel);
            auto* readPtr = monoBuffer.getReadPointer(0);
            std::memcpy(writePtr, readPtr, buffer.getNumSamples() * sizeof(float));
        }

        buffer.applyGain(0.5f); // TODO: remove hardcoded gain
    }
}

//==============================================================================
bool MBDistProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MBDistProcessor::createEditor()
{
    return new MBDistEditor (*this);
}

//==============================================================================
void MBDistProcessor::getStateInformation (juce::MemoryBlock& /*destData*/)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MBDistProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MBDistProcessor();
}
