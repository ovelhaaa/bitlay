#include "PluginProcessor.h"
#include "PluginEditor.h"

BitlayAudioProcessor::BitlayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
    :
#endif
    apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    createDefaultPresetsIfNeeded();
    markCurrentPresetClean();
}

BitlayAudioProcessor::~BitlayAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout BitlayAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("delayTime", 1), "Delay Time", 10.0f, 2000.0f, 350.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("feedback", 1), "Feedback", 0.0f, 1.0f, 0.6f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("mix", 1), "Mix", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("bypass", 1), "Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("stepSize", 1), "Step Size", 0.001f, 0.2f, 0.02f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("clockJitter", 1), "Clock Jitter", 0.0f, 100.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("integratorLag", 1), "Integrator Lag", 1.0f, 50.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reconstructionCutoff", 1), "Recon Cutoff", 100.0f, 20000.0f, 3500.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("integratorLeak", 1), "Integrator Leak", 0.9f, 1.0f, 0.995f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("character", 1), "Character", 0.0f, 100.0f, 25.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("dynamicResponse", 1), "Dyn Response", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("wobbleDepth", 1), "Wobble Depth", 0.0f, 50.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("wobbleRate", 1), "Wobble Rate", 0.1f, 20.0f, 1.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("feedbackTone", 1), "Feedback Tone", 200.0f, 20000.0f, 4000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("stereoSpread", 1), "Stereo Spread", 0.0f, 100.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("freeze", 1), "Freeze", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("coupledMode", 1), "Coupled Mode", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("envAttack", 1), "Env Attack", 0.001f, 1.0f, 0.005f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("envRelease", 1), "Env Release", 0.01f, 2.0f, 0.05f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("circuitType", 1), "Circuit Type", juce::StringArray{"Discrete", "Companded"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("minStepSize", 1), "Min Step Size", 0.001f, 0.1f, 0.002f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("maxStepSize", 1), "Max Step Size", 0.1f, 2.0f, 0.12f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("syllabicTime", 1), "Syllabic Time", 1.0f, 500.0f, 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("numTaps", 1), "Num Taps", 1, 4, 2));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("tapDecay", 1), "Tap Decay", 0.0f, 1.0f, 1.0f));
    
    juce::StringArray subdivs { "1/16", "1/3T", "1/8", "1/8D", "1/4", "1/2D", "1/2", "custom" };
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("bpmSync", 1), "BPM Sync", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("internalBpm", 1), "Internal BPM", 30.0f, 300.0f, 120.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("mainSubdivision", 1), "Main Subdiv", subdivs, 4));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("tap1_mult", 1), "Tap 1 Mult", 0.1f, 2.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("tap1_mix", 1), "Tap 1 Mix", 0.0f, 1.0f, 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("tap1_subdiv", 1), "Tap 1 Subdiv", subdivs, 2));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("tap2_mult", 1), "Tap 2 Mult", 0.1f, 2.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("tap2_mix", 1), "Tap 2 Mix", 0.0f, 1.0f, 0.6f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("tap2_subdiv", 1), "Tap 2 Subdiv", subdivs, 4));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("tap3_mult", 1), "Tap 3 Mult", 0.1f, 2.0f, 1.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("tap3_mix", 1), "Tap 3 Mix", 0.0f, 1.0f, 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("tap3_subdiv", 1), "Tap 3 Subdiv", subdivs, 5));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("tap4_mult", 1), "Tap 4 Mult", 0.1f, 2.0f, 2.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("tap4_mix", 1), "Tap 4 Mix", 0.0f, 1.0f, 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("tap4_subdiv", 1), "Tap 4 Subdiv", subdivs, 6));

    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("reverseMode", 1), "Reverse Mode", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverseChunkSize", 1), "Rev Chunk Size", 10.0f, 1000.0f, 350.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverseFeedback", 1), "Rev Feedback", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("wobbleSync", 1), "Wobble Sync", 0.0f, 1.0f, 0.5f));

    return { params.begin(), params.end() };
}

