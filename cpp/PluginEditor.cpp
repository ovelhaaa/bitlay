#include "PluginEditor.h"
#include <cmath>
#include <limits>

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

    if (! button.isEnabled())
        base = base.withAlpha(0.45f);

    g.setColour(base);
    g.fillRoundedRectangle(bounds, (float) BitlayUi::controlRadius);
    g.setColour((button.getToggleState() ? accent : borderStrong).withAlpha(button.isEnabled() ? 0.8f : 0.35f));
    g.drawRoundedRectangle(bounds, (float) BitlayUi::controlRadius, 1.0f);
}

void BitlayLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                        bool, bool)
{
    g.setColour((button.getToggleState() ? textPrimary : textSecondary).withAlpha(button.isEnabled() ? 1.0f : 0.42f));
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

TapPatternVisualizer::TapPatternVisualizer(BitlayAudioProcessor& p) : processor(p)
{
    startTimerHz(15);
}

TapPatternVisualizer::~TapPatternVisualizer() {}

void TapPatternVisualizer::timerCallback()
{
    repaint();
}

void TapPatternVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(BitlayLookAndFeel::panel.withAlpha(0.7f));
    g.fillRoundedRectangle(bounds, (float) BitlayUi::panelRadius);
    g.setColour(BitlayLookAndFeel::borderSubtle);
    g.drawRoundedRectangle(bounds, (float) BitlayUi::panelRadius, 1.0f);

    auto plot = getPlotBounds();
    auto centerY = plot.getCentreY();
    auto bpmSync = readParam("bpmSync", 0.0f) > 0.5f;
    auto mainSubdivisionIndex = juce::roundToInt(readParam("mainSubdivision", 4.0f));
    auto syncIsMusical = bpmSync && mainSubdivisionIndex != 7;

    g.setColour(BitlayLookAndFeel::textSecondary);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(syncIsMusical ? "HOST SYNC" : "FREE TIME",
               bounds.withTrimmedLeft(16.0f).withTrimmedTop(8.0f).withHeight(16.0f),
               juce::Justification::left, true);

    g.setColour(BitlayLookAndFeel::textMuted);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(syncIsMusical ? (juce::String("MAIN ") + getSubdivisionLabel(mainSubdivisionIndex)) : "DRAG TAPS: TIME + LEVEL",
               bounds.withTrimmedRight(16.0f).withTrimmedTop(8.0f).withHeight(16.0f),
               juce::Justification::right, true);

    g.setColour(BitlayLookAndFeel::borderStrong.withAlpha(0.22f));
    for (int i = 0; i <= 8; ++i)
    {
        auto x = plot.getX() + plot.getWidth() * ((float) i / 8.0f);
        g.drawVerticalLine(juce::roundToInt(x), plot.getY(), plot.getBottom());
    }

    g.setColour(BitlayLookAndFeel::borderStrong.withAlpha(0.55f));
    g.drawHorizontalLine(juce::roundToInt(centerY), plot.getX(), plot.getRight());

    auto activeTaps = juce::jlimit(1, 4, juce::roundToInt(readParam("numTaps", 1.0f)));
    const juce::String tapNames[] { "T1", "T2", "T3", "T4" };
    const juce::String mixIds[] { "tap1_mix", "tap2_mix", "tap3_mix", "tap4_mix" };
    const juce::String subdivIds[] { "tap1_subdiv", "tap2_subdiv", "tap3_subdiv", "tap4_subdiv" };

    for (int i = 0; i < 4; ++i)
    {
        auto mult = juce::jlimit(0.1f, 2.0f, getEffectiveTapMultiplier(i));
        auto mix = juce::jlimit(0.0f, 1.0f, readParam(mixIds[i], 0.0f));
        auto normalizedTime = (mult - 0.1f) / 1.9f;
        auto x = plot.getX() + normalizedTime * plot.getWidth();
        auto markerHeight = 22.0f + mix * 46.0f;
        auto markerBounds = juce::Rectangle<float>(x - 8.0f, centerY - markerHeight, 16.0f, markerHeight);
        auto isActive = i < activeTaps;
        auto isLocked = isTapTimeLocked(i);
        auto tapColour = i == activeDragTap ? BitlayLookAndFeel::meter : (isActive ? BitlayLookAndFeel::accent : BitlayLookAndFeel::borderStrong);

        if (isLocked)
        {
            g.setColour(BitlayLookAndFeel::meter.withAlpha(isActive ? 0.42f : 0.16f));
            g.drawVerticalLine(juce::roundToInt(x), plot.getY(), plot.getBottom());
        }

        g.setColour(tapColour.withAlpha(isActive ? 0.72f : 0.28f));
        g.fillRoundedRectangle(markerBounds, 4.0f);
        g.setColour(tapColour);
        g.fillEllipse(x - 8.0f, centerY - markerHeight - 10.0f, 16.0f, 16.0f);

        g.setColour(isActive ? BitlayLookAndFeel::textPrimary : BitlayLookAndFeel::textMuted);
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText(tapNames[i], juce::Rectangle<float>(x - 18.0f, centerY + 10.0f, 36.0f, 16.0f),
                   juce::Justification::centred, true);

        g.setFont(juce::Font(10.0f));
        auto timeLabel = isLocked ? getSubdivisionLabel(juce::roundToInt(readParam(subdivIds[i], 7.0f)))
                                  : (juce::String(mult, 2) + "x");
        g.drawText(timeLabel,
                   juce::Rectangle<float>(x - 24.0f, centerY + 26.0f, 48.0f, 14.0f),
                   juce::Justification::centred, true);
        g.drawText(juce::String(juce::roundToInt(mix * 100.0f)) + "%",
                   juce::Rectangle<float>(x - 20.0f, centerY + 40.0f, 40.0f, 14.0f),
                   juce::Justification::centred, true);

        if (! isActive)
        {
            g.setColour(BitlayLookAndFeel::textMuted.withAlpha(0.45f));
            g.drawText("OFF", juce::Rectangle<float>(x - 18.0f, centerY - 18.0f, 36.0f, 14.0f),
                       juce::Justification::centred, true);
        }
    }

    if (activeDragTap >= 0)
    {
        auto mix = juce::jlimit(0.0f, 1.0f, readParam(mixIds[activeDragTap], 0.0f));
        auto mult = juce::jlimit(0.1f, 2.0f, getEffectiveTapMultiplier(activeDragTap));
        auto readout = tapNames[activeDragTap] + "  "
                     + (isTapTimeLocked(activeDragTap) ? "SYNC LOCK" : (juce::String(mult, 2) + "x"))
                     + "  " + juce::String(juce::roundToInt(mix * 100.0f)) + "%";

        auto bubble = juce::Rectangle<float>(plot.getCentreX() - 78.0f, plot.getY() - 19.0f, 156.0f, 18.0f);
        g.setColour(BitlayLookAndFeel::background.withAlpha(0.86f));
        g.fillRoundedRectangle(bubble, 5.0f);
        g.setColour(BitlayLookAndFeel::meter);
        g.drawRoundedRectangle(bubble, 5.0f, 1.0f);
        g.setColour(BitlayLookAndFeel::textPrimary);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText(readout, bubble, juce::Justification::centred, true);
    }

    g.setColour(BitlayLookAndFeel::textMuted);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("EARLY", plot.getX(), plot.getBottom() - 4.0f, 48.0f, 14.0f, juce::Justification::left, true);
    g.drawText("LATE", plot.getRight() - 48.0f, plot.getBottom() - 4.0f, 48.0f, 14.0f, juce::Justification::right, true);
}

