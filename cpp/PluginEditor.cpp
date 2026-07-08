#include "PluginEditor.h"

const juce::Colour WebStyleLookAndFeel::colorRetro = juce::Colour::fromString("#f27d26");
const juce::Colour WebStyleLookAndFeel::colorBgDark = juce::Colour::fromString("#0a0a0b");
const juce::Colour WebStyleLookAndFeel::colorPanel = juce::Colour::fromString("#121214");
const juce::Colour WebStyleLookAndFeel::colorBorderDark = juce::Colour::fromString("#1f1f22");
const juce::Colour WebStyleLookAndFeel::colorBorderDim = juce::Colour::fromString("#2a2a2d");
const juce::Colour WebStyleLookAndFeel::colorTextWhite = juce::Colour::fromString("#fefefe");
const juce::Colour WebStyleLookAndFeel::colorTextMuted = juce::Colour::fromString("#88888b");

WebStyleLookAndFeel::WebStyleLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, colorRetro);
    setColour(juce::Slider::rotarySliderFillColourId, colorRetro.withAlpha(0.8f));
    setColour(juce::Slider::rotarySliderOutlineColourId, colorBorderDim);
    setColour(juce::ToggleButton::tickColourId, colorRetro);
    setColour(juce::ComboBox::backgroundColourId, colorPanel);
    setColour(juce::ComboBox::outlineColourId, colorBorderDim);
    setColour(juce::ComboBox::textColourId, colorTextWhite);
    setColour(juce::TabbedComponent::backgroundColourId, colorBgDark);
    setColour(juce::TabbedComponent::outlineColourId, colorBorderDark);
    setColour(juce::TabbedButtonBar::tabTextColourId, colorTextMuted);
    setColour(juce::TabbedButtonBar::frontTextColourId, colorRetro);
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
    
    g.setColour (colorBorderDim);
    g.drawEllipse (rx, ry, rw, rw, 3.0f);
    
    juce::Path p;
    auto pointerLength = radius * 0.7f;
    auto pointerThickness = 3.0f;
    p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    
    g.setColour (colorRetro);
    g.fillPath (p);
}

void WebStyleLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, 
                                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto fontSize = juce::jmin (15.0f, (float) button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    g.setColour (button.getToggleState() ? colorRetro : colorBorderDim);
    g.fillRoundedRectangle(0.0f, (button.getHeight() - tickWidth) * 0.5f, tickWidth, tickWidth, 4.0f);

    g.setColour (colorTextWhite);
    g.setFont (fontSize);
    if (! button.isEnabled())
        g.setOpacity (0.5f);

    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().withTrimmedLeft (juce::roundToInt (tickWidth) + 10)
                                             .withTrimmedRight (2),
                      juce::Justification::centredLeft, 10);
}

void WebStyleLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown, 
                                        int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box)
{
    g.setColour (colorPanel);
    g.fillRoundedRectangle (0.0f, 0.0f, (float) width, (float) height, 4.0f);
    g.setColour (colorBorderDim);
    g.drawRoundedRectangle (0.0f, 0.0f, (float) width, (float) height, 4.0f, 1.0f);
}

juce::Font WebStyleLookAndFeel::getLabelFont (juce::Label&) { return juce::Font (13.0f); }
juce::Font WebStyleLookAndFeel::getComboBoxFont (juce::ComboBox&) { return juce::Font (14.0f); }

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
        
        // Very basic subsampling to 256 points for display
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
    g.fillAll(WebStyleLookAndFeel::colorPanel);
    g.setColour(WebStyleLookAndFeel::colorBorderDark);
    g.drawRect(getLocalBounds(), 1);

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

    g.setColour(juce::Colours::cyan.withAlpha(0.6f));
    g.strokePath(pInt, juce::PathStrokeType(1.5f));
    
    g.setColour(WebStyleLookAndFeel::colorRetro.withAlpha(0.6f));
    g.strokePath(pStep, juce::PathStrokeType(1.0f));
}

