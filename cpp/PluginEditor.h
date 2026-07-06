#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

// Custom LookAndFeel for Hardware/Lo-Fi Aesthetic
class BitlayLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BitlayLookAndFeel();
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

// Oscilloscope Component
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

class BitlayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BitlayAudioProcessorEditor (BitlayAudioProcessor&);
    ~BitlayAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BitlayAudioProcessor& audioProcessor;
    BitlayLookAndFeel customLookAndFeel;

    OscilloscopeVisualizer scope;

    juce::Slider delayTimeSlider;
    juce::Slider feedbackSlider;
    juce::Slider mixSlider;
    juce::Slider wobbleDepthSlider;
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