const juce::String BitlayAudioProcessor::getName() const { return "Bitlay"; }
bool BitlayAudioProcessor::acceptsMidi() const { return false; }
bool BitlayAudioProcessor::producesMidi() const { return false; }
bool BitlayAudioProcessor::isMidiEffect() const { return false; }
double BitlayAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int BitlayAudioProcessor::getNumPrograms() { return 1; }
int BitlayAudioProcessor::getCurrentProgram() { return 0; }
void BitlayAudioProcessor::setCurrentProgram (int index) {}
const juce::String BitlayAudioProcessor::getProgramName (int index) { return {}; }
void BitlayAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void BitlayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    core.prepare(sampleRate, getTotalNumOutputChannels());
    tempIntegrator.resize(samplesPerBlock, 0.0f);
    tempStepSize.resize(samplesPerBlock, 0.0f);
}

void BitlayAudioProcessor::releaseResources()
{
}

bool BitlayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void BitlayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        if (totalNumInputChannels > 0) {
            buffer.copyFrom (i, 0, buffer, 0, 0, buffer.getNumSamples());
        } else {
            buffer.clear (i, 0, buffer.getNumSamples());
        }
    }

    auto setCoreParam = [this](const juce::String& id, ParamID coreId) {
        if (auto* ptr = apvts.getRawParameterValue(id))
            core.setParameter(coreId, ptr->load());
    };

    auto getSubdivMultiplier = [](int index) -> double {
        switch (index) {
            case 0: return 0.25; // 1/16
            case 1: return 1.0 / 3.0; // 1/3T
            case 2: return 0.5; // 1/8
            case 3: return 0.75; // 1/8D
            case 4: return 1.0; // 1/4
            case 5: return 1.5; // 1/2D
            case 6: return 2.0; // 1/2
            default: return 1.0; // custom
        }
    };

    double currentBpm = *apvts.getRawParameterValue("internalBpm");
    bool bpmSync = *apvts.getRawParameterValue("bpmSync") > 0.5f;
    int mainSubdivIdx = static_cast<int>(*apvts.getRawParameterValue("mainSubdivision"));

    if (auto* playHead = getPlayHead()) {
        if (auto posInfo = playHead->getPosition()) {
            if (posInfo->getBpm().hasValue()) {
                currentBpm = *posInfo->getBpm();
            }
        }
    }

    double finalDelayTime = *apvts.getRawParameterValue("delayTime");
    
    if (bpmSync && mainSubdivIdx != 7) { // 7 is 'custom'
        double mult = getSubdivMultiplier(mainSubdivIdx);
        double calculatedDelay = std::round((60000.0 / currentBpm) * mult);
        finalDelayTime = std::max(10.0, std::min(2000.0, calculatedDelay));
    }
    
    core.setParameter(ParamID::delayTime, finalDelayTime);

    float character = *apvts.getRawParameterValue("character");
    double computedDrive = 1.0;
    if (character <= 30.0f) {
        computedDrive = 1.0 + (character / 30.0) * 0.5;
    } else if (character <= 70.0f) {
        computedDrive = 1.5 + ((character - 30.0) / 40.0) * 3.5;
    } else {
        computedDrive = 5.0 + ((character - 70.0) / 30.0) * 5.0;
    }
    core.setParameter(ParamID::inputDrive, computedDrive);

    auto setTapMult = [&](const juce::String& paramId, ParamID coreId, int subdivIdx) {
        double mult = *apvts.getRawParameterValue(paramId);
        if (bpmSync && subdivIdx != 7 && mainSubdivIdx != 7) {
            double mainMult = getSubdivMultiplier(mainSubdivIdx);
            double tapMult = getSubdivMultiplier(subdivIdx) / mainMult;
            mult = std::max(0.1, std::min(2.0, tapMult));
        }
        core.setParameter(coreId, mult);
    };

    setTapMult("tap1_mult", ParamID::tap1_mult, static_cast<int>(*apvts.getRawParameterValue("tap1_subdiv")));
    setTapMult("tap2_mult", ParamID::tap2_mult, static_cast<int>(*apvts.getRawParameterValue("tap2_subdiv")));
    setTapMult("tap3_mult", ParamID::tap3_mult, static_cast<int>(*apvts.getRawParameterValue("tap3_subdiv")));
    setTapMult("tap4_mult", ParamID::tap4_mult, static_cast<int>(*apvts.getRawParameterValue("tap4_subdiv")));

    setCoreParam("feedback", ParamID::feedback);
    setCoreParam("mix", ParamID::mix);
    setCoreParam("bypass", ParamID::bypass);
    setCoreParam("stepSize", ParamID::stepSize);
    setCoreParam("clockJitter", ParamID::clockJitter);
    setCoreParam("integratorLag", ParamID::integratorLag);
    setCoreParam("reconstructionCutoff", ParamID::reconstructionCutoff);
    setCoreParam("integratorLeak", ParamID::integratorLeak);
    setCoreParam("dynamicResponse", ParamID::dynamicResponse);
    setCoreParam("wobbleDepth", ParamID::wobbleDepth);
    setCoreParam("wobbleRate", ParamID::wobbleRate);
    setCoreParam("feedbackTone", ParamID::feedbackTone);
    setCoreParam("stereoSpread", ParamID::stereoSpread);
    setCoreParam("freeze", ParamID::freeze);
    setCoreParam("coupledMode", ParamID::coupledMode);
    setCoreParam("envAttack", ParamID::envAttack);
    setCoreParam("envRelease", ParamID::envRelease);
    setCoreParam("circuitType", ParamID::circuitType);
    setCoreParam("minStepSize", ParamID::minStepSize);
    setCoreParam("maxStepSize", ParamID::maxStepSize);
    setCoreParam("syllabicTime", ParamID::syllabicTime);
    setCoreParam("numTaps", ParamID::numTaps);
    setCoreParam("tapDecay", ParamID::tapDecay);
    setCoreParam("tap1_mix", ParamID::tap1_mix);
    setCoreParam("tap2_mix", ParamID::tap2_mix);
    setCoreParam("tap3_mix", ParamID::tap3_mix);
    setCoreParam("tap4_mix", ParamID::tap4_mix);
    setCoreParam("reverseMode", ParamID::reverseMode);
    setCoreParam("reverseChunkSize", ParamID::reverseChunkSize);
    setCoreParam("reverseFeedback", ParamID::reverseFeedback);
    setCoreParam("wobbleSync", ParamID::wobbleSync);

    int numSamples = buffer.getNumSamples();
    
    if (tempIntegrator.size() < numSamples) {
        tempIntegrator.resize(numSamples);
        tempStepSize.resize(numSamples);
    }

    core.process(buffer.getArrayOfReadPointers(), buffer.getArrayOfWritePointers(), 
                 tempIntegrator.data(), tempStepSize.data(), 
                 totalNumOutputChannels, numSamples);

    int start1, size1, start2, size2;
    scopeFifo.prepareToWrite(numSamples, start1, size1, start2, size2);
    
    if (size1 > 0)
    {
        std::copy(tempIntegrator.begin(), tempIntegrator.begin() + size1, diagIntegratorBuffer.begin() + start1);
        std::copy(tempStepSize.begin(), tempStepSize.begin() + size1, diagStepSizeBuffer.begin() + start1);
    }
    if (size2 > 0)
    {
        std::copy(tempIntegrator.begin() + size1, tempIntegrator.begin() + size1 + size2, diagIntegratorBuffer.begin() + start2);
        std::copy(tempStepSize.begin() + size1, tempStepSize.begin() + size1 + size2, diagStepSizeBuffer.begin() + start2);
    }
    
    scopeFifo.finishedWrite(size1 + size2);
}