BitlayAudioProcessorEditor::BitlayAudioProcessorEditor (BitlayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), scope(p), tabs(juce::TabbedButtonBar::TabsAtTop)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&webLookAndFeel);

    juce::StringArray subdivs { "1/16", "1/3T", "1/8", "1/8D", "1/4", "1/2D", "1/2", "custom" };
    juce::StringArray circuits { "Discrete", "Companded" };

    auto& apvts = audioProcessor.apvts;

    // Main
    delayTime.init("Delay (ms)", apvts, "delayTime");
    feedback.init("Feedback", apvts, "feedback");
    mix.init("Mix", apvts, "mix");
    character.init("Character", apvts, "character");
    internalBpm.init("Int. BPM", apvts, "internalBpm");
    bpmSync.init("BPM Sync", apvts, "bpmSync");
    freeze.init("Freeze", apvts, "freeze");
    bypass.init("Bypass", apvts, "bypass");
    mainSubdivision.init("Subdiv", apvts, "mainSubdivision", subdivs);

    // LFO
    wobbleRate.init("Wobble Rate", apvts, "wobbleRate");
    wobbleDepth.init("Wobble Depth", apvts, "wobbleDepth");
    wobbleSync.init("Wobble Sync", apvts, "wobbleSync");

    // Circuit
    circuitType.init("Circuit Type", apvts, "circuitType", circuits);
    coupledMode.init("Coupled Mode", apvts, "coupledMode");
    stepSize.init("Step Size", apvts, "stepSize");
    clockJitter.init("Clock Jitter", apvts, "clockJitter");
    integratorLag.init("Int Lag", apvts, "integratorLag");
    reconCutoff.init("Recon Cutoff", apvts, "reconstructionCutoff");
    integratorLeak.init("Int Leak", apvts, "integratorLeak");
    dynamicResponse.init("Dyn Resp", apvts, "dynamicResponse");
    feedbackTone.init("FB Tone", apvts, "feedbackTone");
    stereoSpread.init("Stereo Spread", apvts, "stereoSpread");
    envAttack.init("Env Attack", apvts, "envAttack");
    envRelease.init("Env Release", apvts, "envRelease");
    minStepSize.init("Min Step", apvts, "minStepSize");
    maxStepSize.init("Max Step", apvts, "maxStepSize");
    syllabicTime.init("Syllabic", apvts, "syllabicTime");

    // Taps
    numTaps.init("Num Taps", apvts, "numTaps");
    tapDecay.init("Tap Decay", apvts, "tapDecay");
    
    tap1Mult.init("Tap1 Mult", apvts, "tap1_mult");
    tap1Mix.init("Tap1 Mix", apvts, "tap1_mix");
    tap1Subdiv.init("T1 Subdiv", apvts, "tap1_subdiv", subdivs);

    tap2Mult.init("Tap2 Mult", apvts, "tap2_mult");
    tap2Mix.init("Tap2 Mix", apvts, "tap2_mix");
    tap2Subdiv.init("T2 Subdiv", apvts, "tap2_subdiv", subdivs);

    tap3Mult.init("Tap3 Mult", apvts, "tap3_mult");
    tap3Mix.init("Tap3 Mix", apvts, "tap3_mix");
    tap3Subdiv.init("T3 Subdiv", apvts, "tap3_subdiv", subdivs);

    tap4Mult.init("Tap4 Mult", apvts, "tap4_mult");
    tap4Mix.init("Tap4 Mix", apvts, "tap4_mix");
    tap4Subdiv.init("T4 Subdiv", apvts, "tap4_subdiv", subdivs);

    // Reverse
    reverseMode.init("Reverse Mode", apvts, "reverseMode");
    reverseChunkSize.init("Chunk Size", apvts, "reverseChunkSize");
    reverseFeedback.init("Rev Feedback", apvts, "reverseFeedback");

    mainTabComp = new juce::Component();
    lfoTabComp = new juce::Component();
    circuitTabComp = new juce::Component();
    tapsTabComp = new juce::Component();
    reverseTabComp = new juce::Component();

    // Add children to components
    auto addComponents = [](juce::Component* p, std::vector<juce::Component*> c) {
        for (auto* comp : c) p->addAndMakeVisible(comp);
    };

    addComponents(mainTabComp, {&delayTime.slider, &delayTime.label, &feedback.slider, &feedback.label, &mix.slider, &mix.label, &character.slider, &character.label, &internalBpm.slider, &internalBpm.label, &mainSubdivision.combo, &mainSubdivision.label, &bpmSync.button, &freeze.button, &bypass.button});
    addComponents(lfoTabComp, {&wobbleRate.slider, &wobbleRate.label, &wobbleDepth.slider, &wobbleDepth.label, &wobbleSync.slider, &wobbleSync.label});
    addComponents(circuitTabComp, {&circuitType.combo, &circuitType.label, &coupledMode.button, &stepSize.slider, &stepSize.label, &clockJitter.slider, &clockJitter.label, &integratorLag.slider, &integratorLag.label, &reconCutoff.slider, &reconCutoff.label, &integratorLeak.slider, &integratorLeak.label, &dynamicResponse.slider, &dynamicResponse.label, &feedbackTone.slider, &feedbackTone.label, &stereoSpread.slider, &stereoSpread.label, &envAttack.slider, &envAttack.label, &envRelease.slider, &envRelease.label, &minStepSize.slider, &minStepSize.label, &maxStepSize.slider, &maxStepSize.label, &syllabicTime.slider, &syllabicTime.label});
    addComponents(tapsTabComp, {&numTaps.slider, &numTaps.label, &tapDecay.slider, &tapDecay.label, &tap1Mult.slider, &tap1Mult.label, &tap1Mix.slider, &tap1Mix.label, &tap1Subdiv.combo, &tap1Subdiv.label, &tap2Mult.slider, &tap2Mult.label, &tap2Mix.slider, &tap2Mix.label, &tap2Subdiv.combo, &tap2Subdiv.label, &tap3Mult.slider, &tap3Mult.label, &tap3Mix.slider, &tap3Mix.label, &tap3Subdiv.combo, &tap3Subdiv.label, &tap4Mult.slider, &tap4Mult.label, &tap4Mix.slider, &tap4Mix.label, &tap4Subdiv.combo, &tap4Subdiv.label});
    addComponents(reverseTabComp, {&reverseMode.button, &reverseChunkSize.slider, &reverseChunkSize.label, &reverseFeedback.slider, &reverseFeedback.label});

    tabs.addTab("Main", WebStyleLookAndFeel::colorBgDark, mainTabComp, true);
    tabs.addTab("LFO", WebStyleLookAndFeel::colorBgDark, lfoTabComp, true);
    tabs.addTab("Circuit", WebStyleLookAndFeel::colorBgDark, circuitTabComp, true);
    tabs.addTab("Taps", WebStyleLookAndFeel::colorBgDark, tapsTabComp, true);
    tabs.addTab("Reverse", WebStyleLookAndFeel::colorBgDark, reverseTabComp, true);

    addAndMakeVisible(tabs);
    addAndMakeVisible(scope);

    setSize (700, 500);
}

