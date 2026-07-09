#include "PluginEditor.h"

const juce::Colour WebStyleLookAndFeel::colorRetro = juce::Colour(0xfff27d26);
const juce::Colour WebStyleLookAndFeel::colorBgDark = juce::Colour(0xff08080c);
const juce::Colour WebStyleLookAndFeel::colorPanel = juce::Colour(0xff121218);
const juce::Colour WebStyleLookAndFeel::colorBorderDark = juce::Colour(0xff222233);
const juce::Colour WebStyleLookAndFeel::colorBorderDim = juce::Colour(0xff303040);
const juce::Colour WebStyleLookAndFeel::colorTextWhite = juce::Colour(0xffffffff);
const juce::Colour WebStyleLookAndFeel::colorTextMuted = juce::Colour(0xff888888);
const juce::Colour WebStyleLookAndFeel::colorNeonCyan = juce::Colour(0xff00e5ff);
const juce::Colour WebStyleLookAndFeel::colorNeonOrange = juce::Colour(0xffff9900);
const juce::Colour WebStyleLookAndFeel::colorNeonPurple = juce::Colour(0xffaa33ff);
const juce::Colour WebStyleLookAndFeel::colorNeonRed = juce::Colour(0xffff3333);

WebStyleLookAndFeel::WebStyleLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, colorNeonCyan);
    setColour(juce::Slider::rotarySliderFillColourId, colorNeonCyan.withAlpha(0.8f));
    setColour(juce::Slider::rotarySliderOutlineColourId, colorBorderDim);
    setColour(juce::Slider::trackColourId, colorNeonCyan.withAlpha(0.3f));
    setColour(juce::Slider::backgroundColourId, colorBorderDim);
    setColour(juce::ToggleButton::tickColourId, colorNeonRed);
    setColour(juce::ComboBox::backgroundColourId, colorPanel);
    setColour(juce::ComboBox::outlineColourId, colorBorderDim);
    setColour(juce::ComboBox::textColourId, colorTextWhite);
    setColour(juce::TabbedComponent::backgroundColourId, colorBgDark);
    setColour(juce::TabbedComponent::outlineColourId, colorBorderDark);
    setColour(juce::TabbedButtonBar::tabTextColourId, colorTextMuted);
    setColour(juce::TabbedButtonBar::frontTextColourId, colorNeonCyan);
    setColour(juce::GroupComponent::textColourId, colorNeonCyan);
    setColour(juce::GroupComponent::outlineColourId, colorBorderDim);
}

void WebStyleLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                            const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    juce::Colour neonColor = colorNeonCyan;
    if (slider.getName() == "Movement" || slider.getName() == "Wobble") neonColor = colorNeonPurple;
    else if (slider.getName() == "Texture" || slider.getName() == "Drive") neonColor = colorNeonOrange;

    // Body
    juce::ColourGradient bgGrad(juce::Colour(0xff323244), centreX, ry, juce::Colour(0xff171720), centreX, ry + rw, false);
    g.setGradientFill(bgGrad);
    g.fillEllipse (rx, ry, rw, rw);
    
    g.setColour (juce::Colour(0xff111111));
    g.drawEllipse (rx, ry, rw, rw, 2.0f);
    
    // Track
    juce::Path track;
    track.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff222222));
    g.strokePath(track, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Fill arc
    juce::Path fill;
    fill.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour(neonColor.withAlpha(0.3f));
    g.strokePath(fill, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded)); // Glow
    g.setColour(neonColor);
    g.strokePath(fill, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded)); // Core
    
    // Pointer
    juce::Path p;
    auto pointerLength = radius * 0.7f;
    auto pointerThickness = 3.0f;
    p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    g.setColour (juce::Colours::white);
    g.fillPath (p);
}

void WebStyleLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, 
                                            bool, bool)
{
    auto bounds = button.getLocalBounds().toFloat();
    bool isReverse = button.getButtonText().containsIgnoreCase("Reverse");
    juce::Colour activeColor = isReverse ? colorNeonRed : colorNeonCyan;
    
    if (isReverse) {
        g.setColour(juce::Colour(0xff301015));
        g.fillRoundedRectangle(bounds, 8.0f);
        g.setColour(activeColor.withAlpha(0.5f));
        g.drawRoundedRectangle(bounds, 2.0f, 8.0f);
        
        float circleSize = 10.0f;
        if (button.getToggleState()) {
            g.setColour(activeColor.withAlpha(0.3f));
            g.fillEllipse(10, (bounds.getHeight() - circleSize) / 2.0f, circleSize, circleSize); // Glow
            g.setColour(activeColor);
        } else {
            g.setColour(colorBorderDim);
        }
        g.fillEllipse(10, (bounds.getHeight() - circleSize) / 2.0f, circleSize, circleSize);
        
        g.setColour (colorTextWhite);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawFittedText(button.getButtonText(), bounds.withTrimmedLeft(25).toNearestInt(), juce::Justification::centred, 1);
    } else {
        auto fontSize = juce::jmin (13.0f, (float) button.getHeight() * 0.75f);
        auto tickWidth = fontSize * 1.1f;
        g.setColour (button.getToggleState() ? activeColor : colorBorderDim);
        g.fillRoundedRectangle(0.0f, (button.getHeight() - tickWidth) * 0.5f, tickWidth, tickWidth, 4.0f);

        g.setColour (colorTextWhite);
        g.setFont (fontSize);
        g.drawFittedText (button.getButtonText(),
                          button.getLocalBounds().withTrimmedLeft (juce::roundToInt (tickWidth) + 10),
                          juce::Justification::centredLeft, 10);
    }
}

void WebStyleLookAndFeel::drawTabButton (juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown)
{
    auto area = button.getLocalBounds().toFloat();
    bool isActive = button.isFrontTab();
    
    if (isActive) {
        juce::ColourGradient grad(colorNeonCyan.withAlpha(0.15f), 0, 0, colorNeonCyan.withAlpha(0.0f), 0, area.getHeight(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(area, 8.0f);
        g.setColour(colorNeonCyan);
        g.drawRoundedRectangle(area, 2.0f, 8.0f);
    }
    
    g.setColour(isActive ? colorNeonCyan : colorTextMuted);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), area, juce::Justification::centred, true);
}

void WebStyleLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    juce::Rectangle<int> bounds(0, 0, width, height);
    g.setColour(juce::Colour(0xff0a0a0f));
    g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
    g.setColour(colorNeonCyan.withAlpha(0.4f));
    g.drawRoundedRectangle(bounds.toFloat(), 1.0f, 6.0f);
    
    juce::Path path;
    path.addTriangle(width - 15.0f, height * 0.4f, width - 5.0f, height * 0.4f, width - 10.0f, height * 0.6f);
    g.setColour(colorNeonCyan);
    g.fillPath(path);
}

juce::Font WebStyleLookAndFeel::getLabelFont (juce::Label&) { return juce::Font (11.0f); }
juce::Font WebStyleLookAndFeel::getComboBoxFont (juce::ComboBox&) { return juce::Font (13.0f, juce::Font::bold); }

WebLabel::WebLabel(const juce::String& name, const juce::String& labelText) : juce::Label(name, labelText)
{
    setColour(juce::Label::textColourId, WebStyleLookAndFeel::colorTextMuted);
}

OscilloscopeVisualizer::OscilloscopeVisualizer(BitlayAudioProcessor& p) : processor(p)
{
    integratorData.resize(256, 0.0f);
    stepSizeData.resize(256, 0.0f);
    startTimerHz(30);
}

OscilloscopeVisualizer::~OscilloscopeVisualizer() {}

void OscilloscopeVisualizer::timerCallback()
{
    int numSamples = processor.scopeFifo.getNumReady();
    if (numSamples > 0)
    {
        int start1, size1, start2, size2;
        processor.scopeFifo.prepareToRead (numSamples, start1, size1, start2, size2);
        if (size1 + size2 >= 256)
        {
            int step = (size1 + size2) / 256;
            for (int i = 0; i < 256; ++i)
            {
                int readIdx = (start1 + i * step) % BitlayAudioProcessor::fifoSize;
                integratorData[i] = processor.diagIntegratorBuffer[readIdx];
                stepSizeData[i] = processor.diagStepSizeBuffer[readIdx];
            }
        }
        processor.scopeFifo.finishedRead (size1 + size2);
        repaint();
    }
}

