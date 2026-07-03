export const CVSD_WORKLET_CODE = `
class CVSDDelayProcessor extends AudioWorkletProcessor {
  constructor() {
    super();
    this.bufferSize = sampleRate * 5; // 5 seconds max delay
    this.delayBuffers = []; // delayBuffers[tapIndex][channelIndex]
    this.writePtr = 0;
    this.channels = [];
    this.lfoPhase = 0.0;
  }

  static get parameterDescriptors() {
    return [
      { name: 'delayTime', defaultValue: 300, minValue: 10, maxValue: 2000, automationRate: 'a-rate' },
      { name: 'feedback', defaultValue: 0.4, minValue: 0, maxValue: 1.5, automationRate: 'a-rate' },
      { name: 'mix', defaultValue: 0.5, minValue: 0, maxValue: 1.0, automationRate: 'a-rate' },
      { name: 'bypass', defaultValue: 0, minValue: 0, maxValue: 1, automationRate: 'a-rate' },
      
      { name: 'stepSize', defaultValue: 0.01, minValue: 0.001, maxValue: 0.2 },
      { name: 'clockJitter', defaultValue: 0, minValue: 0, maxValue: 100 },
      { name: 'integratorLag', defaultValue: 1.0, minValue: 1.0, maxValue: 20.0 },
      { name: 'reconstructionCutoff', defaultValue: 4000, minValue: 100, maxValue: 10000 },
      { name: 'integratorLeak', defaultValue: 0.99, minValue: 0.90, maxValue: 1.0 },
      
      { name: 'inputDrive', defaultValue: 1.0, minValue: 1.0, maxValue: 10.0 },
      { name: 'dynamicResponse', defaultValue: 0.0, minValue: 0.0, maxValue: 1.0 },
      
      { name: 'wobbleDepth', defaultValue: 0, minValue: 0, maxValue: 20.0 },
      { name: 'wobbleRate', defaultValue: 1.0, minValue: 0.1, maxValue: 10.0 },
      { name: 'feedbackTone', defaultValue: 4000, minValue: 100, maxValue: 20000 },
      { name: 'stereoSpread', defaultValue: 0, minValue: 0, maxValue: 100 },
      
      // Parameters for coupling and internal state
      { name: 'freeze', defaultValue: 0, minValue: 0, maxValue: 1 },
      { name: 'coupledMode', defaultValue: 1, minValue: 0, maxValue: 1 },
      { name: 'envAttack', defaultValue: 0.005, minValue: 0.001, maxValue: 0.1 },
      { name: 'envRelease', defaultValue: 0.050, minValue: 0.010, maxValue: 0.5 },

      // CVSD / Companded mode parameters
      { name: 'circuitType', defaultValue: 0, minValue: 0, maxValue: 1 }, // 0 = Discrete, 1 = Companded
      { name: 'minStepSize', defaultValue: 0.002, minValue: 0.001, maxValue: 0.2 },
      { name: 'maxStepSize', defaultValue: 0.12, minValue: 0.01, maxValue: 0.5 },
      { name: 'syllabicTime', defaultValue: 20, minValue: 5, maxValue: 100 },

      // NEW Multi-tap parameters
      { name: 'numTaps', defaultValue: 2, minValue: 1, maxValue: 4 },
      { name: 'tapDecay', defaultValue: 1, minValue: 0, maxValue: 1 },
      { name: 'tap1_mult', defaultValue: 0.5, minValue: 0.1, maxValue: 2.0 },
      { name: 'tap1_mix', defaultValue: 0.8, minValue: 0.0, maxValue: 1.0 },
      { name: 'tap2_mult', defaultValue: 1.0, minValue: 0.1, maxValue: 2.0 },
      { name: 'tap2_mix', defaultValue: 0.6, minValue: 0.0, maxValue: 1.0 },
      { name: 'tap3_mult', defaultValue: 1.5, minValue: 0.1, maxValue: 2.0 },
      { name: 'tap3_mix', defaultValue: 0.4, minValue: 0.0, maxValue: 1.0 },
      { name: 'tap4_mult', defaultValue: 2.0, minValue: 0.1, maxValue: 2.0 },
      { name: 'tap4_mix', defaultValue: 0.2, minValue: 0.0, maxValue: 1.0 },

      // NEW Reverse Mode parameters
      { name: 'reverseMode', defaultValue: 0, minValue: 0, maxValue: 1 },
      { name: 'reverseChunkSize', defaultValue: 350, minValue: 50, maxValue: 1500 },
      { name: 'reverseFeedback', defaultValue: 0, minValue: 0, maxValue: 1 },

      // NEW Wobble Sync parameter
      { name: 'wobbleSync', defaultValue: 0.5, minValue: 0.0, maxValue: 1.0 }
    ];
  }

  createChannelState() {
    const createTapState = () => ({
      clockPhase: 0.0,
      integrator: 0.0,
      comparatorState: 0.0,
      lpState: 0.0,
      fbHpState: 0.0,
      fbHpPrevIn: 0.0,
      fbLpState: 0.0,
      envelopeState: 0.0,
      hfEnvelope: 0.0,
      
      // Companded states
      bitHistory: 0,
      syllabicFilterState: 0.0,
      circuitTypeSmoothed: 0.0,

      // Reverse tracking
      reverseTime: 0
    });

    return {
      taps: [
        createTapState(),
        createTapState(),
        createTapState(),
        createTapState()
      ]
    };
  }

  process(inputs, outputs, parameters) {
    const input = inputs[0];
    const output = outputs[0];
    if (!output || output.length === 0) return true;

    // Get parameters
    const delayTimeParam = parameters.delayTime;
    const feedbackParam = parameters.feedback;
    const mixParam = parameters.mix;
    const bypassParam = parameters.bypass;
    const freezeParam = parameters.freeze;
    const circuitTypeParam = parameters.circuitType;
    
    // k-rate parameters
    const wobbleRate = parameters.wobbleRate[0];
    const wobbleDepth = parameters.wobbleDepth[0];
    const feedbackTone = parameters.feedbackTone[0];
    const inputDrive = parameters.inputDrive[0];
    const dynamicResponse = parameters.dynamicResponse[0];
    const stereoSpread = parameters.stereoSpread[0] / 100.0;
    
    const baseStepSize = parameters.stepSize[0];
    const clockJitter = parameters.clockJitter[0] / 100.0;
    const integratorLag = parameters.integratorLag[0];
    const reconstructionCutoff = parameters.reconstructionCutoff[0];
    const integratorLeak = parameters.integratorLeak[0];
    
    const coupledMode = parameters.coupledMode[0] >= 0.5;
    const envAttack = parameters.envAttack[0];
    const envRelease = parameters.envRelease[0];

    const minStepSize = parameters.minStepSize[0];
    const maxStepSize = parameters.maxStepSize[0];
    const syllabicTime = parameters.syllabicTime[0];

    const envAttCoeff = 1.0 - Math.exp(-1.0 / (envAttack * sampleRate));
    const envRelCoeff = 1.0 - Math.exp(-1.0 / (envRelease * sampleRate));
    
    // HF detection for coupling
    const hfAttCoeff = 1.0 - Math.exp(-1.0 / (0.01 * sampleRate));
    const hfRelCoeff = 1.0 - Math.exp(-1.0 / (0.1 * sampleRate));

    // Syllabic filter coefficient (time constant in seconds = syllabicTime / 1000)
    const syllabicCoeff = 1.0 - Math.exp(-1.0 / ((syllabicTime / 1000.0) * sampleRate));

    const numInputChannels = input ? input.length : 0;
    const numOutputChannels = output.length;
    const activeChannels = Math.max(numInputChannels, numOutputChannels, 1);

    while (this.channels.length < activeChannels) {
      this.channels.push(this.createChannelState());
    }

    // Allocate delayBuffers: delayBuffers[tapIndex][channelIndex]
    while (this.delayBuffers.length < 4) {
      this.delayBuffers.push([]);
    }
    for (let t = 0; t < 4; t++) {
      while (this.delayBuffers[t].length < activeChannels) {
        this.delayBuffers[t].push(new Float32Array(this.bufferSize));
      }
    }

    const numSamples = output[0].length;
    const diagIntegrator = outputs[1] ? outputs[1][0] : null;
    const diagStepSize = outputs[2] ? outputs[2][0] : null;

    const lagAlpha = 1.0 / integratorLag;

    for (let i = 0; i < numSamples; i++) {
      const delayMs = delayTimeParam.length > 1 ? delayTimeParam[i] : delayTimeParam[0];
      const feedbackRaw = feedbackParam.length > 1 ? feedbackParam[i] : feedbackParam[0];
      const mix = mixParam.length > 1 ? mixParam[i] : mixParam[0];
      const bypass = bypassParam.length > 1 ? bypassParam[i] : bypassParam[0];
      const isFreeze = (freezeParam && freezeParam.length > 1) ? freezeParam[i] >= 0.5 : (freezeParam && freezeParam[0] >= 0.5);
      const circuitTypeTarget = circuitTypeParam.length > 1 ? circuitTypeParam[i] : circuitTypeParam[0];

      const feedback = isFreeze ? 1.0 : feedbackRaw;

      const wobbleSync = parameters.wobbleSync ? (parameters.wobbleSync.length > 1 ? parameters.wobbleSync[i] : parameters.wobbleSync[0]) : 0.0;
      let currentWobbleRate = wobbleRate;
      if (wobbleSync > 0) {
        const scale = 500.0 / Math.max(10.0, delayMs);
        currentWobbleRate = wobbleRate * (1.0 - wobbleSync + wobbleSync * scale);
        currentWobbleRate = Math.max(0.01, Math.min(25.0, currentWobbleRate));
      }

      this.lfoPhase += (2.0 * Math.PI * currentWobbleRate) / sampleRate;
      if (this.lfoPhase > 2.0 * Math.PI) {
        this.lfoPhase -= 2.0 * Math.PI;
      }      
      const lfoVal = Math.sin(this.lfoPhase);

      const activeTaps = Math.max(1, Math.min(4, Math.round(parameters.numTaps.length > 1 ? parameters.numTaps[i] : parameters.numTaps[0])));
      const activeTapDecay = (parameters.tapDecay.length > 1 ? parameters.tapDecay[i] : parameters.tapDecay[0]) >= 0.5;
      const reverseModeActive = (parameters.reverseMode.length > 1 ? parameters.reverseMode[i] : parameters.reverseMode[0]) >= 0.5;
      const reverseFeedbackActive = (parameters.reverseFeedback.length > 1 ? parameters.reverseFeedback[i] : parameters.reverseFeedback[0]) >= 0.5;

      for (let c = 0; c < activeChannels; c++) {
        const inputChannel = numInputChannels > c ? input[c] : (numInputChannels > 0 ? input[0] : null);
        const outputChannel = output[c];
        const state = this.channels[c];

        let inSample = inputChannel ? inputChannel[i] : 0.0;
        if (isFreeze) inSample = 0.0; // Freeze ignores new input

        const isRightChannel = c === 1;
        const channelSpread = isRightChannel ? stereoSpread : 0.0;
        const channelLfo = isRightChannel ? -lfoVal : lfoVal;

        let mixedOutput = 0.0;
        let diagLpState = 0.0;
        let diagCurrentStepSize = 0.0;

        for (let t = 0; t < activeTaps; t++) {
          const tapState = state.taps[t];
          const delayBuf = this.delayBuffers[t][c];

          const tapMultParam = parameters['tap' + (t + 1) + '_mult'];
          const tapMixParam = parameters['tap' + (t + 1) + '_mix'];
          
          const multiplier = tapMultParam.length > 1 ? tapMultParam[i] : tapMultParam[0];
          const tapMix = tapMixParam.length > 1 ? tapMixParam[i] : tapMixParam[0];

          // Compute delay time for this tap
          const modDelayMs = (delayMs * multiplier) + channelLfo * wobbleDepth;
          const delaySamples = (Math.max(1, modDelayMs) * sampleRate) / 1000.0;

          let readPos = this.writePtr - delaySamples;
          if (readPos < 0) {
            readPos += this.bufferSize;
            if (readPos < 0) {
              readPos = (readPos % this.bufferSize) + this.bufferSize;
            }
          }
          
          const index0 = Math.floor(readPos) % this.bufferSize;
          const index1 = (index0 + 1) % this.bufferSize;
          const frac = readPos - Math.floor(readPos);
          const delayedSample = delayBuf[index0] + frac * (delayBuf[index1] - delayBuf[index0]);

          // --- Reverse Playback Processing ---
          let finalDelayedSample = delayedSample;
          let outputSample = 0.0;

          if (reverseModeActive) {
            const chunkSizeMs = parameters.reverseChunkSize.length > 1 ? parameters.reverseChunkSize[i] : parameters.reverseChunkSize[0];
            // Minimum 5ms chunk to avoid divide-by-zero or extreme glitches, max 1500ms
            const L = Math.max(240, Math.round((Math.max(5, chunkSizeMs) * sampleRate) / 1000.0));

            // Track reverse index separately per tap and channel
            tapState.reverseTime = (tapState.reverseTime || 0) + 1;

            const phaseA = tapState.reverseTime % L;
            const phaseB = (tapState.reverseTime + Math.floor(L / 2)) % L;

            // Head A
            let readPosA = this.writePtr - delaySamples - 2 * phaseA;
            if (readPosA < 0) {
              readPosA += this.bufferSize;
              if (readPosA < 0) {
                readPosA = (readPosA % this.bufferSize) + this.bufferSize;
              }
            }
            const idxA0 = Math.floor(readPosA) % this.bufferSize;
            const idxA1 = (idxA0 + 1) % this.bufferSize;
            const fracA = readPosA - Math.floor(readPosA);
            const sampleA = delayBuf[idxA0] + fracA * (delayBuf[idxA1] - delayBuf[idxA0]);

            // Head B
            let readPosB = this.writePtr - delaySamples - 2 * phaseB;
            if (readPosB < 0) {
              readPosB += this.bufferSize;
              if (readPosB < 0) {
                readPosB = (readPosB % this.bufferSize) + this.bufferSize;
              }
            }
            const idxB0 = Math.floor(readPosB) % this.bufferSize;
            const idxB1 = (idxB0 + 1) % this.bufferSize;
            const fracB = readPosB - Math.floor(readPosB);
            const sampleB = delayBuf[idxB0] + fracB * (delayBuf[idxB1] - delayBuf[idxB0]);

            const halfL = L / 2;
            let winA = 0.0;
            if (phaseA < halfL) {
              winA = phaseA / halfL;
            } else {
              winA = (L - phaseA) / halfL;
            }

            let winB = 0.0;
            if (phaseB < halfL) {
              winB = phaseB / halfL;
            } else {
              winB = (L - phaseB) / halfL;
            }

            const sumWin = winA + winB;
            if (sumWin > 0.0) {
              winA /= sumWin;
              winB /= sumWin;
            }

            const reversedSample = sampleA * winA + sampleB * winB;

            if (reverseFeedbackActive) {
              finalDelayedSample = reversedSample;
            } else {
              finalDelayedSample = delayedSample;
            }

            outputSample = reversedSample;
          }

          // --- Degradation factor for this tap ---
          const decayFactor = activeTapDecay ? (1.0 + (multiplier - 0.5) * 1.5) : 1.0;

          // Scale LPF cutoffs down for further taps (degrades tone)
          const tapFeedbackTone = feedbackTone / (activeTapDecay ? (1.0 + (multiplier - 0.5) * 1.0) : 1.0);
          const tapReconCutoff = reconstructionCutoff / (activeTapDecay ? (1.0 + (multiplier - 0.5) * 0.6) : 1.0);

          const thetaRecon = (2.0 * Math.PI * Math.max(50, tapReconCutoff)) / sampleRate;
          const reconLpCoeff = 1.0 - Math.exp(-thetaRecon);

          const thetaLp = (2.0 * Math.PI * Math.max(50, tapFeedbackTone)) / sampleRate;
          const fbLpCoeff = 1.0 - Math.exp(-thetaLp);

          // 1. Highpass Filter (Fixed to remove DC drift)
          const thetaHp = (2.0 * Math.PI * 150) / sampleRate;
          const hpAlpha = 1.0 / (1.0 + thetaHp);
          const fbHpOut = hpAlpha * (tapState.fbHpState + finalDelayedSample - tapState.fbHpPrevIn);
          tapState.fbHpPrevIn = finalDelayedSample;
          tapState.fbHpState = fbHpOut;
          
          // 2. Feedback Tone (Lowpass Filter)
          tapState.fbLpState = tapState.fbLpState + fbLpCoeff * (fbHpOut - tapState.fbLpState);

          // --- Feedback mixing per tap ---
          const mixed = inSample + tapState.fbLpState * feedback;

          // Asymmetrical envelope follower for dynamic response
          const absMixed = Math.abs(mixed);
          if (absMixed > tapState.envelopeState) {
            tapState.envelopeState += envAttCoeff * (absMixed - tapState.envelopeState);
          } else {
            tapState.envelopeState += envRelCoeff * (absMixed - tapState.envelopeState);
          }

          // Soft clipping
          let driven = mixed * inputDrive;
          let encoderInput = Math.tanh(driven);

          // --- Transition crossfade smoothing ---
          const crossfadeTimeMs = 30.0;
          const crossfadeCoeff = 1.0 - Math.exp(-1.0 / ((crossfadeTimeMs / 1000.0) * sampleRate));
          tapState.circuitTypeSmoothed += crossfadeCoeff * (circuitTypeTarget - tapState.circuitTypeSmoothed);

          // --- 1. Discrete Step Size Calculation ---
          // Coupling 1: Tone -> Slope Overload
          const hfDiff = delayedSample - tapState.fbLpState;
          const hfEnergy = Math.abs(hfDiff);
          if (hfEnergy > tapState.hfEnvelope) {
            tapState.hfEnvelope += hfAttCoeff * (hfEnergy - tapState.hfEnvelope);
          } else {
            tapState.hfEnvelope += hfRelCoeff * (hfEnergy - tapState.hfEnvelope);
          }
          const hfFactor = 0.5 + 2.0 * tapState.hfEnvelope;
          
          // Coupling 2: Feedback -> Step Size
          const fbFactor = 1.0 - Math.min(feedback, 1.0) * 0.2;
          
          // Dynamic response (envelope follower) opens step size on transients
          const envFactor = 1.0 + dynamicResponse * (tapState.envelopeState * 2.0);
          
          let discreteStepScale = 1.0;
          if (coupledMode) {
            discreteStepScale = hfFactor * fbFactor * envFactor;
          } else {
            discreteStepScale = 1.0 + dynamicResponse * (tapState.envelopeState * 2.0);
          }
          // Scale step size with decay factor
          let discreteStepSize = baseStepSize * discreteStepScale * decayFactor * (1.0 + channelSpread * 0.2);
          discreteStepSize = Math.max(0.0001, Math.min(discreteStepSize, 0.5));

          // --- 2. Companded / Syllabic Step Size Calculation ---
          const coincidence = (tapState.bitHistory === 0x0F || tapState.bitHistory === 0x00) ? 1.0 : 0.0;
          tapState.syllabicFilterState += syllabicCoeff * (coincidence - tapState.syllabicFilterState);
          
          let compandedStepSize = (minStepSize * decayFactor + tapState.syllabicFilterState * (maxStepSize * decayFactor - minStepSize * decayFactor)) * (1.0 + channelSpread * 0.2);
          compandedStepSize = Math.max(0.0001, Math.min(compandedStepSize, 0.5));

          // --- 3. Interpolated Step Size ---
          const currentStepSize = (1.0 - tapState.circuitTypeSmoothed) * discreteStepSize + tapState.circuitTypeSmoothed * compandedStepSize;

          // --- DELTA ENCODER / DECODER CORE ---
          const channelClockJitter = clockJitter * (1.0 + channelSpread * 0.5);
          tapState.clockPhase += 1.0 + (Math.random() * 2.0 - 1.0) * channelClockJitter + channelLfo * (channelClockJitter * 0.2);
          
          if (tapState.clockPhase >= 1.0) {
            tapState.clockPhase -= 1.0;
            
            const bit = (encoderInput >= tapState.integrator) ? 1.0 : -1.0;
            
            // Shift comparator decision into history shift register
            tapState.bitHistory = ((tapState.bitHistory << 1) | (bit > 0.0 ? 1 : 0)) & 0x0F;
            
            const channelLagAlpha = lagAlpha * (1.0 - channelSpread * 0.15);
            tapState.comparatorState = tapState.comparatorState + channelLagAlpha * (bit - tapState.comparatorState);
            
            tapState.integrator = tapState.integrator * integratorLeak + tapState.comparatorState * currentStepSize;
          }
          
          if (isNaN(tapState.integrator)) {
            tapState.integrator = 0.0;
          } else {
            if (tapState.integrator > 3.0) tapState.integrator = 3.0;
            else if (tapState.integrator < -3.0) tapState.integrator = -3.0;
          }
          
          const channelReconCoeff = reconLpCoeff * (1.0 - channelSpread * 0.2);
          tapState.lpState = tapState.lpState + channelReconCoeff * (tapState.integrator - tapState.lpState);
          
          delayBuf[this.writePtr] = tapState.lpState;

          if (!reverseModeActive) {
            outputSample = tapState.lpState;
          }

          // Accumulate this tap output
          mixedOutput += outputSample * tapMix;

          // For first tap, store diagnostics
          if (t === 0) {
            diagLpState = outputSample;
            diagCurrentStepSize = currentStepSize;
          }
        }

        if (c === 0) {
          if (diagIntegrator) diagIntegrator[i] = (bypass >= 0.5) ? 0 : diagLpState; 
          if (diagStepSize) diagStepSize[i] = (bypass >= 0.5) ? 0 : diagCurrentStepSize * 10.0;
        }

        if (outputChannel) {
          if (bypass >= 0.5) {
            outputChannel[i] = inSample;
          } else {
            if (isFreeze) {
               outputChannel[i] = diagLpState * mix;
            } else {
               outputChannel[i] = inSample * (1.0 - mix) + mixedOutput * mix;
            }
          }
        }
      }
      this.writePtr = (this.writePtr + 1) % this.bufferSize;
    }
    return true;
  }
}
registerProcessor('cvsd-delay-processor', CVSDDelayProcessor);
`;