void TapPatternVisualizer::mouseDown(const juce::MouseEvent& event)
{
    auto plot = getPlotBounds();
    auto activeTaps = juce::jlimit(1, 4, juce::roundToInt(readParam("numTaps", 1.0f)));
    const juce::String multIds[] { "tap1_mult", "tap2_mult", "tap3_mult", "tap4_mult" };

    auto closestDistance = std::numeric_limits<float>::max();
    activeDragTap = 0;

    for (int i = 0; i < activeTaps; ++i)
    {
        auto mult = juce::jlimit(0.1f, 2.0f, getEffectiveTapMultiplier(i));
        auto normalizedTime = (mult - 0.1f) / 1.9f;
        auto x = plot.getX() + normalizedTime * plot.getWidth();
        auto distance = std::abs(event.position.x - x);

        if (distance < closestDistance)
        {
            closestDistance = distance;
            activeDragTap = i;
        }
    }

    const juce::String mixIds[] { "tap1_mix", "tap2_mix", "tap3_mix", "tap4_mix" };
    if (! isTapTimeLocked(activeDragTap))
        if (auto* parameter = processor.apvts.getParameter(multIds[activeDragTap])) parameter->beginChangeGesture();
    if (auto* parameter = processor.apvts.getParameter(mixIds[activeDragTap])) parameter->beginChangeGesture();

    updateTapFromMouse(event);
}

void TapPatternVisualizer::mouseDrag(const juce::MouseEvent& event)
{
    updateTapFromMouse(event);
}

void TapPatternVisualizer::mouseUp(const juce::MouseEvent&)
{
    if (activeDragTap >= 0)
    {
        const juce::String multIds[] { "tap1_mult", "tap2_mult", "tap3_mult", "tap4_mult" };
        const juce::String mixIds[] { "tap1_mix", "tap2_mix", "tap3_mix", "tap4_mix" };
        if (! isTapTimeLocked(activeDragTap))
            if (auto* parameter = processor.apvts.getParameter(multIds[activeDragTap])) parameter->endChangeGesture();
        if (auto* parameter = processor.apvts.getParameter(mixIds[activeDragTap])) parameter->endChangeGesture();
    }

    activeDragTap = -1;
    repaint();
}

juce::Rectangle<float> TapPatternVisualizer::getPlotBounds() const
{
    return getLocalBounds().toFloat().reduced(25.0f, 28.0f).withTrimmedTop(10.0f);
}

void TapPatternVisualizer::updateTapFromMouse(const juce::MouseEvent& event)
{
    if (activeDragTap < 0)
        return;

    auto plot = getPlotBounds();
    auto normalizedTime = juce::jlimit(0.0f, 1.0f, (event.position.x - plot.getX()) / plot.getWidth());
    auto normalizedLevel = juce::jlimit(0.0f, 1.0f, (plot.getBottom() - event.position.y) / plot.getHeight());
    auto mult = 0.1f + normalizedTime * 1.9f;

    const juce::String multIds[] { "tap1_mult", "tap2_mult", "tap3_mult", "tap4_mult" };
    const juce::String mixIds[] { "tap1_mix", "tap2_mix", "tap3_mix", "tap4_mix" };
    if (! isTapTimeLocked(activeDragTap))
        setParam(multIds[activeDragTap], mult);
    setParam(mixIds[activeDragTap], normalizedLevel);
    repaint();
}

float TapPatternVisualizer::readParam(const juce::String& id, float fallback) const
{
    if (auto* value = processor.apvts.getRawParameterValue(id))
        return value->load();
    return fallback;
}

void TapPatternVisualizer::setParam(const juce::String& id, float value)
{
    if (auto* parameter = processor.apvts.getParameter(id))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}

juce::String TapPatternVisualizer::getSubdivisionLabel(int index) const
{
    const juce::String labels[] { "1/16", "1/3T", "1/8", "1/8D", "1/4", "1/2D", "1/2", "CUSTOM" };
    return labels[juce::jlimit(0, 7, index)];
}

