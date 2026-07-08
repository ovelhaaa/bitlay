#include "PluginProcessor.h"
#include "PluginEditor.h"

// ==============================================================================
// WebStyleLookAndFeel
// ==============================================================================
const juce::Colour WebStyleLookAndFeel::colorRetro        = juce::Colour(0xfff27d26);
const juce::Colour WebStyleLookAndFeel::colorBgDark       = juce::Colour(0xff0a0a0b);
const juce::Colour WebStyleLookAndFeel::colorPanel        = juce::Colour(0xff121214);
const juce::Colour WebStyleLookAndFeel::colorBorderDark   = juce::Colour(0xff1f1f22);
const juce::Colour WebStyleLookAndFeel::colorBorderDim    = juce::Colour(0xff1a1a1c);
const juce::Colour WebStyleLookAndFeel::colorTextWhite    = juce::Colour(0xffffffff);
const juce::Colour WebStyleLookAndFeel::colorTextMuted    = juce::Colour(0xffa1a1aa);

WebStyleLookAndFeel::WebStyleLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, colorTextWhite);
    setColour(juce::Slider::textBoxBackgroundColourId, colorPanel);
    setColour(juce::Slider::textBoxOutlineColourId, colorBorderDark);
    setColour(juce::Slider::textBoxHighlightColourId, colorRetro.withAlpha(0.3f));
    
    setColour(juce::ComboBox::backgroundColourId, colorPanel);
    setColour(juce::ComboBox::textColourId, colorTextWhite);
    setColour(juce::ComboBox::outlineColourId, colorBorderDark);
    setColour(juce::ComboBox::arrowColourId, colorRetro);
    setColour(juce::PopupMenu::backgroundColourId, colorPanel);
    setColour(juce::PopupMenu::textColourId, colorTextMuted);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, colorRetro.withAlpha(0.2f));
    setColour(juce::PopupMenu::highlightedTextColourId, colorRetro);
}

juce::Font WebStyleLookAndFeel::getLabelFont (juce::Label&)
{
    // Simulating 'Outfit' sans-serif modern font
    return juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain);
}

juce::Font WebStyleLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain);
}

void WebStyleLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                            const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = (float) juce::jmin (width / 2, height / 2) - 8.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Background track (dark circle)
    g.setColour (colorPanel);
    g.fillEllipse (rx, ry, rw, rw);
    g.setColour (colorBorderDark);
    g.drawEllipse (rx, ry, rw, rw, 2.0f);

    // Value Arc
    juce::Path valueArc;
    valueArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (colorRetro);
    g.strokePath (valueArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Pointer dot
    juce::Path p;
    auto pointerRadius = 3.5f;
    p.addEllipse(-pointerRadius, -radius + 6.0f, pointerRadius * 2.0f, pointerRadius * 2.0f);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    
    g.setColour (colorTextWhite);
    g.fillPath (p);
}

void WebStyleLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, 
                                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    bool isOn = button.getToggleState();
    
    g.setColour(isOn ? colorRetro.withAlpha(0.15f) : colorPanel);
    g.fillRoundedRectangle(bounds, 6.0f);
    
    g.setColour(isOn ? colorRetro : colorBorderDark);
    g.drawRoundedRectangle(bounds, 6.0f, 1.5f);

    g.setColour(isOn ? colorRetro : colorTextMuted);
    g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
}

void WebStyleLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown, 
                                        int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box)
{
    auto bounds = box.getLocalBounds().toFloat();
    
    g.setColour(colorPanel);
    g.fillRoundedRectangle(bounds, 6.0f);
    
    g.setColour(colorBorderDark);
    g.drawRoundedRectangle(bounds, 6.0f, 1.5f);
    
    // Draw Arrow
    juce::Path path;
    path.addTriangle(buttonX + buttonW * 0.3f, buttonY + buttonH * 0.4f,
                     buttonX + buttonW * 0.7f, buttonY + buttonH * 0.4f,
                     buttonX + buttonW * 0.5f, buttonY + buttonH * 0.6f);
                     
    g.setColour(colorTextMuted);
    g.fillPath(path);
}

