/*
  Degrade - OnyxDSP
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "laf.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;


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

    std::unique_ptr<juce::Drawable> background_svg_drawable;
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
        {"40", "500 Hz"},
        {"500", "1 KHz"},
        {"1", "1.6 KHz"},
        {"1.6", "2.7 KHz"},
        {"2.7", "4.5 kHz"},
        {"4.5", "7.4 kHz"},
        {"7.4", "12 kHz"},
        {"12", "20 kHz"}
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
                                    
    // Rotary sliders
    juce::Slider octave_knob{ "octave_knob"},
                 osc2tune_knob{ "osc2tune_knob"},
                 osc2fine_knob{ "osc2fine_knob"},
                 tuning_knob{ "tuning_knob"};
    std::unique_ptr<SliderAttachment> octaveAttachment,
                                      osc2tuneAttachment,
                                      osc2fineAttachment,
                                      tuningAttachment;

    juce::Label currentProgram, sllinkStatus;
    juce::TextButton programButton;
    InvisibleTextButtonLookAndFeel invisibleButtonLaF;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MBDistEditor)
};