float TapPatternVisualizer::getSubdivisionMultiplier(int index) const
{
    switch (index)
    {
        case 0: return 0.25f;
        case 1: return 1.0f / 3.0f;
        case 2: return 0.5f;
        case 3: return 0.75f;
        case 4: return 1.0f;
        case 5: return 1.5f;
        case 6: return 2.0f;
        default: return 1.0f;
    }
}

float TapPatternVisualizer::getEffectiveTapMultiplier(int tapIndex) const
{
    const juce::String multIds[] { "tap1_mult", "tap2_mult", "tap3_mult", "tap4_mult" };
    const juce::String subdivIds[] { "tap1_subdiv", "tap2_subdiv", "tap3_subdiv", "tap4_subdiv" };

    if (isTapTimeLocked(tapIndex))
    {
        auto mainMult = getSubdivisionMultiplier(juce::roundToInt(readParam("mainSubdivision", 4.0f)));
        auto tapMult = getSubdivisionMultiplier(juce::roundToInt(readParam(subdivIds[tapIndex], 7.0f)));
        return juce::jlimit(0.1f, 2.0f, tapMult / mainMult);
    }

    return readParam(multIds[tapIndex], 1.0f);
}

bool TapPatternVisualizer::isTapTimeLocked(int tapIndex) const
{
    const juce::String subdivIds[] { "tap1_subdiv", "tap2_subdiv", "tap3_subdiv", "tap4_subdiv" };
    auto bpmSync = readParam("bpmSync", 0.0f) > 0.5f;
    auto mainSubdivisionIndex = juce::roundToInt(readParam("mainSubdivision", 4.0f));
    auto tapSubdivisionIndex = juce::roundToInt(readParam(subdivIds[tapIndex], 7.0f));
    return bpmSync && mainSubdivisionIndex != 7 && tapSubdivisionIndex != 7;
}
BitlayAudioProcessorEditor::BitlayAudioProcessorEditor (BitlayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), scope(p), tapPattern(p), tabs(juce::TabbedButtonBar::TabsAtTop)
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
    addAndMakeVisible(presetCategoryCombo);
    addAndMakeVisible(previousPresetButton);
    addAndMakeVisible(nextPresetButton);
    addAndMakeVisible(presetComboBox);
    addAndMakeVisible(savePresetButton);
    addAndMakeVisible(newPresetButton);
    addAndMakeVisible(favoritePresetButton);
    presetStatusLabel.setText("EDITED", juce::dontSendNotification);
    presetStatusLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    presetStatusLabel.setColour(juce::Label::textColourId, BitlayLookAndFeel::accent);
    presetStatusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(presetStatusLabel);
    presetDescriptionLabel.setFont(juce::Font(11.0f));
    presetDescriptionLabel.setColour(juce::Label::textColourId, BitlayLookAndFeel::textSecondary);
    presetDescriptionLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(presetDescriptionLabel);
    presetCategoryCombo.addItemList({"All", "Favorites", "Clean", "Vintage", "Dirty", "Rhythmic", "Reverse", "Experimental", "User"}, 1);
    presetCategoryCombo.setSelectedItemIndex(0, juce::dontSendNotification);
    previousPresetButton.setButtonText("<");
    nextPresetButton.setButtonText(">");
    savePresetButton.setButtonText("Save");
    newPresetButton.setButtonText("Save As...");
    favoritePresetButton.setButtonText("FAV");
    favoritePresetButton.setTooltip("Marks the current preset as a favorite.");
    loadFavoritePresets();
    updatePresetList();
    updatePresetStatus();
    updatePresetDescription();
    presetComboBox.onChange = [this] {
        if (presetComboBox.getSelectedItemIndex() >= 0)
        {
            audioProcessor.loadPreset(presetComboBox.getText());
            updatePresetStatus();
            updatePresetDescription();
        }
    };
    presetCategoryCombo.onChange = [this] { updatePresetList(); };
    previousPresetButton.onClick = [this] {
        auto itemCount = presetComboBox.getNumItems();
        auto selectedIndex = juce::jmax(0, presetComboBox.getSelectedItemIndex());
        if (itemCount > 0)
            presetComboBox.setSelectedItemIndex((selectedIndex + itemCount - 1) % itemCount);
    };
    nextPresetButton.onClick = [this] {
        auto itemCount = presetComboBox.getNumItems();
        auto selectedIndex = juce::jmax(0, presetComboBox.getSelectedItemIndex());
        if (itemCount > 0)
            presetComboBox.setSelectedItemIndex((selectedIndex + 1) % itemCount);
    };
    savePresetButton.onClick = [this] {
        auto presetName = presetComboBox.getText().trim();
        if (presetName.isNotEmpty())
        {
            audioProcessor.savePreset(presetName);
            updatePresetList();
            updatePresetStatus();
            updatePresetDescription();
        }
    };
    newPresetButton.onClick = [this] { showSaveAsDialog(); };
    favoritePresetButton.onClick = [this] { toggleFavoritePreset(); };
    startTimerHz(8);

    monitorVisible = loadMonitorVisibility();
    monitorToggleButton.setButtonText("SCOPE");
    monitorToggleButton.setClickingTogglesState(true);
    monitorToggleButton.setToggleState(monitorVisible, juce::dontSendNotification);
    monitorToggleButton.setTooltip("Shows the technical CVSD monitor.");
    monitorToggleButton.onClick = [this] {
        monitorVisible = monitorToggleButton.getToggleState();
        scope.setVisible(monitorVisible);
        saveMonitorVisibility();
        resized();
    };
    addAndMakeVisible(monitorToggleButton);

    abStateA = audioProcessor.apvts.copyState().createCopy();
    abStateB = abStateA.createCopy();
    abAButton.setButtonText("A");
    abBButton.setButtonText("B");
    abCopyButton.setButtonText("A>B");
    abAButton.setTooltip("Recall A. Shift-click captures current settings into A.");
    abBButton.setTooltip("Recall B. Shift-click captures current settings into B.");
    abCopyButton.setTooltip("Copy the active A/B slot to the other slot.");
    abAButton.onClick = [this] {
        if (juce::ModifierKeys::getCurrentModifiers().isShiftDown()) captureAbSlot(0);
        else recallAbSlot(0);
    };
    abBButton.onClick = [this] {
        if (juce::ModifierKeys::getCurrentModifiers().isShiftDown()) captureAbSlot(1);
        else recallAbSlot(1);
    };
    abCopyButton.onClick = [this] {
        if (activeAbSlot == 0) copyAbSlot(0, 1);
        else copyAbSlot(1, 0);
    };
    addAndMakeVisible(abAButton);
    addAndMakeVisible(abBButton);
    addAndMakeVisible(abCopyButton);
    updateAbButtons();

    undoButton.setButtonText("UN");
    redoButton.setButtonText("RE");
    undoButton.setTooltip("Undo the last parameter edit.");
    redoButton.setTooltip("Redo the last undone parameter edit.");
    undoButton.onClick = [this] {
        audioProcessor.undoManager.undo();
        updatePresetStatus();
        updateUndoRedoButtons();
    };
    redoButton.onClick = [this] {
        audioProcessor.undoManager.redo();
        updatePresetStatus();
        updateUndoRedoButtons();
    };
    addAndMakeVisible(undoButton);
    addAndMakeVisible(redoButton);
    updateUndoRedoButtons();

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
    freeze.init("Freeze", apvts, "freeze");
    mainSubdivision.init("Subdivision", apvts, "mainSubdivision", subdivs);

    // Macros
    macroTexture.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    macroTexture.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
    macroTexture.setName("Texture");
    macroTexture.setRange(0.0, 1.0);
    macroTexture.setValue(0.5);
    macroTexture.setMouseDragSensitivity(180);
    macroTexture.setDoubleClickReturnValue(true, 0.5);
    macroTexture.addListener(this);
    labelTexture.setText("DEGRADE", juce::dontSendNotification);
    labelTexture.setJustificationType(juce::Justification::centred);

    macroMovement.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    macroMovement.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
    macroMovement.setName("Movement");
    macroMovement.setRange(0.0, 1.0);
    macroMovement.setValue(0.5);
    macroMovement.setMouseDragSensitivity(180);
    macroMovement.setDoubleClickReturnValue(true, 0.5);
    macroMovement.addListener(this);
    labelMovement.setText("MOVEMENT", juce::dontSendNotification);
    labelMovement.setJustificationType(juce::Justification::centred);

    // Modulation
    wobbleRate.slider.setName("Wobble"); wobbleRate.init("Drift Rate", apvts, "wobbleRate");
    wobbleDepth.slider.setName("Wobble"); wobbleDepth.init("Drift Depth", apvts, "wobbleDepth");
    wobbleSync.slider.setName("Wobble"); wobbleSync.init("Sync Amount", apvts, "wobbleSync");

    // Circuit
    circuitType.init("Architecture", apvts, "circuitType", circuits);
    coupledMode.init("Musical Coupling", apvts, "coupledMode");
    character.slider.setName("Drive"); character.init("Drive", apvts, "character");
    stepSize.init("Step", apvts, "stepSize");
    clockJitter.init("Instability", apvts, "clockJitter");
    integratorLag.init("Slew", apvts, "integratorLag");
    reconCutoff.init("Output Filter", apvts, "reconstructionCutoff");
    integratorLeak.init("Memory", apvts, "integratorLeak");
    dynamicResponse.init("Tracking", apvts, "dynamicResponse");
    feedbackTone.init("Tone", apvts, "feedbackTone");
    stereoSpread.init("Width", apvts, "stereoSpread");
    
    envAttack.init("Attack", apvts, "envAttack");
    envRelease.init("Release", apvts, "envRelease");
    minStepSize.init("Min Step", apvts, "minStepSize");
    maxStepSize.init("Max Step", apvts, "maxStepSize");
    syllabicTime.init("Adapt Time", apvts, "syllabicTime");

    // Taps
    numTaps.init("Active Taps", apvts, "numTaps");
    tapDecay.init("Volume Decay", apvts, "tapDecay");
    
    tap1Mult.init("Time Mult", apvts, "tap1_mult"); tap1Mix.init("Tap Level", apvts, "tap1_mix"); tap1Subdiv.init("Subdiv", apvts, "tap1_subdiv", subdivs);
    tap2Mult.init("Time Mult", apvts, "tap2_mult"); tap2Mix.init("Tap Level", apvts, "tap2_mix"); tap2Subdiv.init("Subdiv", apvts, "tap2_subdiv", subdivs);
    tap3Mult.init("Time Mult", apvts, "tap3_mult"); tap3Mix.init("Tap Level", apvts, "tap3_mix"); tap3Subdiv.init("Subdiv", apvts, "tap3_subdiv", subdivs);
    tap4Mult.init("Time Mult", apvts, "tap4_mult"); tap4Mix.init("Tap Level", apvts, "tap4_mix"); tap4Subdiv.init("Subdiv", apvts, "tap4_subdiv", subdivs);

    // Reverse Details
    reverseChunkSize.init("Slice Size", apvts, "reverseChunkSize");
    reverseFeedback.init("Build-Up", apvts, "reverseFeedback");

    auto setSliderTip = [](SliderWithLabel& control, const juce::String& tip) {
        control.slider.setTooltip(tip + " Double-click resets.");
        control.label.setTooltip(tip);
    };
    auto setLinearTip = [](LinearSliderWithLabel& control, const juce::String& tip) {
        control.slider.setTooltip(tip + " Double-click resets.");
        control.label.setTooltip(tip);
    };
    auto setComboTip = [](ComboWithLabel& control, const juce::String& tip) {
        control.combo.setTooltip(tip);
        control.label.setTooltip(tip);
    };
    auto setToggleTip = [](ToggleWithLabel& control, const juce::String& tip) {
        control.button.setTooltip(tip);
    };

    setSliderTip(delayTime, "Delay time in free mode, or the resolved time when host sync is off.");
    setSliderTip(feedback, "How much delayed signal returns into the echo path.");
    setSliderTip(mix, "Balance between dry signal and processed echoes.");
    setLinearTip(internalBpm, "Internal tempo used when host sync is unavailable or disabled.");
    setComboTip(mainSubdivision, "Musical division used for synced delay time.");
    setToggleTip(bpmSync, "Locks the main delay time to host tempo and subdivision.");
    setToggleTip(freeze, "Holds the current delay buffer for sustained repeats.");
    setToggleTip(reverseMode, "Reverses delay slices for backwards echoes.");
    setToggleTip(bypass, "Bypasses Bitlay processing.");

    macroTexture.setTooltip("Macro for dirt, slew and filtering. Double-click resets.");
    labelTexture.setTooltip("Macro for dirt, slew and filtering. Double-click resets.");
    macroMovement.setTooltip("Macro for modulation depth and rate. Double-click resets.");
    labelMovement.setTooltip("Macro for modulation depth and rate. Double-click resets.");

    engineGuideLabel.setText("Shape the primitive 1-bit engine by cause: delta step, bandwidth, drift and adaptive response.",
                             juce::dontSendNotification);
    engineGuideLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    engineGuideLabel.setColour(juce::Label::textColourId, BitlayLookAndFeel::textSecondary);
    engineGuideLabel.setJustificationType(juce::Justification::centredLeft);
    labGuideLabel.setText("Lab controls for reverse texture and modulation behaviour. Use when the main page needs extra weird.",
                          juce::dontSendNotification);
    labGuideLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    labGuideLabel.setColour(juce::Label::textColourId, BitlayLookAndFeel::textSecondary);
    labGuideLabel.setJustificationType(juce::Justification::centredLeft);

    setComboTip(circuitType, "Selects the primitive delta circuit behavior.");
    setToggleTip(coupledMode, "Keeps deeper engine controls musically related.");
    setSliderTip(character, "Input drive and CVSD stress before the delay path.");
    setSliderTip(stepSize, "Base 1-bit delta step; higher values get rougher and more angular.");
    setSliderTip(minStepSize, "Smallest adaptive delta step in companded mode.");
    setSliderTip(maxStepSize, "Largest adaptive delta step in companded mode.");
    setSliderTip(clockJitter, "Clock looseness and timebase instability.");
    setSliderTip(integratorLag, "Slew in the delta integrator; softens fast movement.");
    setSliderTip(reconCutoff, "Output reconstruction filter; lower values darken the repeats.");
    setSliderTip(integratorLeak, "How quickly the primitive integrator forgets its previous state.");
    setSliderTip(dynamicResponse, "How strongly the adaptive engine follows input dynamics.");
    setSliderTip(feedbackTone, "Low-pass tone inside the feedback path.");
    setSliderTip(stereoSpread, "Left/right timing and circuit variation.");
    setSliderTip(envAttack, "How quickly the compander reacts to louder input.");
    setSliderTip(envRelease, "How quickly the compander relaxes after transients.");
    setSliderTip(syllabicTime, "Adaptive response time for the CVSD syllabic filter.");
    setSliderTip(wobbleRate, "Speed of delay-time drift.");
    setSliderTip(wobbleDepth, "Depth of delay-time drift.");
    setSliderTip(wobbleSync, "How strongly motion follows rhythmic sync.");
    setSliderTip(reverseChunkSize, "Length of each reversed slice.");
    setSliderTip(reverseFeedback, "How much reverse material accumulates in feedback.");

    mainTabComp = new juce::Component();
    tapsTabComp = new juce::Component();
    circuitTabComp = new juce::Component();
    reverseTabComp = new juce::Component();

    auto addGroup = [](juce::Component* p, juce::GroupComponent* g) { p->addAndMakeVisible(g); };
    
    // Add groups FIRST so they render behind
    addGroup(mainTabComp, &groupTime); addGroup(mainTabComp, &groupMacros); addGroup(mainTabComp, &groupMix);
    addGroup(tapsTabComp, &groupTapPattern); addGroup(tapsTabComp, &groupTapGlobal); addGroup(tapsTabComp, &groupTap1); addGroup(tapsTabComp, &groupTap2); addGroup(tapsTabComp, &groupTap3); addGroup(tapsTabComp, &groupTap4);
    addGroup(circuitTabComp, &groupEngine); addGroup(circuitTabComp, &groupFilters); addGroup(circuitTabComp, &groupEnv);
    addGroup(reverseTabComp, &groupRev); addGroup(reverseTabComp, &groupLfo);

    auto addComps = [](juce::Component* p, std::vector<juce::Component*> c) { for (auto* comp : c) p->addAndMakeVisible(comp); };

    addComps(mainTabComp, {&delayTime.slider, &delayTime.label, &feedback.slider, &feedback.label, &mix.slider, &mix.label, &internalBpm.slider, &internalBpm.label, &mainSubdivision.combo, &mainSubdivision.label, &bpmSync.button, &freeze.button, &macroTexture, &labelTexture, &macroMovement, &labelMovement, &character.slider, &character.label, &feedbackTone.slider, &feedbackTone.label, &stereoSpread.slider, &stereoSpread.label});
    
    addComps(circuitTabComp, {&engineGuideLabel, &circuitType.combo, &circuitType.label, &coupledMode.button, &stepSize.slider, &stepSize.label, &clockJitter.slider, &clockJitter.label, &integratorLag.slider, &integratorLag.label, &reconCutoff.slider, &reconCutoff.label, &integratorLeak.slider, &integratorLeak.label, &dynamicResponse.slider, &dynamicResponse.label, &envAttack.slider, &envAttack.label, &envRelease.slider, &envRelease.label, &minStepSize.slider, &minStepSize.label, &maxStepSize.slider, &maxStepSize.label, &syllabicTime.slider, &syllabicTime.label});
    
    addComps(tapsTabComp, {&tapPattern, &numTaps.slider, &numTaps.label, &tapDecay.slider, &tapDecay.label, &tap1Mult.slider, &tap1Mult.label, &tap1Mix.slider, &tap1Mix.label, &tap1Subdiv.combo, &tap1Subdiv.label, &tap2Mult.slider, &tap2Mult.label, &tap2Mix.slider, &tap2Mix.label, &tap2Subdiv.combo, &tap2Subdiv.label, &tap3Mult.slider, &tap3Mult.label, &tap3Mix.slider, &tap3Mix.label, &tap3Subdiv.combo, &tap3Subdiv.label, &tap4Mult.slider, &tap4Mult.label, &tap4Mix.slider, &tap4Mix.label, &tap4Subdiv.combo, &tap4Subdiv.label});
    
    addComps(reverseTabComp, {&labGuideLabel, &reverseChunkSize.slider, &reverseChunkSize.label, &reverseFeedback.slider, &reverseFeedback.label, &wobbleRate.slider, &wobbleRate.label, &wobbleDepth.slider, &wobbleDepth.label, &wobbleSync.slider, &wobbleSync.label});

    tabs.addTab("MAIN", BitlayLookAndFeel::background, mainTabComp, true);
    tabs.addTab("RHYTHM", BitlayLookAndFeel::background, tapsTabComp, true);
    tabs.addTab("ENGINE", BitlayLookAndFeel::background, circuitTabComp, true);
    tabs.addTab("LAB", BitlayLookAndFeel::background, reverseTabComp, true);

    addAndMakeVisible(tabs);
    addAndMakeVisible(scope);
    scope.setVisible(monitorVisible);

    auto editorSize = loadEditorSize();
    setResizable(true, true);
    setResizeLimits(1000, 640, 1400, 980);
    setSize(editorSize.x, editorSize.y);
}

