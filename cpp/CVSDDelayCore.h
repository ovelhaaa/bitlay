#pragma once

#include <vector>
#include <random>
#include <cstdint>
#include <cmath>

enum class ParamID {
    delayTime = 0,
    feedback,
    mix,
    bypass,
    stepSize,
    clockJitter,
    integratorLag,
    reconstructionCutoff,
    integratorLeak,
    inputDrive,
    dynamicResponse,
    wobbleDepth,
    wobbleRate,
    feedbackTone,
    stereoSpread,
    freeze,
    coupledMode,
    envAttack,
    envRelease,
    circuitType,
    minStepSize,
    maxStepSize,
    syllabicTime,
    numTaps,
    tapDecay,
    tap1_mult,
    tap1_mix,
    tap2_mult,
    tap2_mix,
    tap3_mult,
    tap3_mix,
    tap4_mult,
    tap4_mix,
    reverseMode,
    reverseChunkSize,
    reverseFeedback,
    wobbleSync,
    NUM_PARAMS
};

class CVSDDelayCore {
public:
    CVSDDelayCore();
    ~CVSDDelayCore();

    void prepare(double sampleRate, int maxChannels = 2);
    void reset();

    // Parâmetros k-rate
    void setParameter(ParamID id, double value);

    // Parâmetros a-rate
    void setParameterRamp(ParamID id, const double* perSampleValues, int numSamples);

    // Opcional: define a seed para testes determinísticos
    void setRandomSeed(uint32_t seed);

    void process(const float* const* inputs, float* const* outputs,
                 int numChannels, int numSamples);

    void process(const float* const* inputs, float* const* outputs,
                 float* diagIntegrator, float* diagStepSize,
                 int numChannels, int numSamples);

private:
    struct TapState {
        double clockPhase = 0.0;
        double integrator = 0.0;
        double comparatorState = 0.0;
        double lpState = 0.0;
        double fbHpState = 0.0;
        double fbHpPrevIn = 0.0;
        double fbLpState = 0.0;
        double envelopeState = 0.0;
        double hfEnvelope = 0.0;

        uint32_t bitHistory = 0;
        double syllabicFilterState = 0.0;
        double circuitTypeSmoothed = 0.0;

        uint32_t reverseTime = 0;
    };

    struct ChannelState {
        TapState taps[4];
    };

    double flushDenormal(double val) const {
        // Anti-denormal branchless protection (adiciona e remove valor DC minúsculo)
        val += 1e-18;
        val -= 1e-18;
        return val;
    }

    double m_sampleRate = 44100.0;
    int m_bufferSize = 0;
    int m_writePtr = 0;
    double m_lfoPhase = 0.0;

    std::vector<ChannelState> m_channels;

    // delayBuffers[tapIndex][channelIndex]
    std::vector<std::vector<std::vector<float>>> m_delayBuffers;

    // k-rate parameters
    double m_params[static_cast<int>(ParamID::NUM_PARAMS)];
    
    // a-rate parameter pointers
    const double* m_paramRamps[static_cast<int>(ParamID::NUM_PARAMS)];
    
    std::mt19937 m_rng;
    
    double getParam(ParamID id, int sampleIndex) const;
};