// ==============================================================================
// WebLabel
// ==============================================================================
WebLabel::WebLabel(const juce::String& name, const juce::String& labelText)
    : juce::Label(name, labelText)
{
    setJustificationType(juce::Justification::centred);
    setColour(juce::Label::textColourId, WebStyleLookAndFeel::colorTextMuted);
}


// ==============================================================================
// OscilloscopeVisualizer
// ==============================================================================
OscilloscopeVisualizer::OscilloscopeVisualizer(BitlayAudioProcessor& p) : processor(p)
{
    integratorData.resize(1024, 0.0f);
    stepSizeData.resize(1024, 0.0f);
    startTimerHz(30);
}

OscilloscopeVisualizer::~OscilloscopeVisualizer()
{
    stopTimer();
}

void OscilloscopeVisualizer::timerCallback()
{
    int start1, size1, start2, size2;
    processor.scopeFifo.prepareToRead(processor.scopeFifo.getNumReady(), start1, size1, start2, size2);

    if (size1 > 0 || size2 > 0)
    {
        int totalRead = size1 + size2;
        int maxLen = (int)integratorData.size();
        int shift = juce::jmin(totalRead, maxLen);
        
        if (shift < maxLen)
        {
            std::copy(integratorData.begin() + shift, integratorData.end(), integratorData.begin());
            std::copy(stepSizeData.begin() + shift, stepSizeData.end(), stepSizeData.begin());
        }

        int writePos = maxLen - shift;
        
        auto readFromBuffer = [&](int start, int size, int& wp) {
            int toRead = juce::jmin(size, maxLen - wp);
            if (toRead > 0) {
                int readOffset = size - toRead; 
                std::copy(processor.diagIntegratorBuffer.begin() + start + readOffset, 
                          processor.diagIntegratorBuffer.begin() + start + readOffset + toRead, 
                          integratorData.begin() + wp);
                std::copy(processor.diagStepSizeBuffer.begin() + start + readOffset, 
                          processor.diagStepSizeBuffer.begin() + start + readOffset + toRead, 
                          stepSizeData.begin() + wp);
                wp += toRead;
            }
        };

        int wp1 = writePos;
        readFromBuffer(start1, size1, wp1);
        int wp2 = wp1;
        readFromBuffer(start2, size2, wp2);

        processor.scopeFifo.finishedRead(totalRead);
        repaint();
    }
}

void OscilloscopeVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background panel
    g.setColour(WebStyleLookAndFeel::colorPanel);
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(WebStyleLookAndFeel::colorBorderDark);
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);

    if (integratorData.empty()) return;

    juce::Path pInt;
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();
    auto midY = h / 2.0f;
    
    // Integrator (White/Muted)
    g.setColour(WebStyleLookAndFeel::colorTextWhite.withAlpha(0.7f));
    pInt.startNewSubPath(0, midY - integratorData[0] * midY);
    for (size_t i = 1; i < integratorData.size(); ++i)
    {
        float x = (float)i / (float)integratorData.size() * w;
        float y = midY - integratorData[i] * midY * 0.5f; 
        pInt.lineTo(x, y);
    }
    g.strokePath(pInt, juce::PathStrokeType(1.5f));

    // Step Size (Retro color)
    juce::Path pStep;
    g.setColour(WebStyleLookAndFeel::colorRetro.withAlpha(0.85f));
    pStep.startNewSubPath(0, midY - stepSizeData[0] * midY);
    for (size_t i = 1; i < stepSizeData.size(); ++i)
    {
        float x = (float)i / (float)stepSizeData.size() * w;
        float y = h - stepSizeData[i] * h * 5.0f; 
        pStep.lineTo(x, y);
    }
    g.strokePath(pStep, juce::PathStrokeType(1.2f));
}

