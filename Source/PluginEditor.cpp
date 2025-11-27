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

    setResizable(true, true);

    
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

    //websiteButton
    addAndMakeVisible(websiteButton);
    websiteButton.setLookAndFeel(&invisibleButtonLaF);
    websiteButton.onClick = []() {
        juce::URL url("https://www.github.com/domenicostefani/prism-plugin");
        url.launchInDefaultBrowser();
    };

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

    // Volume slider
    addAndMakeVisible(volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setLookAndFeel(&_MBDistLaF);
    volumeAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "OutputVolume", volumeSlider);

    addAndMakeVisible(programButton);
    programButton.setButtonText("Program---");
    programButton.setLookAndFeel(&invisibleButtonLaF);
    programButton.onClick = [this]() { showFileMenu(&programButton); };

    addAndMakeVisible(currentProgram);
    currentProgram.setFont(_MBDistLaF.mainFont);
    currentProgram.setJustificationType(juce::Justification::centredLeft);
    currentProgram.setColour(juce::Label::textColourId, juce::Colours::white);


    Timer::startTimerHz(25); // 25 Hz update rate for Program change to update label
}


MBDistEditor::~MBDistEditor()
{
    for (int i = 0; i < bandSliders.size(); ++i)
    {
        bandAttachments[i].reset();
        bandGainAttachments[i].reset();
        bandToneAttachments[i].reset();
    }
    volumeAttachment.reset();
    bypassAttachment.reset();
}
//==============================================================================
void MBDistEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto area = getLocalBounds();
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

    auto footerArea = area.removeFromBottom(static_cast<int>(FOOTER_HEIGHT * scale));

    // Create and apply transform
    juce::AffineTransform transform = juce::AffineTransform::scale(scale)
                                        .translated(xOffset, yOffset);

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
    int lexX = 440, ledY = 320;
    // affine transform for led
    transform.transformPoint(lexX, ledY);
    int ledW = 18, ledH = 18;
    transform.transformPoint(ledW, ledH);
    juce::Rectangle<int> ledArea(lexX, ledY, ledW, ledH);
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
    int bandX = 200, bandY = 160, offsetX = 60;
    // First gain knob at (191, 63) size 31x31, offsetX = 71
    int gainKnobX = bandX - (GAIN_KNOB_SIZE-BAND_WIDTH)/2, 
        gainKnobY = 47;

    // First tone knob at (191, 102) size 31x31, offset
    int toneKnobX = gainKnobX, toneKnobY = 102, toneKnobSize = 31;

    int textY = bandY + BAND_HEIGHT + 10;
    const int margin = 7;
    
    int VOL_SLIDER_X = 0;
    for (int i = 0; i < bandSliders.size(); ++i)
    {
        // g.setColour(juce::Colour::fromString("#ffd1d1d1"));
        g.setColour(bandColors[i]);

        juce::Rectangle<int> gainToneArea(gainKnobX, gainKnobY, jmax(GAIN_KNOB_SIZE, toneKnobSize), (toneKnobY + toneKnobSize) - gainKnobY);
        auto expGainToneArea = gainToneArea.expanded(margin, margin).toFloat();

        float gt_x = static_cast<float>(expGainToneArea.getX());
        float gt_y = static_cast<float>(expGainToneArea.getY());
        float gt_w = static_cast<float>(expGainToneArea.getWidth());
        float gt_h = static_cast<float>(expGainToneArea.getHeight());
        transform.transformPoints(gt_x, gt_y, gt_w, gt_h);
        auto transformedExpGainToneArea = juce::Rectangle<float>(gt_x, gt_y, gt_w, gt_h);
        int cmargin = margin, cmargin2 = margin;
        transform.transformPoint(cmargin, cmargin2);
        g.fillRoundedRectangle(transformedExpGainToneArea, cmargin/2.0f);

        bandGainSliders[i].setBounds(juce::Rectangle<int>(gainKnobX, gainKnobY, GAIN_KNOB_SIZE, GAIN_KNOB_SIZE));
        gainKnobX += offsetX;

        bandToneSliders[i].setBounds(juce::Rectangle<int>(toneKnobX, toneKnobY, toneKnobSize, toneKnobSize));
        toneKnobX += offsetX;
        
        float bandRR_x = expGainToneArea.getX();
        float bandRR_y = static_cast<float>(bandY - margin);
        float bandRR_w = expGainToneArea.getWidth();
        float bandRR_h = static_cast<float>(BAND_HEIGHT + 2 * margin);
        transform.transformPoints(bandRR_x, bandRR_y, bandRR_w, bandRR_h);
        auto transformedBandRRArea = juce::Rectangle<float>(bandRR_x, bandRR_y, bandRR_w, bandRR_h);
        g.fillRoundedRectangle(transformedBandRRArea, cmargin/2.0f);

        bandSliders[i].setBounds(juce::Rectangle<int>(bandX, bandY, BAND_WIDTH, BAND_HEIGHT));

        int TEXT_HEIGHT = 15;
        transform.transformPoint(TEXT_HEIGHT, cmargin2);// cmargin2 reused, not used later

        g.setColour(juce::Colours::black);
        g.setFont(_MBDistLaF.mainFont.withHeight(static_cast<float>(TEXT_HEIGHT)));

        int bandText_x = bandX - 40,
              bandText_y = textY+10,
              bandText_w = BAND_WIDTH + 20,
              bandText_h = TEXT_HEIGHT;
        transform.transformPoints(bandText_x, bandText_y, bandText_w, bandText_h);

        g.drawText(bandFrequencies[i].first,
                   bandText_x,
                   bandText_y,
                   bandText_w,
                   bandText_h,
                   juce::Justification::left);
        if (i == bandSliders.size() -1)
        {
            int bandend_x = bandX - 10,
                bandend_textY = textY + 10,
                bandend_w = BAND_WIDTH + 30,
                bandend_h = TEXT_HEIGHT;
            transform.transformPoints(bandend_x, bandend_textY, bandend_w, bandend_h);
            g.drawText(bandFrequencies[i].second,
                   bandend_x,
                   bandend_textY,
                   bandend_w,
                   bandend_h,
                   juce::Justification::right);
        }

        bandX += offsetX;
        VOL_SLIDER_X = bandX;
    }

    // Volume slider
    const int VOL_SLIDER_W = BAND_WIDTH;
    const int VOL_SLIDER_H = bandY- gainKnobY + BAND_HEIGHT;
    const int VOL_SLIDER_Y = gainKnobY;
    volumeSlider.setBounds(juce::Rectangle<int>(VOL_SLIDER_X, VOL_SLIDER_Y, VOL_SLIDER_W, VOL_SLIDER_H));

    // Bypass button

    const int buttonX = 383, buttonY = 340, buttonW = 55, buttonH = 55;
    bypassButton.setBounds(juce::Rectangle<int>(buttonX, buttonY, buttonW, buttonH));

    //76	341	179	53
    const int webX = 76, webY = 341, webW = 179, webH = 53;
    websiteButton.setBounds(juce::Rectangle<int>(webX, webY, webW, webH));

    // Footer
    g.setColour(juce::Colour::fromString("#ffffffff"));

    auto programTextRect = footerArea.removeFromLeft(static_cast<int>(100 * scale));
    g.drawText("Presets:", 
               programTextRect.getX() + 10, 
               programTextRect.getY() + 5, 
               programTextRect.getWidth() - 20, 
               programTextRect.getHeight() - 10, 
               juce::Justification::centredRight);
    auto currentProgramRect = footerArea.removeFromLeft(static_cast<int>(150 * scale));
    currentProgram.setBounds(currentProgramRect);
    programButton.setBounds(programTextRect.getX(),
                            programTextRect.getY(),
                            programTextRect.getWidth()+currentProgramRect.getWidth(),
                            programTextRect.getHeight());
    