BitlayAudioProcessorEditor::~BitlayAudioProcessorEditor()
{
    saveMonitorVisibility();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

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
    presets.sort(true);
    auto selectedCategory = presetCategoryCombo.getText();
    juce::StringArray filteredPresets;

    for (auto presetName : presets)
    {
        auto category = getPresetCategory(presetName);
        auto matchesFavorites = selectedCategory == "Favorites" && isFavoritePreset(presetName);
        if (selectedCategory == "All" || selectedCategory.isEmpty() || category == selectedCategory || matchesFavorites)
            filteredPresets.add(presetName);
    }

    if (audioProcessor.currentPreset.isNotEmpty() && ! filteredPresets.contains(audioProcessor.currentPreset))
        filteredPresets.insert(0, audioProcessor.currentPreset);

    presetComboBox.addItemList(filteredPresets, 1);
    int index = filteredPresets.indexOf(audioProcessor.currentPreset);
    if (index >= 0) presetComboBox.setSelectedItemIndex(index, juce::dontSendNotification);
    else if (filteredPresets.size() > 0) presetComboBox.setSelectedItemIndex(0, juce::dontSendNotification);
    updatePresetDescription();
}

juce::String BitlayAudioProcessorEditor::getPresetCategory(const juce::String& presetName) const
{
    if (presetName == "Default")
        return "Clean";
    if (presetName.containsIgnoreCase("Vintage") || presetName.containsIgnoreCase("Companded Space"))
        return "Vintage";
    if (presetName.containsIgnoreCase("Lo-Fi") || presetName.containsIgnoreCase("Crunchy"))
        return "Dirty";
    if (presetName.containsIgnoreCase("Rhythmic") || presetName.containsIgnoreCase("Ping-Pong"))
        return "Rhythmic";
    if (presetName.containsIgnoreCase("Reverse"))
        return "Reverse";
    if (presetName.containsIgnoreCase("Sludge") || presetName.containsIgnoreCase("Chaos"))
        return "Experimental";
    return "User";
}

