/*
  Degrade - OnyxDSP
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "laf.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;


//==============================================================================
/**
*/
class MBDistEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    MBDistEditor (MBDistProcessor&);
    ~MBDistEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void showFileMenu(juce::TextButton*);
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MBDistProcessor& audioProcessor;

    std::unique_ptr<juce::Drawable> pedal_svg_drawable,
                                    background_jpg_drawable,
                                    ledOn_png_drawable,
                                    ledOff_png_drawable;
    juce::ComponentBoundsConstrainer constrainer;

    const int ORIGIN_WIDTH = 794;
    const int ORIGIN_HEIGHT = 447;	
    const float ASPECT_RATIO = (float)ORIGIN_WIDTH/ORIGIN_HEIGHT;

    MBDistLaF _MBDistLaF;

    // Linear sliders
    // juce::Slider osc2mix_sld{ "osc2mix_sld"},
    //              noise_sld{ "noise_sld"},
    //              vcfFreq_sld{ "vcfFreq_sld"},
    //              vcfRes_sld{ "vcfRes_sld"},
    //              glideMode_sld{ "glideMode_sld"},
    //              vcfEnv_sld{ "vcfEnv_sld"},
    //              vcfVel_sld{ "vcfVel_sld"},
    //              glideRate_sld{ "glideRate_sld"},
    //              glideBend_sld{ "glideBend_sld"},
    //              lfoRate_sld{ "lfoRate_sld"},
    //              lfoAmt_sld{ "lfoAmt_sld"},
    //              vibratoAmt_sld{ "vibratoAmt_sld"},
    //              vcfEnvA_sld{ "vcfEnvA_sld"},
    //              vcfEnvD_sld{ "vcfEnvD_sld"},
    //              vcfEnvS_sld{ "vcfEnvS_sld"},
    //              vcfEnvR_sld{ "vcfEnvR_sld"},
    //              ampEnvA_sld{ "ampEnvA_sld"},
    //              ampEnvD_sld{ "ampEnvD_sld"},
    //              ampEnvS_sld{ "ampEnvS_sld"},
    //              ampEnvR_sld{ "ampEnvR_sld"};
    // std::unique_ptr<SliderAttachment> driveAttachment,
    //                                   depthAttachment,
    //                                   srAttachment,
    //                                   mixAttachment,
    //                                   glideModeAttachment,
    //                                   vcfEnvAttachment,
    //                                   vcfVelAttachment,
    //                                   glideRateAttachment,
    //                                   glideBendAttachment,
    //                                   lfoRateAttachment,
    //                                   lfoAmtAttachment,
    //                                   vibratoAmtAttachment,
    //                                   vcfEnvAAttachment,
    //                                   vcfEnvDAttachment,
    //                                   vcfEnvSAttachment,
    //                                   vcfEnvRAttachment,
    //                                   ampEnvAAttachment,
    //                                   ampEnvDAttachment,
    //                                   ampEnvSAttachment,
    //                                   ampEnvRAttachment;

    std::array<std::pair<std::string,std::string>,8> bandFrequencies = {{
        {"40", "500"},
        {"500", "1k"},
        {"1k", "1.6k"},
        {"1.6k", "2.7k"},
        {"2.7k", "4.5k"},
        {"4.5k", "7.4k"},
        {"7.4k", "12k"},
        {"12k", "20k"}
    }};


    // 8 band sliders
    std::array<juce::Slider, 8> bandSliders;
    std::array<std::unique_ptr<SliderAttachment>, 8> bandAttachments;

    // 8 pairs of Sliders (rotary) and Labels for gain and tone control
    std::array<juce::Slider, 8> bandGainSliders;
    std::array<juce::Slider, 8> bandToneSliders;

    // attachments for the gain and tone sliders
    std::array<std::unique_ptr<SliderAttachment>, 8> bandGainAttachments;
    std::array<std::unique_ptr<SliderAttachment>, 8> bandToneAttachments;

    // Effect type colors:
    // Overdrive: #6696ed
    // Distortion: #ffd703
    // Fuzz: #ff0303

    const std::map<juce::String, juce::Colour> effectTypeColors = {
        {"Overdrive", juce::Colour::fromString("#886696ed")},
        {"Distortion", juce::Colour::fromString("#88ffd703")},
        {"Fuzz", juce::Colour::fromString("#88ff0303")}
    };

    std::array<juce::Colour, 8> bandColors = {};
                                    
    // Rotary sliders
    juce::Slider octave_knob{ "octave_knob"},
                 osc2tune_knob{ "osc2tune_knob"},
                 osc2fine_knob{ "osc2fine_knob"},
                 tuning_knob{ "tuning_knob"};
    std::unique_ptr<SliderAttachment> octaveAttachment,
                                      osc2tuneAttachment,
                                      osc2fineAttachment,
                                      tuningAttachment;

    juce::TextButton bypassButton{ "bypassButton"};
    std::unique_ptr<ButtonAttachment> bypassAttachment;
    bool bypass = false;

    juce::Label currentProgram, sllinkStatus;
    juce::TextButton programButton;
    InvisibleTextButtonLookAndFeel invisibleButtonLaF;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MBDistEditor)
};