#ifndef OSC
    // If not using OSC, we are using the neural network.
    // Print a biig warning if sample rate is not 48000 Hz
    if (this->_sampleRate != 48000.0)
    {
        g.setFont(_MBDistLaF.mainFont.withHeight(20.0f));
        juce::String warningText = "Warning: Sample rate is not 48kHz!";
        int warningX = 150, warningY = 10, warningW = 500, warningH = 30;
        transform.transformPoints(warningX, warningY, warningW, warningH);
        g.setColour(juce::Colours::grey);
        g.drawRect(warningX, warningY, warningW, warningH);
        g.setColour(juce::Colours::red);
        g.drawText(warningText, warningX, warningY, warningW, warningH, juce::Justification::centred);
    }
#endif
    

    // Apply the transform to components
    for (int i = 0; i < bandSliders.size(); ++i)
    {
        bandSliders[i].setTransform(transform);
        bandGainSliders[i].setTransform(transform);
        bandToneSliders[i].setTransform(transform);
    }
    bypassButton.setTransform(transform);
    websiteButton.setTransform(transform);
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
    auto programIndex = audioProcessor.getCurrentProgram();
    auto programName = audioProcessor.getProgramName(programIndex);
    currentProgram.setText(programName, juce::dontSendNotification);
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

    this->_sampleRate = audioProcessor._sampleRate.load();

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