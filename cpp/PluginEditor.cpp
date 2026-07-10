#include "PluginEditor.h"

const juce::Colour BitlayLookAndFeel::accent = juce::Colour(0xffd87932);
const juce::Colour BitlayLookAndFeel::accentSoft = juce::Colour(0xff8b4a22);
const juce::Colour BitlayLookAndFeel::background = juce::Colour(0xff080807);
const juce::Colour BitlayLookAndFeel::panel = juce::Colour(0xff151412);
const juce::Colour BitlayLookAndFeel::panelRaised = juce::Colour(0xff24211d);
const juce::Colour BitlayLookAndFeel::borderStrong = juce::Colour(0xff4b4339);
const juce::Colour BitlayLookAndFeel::borderSubtle = juce::Colour(0xff2c2925);
const juce::Colour BitlayLookAndFeel::textPrimary = juce::Colour(0xfff2eee7);
const juce::Colour BitlayLookAndFeel::textSecondary = juce::Colour(0xffb5aaa0);
const juce::Colour BitlayLookAndFeel::textMuted = juce::Colour(0xff736a62);
const juce::Colour BitlayLookAndFeel::meter = juce::Colour(0xff86b887);
const juce::Colour BitlayLookAndFeel::warning = juce::Colour(0xffd14d3f);

BitlayLookAndFeel::BitlayLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, accent);
    setColour(juce::Slider::rotarySliderFillColourId, accent);
    setColour(juce::Slider::rotarySliderOutlineColourId, borderSubtle);
    setColour(juce::Slider::trackColourId, accent.withAlpha(0.45f));
    setColour(juce::Slider::backgroundColourId, borderSubtle);
    setColour(juce::ToggleButton::tickColourId, accent);
    setColour(juce::ComboBox::backgroundColourId, panelRaised);
    setColour(juce::ComboBox::outlineColourId, borderSubtle);
    setColour(juce::ComboBox::textColourId, textPrimary);
    setColour(juce::TextButton::buttonColourId, panelRaised);
    setColour(juce::TextButton::buttonOnColourId, accentSoft);
    setColour(juce::TextButton::textColourOffId, textSecondary);
    setColour(juce::TextButton::textColourOnId, textPrimary);
    setColour(juce::TabbedComponent::backgroundColourId, background);
    setColour(juce::TabbedComponent::outlineColourId, borderSubtle);
    setColour(juce::TabbedButtonBar::tabTextColourId, textMuted);
    setColour(juce::TabbedButtonBar::frontTextColourId, accent);
    setColour(juce::GroupComponent::textColourId, textSecondary);
    setColour(juce::GroupComponent::outlineColourId, borderSubtle);
}

void BitlayLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                            const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = (float) juce::jmin (width / 2, height / 2) - 7.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    juce::Colour controlColor = accent;
    if (slider.getName() == "Movement" || slider.getName() == "Wobble") controlColor = meter;
    else if (slider.getName() == "Texture" || slider.getName() == "Drive") controlColor = accent;

    juce::ColourGradient bgGrad(panelRaised.brighter(0.16f), centreX, ry, panel.darker(0.55f), centreX, ry + rw, false);
    g.setGradientFill(bgGrad);
    g.fillEllipse (rx, ry, rw, rw);
    
    g.setColour (juce::Colours::black.withAlpha(0.55f));
    g.drawEllipse (rx + 1.0f, ry + 2.0f, rw - 2.0f, rw - 2.0f, 2.0f);
    g.setColour (borderStrong.withAlpha(0.75f));
    g.drawEllipse (rx, ry, rw, rw, 1.4f);
    
    juce::Path track;
    track.addCentredArc(centreX, centreY, radius + 2.0f, radius + 2.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(borderSubtle);
    g.strokePath(track, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path fill;
    fill.addCentredArc(centreX, centreY, radius + 2.0f, radius + 2.0f, 0.0f, rotaryStartAngle, angle, true);
    g.setColour(controlColor);
    g.strokePath(fill, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    juce::Path p;
    auto pointerLength = radius * 0.62f;
    auto pointerThickness = 2.6f;
    p.addRoundedRectangle (-pointerThickness * 0.5f, -radius + 8.0f, pointerThickness, pointerLength, 1.2f);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    g.setColour (textPrimary);
    g.fillPath (p);
}

void BitlayLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                            bool, bool)
{
    auto bounds = button.getLocalBounds().toFloat();
    bool isReverse = button.getButtonText().containsIgnoreCase("Reverse");
    bool isBypass = button.getButtonText().containsIgnoreCase("Bypass");
    juce::Colour activeColor = isReverse || isBypass ? warning : accent;
    
    if (isReverse) {
        g.setColour(button.getToggleState() ? warning.withAlpha(0.18f) : panelRaised);
        g.fillRoundedRectangle(bounds, (float) BitlayUi::controlRadius);
        g.setColour(button.getToggleState() ? activeColor : borderSubtle);
        g.drawRoundedRectangle(bounds.reduced(0.5f), (float) BitlayUi::controlRadius, 1.2f);
        
        float circleSize = 10.0f;
        if (button.getToggleState()) {
            g.setColour(activeColor);
        } else {
            g.setColour(textMuted);
        }
        g.fillEllipse(10, (bounds.getHeight() - circleSize) / 2.0f, circleSize, circleSize);
        
        g.setColour (button.getToggleState() ? textPrimary : textSecondary);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawFittedText(button.getButtonText(), bounds.withTrimmedLeft(25).toNearestInt(), juce::Justification::centred, 1);
    } else {
        auto fontSize = juce::jmin (13.0f, (float) button.getHeight() * 0.75f);
        auto tickWidth = fontSize * 1.1f;
        g.setColour (button.getToggleState() ? activeColor : borderSubtle);
        g.fillRoundedRectangle(0.0f, (button.getHeight() - tickWidth) * 0.5f, tickWidth, tickWidth, (float) BitlayUi::controlRadius);

        g.setColour (button.getToggleState() ? textPrimary : textSecondary);
        g.setFont (fontSize);
        g.drawFittedText (button.getButtonText(),
                          button.getLocalBounds().withTrimmedLeft (juce::roundToInt (tickWidth) + 10),
                          juce::Justification::centredLeft, 10);
    }
}

void BitlayLookAndFeel::drawTabButton (juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown)
{
    auto area = button.getLocalBounds().toFloat();
    bool isActive = button.isFrontTab();
    
    if (isActive) {
        g.setColour(panelRaised);
        g.fillRoundedRectangle(area, 8.0f);
        g.setColour(accent);
        g.fillRoundedRectangle(area.removeFromBottom(2.0f), 1.0f);
    } else if (isMouseOver || isMouseDown) {
        g.setColour(panelRaised.withAlpha(0.55f));
        g.fillRoundedRectangle(area.reduced(1.0f), 7.0f);
    }
    
    g.setColour(isActive ? textPrimary : textMuted);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), area, juce::Justification::centred, true);
}

void BitlayLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    juce::Rectangle<int> bounds(0, 0, width, height);
    g.setColour(panelRaised);
    g.fillRoundedRectangle(bounds.toFloat(), (float) BitlayUi::controlRadius);
    g.setColour(borderStrong.withAlpha(0.75f));
    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), (float) BitlayUi::controlRadius, 1.0f);
    
    juce::Path path;
    path.addTriangle(width - 15.0f, height * 0.4f, width - 5.0f, height * 0.4f, width - 10.0f, height * 0.6f);
    g.setColour(accent);
    g.fillPath(path);
}

void BitlayLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto base = button.getToggleState() ? accentSoft : panelRaised;
    if (shouldDrawButtonAsDown) base = base.darker(0.25f);
    else if (shouldDrawButtonAsHighlighted) base = base.brighter(0.12f);

    g.setColour(base);
    g.fillRoundedRectangle(bounds, (float) BitlayUi::controlRadius);
    g.setColour((button.getToggleState() ? accent : borderStrong).withAlpha(0.8f));
    g.drawRoundedRectangle(bounds, (float) BitlayUi::controlRadius, 1.0f);
}

void BitlayLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                        bool, bool)
{
    g.setColour(button.getToggleState() ? textPrimary : textSecondary);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(8, 0),
                     juce::Justification::centred, 1);
}

void BitlayLookAndFeel::drawGroupComponentOutline (juce::Graphics& g, int width, int height, const juce::String& text,
                                                   const juce::Justification&, juce::GroupComponent&)
{
    auto bounds = juce::Rectangle<float>(0.5f, 9.5f, (float) width - 1.0f, (float) height - 10.0f);
    g.setColour(panel.withAlpha(0.66f));
    g.fillRoundedRectangle(bounds, (float) BitlayUi::panelRadius);
    g.setColour(borderSubtle);
    g.drawRoundedRectangle(bounds, (float) BitlayUi::panelRadius, 1.0f);

    g.setColour(textSecondary);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText(text.toUpperCase(), 12, 0, width - 24, 18, juce::Justification::centredLeft, true);
}

