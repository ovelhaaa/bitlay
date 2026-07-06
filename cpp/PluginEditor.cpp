#include "PluginProcessor.h"
#include "PluginEditor.h"

// ==============================================================================
// BitlayLookAndFeel
// ==============================================================================
BitlayLookAndFeel::BitlayLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffe6c229)); // warm yellow/orange
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
}

void BitlayLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Knob Body
    g.setColour (juce::Colour(0xff2a2a2a));
    g.fillEllipse (rx, ry, rw, rw);

    // Outline
    g.setColour (juce::Colours::black);
    g.drawEllipse (rx, ry, rw, rw, 2.0f);

    // Pointer
    juce::Path p;
    auto pointerLength = radius * 0.7f;
    auto pointerThickness = 3.0f;
    p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));

    g.setColour (findColour(juce::Slider::rotarySliderFillColourId));
    g.fillPath (p);
}

void BitlayLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, 
                                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto toggleSize = bounds.getHeight() * 0.6f;
    juce::Rectangle<float> toggleRect(bounds.getX(), bounds.getCentreY() - toggleSize * 0.5f, toggleSize, toggleSize);

    g.setColour(button.getToggleState() ? juce::Colour(0xffe6c229) : juce::Colours::darkgrey);
    g.fillRoundedRectangle(toggleRect, 4.0f);
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(toggleRect, 4.0f, 2.0f);

    g.setColour(button.findColour(juce::ToggleButton::textColourId));
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), bounds.withTrimmedLeft(toggleSize + 5.0f), juce::Justification::centredLeft);
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
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::darkgreen);
    g.drawRect(getLocalBounds(), 2);

    if (integratorData.empty()) return;

    juce::Path p;
    auto w = (float)getWidth();
    auto h = (float)getHeight();
    auto midY = h / 2.0f;
    
    g.setColour(juce::Colours::lightgreen);
    p.startNewSubPath(0, midY - integratorData[0] * midY);
    for (size_t i = 1; i < integratorData.size(); ++i)
    {
        float x = (float)i / (float)integratorData.size() * w;
        float y = midY - integratorData[i] * midY * 0.5f; 
        p.lineTo(x, y);
    }
    g.strokePath(p, juce::PathStrokeType(1.5f));

    juce::Path pStep;
    g.setColour(juce::Colours::cyan.withAlpha(0.6f));
    pStep.startNewSubPath(0, midY - stepSizeData[0] * midY);
    for (size_t i = 1; i < stepSizeData.size(); ++i)
    {
        float x = (float)i / (float)stepSizeData.size() * w;
        float y = h - stepSizeData[i] * h * 5.0f; 
        pStep.lineTo(x, y);
    }
    g.strokePath(pStep, juce::PathStrokeType(1.0f));
}


// ==============================================================================
// BitlayAudioProcessorEditor
// ==============================================================================
BitlayAudioProcessorEditor::BitlayAudioProcessorEditor (BitlayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), scope(p)
{
    setLookAndFeel(&customLookAndFeel);

    addAndMakeVisible(scope);

    addAndMakeVisible(delayTimeSlider);
    delayTimeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "delayTime", delayTimeSlider);

    addAndMakeVisible(feedbackSlider);
    feedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "feedback", feedbackSlider);

    addAndMakeVisible(mixSlider);
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mix", mixSlider);

    addAndMakeVisible(wobbleDepthSlider);
    wobbleDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    wobbleDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    wobbleDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "wobbleDepth", wobbleDepthSlider);

    addAndMakeVisible(bypassButton);
    bypassButton.setButtonText("Bypass");
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "bypass", bypassButton);

    addAndMakeVisible(reverseButton);
    reverseButton.setButtonText("Reverse");
    reverseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "reverseMode", reverseButton);

    addAndMakeVisible(circuitTypeCombo);
    circuitTypeCombo.addItemList({"Clean", "Gritty", "Broken"}, 1);
    circuitTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "circuitType", circuitTypeCombo);

    setSize (600, 400);
}

BitlayAudioProcessorEditor::~BitlayAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void BitlayAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff333333));

    g.setColour (juce::Colours::white);
    g.setFont (30.0f);
    g.drawFittedText ("BITLAY CVSD DELAY", getLocalBounds().removeFromTop(50), juce::Justification::centred, 1);
}

void BitlayAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(50); // Header

    auto topArea = area.removeFromTop(150);
    
    int knobWidth = topArea.getWidth() / 4;
    delayTimeSlider.setBounds(topArea.removeFromLeft(knobWidth).reduced(10));
    feedbackSlider.setBounds(topArea.removeFromLeft(knobWidth).reduced(10));
    mixSlider.setBounds(topArea.removeFromLeft(knobWidth).reduced(10));
    wobbleDepthSlider.setBounds(topArea.removeFromLeft(knobWidth).reduced(10));

    auto midArea = area.removeFromTop(50);
    bypassButton.setBounds(midArea.removeFromLeft(100).reduced(10));
    reverseButton.setBounds(midArea.removeFromLeft(100).reduced(10));
    
    auto comboArea = midArea.removeFromRight(150).reduced(10);
    circuitTypeCombo.setBounds(comboArea);

    scope.setBounds(area.reduced(20));
}