void OscilloscopeVisualizer::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff050508));
    g.setColour(WebStyleLookAndFeel::colorBorderDark);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 2.0f, 12.0f);

    if (integratorData.empty()) return;

    juce::Path pInt, pStep;
    float width = (float)getWidth();
    float height = (float)getHeight();
    float halfHeight = height / 2.0f;

    pInt.startNewSubPath(0, halfHeight - integratorData[0] * halfHeight);
    pStep.startNewSubPath(0, height - stepSizeData[0] * height);

    for (size_t i = 1; i < integratorData.size(); ++i) {
        float x = (float)i / (float)integratorData.size() * width;
        pInt.lineTo(x, halfHeight - integratorData[i] * halfHeight);
        pStep.lineTo(x, height - stepSizeData[i] * height);
    }

    g.setColour(WebStyleLookAndFeel::colorNeonCyan.withAlpha(0.3f));
    g.strokePath(pInt, juce::PathStrokeType(4.0f));
    g.setColour(WebStyleLookAndFeel::colorNeonCyan);
    g.strokePath(pInt, juce::PathStrokeType(1.5f));
    
    g.setColour(WebStyleLookAndFeel::colorNeonRed.withAlpha(0.6f));
    g.strokePath(pStep, juce::PathStrokeType(2.0f));
}