juce::Font BitlayLookAndFeel::getLabelFont (juce::Label&) { return juce::Font (11.0f); }
juce::Font BitlayLookAndFeel::getComboBoxFont (juce::ComboBox&) { return juce::Font (13.0f, juce::Font::bold); }

BitlayLabel::BitlayLabel(const juce::String& name, const juce::String& labelText) : juce::Label(name, labelText)
{
    setColour(juce::Label::textColourId, BitlayLookAndFeel::textMuted);
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
    g.fillAll(BitlayLookAndFeel::background.darker(0.35f));
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(BitlayLookAndFeel::panel.withAlpha(0.7f));
    g.fillRoundedRectangle(bounds, (float) BitlayUi::panelRadius);
    g.setColour(BitlayLookAndFeel::borderSubtle);
    g.drawRoundedRectangle(bounds, (float) BitlayUi::panelRadius, 1.0f);

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

    g.setColour(BitlayLookAndFeel::meter.withAlpha(0.25f));
    g.strokePath(pInt, juce::PathStrokeType(4.0f));
    g.setColour(BitlayLookAndFeel::meter);
    g.strokePath(pInt, juce::PathStrokeType(1.5f));
    
    g.setColour(BitlayLookAndFeel::accent.withAlpha(0.72f));
    g.strokePath(pStep, juce::PathStrokeType(2.0f));
}

BitlayAudioProcessorEditor::BitlayAudioProcessorEditor (BitlayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), scope(p), tabs(juce::TabbedButtonBar::TabsAtTop)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&bitlayLookAndFeel);
    juce::StringArray subdivs { "1/16", "1/3T", "1/8", "1/8D", "1/4", "1/2D", "1/2", "custom" };
    juce::StringArray circuits { "Discrete", "Companded" };
    auto& apvts = audioProcessor.apvts;

    pluginTitle.setText("BITLAY", juce::dontSendNotification);
    pluginTitle.setFont(juce::Font(30.0f, juce::Font::bold));
    pluginTitle.setColour(juce::Label::textColourId, BitlayLookAndFeel::textPrimary);
    addAndMakeVisible(pluginTitle);
    
    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    presetLabel.setColour(juce::Label::textColourId, BitlayLookAndFeel::textMuted);
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
    delayTime.slider.setName("Main"); delayTime.init("Time", apvts, "delayTime");
    feedback.slider.setName("Main"); feedback.init("Feedback", apvts, "feedback");
    mix.slider.setName("Main"); mix.init("Mix", apvts, "mix");
    internalBpm.slider.setName("Main"); internalBpm.init("BPM", apvts, "internalBpm");
    bpmSync.init("Host Sync", apvts, "bpmSync");
    freeze.init("Freeze Loop", apvts, "freeze");
    mainSubdivision.init("Subdivision", apvts, "mainSubdivision", subdivs);

    // Macros
    macroTexture.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    macroTexture.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
    macroTexture.setName("Texture");
    macroTexture.setRange(0.0, 1.0);
    macroTexture.setValue(0.5);
    macroTexture.addListener(this);
    labelTexture.setText("DEGRADE", juce::dontSendNotification);
    labelTexture.setJustificationType(juce::Justification::centred);

    macroMovement.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    macroMovement.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
    macroMovement.setName("Movement");
    macroMovement.setRange(0.0, 1.0);
    macroMovement.setValue(0.5);
    macroMovement.addListener(this);
    labelMovement.setText("MOVEMENT", juce::dontSendNotification);
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
    feedbackTone.init("Tone", apvts, "feedbackTone");
    stereoSpread.init("Width", apvts, "stereoSpread");
    
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

    addComps(mainTabComp, {&delayTime.slider, &delayTime.label, &feedback.slider, &feedback.label, &mix.slider, &mix.label, &internalBpm.slider, &internalBpm.label, &mainSubdivision.combo, &mainSubdivision.label, &bpmSync.button, &freeze.button, &macroTexture, &labelTexture, &macroMovement, &labelMovement, &character.slider, &character.label, &feedbackTone.slider, &feedbackTone.label, &stereoSpread.slider, &stereoSpread.label});
    
    addComps(circuitTabComp, {&circuitType.combo, &circuitType.label, &coupledMode.button, &stepSize.slider, &stepSize.label, &clockJitter.slider, &clockJitter.label, &integratorLag.slider, &integratorLag.label, &reconCutoff.slider, &reconCutoff.label, &integratorLeak.slider, &integratorLeak.label, &dynamicResponse.slider, &dynamicResponse.label, &envAttack.slider, &envAttack.label, &envRelease.slider, &envRelease.label, &minStepSize.slider, &minStepSize.label, &maxStepSize.slider, &maxStepSize.label, &syllabicTime.slider, &syllabicTime.label});
    
    addComps(tapsTabComp, {&numTaps.slider, &numTaps.label, &tapDecay.slider, &tapDecay.label, &tap1Mult.slider, &tap1Mult.label, &tap1Mix.slider, &tap1Mix.label, &tap1Subdiv.combo, &tap1Subdiv.label, &tap2Mult.slider, &tap2Mult.label, &tap2Mix.slider, &tap2Mix.label, &tap2Subdiv.combo, &tap2Subdiv.label, &tap3Mult.slider, &tap3Mult.label, &tap3Mix.slider, &tap3Mix.label, &tap3Subdiv.combo, &tap3Subdiv.label, &tap4Mult.slider, &tap4Mult.label, &tap4Mix.slider, &tap4Mix.label, &tap4Subdiv.combo, &tap4Subdiv.label});
    
    addComps(reverseTabComp, {&reverseChunkSize.slider, &reverseChunkSize.label, &reverseFeedback.slider, &reverseFeedback.label, &wobbleRate.slider, &wobbleRate.label, &wobbleDepth.slider, &wobbleDepth.label, &wobbleSync.slider, &wobbleSync.label});

    tabs.addTab("MAIN", BitlayLookAndFeel::background, mainTabComp, true);
    tabs.addTab("RHYTHM", BitlayLookAndFeel::background, tapsTabComp, true);
    tabs.addTab("ENGINE", BitlayLookAndFeel::background, circuitTabComp, true);
    tabs.addTab("LAB", BitlayLookAndFeel::background, reverseTabComp, true);

    addAndMakeVisible(tabs);
    addAndMakeVisible(scope);

    setSize (1000, 720);
}

