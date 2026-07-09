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

    void drawTabButton (juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown) override;
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;

    juce::Font getLabelFont (juce::Label& label) override;
    juce::Font getComboBoxFont (juce::ComboBox& box) override;

    static const juce::Colour colorRetro;
    static const juce::Colour colorBgDark;
    static const juce::Colour colorPanel;
    static const juce::Colour colorBorderDark;
    static const juce::Colour colorBorderDim;
    static const juce::Colour colorTextWhite;
    static const juce::Colour colorTextMuted;
    
    // Neon Colors
    static const juce::Colour colorNeonCyan;
    static const juce::Colour colorNeonOrange;
    static const juce::Colour colorNeonPurple;
    static const juce::Colour colorNeonRed;
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

class BitlayAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    BitlayAudioProcessorEditor (BitlayAudioProcessor&);
    ~BitlayAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void updatePresetList();
    void sliderValueChanged (juce::Slider* slider) override;
    
private:
    BitlayAudioProcessor& audioProcessor;
    WebStyleLookAndFeel webLookAndFeel;
    OscilloscopeVisualizer scope;

    juce::TabbedComponent tabs;

    // Preset Header
    juce::ComboBox presetComboBox;
    juce::TextButton savePresetButton;
    juce::TextButton newPresetButton;

    // Global Top Bar
    ToggleWithLabel reverseMode;
    ToggleWithLabel bypass;
    juce::Label pluginTitle;
    juce::Label presetLabel;

    // Components
    // MAIN / MACROS
    SliderWithLabel delayTime, feedback, mix, internalBpm;
    ToggleWithLabel freeze, bpmSync;
    ComboWithLabel mainSubdivision;
    
    // Macros (Not connected to APVTS directly)
    juce::Slider macroTexture;
    WebLabel labelTexture;
    juce::Slider macroMovement;
    WebLabel labelMovement;

    // MODULATION (LFO)
    SliderWithLabel wobbleRate, wobbleDepth, wobbleSync;

    // CIRCUIT (CVSD)
    ComboWithLabel circuitType;
    ToggleWithLabel coupledMode;
    SliderWithLabel stepSize, clockJitter, integratorLag, reconCutoff, integratorLeak, dynamicResponse, feedbackTone, stereoSpread;
    SliderWithLabel envAttack, envRelease, minStepSize, maxStepSize, syllabicTime;
    SliderWithLabel character; // Moved to circuit

    // TAPS
    SliderWithLabel numTaps, tapDecay;
    SliderWithLabel tap1Mult, tap1Mix, tap2Mult, tap2Mix, tap3Mult, tap3Mix, tap4Mult, tap4Mix;
    ComboWithLabel tap1Subdiv, tap2Subdiv, tap3Subdiv, tap4Subdiv;

    // REVERSE SETTINGS (Inside Tab)
    SliderWithLabel reverseChunkSize, reverseFeedback;

    juce::Component* mainTabComp;
    juce::Component* tapsTabComp;
    juce::Component* circuitTabComp;
    juce::Component* reverseTabComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitlayAudioProcessorEditor)
};

