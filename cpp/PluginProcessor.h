#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "CVSDDelayCore.h"
#include <array>

class BitlayAudioProcessor  : public juce::AudioProcessor
{
public:
    BitlayAudioProcessor();
    ~BitlayAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Preset Management
    juce::File getPresetsDirectory() const;
    juce::StringArray getPresetNames() const;
    void loadPreset (const juce::String& presetName);
    void savePreset (const juce::String& presetName);
    void createDefaultPresetsIfNeeded();
    bool isCurrentPresetEdited();

    juce::String currentPreset = "Default";
    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;

    // FIFO for Oscilloscope
    static constexpr int fifoSize = 2048;
    juce::AbstractFifo scopeFifo { fifoSize };
    std::array<float, fifoSize> diagIntegratorBuffer;
    std::array<float, fifoSize> diagStepSizeBuffer;

private:
    CVSDDelayCore core;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void markCurrentPresetClean();

    std::vector<float> tempIntegrator;
    std::vector<float> tempStepSize;
    juce::ValueTree cleanPresetState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitlayAudioProcessor)
};