bool BitlayAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* BitlayAudioProcessor::createEditor() { return new BitlayAudioProcessorEditor (*this); }

void BitlayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xmlState = apvts.copyState().createXml())
        copyXmlToBinary (*xmlState, destData);
}

void BitlayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
        markCurrentPresetClean();
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitlayAudioProcessor();
}

// ==============================================================================
// Preset Management
// ==============================================================================

juce::File BitlayAudioProcessor::getPresetsDirectory() const
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Bitlay")
        .getChildFile("Presets");
    
    if (!dir.exists())
        dir.createDirectory();
        
    return dir;
}

juce::StringArray BitlayAudioProcessor::getPresetNames() const
{
    juce::StringArray names;
    auto dir = getPresetsDirectory();
    auto files = dir.findChildFiles(juce::File::findFiles, false, "*.bitlay");
    for (auto f : files)
        names.add(f.getFileNameWithoutExtension());
    return names;
}

void BitlayAudioProcessor::loadPreset(const juce::String& presetName)
{
    auto file = getPresetsDirectory().getChildFile(presetName + ".bitlay");
    if (file.existsAsFile())
    {
        if (auto xmlState = juce::XmlDocument::parse(file))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
            currentPreset = presetName;
            markCurrentPresetClean();
        }
    }
}