juce::String BitlayAudioProcessorEditor::getPresetDescription(const juce::String& presetName) const
{
    if (presetName == "Default")
        return "Clean reference delay with balanced repeats.";
    if (presetName == "Vintage Slapback")
        return "Short, driven echo for guitar and vocal thickening.";
    if (presetName == "Lo-Fi Wobble")
        return "Dark unstable repeats with degraded 1-bit texture.";
    if (presetName == "Rhythmic Tap Dance")
        return "Tempo-locked multi-tap pattern for rhythmic movement.";
    if (presetName == "Deep Companded Space")
        return "Wide companded ambience with soft dark feedback.";
    if (presetName == "Ghostly Reverse")
        return "Reverse slices with floating feedback trails.";
    if (presetName == "Crunchy Sludge")
        return "Aggressive slope overload and crushed delay tone.";
    if (presetName == "Cyberpunk Ping-Pong")
        return "Synced stereo taps with animated digital motion.";
    return getPresetCategory(presetName) + " preset.";
}

void BitlayAudioProcessorEditor::updatePresetDescription()
{
    auto presetName = presetComboBox.getText().trim();
    if (presetName.isEmpty())
        presetName = audioProcessor.currentPreset;

    auto description = getPresetCategory(presetName) + " - " + getPresetDescription(presetName);
    presetDescriptionLabel.setText(description, juce::dontSendNotification);
    presetComboBox.setTooltip(description);
}

