#define _USE_MATH_DEFINES
#include "CVSDDelayCore.h"
#include <algorithm>

CVSDDelayCore::CVSDDelayCore() : m_rng(1337) {
    for (int i = 0; i < static_cast<int>(ParamID::NUM_PARAMS); ++i) {
        m_params[i] = 0.0;
        m_paramRamps[i] = nullptr;
    }

    m_params[static_cast<int>(ParamID::delayTime)] = 300.0;
    m_params[static_cast<int>(ParamID::feedback)] = 0.4;
    m_params[static_cast<int>(ParamID::mix)] = 0.5;
    m_params[static_cast<int>(ParamID::bypass)] = 0.0;
    m_params[static_cast<int>(ParamID::stepSize)] = 0.01;
    m_params[static_cast<int>(ParamID::clockJitter)] = 0.0;
    m_params[static_cast<int>(ParamID::integratorLag)] = 1.0;
    m_params[static_cast<int>(ParamID::reconstructionCutoff)] = 4000.0;
    m_params[static_cast<int>(ParamID::integratorLeak)] = 0.99;
    m_params[static_cast<int>(ParamID::inputDrive)] = 1.0;
    m_params[static_cast<int>(ParamID::dynamicResponse)] = 0.0;
    m_params[static_cast<int>(ParamID::wobbleDepth)] = 0.0;
    m_params[static_cast<int>(ParamID::wobbleRate)] = 1.0;
    m_params[static_cast<int>(ParamID::feedbackTone)] = 4000.0;
    m_params[static_cast<int>(ParamID::stereoSpread)] = 0.0;
    m_params[static_cast<int>(ParamID::freeze)] = 0.0;
    m_params[static_cast<int>(ParamID::coupledMode)] = 1.0;
    m_params[static_cast<int>(ParamID::envAttack)] = 0.005;
    m_params[static_cast<int>(ParamID::envRelease)] = 0.05;
    m_params[static_cast<int>(ParamID::circuitType)] = 0.0;
    m_params[static_cast<int>(ParamID::minStepSize)] = 0.002;
    m_params[static_cast<int>(ParamID::maxStepSize)] = 0.12;
    m_params[static_cast<int>(ParamID::syllabicTime)] = 20.0;
    m_params[static_cast<int>(ParamID::numTaps)] = 2.0;
    m_params[static_cast<int>(ParamID::tapDecay)] = 1.0;
    m_params[static_cast<int>(ParamID::tap1_mult)] = 0.5;
    m_params[static_cast<int>(ParamID::tap1_mix)] = 0.8;
    m_params[static_cast<int>(ParamID::tap2_mult)] = 1.0;
    m_params[static_cast<int>(ParamID::tap2_mix)] = 0.6;
    m_params[static_cast<int>(ParamID::tap3_mult)] = 1.5;
    m_params[static_cast<int>(ParamID::tap3_mix)] = 0.4;
    m_params[static_cast<int>(ParamID::tap4_mult)] = 2.0;
    m_params[static_cast<int>(ParamID::tap4_mix)] = 0.2;
    m_params[static_cast<int>(ParamID::reverseMode)] = 0.0;
    m_params[static_cast<int>(ParamID::reverseChunkSize)] = 350.0;
    m_params[static_cast<int>(ParamID::reverseFeedback)] = 0.0;
    m_params[static_cast<int>(ParamID::wobbleSync)] = 0.5;
}

CVSDDelayCore::~CVSDDelayCore() = default;

void CVSDDelayCore::setRandomSeed(uint32_t seed) {
    m_rng.seed(seed);
}

void CVSDDelayCore::prepare(double sampleRate, int maxChannels) {
    m_sampleRate = sampleRate;
    m_bufferSize = static_cast<int>(sampleRate * 5.0); // 5 seconds max delay
    
    m_channels.resize(maxChannels);
    m_delayBuffers.resize(4);
    for (int t = 0; t < 4; t++) {
        m_delayBuffers[t].assign(maxChannels, std::vector<float>(m_bufferSize, 0.0f));
    }
    
    reset();
}

void CVSDDelayCore::reset() {
    m_writePtr = 0;
    m_lfoPhase = 0.0;
    
    // Clear channel state
    for (auto& ch : m_channels) {
        for (int t = 0; t < 4; ++t) {
            ch.taps[t] = TapState();
        }
    }

    // Clear delay buffers
    for (auto& tapBufs : m_delayBuffers) {
        for (auto& buf : tapBufs) {
            std::fill(buf.begin(), buf.end(), 0.0f);
        }
    }
}