void BitlayAudioProcessor::savePreset(const juce::String& presetName)
{
    auto file = getPresetsDirectory().getChildFile(presetName + ".bitlay");
    if (auto xmlState = apvts.copyState().createXml())
    {
        xmlState->writeTo(file);
        currentPreset = presetName;
        markCurrentPresetClean();
    }
}

bool BitlayAudioProcessor::isCurrentPresetEdited()
{
    return cleanPresetState.isValid() && ! apvts.copyState().isEquivalentTo(cleanPresetState);
}

void BitlayAudioProcessor::markCurrentPresetClean()
{
    cleanPresetState = apvts.copyState().createCopy();
}

void BitlayAudioProcessor::createDefaultPresetsIfNeeded()
{
    auto createPresetFromXml = [this](const juce::String& name, const juce::String& xmlStr) {
        auto file = getPresetsDirectory().getChildFile(name + ".bitlay");
        if (!file.existsAsFile())
        {
            if (auto xml = juce::XmlDocument::parse(xmlStr))
                xml->writeTo(file);
        }
    };

    juce::String defaultXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<PARAMETERS delayTime="350.0" feedback="0.6" mix="0.5" bypass="0.0" stepSize="0.02" clockJitter="0.0" integratorLag="1.0" reconstructionCutoff="3500.0" integratorLeak="0.995" character="25.0" dynamicResponse="0.0" wobbleDepth="0.0" wobbleRate="1.5" feedbackTone="4000.0" stereoSpread="0.0" freeze="0.0" coupledMode="1.0" envAttack="0.005" envRelease="0.05" circuitType="0.0" minStepSize="0.002" maxStepSize="0.12" syllabicTime="20.0" numTaps="2.0" tapDecay="1.0" bpmSync="0.0" internalBpm="120.0" mainSubdivision="4.0" tap1_mult="0.5" tap1_mix="0.8" tap1_subdiv="2.0" tap2_mult="1.0" tap2_mix="0.6" tap2_subdiv="4.0" tap3_mult="1.5" tap3_mix="0.4" tap3_subdiv="5.0" tap4_mult="2.0" tap4_mix="0.2" tap4_subdiv="6.0" reverseMode="0.0" reverseChunkSize="350.0" reverseFeedback="0.0" wobbleSync="0.5"/>)";

    juce::String slapbackXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<PARAMETERS delayTime="110.0" feedback="0.2" mix="0.4" bypass="0.0" stepSize="0.04" clockJitter="2.0" integratorLag="5.0" reconstructionCutoff="2500.0" integratorLeak="0.99" character="80.0" dynamicResponse="0.5" wobbleDepth="0.0" wobbleRate="1.0" feedbackTone="3000.0" stereoSpread="0.0" freeze="0.0" coupledMode="1.0" envAttack="0.01" envRelease="0.1" circuitType="0.0" minStepSize="0.002" maxStepSize="0.12" syllabicTime="20.0" numTaps="1.0" tapDecay="1.0" bpmSync="0.0" internalBpm="120.0" mainSubdivision="7.0" tap1_mult="1.0" tap1_mix="1.0" tap1_subdiv="7.0" tap2_mult="1.0" tap2_mix="0.0" tap2_subdiv="4.0" tap3_mult="1.5" tap3_mix="0.0" tap3_subdiv="5.0" tap4_mult="2.0" tap4_mix="0.0" tap4_subdiv="6.0" reverseMode="0.0" reverseChunkSize="350.0" reverseFeedback="0.0" wobbleSync="0.0"/>)";

    juce::String lofiXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<PARAMETERS delayTime="400.0" feedback="0.75" mix="0.5" bypass="0.0" stepSize="0.06" clockJitter="40.0" integratorLag="15.0" reconstructionCutoff="1200.0" integratorLeak="0.95" character="95.0" dynamicResponse="0.8" wobbleDepth="15.0" wobbleRate="2.2" feedbackTone="1800.0" stereoSpread="25.0" freeze="0.0" coupledMode="0.0" envAttack="0.05" envRelease="0.2" circuitType="1.0" minStepSize="0.005" maxStepSize="0.08" syllabicTime="50.0" numTaps="2.0" tapDecay="0.0" bpmSync="0.0" internalBpm="120.0" mainSubdivision="7.0" tap1_mult="0.7" tap1_mix="0.5" tap1_subdiv="7.0" tap2_mult="1.2" tap2_mix="0.9" tap2_subdiv="7.0" tap3_mult="1.5" tap3_mix="0.0" tap3_subdiv="7.0" tap4_mult="2.0" tap4_mix="0.0" tap4_subdiv="7.0" reverseMode="0.0" reverseChunkSize="350.0" reverseFeedback="0.0" wobbleSync="0.0"/>)";

    juce::String rhythmicXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<PARAMETERS delayTime="500.0" feedback="0.6" mix="0.45" bypass="0.0" stepSize="0.015" clockJitter="0.0" integratorLag="1.0" reconstructionCutoff="5000.0" integratorLeak="0.999" character="15.0" dynamicResponse="0.2" wobbleDepth="2.0" wobbleRate="0.5" feedbackTone="6000.0" stereoSpread="100.0" freeze="0.0" coupledMode="1.0" envAttack="0.001" envRelease="0.02" circuitType="0.0" minStepSize="0.002" maxStepSize="0.12" syllabicTime="10.0" numTaps="4.0" tapDecay="1.0" bpmSync="1.0" internalBpm="120.0" mainSubdivision="4.0" tap1_mult="0.5" tap1_mix="0.8" tap1_subdiv="2.0" tap2_mult="1.0" tap2_mix="0.6" tap2_subdiv="4.0" tap3_mult="1.5" tap3_mix="0.4" tap3_subdiv="5.0" tap4_mult="2.0" tap4_mix="0.2" tap4_subdiv="6.0" reverseMode="0.0" reverseChunkSize="350.0" reverseFeedback="0.0" wobbleSync="0.5"/>)";

    juce::String compandedSpaceXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<PARAMETERS delayTime="850.0" feedback="0.85" mix="0.6" bypass="0.0" stepSize="0.02" clockJitter="5.0" integratorLag="3.0" reconstructionCutoff="2800.0" integratorLeak="0.99" character="40.0" dynamicResponse="0.0" wobbleDepth="8.0" wobbleRate="0.2" feedbackTone="2000.0" stereoSpread="80.0" freeze="0.0" coupledMode="1.0" envAttack="0.005" envRelease="0.05" circuitType="1.0" minStepSize="0.001" maxStepSize="0.15" syllabicTime="80.0" numTaps="3.0" tapDecay="0.0" bpmSync="0.0" internalBpm="120.0" mainSubdivision="7.0" tap1_mult="0.33" tap1_mix="0.4" tap1_subdiv="7.0" tap2_mult="0.66" tap2_mix="0.6" tap2_subdiv="7.0" tap3_mult="1.0" tap3_mix="0.9" tap3_subdiv="7.0" tap4_mult="2.0" tap4_mix="0.0" tap4_subdiv="7.0" reverseMode="0.0" reverseChunkSize="350.0" reverseFeedback="0.0" wobbleSync="0.0"/>)";

    juce::String ghostlyReverseXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<PARAMETERS delayTime="800.0" feedback="0.8" mix="0.7" bypass="0.0" stepSize="0.04" clockJitter="10.0" integratorLag="8.0" reconstructionCutoff="2000.0" integratorLeak="0.95" character="80.0" dynamicResponse="0.3" wobbleDepth="25.0" wobbleRate="0.8" feedbackTone="2500.0" stereoSpread="50.0" freeze="0.0" coupledMode="1.0" envAttack="0.01" envRelease="0.1" circuitType="1.0" minStepSize="0.005" maxStepSize="0.1" syllabicTime="40.0" numTaps="1.0" tapDecay="1.0" bpmSync="0.0" internalBpm="120.0" mainSubdivision="4.0" tap1_mult="1.0" tap1_mix="1.0" tap1_subdiv="4.0" tap2_mult="1.0" tap2_mix="0.0" tap2_subdiv="4.0" tap3_mult="1.0" tap3_mix="0.0" tap3_subdiv="5.0" tap4_mult="1.0" tap4_mix="0.0" tap4_subdiv="6.0" reverseMode="1.0" reverseChunkSize="600.0" reverseFeedback="0.6" wobbleSync="0.0"/>)";

    juce::String crunchySludgeXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<PARAMETERS delayTime="150.0" feedback="0.6" mix="0.5" bypass="0.0" stepSize="0.005" clockJitter="60.0" integratorLag="30.0" reconstructionCutoff="1000.0" integratorLeak="0.90" character="100.0" dynamicResponse="0.8" wobbleDepth="5.0" wobbleRate="3.0" feedbackTone="1500.0" stereoSpread="10.0" freeze="0.0" coupledMode="0.0" envAttack="0.05" envRelease="0.2" circuitType="0.0" minStepSize="0.001" maxStepSize="0.05" syllabicTime="10.0" numTaps="1.0" tapDecay="0.0" bpmSync="0.0" internalBpm="120.0" mainSubdivision="7.0" tap1_mult="1.0" tap1_mix="1.0" tap1_subdiv="7.0" tap2_mult="1.0" tap2_mix="0.0" tap2_subdiv="7.0" tap3_mult="1.0" tap3_mix="0.0" tap3_subdiv="7.0" tap4_mult="1.0" tap4_mix="0.0" tap4_subdiv="7.0" reverseMode="0.0" reverseChunkSize="350.0" reverseFeedback="0.0" wobbleSync="0.0"/>)";

    juce::String cyberpunkPingPongXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<PARAMETERS delayTime="500.0" feedback="0.5" mix="0.5" bypass="0.0" stepSize="0.015" clockJitter="2.0" integratorLag="1.0" reconstructionCutoff="4500.0" integratorLeak="0.99" character="40.0" dynamicResponse="0.4" wobbleDepth="12.0" wobbleRate="0.5" feedbackTone="5000.0" stereoSpread="100.0" freeze="0.0" coupledMode="1.0" envAttack="0.005" envRelease="0.05" circuitType="0.0" minStepSize="0.002" maxStepSize="0.12" syllabicTime="20.0" numTaps="3.0" tapDecay="1.0" bpmSync="1.0" internalBpm="120.0" mainSubdivision="2.0" tap1_mult="0.5" tap1_mix="0.9" tap1_subdiv="0.0" tap2_mult="0.75" tap2_mix="0.8" tap2_subdiv="1.0" tap3_mult="1.0" tap3_mix="0.7" tap3_subdiv="2.0" tap4_mult="1.0" tap4_mix="0.0" tap4_subdiv="7.0" reverseMode="0.0" reverseChunkSize="350.0" reverseFeedback="0.0" wobbleSync="1.0"/>)";

    createPresetFromXml("Default", defaultXml);
    createPresetFromXml("Vintage Slapback", slapbackXml);
    createPresetFromXml("Lo-Fi Wobble", lofiXml);
    createPresetFromXml("Rhythmic Tap Dance", rhythmicXml);
    createPresetFromXml("Deep Companded Space", compandedSpaceXml);
    
    // Novas adições de Sound Design
    createPresetFromXml("Ghostly Reverse", ghostlyReverseXml);
    createPresetFromXml("Crunchy Sludge", crunchySludgeXml);
    createPresetFromXml("Cyberpunk Ping-Pong", cyberpunkPingPongXml);
}