BitlayAudioProcessorEditor::~BitlayAudioProcessorEditor() { juce::LookAndFeel::setDefaultLookAndFeel(nullptr); }

void BitlayAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bgGrad(BitlayLookAndFeel::panel, 0, 0, BitlayLookAndFeel::background, 0, (float)getHeight(), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    auto faceplate = getLocalBounds().reduced(10).toFloat();
    g.setColour(BitlayLookAndFeel::borderSubtle.withAlpha(0.65f));
    g.drawRoundedRectangle(faceplate, 10.0f, 1.0f);
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
    auto area = getLocalBounds().reduced(BitlayUi::outerMargin);

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
    scope.setBounds(area.removeFromTop(112));
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
    groupTime.setBounds(10, 10, 360, 315);
    placeKnob(delayTime, 35, 54, 132, 148);
    placeKnob(feedback, 188, 54, 132, 148);
    placeCombo(mainSubdivision, 42, 236, 108);

    internalBpm.label.setBounds(178, 212, 50, 20);
    internalBpm.slider.setBounds(222, 211, 110, 24);

    placeToggle(bpmSync, 42, 274, 112);
    placeToggle(freeze, 182, 274, 120);

    groupMacros.setBounds(390, 10, 330, 315);
    macroTexture.setBounds(426, 62, 118, 126);
    labelTexture.setBounds(426, 188, 118, 20);
    macroMovement.setBounds(568, 62, 118, 126);
    labelMovement.setBounds(568, 188, 118, 20);
    placeKnob(character, 430, 232, 78, 86);
    placeKnob(feedbackTone, 532, 232, 78, 86);
    placeKnob(stereoSpread, 634, 232, 78, 86);

    groupMix.setBounds(740, 10, 210, 315);
    placeKnob(mix, 782, 70, 128, 148);

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
    placeKnob(stepSize, 40, 80, 80, 90);
    placeKnob(minStepSize, 170, 80, 80, 90);
    placeKnob(maxStepSize, 300, 80, 80, 90);
    
    groupFilters.setBounds(480, 10, 470, 160);
    placeKnob(integratorLag, 520, 60, 80, 90);
    placeKnob(reconCutoff, 650, 60, 80, 90);
    placeKnob(clockJitter, 780, 60, 80, 90);

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