BitlayAudioProcessorEditor::BitlayAudioProcessorEditor (BitlayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), scope(p), tabs(juce::TabbedButtonBar::TabsAtTop)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&webLookAndFeel);
    juce::StringArray subdivs { "1/16", "1/3T", "1/8", "1/8D", "1/4", "1/2D", "1/2", "custom" };
    juce::StringArray circuits { "Discrete", "Companded" };
    auto& apvts = audioProcessor.apvts;

    pluginTitle.setText("BITLAY", juce::dontSendNotification);
    pluginTitle.setFont(juce::Font(28.0f, juce::Font::bold));
    pluginTitle.setColour(juce::Label::textColourId, WebStyleLookAndFeel::colorTextWhite);
    addAndMakeVisible(pluginTitle);
    
    presetLabel.setText("Preset:", juce::dontSendNotification);
    addAndMakeVisible(presetLabel);
    addAndMakeVisible(presetComboBox);
    addAndMakeVisible(savePresetButton);
    addAndMakeVisible(newPresetButton);
    savePresetButton.setButtonText("Save");
    newPresetButton.setButtonText("Save As...");
    updatePresetList();
    presetComboBox.onChange = [this] { if (presetComboBox.getSelectedItemIndex() >= 0) audioProcessor.loadPreset(presetComboBox.getText()); };
    savePresetButton.onClick = [this] { audioProcessor.savePreset(presetComboBox.getText()); };
    newPresetButton.onClick = [this] { /* async logic placeholder */ };

    reverseMode.init("REVERSE", apvts, "reverseMode");
    addAndMakeVisible(reverseMode.button);
    bypass.init("BYPASS", apvts, "bypass");
    addAndMakeVisible(bypass.button);

    // Main
    delayTime.slider.setName("Cyan"); delayTime.init("Delay Time", apvts, "delayTime");
    feedback.slider.setName("Cyan"); feedback.init("Feedback", apvts, "feedback");
    mix.slider.setName("Cyan"); mix.init("Dry/Wet Mix", apvts, "mix");
    internalBpm.slider.setName("Cyan"); internalBpm.init("Tempo (BPM)", apvts, "internalBpm");
    bpmSync.init("Sync Mode", apvts, "bpmSync");
    freeze.init("Freeze Loop", apvts, "freeze");
    mainSubdivision.init("Beat Subdiv", apvts, "mainSubdivision", subdivs);

    // Macros
    macroTexture.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    macroTexture.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    macroTexture.setName("Texture");
    macroTexture.setRange(0.0, 1.0);
    macroTexture.setValue(0.5);
    macroTexture.addListener(this);
    labelTexture.setText("DEGRADE", juce::dontSendNotification);
    labelTexture.setJustificationType(juce::Justification::centred);

    macroMovement.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    macroMovement.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    macroMovement.setName("Movement");
    macroMovement.setRange(0.0, 1.0);
    macroMovement.setValue(0.5);
    macroMovement.addListener(this);
    labelMovement.setText("WOBBLE", juce::dontSendNotification);
    labelMovement.setJustificationType(juce::Justification::centred);

    // Modulation
    wobbleRate.slider.setName("Wobble"); wobbleRate.init("Mod Speed", apvts, "wobbleRate");
    wobbleDepth.slider.setName("Wobble"); wobbleDepth.init("Mod Depth", apvts, "wobbleDepth");
    wobbleSync.slider.setName("Wobble"); wobbleSync.init("Mod Sync", apvts, "wobbleSync");

    // Circuit
    circuitType.init("Architecture", apvts, "circuitType", circuits);
    coupledMode.init("Coupled Engine", apvts, "coupledMode");
    character.slider.setName("Drive"); character.init("Pre-Amp Drive", apvts, "character");
    stepSize.init("Delta Step", apvts, "stepSize");
    clockJitter.init("Clock Jitter", apvts, "clockJitter");
    integratorLag.init("Analog Lag", apvts, "integratorLag");
    reconCutoff.init("Lowpass Freq", apvts, "reconstructionCutoff");
    integratorLeak.init("Memory Leak", apvts, "integratorLeak");
    dynamicResponse.init("Dynamic Resp", apvts, "dynamicResponse");
    feedbackTone.init("Echo Tone", apvts, "feedbackTone");
    stereoSpread.init("Stereo Width", apvts, "stereoSpread");
    
    envAttack.init("Env Attack", apvts, "envAttack");
    envRelease.init("Env Release", apvts, "envRelease");
    minStepSize.init("Min Delta", apvts, "minStepSize");
    maxStepSize.init("Max Delta", apvts, "maxStepSize");
    syllabicTime.init("Syllabic Speed", apvts, "syllabicTime");

    // Taps
    numTaps.init("Active Taps", apvts, "numTaps");
    tapDecay.init("Volume Decay", apvts, "tapDecay");
    
    tap1Mult.init("Time Mult", apvts, "tap1_mult"); tap1Mix.init("Tap Level", apvts, "tap1_mix"); tap1Subdiv.init("Subdiv", apvts, "tap1_subdiv", subdivs);
    tap2Mult.init("Time Mult", apvts, "tap2_mult"); tap2Mix.init("Tap Level", apvts, "tap2_mix"); tap2Subdiv.init("Subdiv", apvts, "tap2_subdiv", subdivs);
    tap3Mult.init("Time Mult", apvts, "tap3_mult"); tap3Mix.init("Tap Level", apvts, "tap3_mix"); tap3Subdiv.init("Subdiv", apvts, "tap3_subdiv", subdivs);
    tap4Mult.init("Time Mult", apvts, "tap4_mult"); tap4Mix.init("Tap Level", apvts, "tap4_mix"); tap4Subdiv.init("Subdiv", apvts, "tap4_subdiv", subdivs);

    // Reverse Details
    reverseChunkSize.init("Buffer Size", apvts, "reverseChunkSize");
    reverseFeedback.init("Accumulate", apvts, "reverseFeedback");

    mainTabComp = new juce::Component();
    tapsTabComp = new juce::Component();
    circuitTabComp = new juce::Component();
    reverseTabComp = new juce::Component();

    auto addGroup = [](juce::Component* p, juce::GroupComponent* g) { p->addAndMakeVisible(g); };
    
    // Add groups FIRST so they render behind
    addGroup(mainTabComp, &groupTime); addGroup(mainTabComp, &groupMacros); addGroup(mainTabComp, &groupMix);
    addGroup(tapsTabComp, &groupTapGlobal); addGroup(tapsTabComp, &groupTap1); addGroup(tapsTabComp, &groupTap2); addGroup(tapsTabComp, &groupTap3); addGroup(tapsTabComp, &groupTap4);
    addGroup(circuitTabComp, &groupEngine); addGroup(circuitTabComp, &groupFilters); addGroup(circuitTabComp, &groupEnv);
    addGroup(reverseTabComp, &groupRev); addGroup(reverseTabComp, &groupLfo);

    auto addComps = [](juce::Component* p, std::vector<juce::Component*> c) { for (auto* comp : c) p->addAndMakeVisible(comp); };

    addComps(mainTabComp, {&delayTime.slider, &delayTime.label, &feedback.slider, &feedback.label, &mix.slider, &mix.label, &internalBpm.slider, &internalBpm.label, &mainSubdivision.combo, &mainSubdivision.label, &bpmSync.button, &freeze.button, &macroTexture, &labelTexture, &macroMovement, &labelMovement});
    
    addComps(circuitTabComp, {&circuitType.combo, &circuitType.label, &coupledMode.button, &character.slider, &character.label, &stepSize.slider, &stepSize.label, &clockJitter.slider, &clockJitter.label, &integratorLag.slider, &integratorLag.label, &reconCutoff.slider, &reconCutoff.label, &integratorLeak.slider, &integratorLeak.label, &dynamicResponse.slider, &dynamicResponse.label, &feedbackTone.slider, &feedbackTone.label, &stereoSpread.slider, &stereoSpread.label, &envAttack.slider, &envAttack.label, &envRelease.slider, &envRelease.label, &minStepSize.slider, &minStepSize.label, &maxStepSize.slider, &maxStepSize.label, &syllabicTime.slider, &syllabicTime.label});
    
    addComps(tapsTabComp, {&numTaps.slider, &numTaps.label, &tapDecay.slider, &tapDecay.label, &tap1Mult.slider, &tap1Mult.label, &tap1Mix.slider, &tap1Mix.label, &tap1Subdiv.combo, &tap1Subdiv.label, &tap2Mult.slider, &tap2Mult.label, &tap2Mix.slider, &tap2Mix.label, &tap2Subdiv.combo, &tap2Subdiv.label, &tap3Mult.slider, &tap3Mult.label, &tap3Mix.slider, &tap3Mix.label, &tap3Subdiv.combo, &tap3Subdiv.label, &tap4Mult.slider, &tap4Mult.label, &tap4Mix.slider, &tap4Mix.label, &tap4Subdiv.combo, &tap4Subdiv.label});
    
    addComps(reverseTabComp, {&reverseChunkSize.slider, &reverseChunkSize.label, &reverseFeedback.slider, &reverseFeedback.label, &wobbleRate.slider, &wobbleRate.label, &wobbleDepth.slider, &wobbleDepth.label, &wobbleSync.slider, &wobbleSync.label});

    tabs.addTab("1. MACROS & CORE", WebStyleLookAndFeel::colorBgDark, mainTabComp, true);
    tabs.addTab("2. MULTI-TAP ENGINE", WebStyleLookAndFeel::colorBgDark, tapsTabComp, true);
    tabs.addTab("3. ADVANCED CVSD", WebStyleLookAndFeel::colorBgDark, circuitTabComp, true);
    tabs.addTab("4. REVERSE & MOD", WebStyleLookAndFeel::colorBgDark, reverseTabComp, true);

    addAndMakeVisible(tabs);
    addAndMakeVisible(scope);

    setSize (1000, 720);
}

