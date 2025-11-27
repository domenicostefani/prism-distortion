/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "VAEdataframe.h"

#define NUM_BANDS 8   // Number of frequency bands
#define LATENT_SIZE 8 // VAE Latent space size
#include "inference.h"

// #define OSC 


// struct JX10Program
// {
//     JX10Program();
//     JX10Program(const char *name,
//                 float p0,  float p1,  float p2,  float p3,
//                 float p4,  float p5,  float p6,  float p7,
//                 float p8,  float p9,  float p10, float p11,
//                 float p12, float p13, float p14, float p15,
//                 float p16, float p17, float p18, float p19,
//                 float p20, float p21, float p22, float p23);
//     char name[24];
//     float param[NPARAMS];
// };

struct PrismProgram
{
    PrismProgram() {
        effectTypes.fill(0.0f);
        gains.fill(0.0f);
        tones.fill(0.0f);
        outputVolume = 1.0f;

        strcpy(name, "Empty Patch");
    }
    PrismProgram(const char *name,
                 std::array<int, NUM_BANDS> effectTypes,
                 std::array<int, NUM_BANDS> gains,
                 std::array<int, NUM_BANDS> tones,
                 float outputVolume) {                 
        strcpy(this->name, name);
        this->effectTypes = effectTypes;
        this->gains = gains;
        this->tones = tones;
        this->outputVolume = outputVolume;
    }
    char name[24];
    std::array<int, NUM_BANDS> effectTypes;
    std::array<int, NUM_BANDS> gains;
    std::array<int, NUM_BANDS> tones;
    float outputVolume;
};

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

    void createPrograms();


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

    std::vector<PrismProgram> _programs;
    // Index of the active preset.
    int _currentProgram = 0;

    // Smoothed gain
    juce::SmoothedValue<float> smoothedGain;
    float targetGain = 1.0f;


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MBDistProcessor)
};
