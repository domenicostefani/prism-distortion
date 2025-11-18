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
}

MBDistProcessor::~MBDistProcessor()
{
}

// Create layout function
juce::AudioProcessorValueTreeState::ParameterLayout MBDistProcessor::createLayout()
{
    // Create a vector to hold the parameters
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for(int i=0; i<NUM_BANDS; ++i)
    {
        const int BAND_OPTIONS = 3; // Distortion, Gain, Tone
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            "Band" + std::to_string(i + 1),
            "Band " + std::to_string(i + 1) + " Effect",
            juce::StringArray{ "Distortion", "Fuzz", "Overdrive" },
            0
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

void MBDistProcessor::setCurrentProgram (int index)
{
}

const juce::String MBDistProcessor::getProgramName (int index)
{
    return {};
}

void MBDistProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MBDistProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
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

void MBDistProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
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
void MBDistProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MBDistProcessor::setStateInformation (const void* data, int sizeInBytes)
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
