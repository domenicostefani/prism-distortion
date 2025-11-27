/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "inference.h"
#include "VAEdataframe.h"

#define NUM_BANDS 8
// #define OSC 

//==============================================================================
/**
*/
class MBDistProcessor  : public juce::AudioProcessor
                        #ifdef OSC
                         ,   AudioProcessorValueTreeState::Listener
                        #endif
{
public:

    //==============================================================================
    MBDistProcessor();
    ~MBDistProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts{ *this, nullptr };
    //createLayout function declaration
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    const static juce::StringArray bandEffects;
#ifdef OSC
    void parameterChanged (const String& parameterID, float newValue) override;
    juce::String oscIP = "127.0.0.1";
    int oscPortOut = 9000;
    // Osc Sender
    std::unique_ptr<juce::OSCSender> oscSender;
#else
    InferenceEngine inferenceEngine;
    std::array<std::array<float, 8>, NUM_BANDS> latents;
    void refreshLatents();
    VAELatentDataFrame latentDataFrame;
#endif

    std::atomic<double> _sampleRate { 44100.0 };
    // Mono buffer
    juce::AudioBuffer<float> monoBuffer;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MBDistProcessor)
};
