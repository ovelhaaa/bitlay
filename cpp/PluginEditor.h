#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include <memory>

class WebStyleLookAndFeel : public juce::LookAndFeel_V4
{
public:
    WebStyleLookAndFeel();
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, 
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getLabelFont (juce::Label& label) override;
    juce::Font getComboBoxFont (juce::ComboBox& box) override;

    static const juce::Colour colorRetro;
    static const juce::Colour colorBgDark;
    static const juce::Colour colorPanel;
    static const juce::Colour colorBorderDark;
    static const juce::Colour colorBorderDim;
    static const juce::Colour colorTextWhite;
    static const juce::Colour colorTextMuted;
};

class WebLabel : public juce::Label
{
public:
    WebLabel(const juce::String& name = "", const juce::String& labelText = "");
};

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

struct SliderWithLabel {
    juce::Slider slider;
    WebLabel label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    void init(const juce::String& labelText, juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, slider);
    }
};

struct ToggleWithLabel {
    juce::ToggleButton button;
    WebLabel label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    void init(const juce::String& labelText, juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId) {
        button.setButtonText(labelText);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, paramId, button);
    }
};

struct ComboWithLabel {
    juce::ComboBox combo;
    WebLabel label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    void init(const juce::String& labelText, juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::StringArray& choices) {
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        combo.addItemList(choices, 1);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, paramId, combo);
    }
};

class BitlayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BitlayAudioProcessorEditor (BitlayAudioProcessor&);
    ~BitlayAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void updatePresetList();
private:
    BitlayAudioProcessor& audioProcessor;
    WebStyleLookAndFeel webLookAndFeel;
    OscilloscopeVisualizer scope;

    juce::TabbedComponent tabs;

    // Preset Header
    juce::ComboBox presetComboBox;
    juce::TextButton savePresetButton;
    juce::TextButton newPresetButton;

    // Components
    // MAIN
    SliderWithLabel delayTime, feedback, mix, character, internalBpm;
    ToggleWithLabel bypass, freeze, bpmSync;
    ComboWithLabel mainSubdivision;

    // LFO
    SliderWithLabel wobbleRate, wobbleDepth, wobbleSync;

    // CIRCUIT
    ComboWithLabel circuitType;
    ToggleWithLabel coupledMode;
    SliderWithLabel stepSize, clockJitter, integratorLag, reconCutoff, integratorLeak, dynamicResponse, feedbackTone, stereoSpread;
    SliderWithLabel envAttack, envRelease, minStepSize, maxStepSize, syllabicTime;

    // TAPS
    SliderWithLabel numTaps, tapDecay;
    SliderWithLabel tap1Mult, tap1Mix, tap2Mult, tap2Mix, tap3Mult, tap3Mix, tap4Mult, tap4Mix;
    ComboWithLabel tap1Subdiv, tap2Subdiv, tap3Subdiv, tap4Subdiv;

    // REVERSE
    ToggleWithLabel reverseMode;
    SliderWithLabel reverseChunkSize, reverseFeedback;



    juce::Component* mainTabComp;
    juce::Component* lfoTabComp;
    juce::Component* circuitTabComp;
    juce::Component* tapsTabComp;
    juce::Component* reverseTabComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitlayAudioProcessorEditor)
};
