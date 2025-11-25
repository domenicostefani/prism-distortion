/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
// #include "laf.h"

//==============================================================================
MBDistEditor::MBDistEditor (MBDistProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (ORIGIN_WIDTH, ORIGIN_HEIGHT);

    // setResizable(true, true);

    
    constrainer.setFixedAspectRatio(ASPECT_RATIO);
    // Set minimum and maximum sizes if needed
    constrainer.setMinimumWidth(ORIGIN_WIDTH);
    constrainer.setMinimumHeight(ORIGIN_HEIGHT);
    constrainer.setMaximumWidth(ORIGIN_WIDTH * 4);
    constrainer.setMaximumHeight(ORIGIN_HEIGHT * 4);
    setConstrainer(&constrainer);
    
    
    // juce::File back_jpg_file = juce::File("C:/Users/cimil/Develop/paper-ideas/Ardan-JAES-25/MultibandDistortion/Source/Data/background.jpg");
    // background_jpg_drawable = juce::Drawable::createFromImageFile(back_jpg_file);
    background_jpg_drawable = juce::Drawable::createFromImageData(BinaryData::background_jpg,BinaryData::background_jpgSize);

    // juce::File back_svg_file = juce::File("C:/Users/cimil/Develop/paper-ideas/Ardan-JAES-25/MultibandDistortion/Source/Data/backing.svg");
    // pedal_svg_drawable = juce::Drawable::createFromSVGFile(back_svg_file);
    std::unique_ptr<juce::XmlElement> xml_background_svg = juce::XmlDocument::parse(BinaryData::backing_svg); // GET THE SVG AS A XML
    pedal_svg_drawable = juce::Drawable::createFromSVG(*xml_background_svg);

    // juce::File ledOn_png_file = juce::File("C:/Users/cimil/Develop/paper-ideas/Ardan-JAES-25/MultibandDistortion/Source/Data/led-on.png");
    // ledOn_png_drawable = juce::Drawable::createFromImageFile(ledOn_png_file);
    ledOn_png_drawable = juce::Drawable::createFromImageData(BinaryData::ledon_png,BinaryData::ledon_pngSize);

    // juce::File ledOff_png_file = juce::File("C:/Users/cimil/Develop/paper-ideas/Ardan-JAES-25/MultibandDistortion/Source/Data/led-off.png");
    // ledOff_png_drawable = juce::Drawable::createFromImageFile(ledOff_png_file);
    ledOff_png_drawable = juce::Drawable::createFromImageData(BinaryData::ledoff_png,BinaryData::ledoff_pngSize);
    
    // Bypassbutton
    addAndMakeVisible(bypassButton);
    bypassButton.setLookAndFeel(&invisibleButtonLaF);
    bypassAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "Bypass", bypassButton);
    bypassButton.setClickingTogglesState(true);

    for (int i = 0; i < bandSliders.size(); ++i)
    {
        auto& slider = bandSliders[i];
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel(&_MBDistLaF);
        bandAttachments[i] = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Band" + std::to_string(i + 1), slider);

        auto& gainKnob = bandGainSliders[i];
        addAndMakeVisible(gainKnob);
        gainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        gainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        gainKnob.setLookAndFeel(&_MBDistLaF);
        bandGainAttachments[i] = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Band" + std::to_string(i + 1) + "Gain", gainKnob);

        auto& toneKnob = bandToneSliders[i];
        addAndMakeVisible(toneKnob);
        toneKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        toneKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        toneKnob.setLookAndFeel(&_MBDistLaF);
        bandToneAttachments[i] = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Band" + std::to_string(i + 1) + "Tone", toneKnob);
    }


    // addAndMakeVisible(programButton);
    // programButton.setButtonText("Program---");
    // programButton.setLookAndFeel(&invisibleButtonLaF);
    // programButton.onClick = [this]() { showFileMenu(&programButton); };

    Timer::startTimerHz(25); // 25 Hz update rate for Program change to update label
}