BitlayAudioProcessorEditor::~BitlayAudioProcessorEditor() { juce::LookAndFeel::setDefaultLookAndFeel(nullptr); }

void BitlayAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bgGrad(juce::Colour(0xff121218), 0, 0, juce::Colour(0xff08080c), 0, (float)getHeight(), false);
    g.setGradientFill(bgGrad);
    g.fillAll();
}

void BitlayAudioProcessorEditor::updatePresetList()
{
    presetComboBox.clear();
    auto presets = audioProcessor.getPresetNames();
    presetComboBox.addItemList(presets, 1);
    int index = presets.indexOf(audioProcessor.currentPreset);
    if (index >= 0) presetComboBox.setSelectedItemIndex(index, juce::dontSendNotification);
    else if (presets.size() > 0) presetComboBox.setSelectedItemIndex(0, juce::dontSendNotification);
}

void BitlayAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &macroTexture) {
        float val = (float)slider->getValue(); 
        if (auto* p = audioProcessor.apvts.getParameter("character")) p->setValueNotifyingHost(val);
        if (auto* p = audioProcessor.apvts.getParameter("integratorLag")) p->setValueNotifyingHost(val);
        if (auto* p = audioProcessor.apvts.getParameter("reconstructionCutoff")) p->setValueNotifyingHost(1.0f - val);
    }
    else if (slider == &macroMovement) {
        float val = (float)slider->getValue();
        if (auto* p = audioProcessor.apvts.getParameter("wobbleDepth")) p->setValueNotifyingHost(val);
        if (auto* p = audioProcessor.apvts.getParameter("wobbleRate")) p->setValueNotifyingHost(val);
    }
}

void BitlayAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    // Header
    auto headerArea = area.removeFromTop(60);
    pluginTitle.setBounds(headerArea.removeFromLeft(150));
    
    auto presetArea = headerArea.removeFromLeft(420).reduced(0, 10);
    presetLabel.setBounds(presetArea.removeFromLeft(55));
    presetComboBox.setBounds(presetArea.removeFromLeft(150));
    savePresetButton.setBounds(presetArea.removeFromLeft(90).reduced(2)); // Buttons are now larger
    newPresetButton.setBounds(presetArea.removeFromLeft(100).reduced(2)); // Much larger
    
    headerArea.removeFromLeft(20);
    reverseMode.button.setBounds(headerArea.removeFromLeft(140).reduced(0, 5));
    headerArea.removeFromLeft(20);
    bypass.button.setBounds(headerArea.removeFromLeft(100).reduced(0, 10));

    area.removeFromTop(10);
    scope.setBounds(area.removeFromTop(180));
    area.removeFromTop(15);
    tabs.setBounds(area);

    auto placeKnob = [&](SliderWithLabel& swl, int x, int y, int w=80, int h=90) {
        swl.slider.setBounds(x, y, w, h-20);
        swl.label.setBounds(x, y + h - 20, w, 20);
    };
    auto placeCombo = [&](ComboWithLabel& cwl, int x, int y, int bw=80) {
        cwl.combo.setBounds(x, y, bw, 24);
        cwl.label.setBounds(x, y - 24, bw, 20);
    };
    auto placeToggle = [&](ToggleWithLabel& twl, int x, int y, int bw=80) { twl.button.setBounds(x, y, bw, 24); };

    // MAIN TAB
    groupTime.setBounds(10, 10, 360, 240);
    placeKnob(delayTime, 30, 50, 100, 110);
    placeKnob(feedback, 150, 50, 100, 110);
    placeCombo(mainSubdivision, 260, 80, 90);
    
    internalBpm.label.setBounds(30, 180, 100, 20);
    internalBpm.slider.setBounds(130, 180, 200, 24); // Linear slider
    
    placeToggle(bpmSync, 30, 210, 100);
    placeToggle(freeze, 150, 210, 100);

    groupMacros.setBounds(390, 10, 320, 240);
    macroTexture.setBounds(430, 80, 100, 90);
    labelTexture.setBounds(430, 170, 100, 20);
    macroMovement.setBounds(560, 80, 100, 90);
    labelMovement.setBounds(560, 170, 100, 20);
    
    groupMix.setBounds(730, 10, 200, 240);
    placeKnob(mix, 780, 80, 100, 110);

    // TAPS TAB
    groupTapGlobal.setBounds(10, 10, 200, 240);
    placeKnob(numTaps, 30, 60);
    placeKnob(tapDecay, 110, 60);
    
    int tx = 230;
    auto setupTapBox = [&](juce::GroupComponent& group, SliderWithLabel& mult, SliderWithLabel& mx, ComboWithLabel& sub, int startX) {
        group.setBounds(startX, 10, 160, 240);
        placeKnob(mult, startX + 10, 50, 60, 80);
        placeKnob(mx, startX + 90, 50, 60, 80);
        placeCombo(sub, startX + 40, 170, 80);
    };
    setupTapBox(groupTap1, tap1Mult, tap1Mix, tap1Subdiv, tx);
    setupTapBox(groupTap2, tap2Mult, tap2Mix, tap2Subdiv, tx + 180);
    setupTapBox(groupTap3, tap3Mult, tap3Mix, tap3Subdiv, tx + 360);
    setupTapBox(groupTap4, tap4Mult, tap4Mix, tap4Subdiv, tx + 540);

    // CIRCUIT TAB
    groupEngine.setBounds(10, 10, 450, 160);
    placeCombo(circuitType, 30, 40, 120);
    placeToggle(coupledMode, 170, 40, 120);
    placeKnob(character, 30, 80, 80, 90);
    placeKnob(stepSize, 130, 80, 80, 90);
    placeKnob(minStepSize, 230, 80, 80, 90);
    placeKnob(maxStepSize, 330, 80, 80, 90);
    
    groupFilters.setBounds(480, 10, 470, 160);
    placeKnob(integratorLag, 500, 60, 80, 90);
    placeKnob(reconCutoff, 590, 60, 80, 90);
    placeKnob(feedbackTone, 680, 60, 80, 90);
    placeKnob(stereoSpread, 770, 60, 80, 90);
    placeKnob(clockJitter, 860, 60, 80, 90);

    groupEnv.setBounds(10, 185, 940, 140);
    placeKnob(envAttack, 100, 220, 80, 90);
    placeKnob(envRelease, 250, 220, 80, 90);
    placeKnob(syllabicTime, 400, 220, 80, 90);
    placeKnob(integratorLeak, 550, 220, 80, 90);
    placeKnob(dynamicResponse, 700, 220, 80, 90);

    // REVERSE & MOD TAB
    groupRev.setBounds(10, 10, 300, 160);
    placeKnob(reverseChunkSize, 40, 50, 80, 90);
    placeKnob(reverseFeedback, 160, 50, 80, 90);

    groupLfo.setBounds(330, 10, 360, 160);
    placeKnob(wobbleRate, 360, 50, 80, 90);
    placeKnob(wobbleDepth, 460, 50, 80, 90);
    placeKnob(wobbleSync, 560, 50, 80, 90);
}
