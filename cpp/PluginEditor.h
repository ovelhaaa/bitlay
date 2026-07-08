#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

// ==============================================================================
// Modern Web-Styled LookAndFeel
// Colors based on React CSS: 
// retro: #f27d26, bg: #0a0a0b, panel: #121214, border: #1f1f22
// ==============================================================================
class WebStyleLookAndFeel : public juce::LookAndFeel_V4
{
public:
    WebStyleLookAndFeel();
    
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
                           
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, 
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
                           
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown, 
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;
                       
    juce::Font getLabelFont (juce::Label& label) override;
    juce::Font getComboBoxFont (juce::ComboBox& box) override;

    // Custom Web colors
    static const juce::Colour colorRetro;
    static const juce::Colour colorBgDark;
    static const juce::Colour colorPanel;
    static const juce::Colour colorBorderDark;
    static const juce::Colour colorBorderDim;
    static const juce::Colour colorTextWhite;
    static const juce::Colour colorTextMuted;
};

// ==============================================================================
// Custom Label for Web-like Typography
// ==============================================================================
class WebLabel : public juce::Label
{
public:
    WebLabel(const juce::String& name = "", const juce::String& labelText = "");
};

// ==============================================================================
// Oscilloscope Component
// ==============================================================================
class OscilloscopeVisualizer : public juce::Component, public juce::Timer
{
public:
    OscilloscopeVisualizer(BitlayAudioProcessor& p);
    ~OscilloscopeVisualizer() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    BitlayAudioProcessor& processor;
    std::vector<float> integratorData;
    std::vector<float> stepSizeData;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscilloscopeVisualizer)
};

// ==============================================================================
// Main Editor Component
// ==============================================================================
class BitlayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BitlayAudioProcessorEditor (BitlayAudioProcessor&);
    ~BitlayAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BitlayAudioProcessor& audioProcessor;
    WebStyleLookAndFeel webLookAndFeel;

    OscilloscopeVisualizer scope;

    juce::Slider delayTimeSlider;
    juce::Slider feedbackSlider;
    juce::Slider mixSlider;
    juce::Slider wobbleDepthSlider;
    
    WebLabel delayTimeLabel;
    WebLabel feedbackLabel;
    WebLabel mixLabel;
    WebLabel wobbleDepthLabel;

    juce::ToggleButton bypassButton;
    juce::ToggleButton reverseButton;
    juce::ComboBox circuitTypeCombo;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wobbleDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> reverseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> circuitTypeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitlayAudioProcessorEditor)
};