void BitlayAudioProcessorEditor::loadFavoritePresets()
{
    favoritePresetNames.clear();
    auto favoritesFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                             .getChildFile("Bitlay")
                             .getChildFile("favorites.txt");

    if (favoritesFile.existsAsFile())
    {
        favoritePresetNames.addLines(favoritesFile.loadFileAsString());
        favoritePresetNames.removeEmptyStrings();
    }
}

void BitlayAudioProcessorEditor::saveFavoritePresets() const
{
    auto settingsDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                           .getChildFile("Bitlay");
    settingsDir.createDirectory();
    settingsDir.getChildFile("favorites.txt").replaceWithText(favoritePresetNames.joinIntoString("\n"));
}

bool BitlayAudioProcessorEditor::isFavoritePreset(const juce::String& presetName) const
{
    return presetName.isNotEmpty() && favoritePresetNames.contains(presetName);
}

void BitlayAudioProcessorEditor::toggleFavoritePreset()
{
    auto presetName = presetComboBox.getText().trim();
    if (presetName.isEmpty())
        presetName = audioProcessor.currentPreset;

    if (presetName.isEmpty())
        return;

    if (favoritePresetNames.contains(presetName))
        favoritePresetNames.removeString(presetName);
    else
        favoritePresetNames.addIfNotAlreadyThere(presetName);

    favoritePresetNames.sort(true);
    saveFavoritePresets();
    updatePresetList();
    updatePresetStatus();
}