// ==============================================================================
// BitlayAudioProcessorEditor
// ==============================================================================
BitlayAudioProcessorEditor::BitlayAudioProcessorEditor (BitlayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), scope(p),
      delayTimeLabel("", "TIME"), feedbackLabel("", "FDBK"), 
      mixLabel("", "MIX"), wobbleDepthLabel("", "WOBBLE")
{
    setLookAndFeel(&webLookAndFeel);

    addAndMakeVisible(scope);

    auto setupSlider = [this](juce::Slider& s, WebLabel& l, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& a, const juce::String& paramID) {
        addAndMakeVisible(s);
        addAndMakeVisible(l);
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        // Simulate JetBrains Mono by using a monospaced font for the text box
        s.setTextBoxIsEditable(true);
        a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramID, s);
    };

    setupSlider(delayTimeSlider, delayTimeLabel, delayTimeAttachment, "delayTime");
    setupSlider(feedbackSlider, feedbackLabel, feedbackAttachment, "feedback");
    setupSlider(mixSlider, mixLabel, mixAttachment, "mix");
    setupSlider(wobbleDepthSlider, wobbleDepthLabel, wobbleDepthAttachment, "wobbleDepth");

    addAndMakeVisible(bypassButton);
    bypassButton.setButtonText("BYPASS");
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "bypass", bypassButton);

    addAndMakeVisible(reverseButton);
    reverseButton.setButtonText("REVERSE");
    reverseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "reverseMode", reverseButton);

    addAndMakeVisible(circuitTypeCombo);
    circuitTypeCombo.addItemList({"Clean", "Gritty", "Broken"}, 1);
    circuitTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "circuitType", circuitTypeCombo);

    setSize (640, 480);
}

BitlayAudioProcessorEditor::~BitlayAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void BitlayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background Dark
    g.fillAll (WebStyleLookAndFeel::colorBgDark);

    // Draw header background
    juce::Rectangle<float> headerRect(0.0f, 0.0f, (float)getWidth(), 60.0f);
    g.setColour(WebStyleLookAndFeel::colorPanel);
    g.fillRect(headerRect);
    g.setColour(WebStyleLookAndFeel::colorBorderDark);
    g.drawLine(0.0f, 60.0f, (float)getWidth(), 60.0f, 2.0f);

    g.setColour (WebStyleLookAndFeel::colorRetro);
    g.setFont (juce::Font(juce::Font::getDefaultSansSerifFontName(), 24.0f, juce::Font::bold));
    g.drawText ("BITLAY", 20, 0, 200, 60, juce::Justification::centredLeft);
    
    g.setColour (WebStyleLookAndFeel::colorTextMuted);
    g.setFont (juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain));
    g.drawText ("CVSD DELAY", 110, 0, 200, 60, juce::Justification::centredLeft);
}

void BitlayAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Header
    auto headerArea = area.removeFromTop(60);
    
    // Header controls
    auto comboArea = headerArea.removeFromRight(150).reduced(0, 15);
    comboArea.removeFromRight(20);
    circuitTypeCombo.setBounds(comboArea);

    area.reduce(20, 20); // main padding

    // Visualizer Area
    auto topArea = area.removeFromTop(180);
    scope.setBounds(topArea);

    area.removeFromTop(20); // spacing

    // Knobs and buttons Area
    auto controlsArea = area;
    
    int knobWidth = 90;
    
    auto layoutKnob = [&](juce::Slider& s, WebLabel& l, juce::Rectangle<int>& r) {
        auto cell = r.removeFromLeft(knobWidth);
        l.setBounds(cell.removeFromTop(20));
        s.setBounds(cell);
        r.removeFromLeft(10); // spacing
    };

    layoutKnob(delayTimeSlider, delayTimeLabel, controlsArea);
    layoutKnob(feedbackSlider, feedbackLabel, controlsArea);
    layoutKnob(mixSlider, mixLabel, controlsArea);
    layoutKnob(wobbleDepthSlider, wobbleDepthLabel, controlsArea);
    
    // Buttons align to the right
    auto buttonArea = controlsArea.removeFromRight(100);
    reverseButton.setBounds(buttonArea.removeFromTop(30));
    buttonArea.removeFromTop(15);
    bypassButton.setBounds(buttonArea.removeFromTop(30));
}
