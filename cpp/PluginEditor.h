#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include <memory>

namespace BitlayUi
{
    static constexpr int outerMargin = 20;
    static constexpr int panelRadius = 8;
    static constexpr int controlRadius = 5;

    inline void prepareSliderInteraction(juce::Slider& slider,
                                         juce::AudioProcessorValueTreeState& apvts,
                                         const juce::String& paramId)
    {
        slider.setMouseDragSensitivity(180);

        if (auto* parameter = apvts.getParameter(paramId))
            slider.setDoubleClickReturnValue(true, parameter->convertFrom0to1(parameter->getDefaultValue()));
    }
}

class BitlayLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BitlayLookAndFeel();
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, 
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawTabButton (juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown) override;
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;
    void drawGroupComponentOutline (juce::Graphics& g, int width, int height, const juce::String& text,
                                    const juce::Justification& position, juce::GroupComponent& group) override;

    juce::Font getLabelFont (juce::Label& label) override;
    juce::Font getComboBoxFont (juce::ComboBox& box) override;

    static const juce::Colour accent;
    static const juce::Colour accentSoft;
    static const juce::Colour background;
    static const juce::Colour panel;
    static const juce::Colour panelRaised;
    static const juce::Colour borderStrong;
    static const juce::Colour borderSubtle;
    static const juce::Colour textPrimary;
    static const juce::Colour textSecondary;
    static const juce::Colour textMuted;
    static const juce::Colour meter;
    static const juce::Colour warning;
};

class BitlayLabel : public juce::Label
{
public:
    BitlayLabel(const juce::String& name = "", const juce::String& labelText = "");
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

class TapPatternVisualizer : public juce::Component, public juce::Timer
{
public:
    TapPatternVisualizer(BitlayAudioProcessor& p);
    ~TapPatternVisualizer() override;
    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    juce::Rectangle<float> getPlotBounds() const;
    void updateTapFromMouse(const juce::MouseEvent& event);
    float readParam(const juce::String& id, float fallback) const;
    void setParam(const juce::String& id, float value);

    BitlayAudioProcessor& processor;
    int activeDragTap = -1;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapPatternVisualizer)
};

struct SliderWithLabel {
    juce::Slider slider;
    BitlayLabel label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    void init(const juce::String& labelText, juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        BitlayUi::prepareSliderInteraction(slider, apvts, paramId);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, slider);
    }
};

struct LinearSliderWithLabel {
    juce::Slider slider;
    BitlayLabel label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    void init(const juce::String& labelText, juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId) {
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
        BitlayUi::prepareSliderInteraction(slider, apvts, paramId);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredLeft);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, slider);
    }
};

struct ToggleWithLabel {
    juce::ToggleButton button;
    BitlayLabel label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    void init(const juce::String& labelText, juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId) {
        button.setButtonText(labelText);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, paramId, button);
    }
};

struct ComboWithLabel {
    juce::ComboBox combo;
    BitlayLabel label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    void init(const juce::String& labelText, juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::StringArray& choices) {
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        combo.addItemList(choices, 1);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, paramId, combo);
    }
};

class BitlayAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener, public juce::Timer
{
public:
    BitlayAudioProcessorEditor (BitlayAudioProcessor&);
    ~BitlayAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void updatePresetList();
    void sliderValueChanged (juce::Slider* slider) override;
    void timerCallback() override;
    
private:
    void showSaveAsDialog();
    void updatePresetStatus();
    juce::String getPresetCategory(const juce::String& presetName) const;
    juce::String getPresetDescription(const juce::String& presetName) const;
    void updatePresetDescription();
    bool loadMonitorVisibility() const;
    juce::Point<int> loadEditorSize() const;
    void saveMonitorVisibility() const;

    BitlayAudioProcessor& audioProcessor;
    BitlayLookAndFeel bitlayLookAndFeel;
    OscilloscopeVisualizer scope;
    TapPatternVisualizer tapPattern;

    juce::TabbedComponent tabs;

    // Preset Header
    juce::ComboBox presetCategoryCombo;
    juce::ComboBox presetComboBox;
    juce::TextButton previousPresetButton;
    juce::TextButton nextPresetButton;
    juce::TextButton savePresetButton;
    juce::TextButton newPresetButton;
    juce::Label presetStatusLabel;
    juce::Label presetDescriptionLabel;
    juce::TextButton monitorToggleButton;
    std::unique_ptr<juce::AlertWindow> saveAsDialog;
    bool monitorVisible = false;

    // Global Top Bar
    ToggleWithLabel reverseMode;
    ToggleWithLabel bypass;
    juce::Label pluginTitle;
    juce::Label presetLabel;

    // Groups (Frames)
    juce::GroupComponent groupTime{"groupTime", "Delay"};
    juce::GroupComponent groupMacros{"groupMacros", "Color & Motion"};
    juce::GroupComponent groupMix{"groupMix", "Output"};
    
    juce::GroupComponent groupTapGlobal{"groupTapGlobal", "Global Taps Settings"};
    juce::GroupComponent groupTapPattern{"groupTapPattern", "Tap Pattern"};
    juce::GroupComponent groupTap1{"groupTap1", "Tap 1"};
    juce::GroupComponent groupTap2{"groupTap2", "Tap 2"};
    juce::GroupComponent groupTap3{"groupTap3", "Tap 3"};
    juce::GroupComponent groupTap4{"groupTap4", "Tap 4"};

    juce::GroupComponent groupEngine{"groupEngine", "Delta Core"};
    juce::GroupComponent groupFilters{"groupFilters", "Bandwidth & Drift"};
    juce::GroupComponent groupEnv{"groupEnv", "CVSD Response"};
    juce::GroupComponent groupLfo{"groupLfo", "Motion"};
    
    juce::GroupComponent groupRev{"groupRev", "Reverse Texture"};

    // Components
    // MAIN / MACROS
    SliderWithLabel delayTime, feedback, mix;
    LinearSliderWithLabel internalBpm;
    ToggleWithLabel freeze, bpmSync;
    ComboWithLabel mainSubdivision;
    
    // Macros
    juce::Slider macroTexture;
    BitlayLabel labelTexture;
    juce::Slider macroMovement;
    BitlayLabel labelMovement;

    // MODULATION (LFO)
    SliderWithLabel wobbleRate, wobbleDepth, wobbleSync;

    // CIRCUIT (CVSD)
    ComboWithLabel circuitType;
    ToggleWithLabel coupledMode;
    SliderWithLabel stepSize, clockJitter, integratorLag, reconCutoff, integratorLeak, dynamicResponse, feedbackTone, stereoSpread;
    SliderWithLabel envAttack, envRelease, minStepSize, maxStepSize, syllabicTime;
    SliderWithLabel character;

    // TAPS
    SliderWithLabel numTaps, tapDecay;
    SliderWithLabel tap1Mult, tap1Mix, tap2Mult, tap2Mix, tap3Mult, tap3Mix, tap4Mult, tap4Mix;
    ComboWithLabel tap1Subdiv, tap2Subdiv, tap3Subdiv, tap4Subdiv;

    // REVERSE SETTINGS
    SliderWithLabel reverseChunkSize, reverseFeedback;

    juce::Component* mainTabComp;
    juce::Component* tapsTabComp;
    juce::Component* circuitTabComp;
    juce::Component* reverseTabComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitlayAudioProcessorEditor)
};