void CVSDDelayCore::setParameter(ParamID id, double value) {
    m_params[static_cast<int>(id)] = value;
}

void CVSDDelayCore::setParameterRamp(ParamID id, const double* perSampleValues, int numSamples) {
    m_paramRamps[static_cast<int>(id)] = perSampleValues;
}

double CVSDDelayCore::getParam(ParamID id, int sampleIndex) const {
    int idx = static_cast<int>(id);
    double val;
    if (m_paramRamps[idx] != nullptr) {
        val = m_paramRamps[idx][sampleIndex];
    } else {
        val = m_params[idx];
    }
    return static_cast<double>(static_cast<float>(val));
}

void CVSDDelayCore::process(const float* const* inputs, float* const* outputs, int numChannels, int numSamples) {
    process(inputs, outputs, nullptr, nullptr, numChannels, numSamples);
}

void CVSDDelayCore::process(const float* const* inputs, float* const* outputs,
                            float* diagIntegrator, float* diagStepSize,
                            int numChannels, int numSamples) {
    if (numChannels == 0 || numSamples == 0) return;

    // k-rate parameters that don't ramp
    double wobbleRate = m_params[static_cast<int>(ParamID::wobbleRate)];
    double wobbleDepth = m_params[static_cast<int>(ParamID::wobbleDepth)];
    double feedbackTone = m_params[static_cast<int>(ParamID::feedbackTone)];
    double inputDrive = m_params[static_cast<int>(ParamID::inputDrive)];
    double dynamicResponse = m_params[static_cast<int>(ParamID::dynamicResponse)];
    double stereoSpread = m_params[static_cast<int>(ParamID::stereoSpread)] / 100.0;

    double baseStepSize = m_params[static_cast<int>(ParamID::stepSize)];
    double clockJitter = m_params[static_cast<int>(ParamID::clockJitter)] / 100.0;
    double integratorLag = m_params[static_cast<int>(ParamID::integratorLag)];
    double reconstructionCutoff = m_params[static_cast<int>(ParamID::reconstructionCutoff)];
    double integratorLeak = m_params[static_cast<int>(ParamID::integratorLeak)];

    bool coupledMode = m_params[static_cast<int>(ParamID::coupledMode)] >= 0.5;
    double envAttack = m_params[static_cast<int>(ParamID::envAttack)];
    double envRelease = m_params[static_cast<int>(ParamID::envRelease)];

    double minStepSize = m_params[static_cast<int>(ParamID::minStepSize)];
    double maxStepSize = m_params[static_cast<int>(ParamID::maxStepSize)];
    double syllabicTime = m_params[static_cast<int>(ParamID::syllabicTime)];

    double envAttCoeff = 1.0 - std::exp(-1.0 / (envAttack * m_sampleRate));
    double envRelCoeff = 1.0 - std::exp(-1.0 / (envRelease * m_sampleRate));

    double hfAttCoeff = 1.0 - std::exp(-1.0 / (0.01 * m_sampleRate));
    double hfRelCoeff = 1.0 - std::exp(-1.0 / (0.1 * m_sampleRate));

    double syllabicCoeff = 1.0 - std::exp(-1.0 / ((syllabicTime / 1000.0) * m_sampleRate));
    double lagAlpha = 1.0 / integratorLag;

    int activeChannels = numChannels;
    if (activeChannels > m_channels.size()) {
        activeChannels = m_channels.size(); // Prevent out-of-bounds if caller exceeds maxChannels
    }

    ParamID tapMultParams[4] = {ParamID::tap1_mult, ParamID::tap2_mult, ParamID::tap3_mult, ParamID::tap4_mult};
    ParamID tapMixParams[4] = {ParamID::tap1_mix, ParamID::tap2_mix, ParamID::tap3_mix, ParamID::tap4_mix};

    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    for (int i = 0; i < numSamples; i++) {
        double delayMs = getParam(ParamID::delayTime, i);
        double feedbackRaw = getParam(ParamID::feedback, i);
        double mixParam = getParam(ParamID::mix, i);
        double bypass = getParam(ParamID::bypass, i);
        bool isFreeze = getParam(ParamID::freeze, i) >= 0.5;
        double circuitTypeTarget = getParam(ParamID::circuitType, i);

        double currentFeedback = isFreeze ? 1.0 : feedbackRaw;

        double currentWobbleSync = getParam(ParamID::wobbleSync, i);
        double currentWobbleRate = wobbleRate;
        if (currentWobbleSync > 0) {
            double scale = 500.0 / std::max(10.0, delayMs);
            currentWobbleRate = wobbleRate * (1.0 - currentWobbleSync + currentWobbleSync * scale);
            currentWobbleRate = std::max(0.01, std::min(25.0, currentWobbleRate));
        }

        m_lfoPhase += (2.0 * M_PI * currentWobbleRate) / m_sampleRate;
        if (m_lfoPhase > 2.0 * M_PI) {
            m_lfoPhase -= 2.0 * M_PI;
        }
        double lfoVal = std::sin(m_lfoPhase);

        int activeTaps = std::max(1, std::min(4, static_cast<int>(std::round(getParam(ParamID::numTaps, i)))));
        bool activeTapDecay = getParam(ParamID::tapDecay, i) >= 0.5;
        bool reverseModeActive = getParam(ParamID::reverseMode, i) >= 0.5;
        bool reverseFeedbackActive = getParam(ParamID::reverseFeedback, i) >= 0.5;

        for (int c = 0; c < activeChannels; c++) {
            const float* inputChannel = (inputs != nullptr && inputs[c] != nullptr) ? inputs[c] : nullptr;
            float* outputChannel = (outputs != nullptr && outputs[c] != nullptr) ? outputs[c] : nullptr;
            
            double inSample = inputChannel ? static_cast<double>(inputChannel[i]) : 0.0;
            if (isFreeze) inSample = 0.0;

            bool isRightChannel = (c == 1);
            double channelSpread = isRightChannel ? stereoSpread : 0.0;
            double channelLfo = isRightChannel ? -lfoVal : lfoVal;

            double mixedOutput = 0.0;
            double diagLpState = 0.0;
            double diagCurrentStepSize = 0.0;

            for (int t = 0; t < activeTaps; t++) {
                TapState& tapState = m_channels[c].taps[t];
                std::vector<float>& delayBuf = m_delayBuffers[t][c];

                double multiplier = getParam(tapMultParams[t], i);
                double tapMix = getParam(tapMixParams[t], i);

                double modDelayMs = delayMs * multiplier + channelLfo * wobbleDepth;
                double delaySamples = (std::max(1.0, modDelayMs) * m_sampleRate) / 1000.0;

                double readPos = static_cast<double>(m_writePtr) - delaySamples;
                if (readPos < 0.0) {
                    readPos += m_bufferSize;
                    if (readPos < 0.0) {
                        readPos = std::fmod(readPos, static_cast<double>(m_bufferSize)) + m_bufferSize;
                    }
                }

                int index0 = static_cast<int>(std::floor(readPos)) % m_bufferSize;
                int index1 = (index0 + 1) % m_bufferSize;
                double frac = readPos - std::floor(readPos);
                
                // Read from Float32Array (std::vector<float>) to double explicitly
                double delayedSample = static_cast<double>(delayBuf[index0]) + frac * (static_cast<double>(delayBuf[index1]) - static_cast<double>(delayBuf[index0]));

                double finalDelayedSample = delayedSample;
                double outputSample = 0.0;

                if (reverseModeActive) {
                    double chunkSizeMs = getParam(ParamID::reverseChunkSize, i);
                    int L = std::max(
                        static_cast<int>(std::round(0.005 * m_sampleRate)),
                        static_cast<int>(std::round((std::max(5.0, chunkSizeMs) * m_sampleRate) / 1000.0))
                    );

                    tapState.reverseTime = tapState.reverseTime + 1;

                    uint32_t phaseA = tapState.reverseTime % L;
                    uint32_t phaseB = (tapState.reverseTime + L / 2) % L;

                    double readPosA = static_cast<double>(m_writePtr) - delaySamples - 2.0 * phaseA;
                    if (readPosA < 0.0) {
                        readPosA += m_bufferSize;
                        if (readPosA < 0.0) readPosA = std::fmod(readPosA, static_cast<double>(m_bufferSize)) + m_bufferSize;
                    }
                    int idxA0 = static_cast<int>(std::floor(readPosA)) % m_bufferSize;
                    int idxA1 = (idxA0 + 1) % m_bufferSize;
                    double fracA = readPosA - std::floor(readPosA);
                    double sampleA = static_cast<double>(delayBuf[idxA0]) + fracA * (static_cast<double>(delayBuf[idxA1]) - static_cast<double>(delayBuf[idxA0]));

                    double readPosB = static_cast<double>(m_writePtr) - delaySamples - 2.0 * phaseB;
                    if (readPosB < 0.0) {
                        readPosB += m_bufferSize;
                        if (readPosB < 0.0) readPosB = std::fmod(readPosB, static_cast<double>(m_bufferSize)) + m_bufferSize;
                    }
                    int idxB0 = static_cast<int>(std::floor(readPosB)) % m_bufferSize;
                    int idxB1 = (idxB0 + 1) % m_bufferSize;
                    double fracB = readPosB - std::floor(readPosB);
                    double sampleB = static_cast<double>(delayBuf[idxB0]) + fracB * (static_cast<double>(delayBuf[idxB1]) - static_cast<double>(delayBuf[idxB0]));

                    double halfL = L / 2.0;
                    double winA = phaseA < halfL ? phaseA / halfL : (L - phaseA) / halfL;
                    double winB = phaseB < halfL ? phaseB / halfL : (L - phaseB) / halfL;

                    double sumWin = winA + winB;
                    if (sumWin > 0.0) {
                        winA /= sumWin;
                        winB /= sumWin;
                    }

                    double reversedSample = sampleA * winA + sampleB * winB;
                    finalDelayedSample = reverseFeedbackActive ? reversedSample : delayedSample;
                    outputSample = reversedSample;
                }

                double decayFactor = activeTapDecay ? 1.0 + (multiplier - 0.5) * 1.5 : 1.0;
                double tapFeedbackTone = feedbackTone / (activeTapDecay ? 1.0 + (multiplier - 0.5) * 1.0 : 1.0);
                double tapReconCutoff = reconstructionCutoff / (activeTapDecay ? 1.0 + (multiplier - 0.5) * 0.6 : 1.0);

                double thetaRecon = (2.0 * M_PI * std::max(50.0, tapReconCutoff)) / m_sampleRate;
                double reconLpCoeff = 1.0 - std::exp(-thetaRecon);

                double thetaLp = (2.0 * M_PI * std::max(50.0, tapFeedbackTone)) / m_sampleRate;
                double fbLpCoeff = 1.0 - std::exp(-thetaLp);

                double thetaHp = (2.0 * M_PI * 150.0) / m_sampleRate;
                double hpAlpha = 1.0 / (1.0 + thetaHp);
                
                double fbHpOut = hpAlpha * (tapState.fbHpState + finalDelayedSample - tapState.fbHpPrevIn);
                tapState.fbHpPrevIn = finalDelayedSample;
                tapState.fbHpState = flushDenormal(fbHpOut);

                tapState.fbLpState = flushDenormal(
                    tapState.fbLpState + fbLpCoeff * (fbHpOut - tapState.fbLpState)
                );

                double mixed = inSample + tapState.fbLpState * currentFeedback;

                double absMixed = std::abs(mixed);
                if (absMixed > tapState.envelopeState) {
                    tapState.envelopeState += envAttCoeff * (absMixed - tapState.envelopeState);
                } else {
                    tapState.envelopeState += envRelCoeff * (absMixed - tapState.envelopeState);
                }
                tapState.envelopeState = flushDenormal(tapState.envelopeState);

                double driven = mixed * inputDrive;
                double encoderInput = std::tanh(driven);

                double crossfadeTimeMs = 30.0;
                double crossfadeCoeff = 1.0 - std::exp(-1.0 / ((crossfadeTimeMs / 1000.0) * m_sampleRate));
                tapState.circuitTypeSmoothed += crossfadeCoeff * (circuitTypeTarget - tapState.circuitTypeSmoothed);

                double hfDiff = delayedSample - tapState.fbLpState;
                double hfEnergy = std::abs(hfDiff);
                if (hfEnergy > tapState.hfEnvelope) {
                    tapState.hfEnvelope += hfAttCoeff * (hfEnergy - tapState.hfEnvelope);
                } else {
                    tapState.hfEnvelope += hfRelCoeff * (hfEnergy - tapState.hfEnvelope);
                }
                tapState.hfEnvelope = flushDenormal(tapState.hfEnvelope);

                double hfFactor = 0.5 + 2.0 * tapState.hfEnvelope;
                double fbFactor = 1.0 - std::min(currentFeedback, 1.0) * 0.2;
                double envFactor = 1.0 + dynamicResponse * (tapState.envelopeState * 2.0);

                double discreteStepScale = coupledMode ? (hfFactor * fbFactor * envFactor) : (1.0 + dynamicResponse * (tapState.envelopeState * 2.0));
                double discreteStepSize = baseStepSize * discreteStepScale * decayFactor * (1.0 + channelSpread * 0.2);
                discreteStepSize = std::max(0.0001, std::min(discreteStepSize, 0.5));

                double coincidence = (tapState.bitHistory == 0x0f || tapState.bitHistory == 0x00) ? 1.0 : 0.0;
                tapState.syllabicFilterState += syllabicCoeff * (coincidence - tapState.syllabicFilterState);
                tapState.syllabicFilterState = flushDenormal(tapState.syllabicFilterState);

                double compandedStepSize = (minStepSize * decayFactor + tapState.syllabicFilterState * (maxStepSize * decayFactor - minStepSize * decayFactor)) * (1.0 + channelSpread * 0.2);
                compandedStepSize = std::max(0.0001, std::min(compandedStepSize, 0.5));

                double currentStepSize = (1.0 - tapState.circuitTypeSmoothed) * discreteStepSize + tapState.circuitTypeSmoothed * compandedStepSize;

                double channelClockJitter = clockJitter * (1.0 + channelSpread * 0.5);
                
                tapState.clockPhase += 1.0 + dist(m_rng) * channelClockJitter + channelLfo * (channelClockJitter * 0.2);

                if (tapState.clockPhase >= 1.0) {
                    tapState.clockPhase -= 1.0;
                    double bit = encoderInput >= tapState.integrator ? 1.0 : -1.0;
                    tapState.bitHistory = ((tapState.bitHistory << 1) | (bit > 0.0 ? 1 : 0)) & 0x0f;
                    double channelLagAlpha = lagAlpha * (1.0 - channelSpread * 0.15);
                    tapState.comparatorState = tapState.comparatorState + channelLagAlpha * (bit - tapState.comparatorState);
                    tapState.integrator = tapState.integrator * integratorLeak + tapState.comparatorState * currentStepSize;
                }

                if (std::isnan(tapState.integrator)) {
                    tapState.integrator = 0.0;
                } else {
                    if (tapState.integrator > 3.0) tapState.integrator = 3.0;
                    else if (tapState.integrator < -3.0) tapState.integrator = -3.0;
                }

                double channelReconCoeff = reconLpCoeff * (1.0 - channelSpread * 0.2);
                tapState.lpState = tapState.lpState + channelReconCoeff * (tapState.integrator - tapState.lpState);
                tapState.lpState = flushDenormal(tapState.lpState);

                // Write to delay buffer - truncates implicitly because delayBuf is std::vector<float>
                delayBuf[m_writePtr] = static_cast<float>(tapState.lpState);

                if (!reverseModeActive) {
                    outputSample = finalDelayedSample;
                }

                mixedOutput += outputSample * tapMix;

                if (t == 0) {
                    diagLpState = outputSample;
                    diagCurrentStepSize = currentStepSize;
                }
            }

            if (c == 0) {
                if (diagIntegrator) diagIntegrator[i] = static_cast<float>(bypass >= 0.5 ? 0.0 : diagLpState);
                if (diagStepSize) diagStepSize[i] = static_cast<float>(bypass >= 0.5 ? 0.0 : diagCurrentStepSize * 10.0);
            }

            if (outputChannel) {
                if (bypass >= 0.5) {
                    outputChannel[i] = static_cast<float>(inSample);
                } else {
                    if (isFreeze) {
                        outputChannel[i] = static_cast<float>(diagLpState * mixParam);
                    } else {
                        outputChannel[i] = static_cast<float>(inSample * (1.0 - mixParam) + mixedOutput * mixParam);
                    }
                }
            }
        }
        m_writePtr = (m_writePtr + 1) % m_bufferSize;
    }
}