BitlayAudioProcessorEditor::~BitlayAudioProcessorEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void BitlayAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (WebStyleLookAndFeel::colorBgDark);
}



// Simpler layout
void BitlayAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    scope.setBounds(area.removeFromTop(100).reduced(10));
    tabs.setBounds(area.reduced(10));

    int w = 80;
    int h = 90;
    
    auto placeKnob = [&](SliderWithLabel& swl, int x, int y) {
        swl.slider.setBounds(x, y, w, h-20);
        swl.label.setBounds(x, y + h - 20, w, 20);
    };
    auto placeCombo = [&](ComboWithLabel& cwl, int x, int y, int bw=100) {
        cwl.combo.setBounds(x, y, bw, 24);
        cwl.label.setBounds(x, y - 20, bw, 20);
    };
    auto placeToggle = [&](ToggleWithLabel& twl, int x, int y, int bw=100) {
        twl.button.setBounds(x, y, bw, 24);
    };

    // Main Tab
    placeKnob(delayTime, 20, 20);
    placeKnob(feedback, 110, 20);
    placeKnob(mix, 200, 20);
    placeKnob(character, 290, 20);
    placeKnob(internalBpm, 380, 20);
    
    placeCombo(mainSubdivision, 20, 150, 80);
    placeToggle(bpmSync, 120, 150);
    placeToggle(freeze, 220, 150);
    placeToggle(bypass, 320, 150);

    // LFO Tab
    placeKnob(wobbleRate, 20, 20);
    placeKnob(wobbleDepth, 110, 20);
    placeKnob(wobbleSync, 200, 20);

    // Circuit Tab
    placeCombo(circuitType, 20, 40);
    placeToggle(coupledMode, 140, 40);
    placeKnob(stepSize, 20, 80);
    placeKnob(clockJitter, 110, 80);
    placeKnob(integratorLag, 200, 80);
    placeKnob(reconCutoff, 290, 80);
    placeKnob(integratorLeak, 380, 80);
    placeKnob(dynamicResponse, 470, 80);
    placeKnob(feedbackTone, 560, 80);
    placeKnob(stereoSpread, 20, 200);
    placeKnob(envAttack, 110, 200);
    placeKnob(envRelease, 200, 200);
    placeKnob(minStepSize, 290, 200);
    placeKnob(maxStepSize, 380, 200);
    placeKnob(syllabicTime, 470, 200);

    // Taps Tab
    placeKnob(numTaps, 20, 20);
    placeKnob(tapDecay, 110, 20);
    
    int rowY = 130;
    placeKnob(tap1Mult, 20, rowY);
    placeKnob(tap1Mix, 110, rowY);
    placeCombo(tap1Subdiv, 200, rowY + 30, 80);

    placeKnob(tap2Mult, 300, rowY);
    placeKnob(tap2Mix, 390, rowY);
    placeCombo(tap2Subdiv, 480, rowY + 30, 80);

    placeKnob(tap3Mult, 20, rowY + 110);
    placeKnob(tap3Mix, 110, rowY + 110);
    placeCombo(tap3Subdiv, 200, rowY + 140, 80);

    placeKnob(tap4Mult, 300, rowY + 110);
    placeKnob(tap4Mix, 390, rowY + 110);
    placeCombo(tap4Subdiv, 480, rowY + 140, 80);

    // Reverse Tab
    placeToggle(reverseMode, 20, 20);
    placeKnob(reverseChunkSize, 140, 20);
    placeKnob(reverseFeedback, 230, 20);
}