void BitlayAudioProcessorEditor::captureAbSlot(int slot)
{
    auto snapshot = audioProcessor.apvts.copyState().createCopy();
    if (slot == 0)
        abStateA = snapshot;
    else
        abStateB = snapshot;

    activeAbSlot = slot;
    updateAbButtons();
}

void BitlayAudioProcessorEditor::recallAbSlot(int slot)
{
    auto state = slot == 0 ? abStateA : abStateB;
    if (state.isValid())
    {
        audioProcessor.apvts.replaceState(state.createCopy());
        activeAbSlot = slot;
        updatePresetStatus();
        updatePresetDescription();
        updateAbButtons();
    }
}

void BitlayAudioProcessorEditor::copyAbSlot(int sourceSlot, int targetSlot)
{
    auto sourceState = sourceSlot == 0 ? abStateA : abStateB;
    if (! sourceState.isValid())
        return;

    if (targetSlot == 0)
        abStateA = sourceState.createCopy();
    else
        abStateB = sourceState.createCopy();

    updateAbButtons();
}

void BitlayAudioProcessorEditor::updateAbButtons()
{
    abAButton.setToggleState(activeAbSlot == 0, juce::dontSendNotification);
    abBButton.setToggleState(activeAbSlot == 1, juce::dontSendNotification);
    abCopyButton.setButtonText(activeAbSlot == 0 ? "A>B" : "B>A");
}

void BitlayAudioProcessorEditor::updateUndoRedoButtons()
{
    undoButton.setEnabled(audioProcessor.undoManager.canUndo());
    redoButton.setEnabled(audioProcessor.undoManager.canRedo());
}

void BitlayAudioProcessorEditor::showSaveAsDialog()
{
    auto suggestedName = presetComboBox.getText().trim();
    if (suggestedName.isEmpty())
        suggestedName = audioProcessor.currentPreset;

    saveAsDialog = std::make_unique<juce::AlertWindow>("Save Preset As",
                                                       "Create a new Bitlay preset.",
                                                       juce::AlertWindow::NoIcon);
    saveAsDialog->addTextEditor("presetName", suggestedName, "Preset name:");
    saveAsDialog->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    saveAsDialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    juce::Component::SafePointer<BitlayAudioProcessorEditor> safeThis(this);
    saveAsDialog->enterModalState(true, juce::ModalCallbackFunction::create([safeThis](int result) {
        if (safeThis.getComponent() == nullptr)
            return;

        auto* editor = safeThis.getComponent();
        if (result == 1 && editor->saveAsDialog != nullptr)
        {
            auto presetName = editor->saveAsDialog->getTextEditorContents("presetName")
                                  .trim()
                                  .removeCharacters("\\/:*?\"<>|");
            if (presetName.isNotEmpty())
            {
                editor->audioProcessor.savePreset(presetName);
                editor->updatePresetList();
                editor->presetComboBox.setText(presetName, juce::dontSendNotification);
                editor->updatePresetStatus();
            }
        }

        if (editor != nullptr)
            editor->saveAsDialog.reset();
    }), false);
}

void BitlayAudioProcessorEditor::updatePresetStatus()
{
    auto edited = audioProcessor.isCurrentPresetEdited();
    presetStatusLabel.setVisible(edited);
    savePresetButton.setButtonText(edited ? "Save*" : "Save");
    favoritePresetButton.setToggleState(isFavoritePreset(presetComboBox.getText().trim()), juce::dontSendNotification);
}

bool BitlayAudioProcessorEditor::loadMonitorVisibility() const
{
    auto settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                            .getChildFile("Bitlay")
                            .getChildFile("ui.settings");

    if (! settingsFile.existsAsFile())
        return false;

    return settingsFile.loadFileAsString().contains("scope=1");
}

juce::Point<int> BitlayAudioProcessorEditor::loadEditorSize() const
{
    auto settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                            .getChildFile("Bitlay")
                            .getChildFile("ui.settings");
    auto width = 1000;
    auto height = 720;

    if (settingsFile.existsAsFile())
    {
        juce::StringArray lines;
        lines.addLines(settingsFile.loadFileAsString());

        for (auto line : lines)
        {
            if (line.startsWith("width="))
                width = line.fromFirstOccurrenceOf("=", false, false).getIntValue();
            else if (line.startsWith("height="))
                height = line.fromFirstOccurrenceOf("=", false, false).getIntValue();
        }
    }

    return { juce::jlimit(1000, 1400, width), juce::jlimit(640, 980, height) };
}