MBDistEditor::~MBDistEditor()
{
    // Reset attachments
    // driveAttachment.reset();
    // depthAttachment.reset();
    // srAttachment.reset();
    // mixAttachment.reset();
    // glideModeAttachment.reset();
    // vcfEnvAttachment.reset();
    // vcfVelAttachment.reset();
    // glideRateAttachment.reset();
    // glideBendAttachment.reset();
    // lfoRateAttachment.reset();
    // lfoAmtAttachment.reset();
    // vibratoAmtAttachment.reset();
    // vcfEnvAAttachment.reset();
    // vcfEnvDAttachment.reset();
    // vcfEnvSAttachment.reset();
    // vcfEnvRAttachment.reset();
    // ampEnvAAttachment.reset();
    // ampEnvDAttachment.reset();
    // ampEnvSAttachment.reset();
    // ampEnvRAttachment.reset();
    // octaveAttachment.reset();
    // osc2tuneAttachment.reset();
    // osc2fineAttachment.reset();
    // tuningAttachment.reset();
    for (int i = 0; i < bandSliders.size(); ++i)
    {
        bandAttachments[i].reset();
        bandGainAttachments[i].reset();
        bandToneAttachments[i].reset();
    }
}
//==============================================================================
void MBDistEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto area = getLocalBounds();
    background_jpg_drawable->drawWithin(
        g, 
        area.toFloat(), 
        juce::RectanglePlacement::fillDestination
        | juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yTop,
        1.0f
    );

    pedal_svg_drawable->drawWithin(
        g, 
        area.toFloat(), 
        juce::RectanglePlacement::fillDestination
        | juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yTop,
        1.0f
    );

    // juce::Colour ledColour = bypass ? juce::Colour::fromString("#ff3c3c3c") : juce::Colour::fromString("#ffcb7bdd");
    const int lexX = 440, ledY = 320;
    juce::Rectangle<int> ledArea(lexX, ledY, 18, 18);
    // g.setColour(ledColour);
    // g.fillEllipse(lexX, ledY, 15, 15);
    // g.setColour(juce::Colours::black);
    // g.drawEllipse(lexX, ledY, 15, 15, 1.0f);

    if (bypass)
    {
        ledOff_png_drawable->drawWithin(
            g, 
            ledArea.toFloat(), 
            juce::RectanglePlacement::fillDestination
            | juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yTop,
            1.0f);
    } else {
        ledOn_png_drawable->drawWithin(
            g, 
            ledArea.toFloat(), 
            juce::RectanglePlacement::fillDestination
            | juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yTop,
            1.0f
        );
    }

    const int BAND_WIDTH = 18;
    const int BAND_HEIGHT = 90;

    const int GAIN_KNOB_SIZE = 30;

    // first band slider at (197,161), all the other +71 x
    int bandX = 200, bandY = 160, offsetX = 68;
    // First gain knob at (191, 63) size 31x31, offsetX = 71
    int gainKnobX = bandX - (GAIN_KNOB_SIZE-BAND_WIDTH)/2, 
        gainKnobY = 47;

    // First tone knob at (191, 102) size 31x31, offset
    int toneKnobX = gainKnobX, toneKnobY = 102, toneKnobSize = 31;

    int textY = bandY + BAND_HEIGHT + 10;
    const int margin = 10;
    for (int i = 0; i < bandSliders.size(); ++i)
    {
        // g.setColour(juce::Colour::fromString("#ffd1d1d1"));
        g.setColour(bandColors[i]);

        juce::Rectangle<int> gainToneArea(gainKnobX, gainKnobY, jmax(GAIN_KNOB_SIZE, toneKnobSize), (toneKnobY + toneKnobSize) - gainKnobY);
        auto expGainToneArea = gainToneArea.expanded(margin, margin).toFloat();
        g.fillRoundedRectangle(expGainToneArea, margin/2.0f);

        bandGainSliders[i].setBounds(juce::Rectangle<int>(gainKnobX, gainKnobY, GAIN_KNOB_SIZE, GAIN_KNOB_SIZE));
        gainKnobX += offsetX;

        bandToneSliders[i].setBounds(juce::Rectangle<int>(toneKnobX, toneKnobY, toneKnobSize, toneKnobSize));
        toneKnobX += offsetX;
        
        g.fillRoundedRectangle(expGainToneArea.getX(),
                               bandY - margin,
                               expGainToneArea.getWidth(),
                               BAND_HEIGHT + 2 * margin,
                               margin/2.0f);
        bandSliders[i].setBounds(juce::Rectangle<int>(bandX, bandY, BAND_WIDTH, BAND_HEIGHT));

        int TEXT_HEIGHT = 15;
        g.setColour(juce::Colours::black);
        g.setFont(_MBDistLaF.mainFont.withHeight(TEXT_HEIGHT));
        g.drawText(bandFrequencies[i].first,
                   bandX - 40,
                   textY+10,
                   BAND_WIDTH + 20,
                   15,
                   juce::Justification::left);
        if (i == bandSliders.size() -1)
        {
            g.drawText(bandFrequencies[i].second,
                   bandX + 10,
                   textY + 10,
                   BAND_WIDTH + 30,
                   15,
                   juce::Justification::right);
        }

        bandX += offsetX;
    }

    const int buttonX = 383, buttonY = 340, buttonW = 55, buttonH = 55;
    bypassButton.setBounds(juce::Rectangle<int>(buttonX, buttonY, buttonW, buttonH));

    // Calculate scale factors
    float scaleX = area.getWidth() / (float)ORIGIN_WIDTH;
    float scaleY = area.getHeight() / (float)ORIGIN_HEIGHT;
    
    // Use the smaller scale factor to maintain aspect ratio
    float scale = juce::jmin(scaleX, scaleY);
    
    // Center the scaled content
    float scaledWidth = ORIGIN_WIDTH * scale;
    float scaledHeight = ORIGIN_HEIGHT * scale;
    float xOffset = (area.getWidth() - scaledWidth) * 0.5f;
    float yOffset = (area.getHeight() - scaledHeight) * 0.5f;
    
    // Create and apply transform
    juce::AffineTransform transform = juce::AffineTransform::scale(scale)
                                        .translated(xOffset, yOffset);

    // Apply the transform to components
    for (int i = 0; i < bandSliders.size(); ++i)
    {
        bandSliders[i].setTransform(transform);
        bandGainSliders[i].setTransform(transform);
        bandToneSliders[i].setTransform(transform);
    }
}

void MBDistEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    // driveSlider->setBounds(0, 0, 50, getHeight());
    // driveLabel->setBounds(0, getHeight() - 20, getWidth() / 3, 20);
    repaint();
    
}

void MBDistEditor::timerCallback()
{
    for (int i = 0; i < bandSliders.size(); ++i)
    {
        // Assign colors based on Band[1-8] parameter
        std::atomic<float>* bandParam = audioProcessor.apvts.getRawParameterValue("Band" + std::to_string(i + 1));
        float bandValue = bandParam->load();

        juce::String effectType = MBDistProcessor::bandEffects[(int)bandValue];
        juce::Colour effectColor = effectTypeColors.at(effectType.toStdString());
        bandColors[i] = effectColor;
    }

    // Bypass
    std::atomic<float>* bypassParam = audioProcessor.apvts.getRawParameterValue("Bypass");
    float bypassValue = bypassParam->load();
    
    bypass = (bypassValue >= 0.5f);

    repaint();
}

void MBDistEditor::showFileMenu(juce::TextButton* button)
{
    if (button == &programButton)
    {
        // Handle file button actions
        juce::PopupMenu menu;
        menu.setLookAndFeel(&_MBDistLaF);
        // Set colour black on white

        // menu.addItem(1, "Save Preset");
        // menu.addItem(2, "Load Preset");
        // menu.addItem(1, "Program");
        // menu.addSeparator();
        // menu.addItem(3, "Info");

        for (int i = 0; i < audioProcessor.getNumPrograms(); ++i) {
            menu.addItem(i + 1, audioProcessor.getProgramName(i));
        }

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&programButton),
            [this](int result) {
                if (result > 0 && result <= audioProcessor.getNumPrograms()) {
                    audioProcessor.setCurrentProgram(result - 1);
                }
            });

    }
    // else if (button == &editButton)
    // {
    //     // Handle edit button actions
    //     juce::PopupMenu menu;
    //     menu.setLookAndFeel(&degradeLookAndFeel);
    //     menu.addItem(1, "Undo");
    //     menu.addItem(2, "Redo");

    //     menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&editButton),
    //         [this](int result) {
    //             if (result == 1) {
    //                 // Handle Undo
    //                 audioProcessor.performUndo();
    //             }
    //             else if (result == 2) {
    //                 // Handle Redo
    //                 audioProcessor.performRedo();
    //             }
    //         });
    // }
    // else if (button == &viewButton)
    // {
    //     // Handle view button actions
    //     juce::PopupMenu menu;
    //     menu.setLookAndFeel(&degradeLookAndFeel);
    //     menu.addItem(1, "Zoom In");
    //     menu.addItem(2, "Zoom Out");
    //     menu.addItem(3, "Reset View");

    //     menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&viewButton),
    //         [this](int result) {
    //             if (result == 1) {
    //                 // Handle Zoom In by increasing the size of the plugin window
    //                 float increase = 0.1f; // Increase size by 10%
    //                 this->setSize(
    //                     static_cast<int>(getWidth() * (1.0f + increase)),
    //                     static_cast<int>(getHeight() * (1.0f + increase))
    //                 );
    //             }
    //             else if (result == 2) {
    //                 // Handle Zoom Out
    //                 float decrease = 0.1f; // Decrease size by 10%
    //                 this->setSize(
    //                     static_cast<int>(getWidth() * (1.0f - decrease)),
    //                     static_cast<int>(getHeight() * (1.0f - decrease))
    //                 );
    //             }
    //             else if (result == 3) {
    //                 // Handle Reset View
    //                 this->setSize(ORIGIN_WIDTH, ORIGIN_HEIGHT); // Reset to original size
    //             }
    //         });
    // }
    // else if (button == &specialButton)
    // {
    //     // Handle special button actions
    //     juce::PopupMenu menu;
    //     menu.setLookAndFeel(&degradeLookAndFeel);
    //     menu.addItem(1, "You are special ;)");

    //     menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&specialButton),
    //         [this](int result) {
    //             if (result == 1) {
                    
    //             }
    //         });
    // }
}