void BitlayAudioProcessorEditor::saveMonitorVisibility() const
{
    auto settingsDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                           .getChildFile("Bitlay");
    settingsDir.createDirectory();
    settingsDir.getChildFile("ui.settings")
        .replaceWithText(juce::String("scope=") + (monitorVisible ? "1" : "0")
                         + "\nwidth=" + juce::String(getWidth())
                         + "\nheight=" + juce::String(getHeight()));
}

void BitlayAudioProcessorEditor::timerCallback()
{
    updatePresetStatus();
    updateUndoRedoButtons();
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
    pluginTitle.setBounds(headerArea.removeFromLeft(100));
    
    auto presetArea = headerArea.removeFromLeft(470).reduced(0, 6);
    auto presetMetaArea = presetArea.removeFromTop(18);
    presetCategoryCombo.setBounds(presetMetaArea.removeFromLeft(96));
    presetDescriptionLabel.setBounds(presetMetaArea.reduced(8, 0));

    auto presetControlArea = presetArea.reduced(0, 2);
    presetLabel.setBounds(presetControlArea.removeFromLeft(42));
    previousPresetButton.setBounds(presetControlArea.removeFromLeft(24).reduced(2));
    presetComboBox.setBounds(presetControlArea.removeFromLeft(128));
    nextPresetButton.setBounds(presetControlArea.removeFromLeft(24).reduced(2));
    presetStatusLabel.setBounds(presetControlArea.removeFromLeft(40).reduced(4, 3));
    favoritePresetButton.setBounds(presetControlArea.removeFromLeft(40).reduced(2));
    savePresetButton.setBounds(presetControlArea.removeFromLeft(56).reduced(2));
    newPresetButton.setBounds(presetControlArea.removeFromLeft(70).reduced(2));
    
    headerArea.removeFromLeft(4);
    abAButton.setBounds(headerArea.removeFromLeft(28).reduced(1, 10));
    abBButton.setBounds(headerArea.removeFromLeft(28).reduced(1, 10));
    abCopyButton.setBounds(headerArea.removeFromLeft(38).reduced(2, 10));
    undoButton.setBounds(headerArea.removeFromLeft(34).reduced(2, 10));
    redoButton.setBounds(headerArea.removeFromLeft(34).reduced(2, 10));
    headerArea.removeFromLeft(4);
    reverseMode.button.setBounds(headerArea.removeFromLeft(88).reduced(0, 5));
    headerArea.removeFromLeft(4);
    bypass.button.setBounds(headerArea.removeFromLeft(64).reduced(0, 10));
    headerArea.removeFromLeft(4);
    monitorToggleButton.setBounds(headerArea.removeFromLeft(54).reduced(0, 10));

    area.removeFromTop(10);
    if (monitorVisible)
    {
        scope.setBounds(area.removeFromTop(112));
        area.removeFromTop(15);
    }
    else
    {
        scope.setBounds({});
        area.removeFromTop(4);
    }
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
    groupTapPattern.setBounds(10, 10, 940, 150);
    tapPattern.setBounds(28, 34, 904, 108);

    groupTapGlobal.setBounds(10, 178, 200, 250);
    placeKnob(numTaps, 30, 232);
    placeKnob(tapDecay, 116, 232);
    
    int tx = 230;
    auto setupTapBox = [&](juce::GroupComponent& group, SliderWithLabel& mult, SliderWithLabel& mx, ComboWithLabel& sub, int startX) {
        group.setBounds(startX, 178, 160, 250);
        placeKnob(mult, startX + 10, 224, 60, 80);
        placeKnob(mx, startX + 90, 224, 60, 80);
        placeCombo(sub, startX + 40, 352, 80);
    };
    setupTapBox(groupTap1, tap1Mult, tap1Mix, tap1Subdiv, tx);
    setupTapBox(groupTap2, tap2Mult, tap2Mix, tap2Subdiv, tx + 180);
    setupTapBox(groupTap3, tap3Mult, tap3Mix, tap3Subdiv, tx + 360);
    setupTapBox(groupTap4, tap4Mult, tap4Mix, tap4Subdiv, tx + 540);

    // CIRCUIT TAB
    engineGuideLabel.setBounds(24, 10, 900, 24);
    groupEngine.setBounds(10, 42, 450, 160);
    placeCombo(circuitType, 30, 72, 120);
    placeToggle(coupledMode, 170, 72, 120);
    placeKnob(stepSize, 40, 112, 80, 90);
    placeKnob(minStepSize, 170, 112, 80, 90);
    placeKnob(maxStepSize, 300, 112, 80, 90);
    
    groupFilters.setBounds(480, 42, 470, 160);
    placeKnob(integratorLag, 520, 92, 80, 90);
    placeKnob(reconCutoff, 650, 92, 80, 90);
    placeKnob(clockJitter, 780, 92, 80, 90);

    groupEnv.setBounds(10, 217, 940, 140);
    placeKnob(envAttack, 100, 252, 80, 90);
    placeKnob(envRelease, 250, 252, 80, 90);
    placeKnob(syllabicTime, 400, 252, 80, 90);
    placeKnob(integratorLeak, 550, 252, 80, 90);
    placeKnob(dynamicResponse, 700, 252, 80, 90);

    // REVERSE & MOD TAB
    labGuideLabel.setBounds(24, 10, 900, 24);
    groupRev.setBounds(10, 42, 300, 160);
    placeKnob(reverseChunkSize, 40, 82, 80, 90);
    placeKnob(reverseFeedback, 160, 82, 80, 90);

    groupLfo.setBounds(330, 42, 360, 160);
    placeKnob(wobbleRate, 360, 82, 80, 90);
    placeKnob(wobbleDepth, 460, 82, 80, 90);
    placeKnob(wobbleSync, 560, 82, 80, 90);
}
