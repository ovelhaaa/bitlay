const fs = require('fs');

const workletCode = `
class CVSDDelayProcessor extends AudioWorkletProcessor {
  constructor() {
    super();
    this.bufferSize = sampleRate * 5; // 5 seconds max delay
    this.delayBuffers = []; // one per channel
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
      
      // New parameters for coupling and internal state
      { name: 'freeze', defaultValue: 0, minValue: 0, maxValue: 1 },
      { name: 'coupledMode', defaultValue: 1, minValue: 0, maxValue: 1 },
      { name: 'envAttack', defaultValue: 0.005, minValue: 0.001, maxValue: 0.1 },
      { name: 'envRelease', defaultValue: 0.050, minValue: 0.010, maxValue: 0.5 }
    ];
  }

  createChannelState() {
    return {
      clockPhase: 0.0,
      integrator: 0.0,
      comparatorState: 0.0,
      lpState: 0.0,
      fbHpState: 0.0,
      fbHpPrevIn: 0.0,
      fbLpState: 0.0,
      envelopeState: 0.0,
      hfEnvelope: 0.0
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

    const thetaRecon = (2.0 * Math.PI * reconstructionCutoff) / sampleRate;
    const reconLpCoeff = 1.0 - Math.exp(-thetaRecon);
    
    const thetaHp = (2.0 * Math.PI * 150) / sampleRate;
    const hpAlpha = 1.0 / (1.0 + thetaHp);
    
    const thetaLp = (2.0 * Math.PI * feedbackTone) / sampleRate;
    const fbLpCoeff = 1.0 - Math.exp(-thetaLp);
    
    const lagAlpha = 1.0 / integratorLag;
    
    const envAttCoeff = 1.0 - Math.exp(-1.0 / (envAttack * sampleRate));
    const envRelCoeff = 1.0 - Math.exp(-1.0 / (envRelease * sampleRate));
    
    // HF detection for coupling
    const hfAttCoeff = 1.0 - Math.exp(-1.0 / (0.01 * sampleRate));
    const hfRelCoeff = 1.0 - Math.exp(-1.0 / (0.1 * sampleRate));

    const numInputChannels = input ? input.length : 0;
    const numOutputChannels = output.length;
    const activeChannels = Math.max(numInputChannels, numOutputChannels, 1);

    while (this.channels.length < activeChannels) {
      this.channels.push(this.createChannelState());
    }
    while (this.delayBuffers.length < activeChannels) {
      this.delayBuffers.push(new Float32Array(this.bufferSize));
    }

    const numSamples = output[0].length;
    const diagIntegrator = outputs[1] ? outputs[1][0] : null;
    const diagStepSize = outputs[2] ? outputs[2][0] : null;

    for (let i = 0; i < numSamples; i++) {
      const delayMs = delayTimeParam.length > 1 ? delayTimeParam[i] : delayTimeParam[0];
      const feedbackRaw = feedbackParam.length > 1 ? feedbackParam[i] : feedbackParam[0];
      const mix = mixParam.length > 1 ? mixParam[i] : mixParam[0];
      const bypass = bypassParam.length > 1 ? bypassParam[i] : bypassParam[0];
      const isFreeze = (freezeParam && freezeParam.length > 1) ? freezeParam[i] >= 0.5 : (freezeParam && freezeParam[0] >= 0.5);

      const feedback = isFreeze ? 1.0 : feedbackRaw;

      this.lfoPhase += (2.0 * Math.PI * wobbleRate) / sampleRate;
      if (this.lfoPhase > 2.0 * Math.PI) {
        this.lfoPhase -= 2.0 * Math.PI;
      }      
      const lfoVal = Math.sin(this.lfoPhase);

      for (let c = 0; c < activeChannels; c++) {
        const inputChannel = numInputChannels > c ? input[c] : (numInputChannels > 0 ? input[0] : null);
        const outputChannel = output[c];
        const state = this.channels[c];
        const delayBuf = this.delayBuffers[c];
        
        let inSample = inputChannel ? inputChannel[i] : 0.0;
        if (isFreeze) inSample = 0.0; // Freeze ignores new input

        const isRightChannel = c === 1;
        const channelSpread = isRightChannel ? stereoSpread : 0.0;
        const channelLfo = isRightChannel ? -lfoVal : lfoVal;

        const modDelayMs = delayMs + channelLfo * wobbleDepth;
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
        
        // 1. Highpass Filter (Fixed to remove DC drift)
        const fbHpOut = hpAlpha * (state.fbHpState + delayedSample - state.fbHpPrevIn);
        state.fbHpPrevIn = delayedSample;
        state.fbHpState = fbHpOut;
        
        // 2. Feedback Tone (Lowpass Filter)
        state.fbLpState = state.fbLpState + fbLpCoeff * (fbHpOut - state.fbLpState);

        let encoderInput = 0.0;
        let stepScale = 1.0;

        if (coupledMode) {
          // Drive is inside the feedback loop
          const mixed = inSample + state.fbLpState * feedback;
          
          // Asymmetrical envelope follower for dynamic response
          const absMixed = Math.abs(mixed);
          if (absMixed > state.envelopeState) {
            state.envelopeState += envAttCoeff * (absMixed - state.envelopeState);
          } else {
            state.envelopeState += envRelCoeff * (absMixed - state.envelopeState);
          }
          
          // Tanh soft clipping
          let driven = mixed * inputDrive;
          encoderInput = Math.tanh(driven); // Real tanh sounds better for saturation
          
          // Coupling 1: Tone -> Slope Overload
          // Estimate HF energy by high-passing the feedback signal
          const hfDiff = delayedSample - state.fbLpState; // very rough HF
          const hfEnergy = Math.abs(hfDiff);
          if (hfEnergy > state.hfEnvelope) {
            state.hfEnvelope += hfAttCoeff * (hfEnergy - state.hfEnvelope);
          } else {
            state.hfEnvelope += hfRelCoeff * (hfEnergy - state.hfEnvelope);
          }
          // Scale step size up when HF is present, down when dark
          const hfFactor = 0.5 + 2.0 * state.hfEnvelope; 
          
          // Coupling 2: Feedback -> Step Size
          // Higher feedback slightly reduces step size to cause granular dissolution instead of white noise
          const fbFactor = 1.0 - Math.min(feedback, 1.0) * 0.2; 
          
          // Dynamic response (envelope follower) opens step size on transients
          const envFactor = 1.0 + dynamicResponse * (state.envelopeState * 2.0);
          
          stepScale = hfFactor * fbFactor * envFactor;
        } else {
          // Uncoupled (classic) mode
          let driven = inSample * inputDrive;
          driven = driven / (1.0 + Math.abs(driven));
          encoderInput = driven + state.fbLpState * feedback;
          
          const absIn = Math.abs(inSample * inputDrive);
          if (absIn > state.envelopeState) {
            state.envelopeState += envAttCoeff * (absIn - state.envelopeState);
          } else {
            state.envelopeState += envRelCoeff * (absIn - state.envelopeState);
          }
          
          stepScale = 1.0 + dynamicResponse * (state.envelopeState * 2.0);
        }

        let currentStepSize = baseStepSize * stepScale * (1.0 + channelSpread * 0.2);
        // clamp step size to sane values
        currentStepSize = Math.max(0.0001, Math.min(currentStepSize, 0.5));

        // --- DELTA ENCODER / DECODER CORE ---
        const channelClockJitter = clockJitter * (1.0 + channelSpread * 0.5);
        state.clockPhase += 1.0 + (Math.random() * 2.0 - 1.0) * channelClockJitter + channelLfo * (channelClockJitter * 0.2);
        
        if (state.clockPhase >= 1.0) {
          state.clockPhase -= 1.0;
          
          const bit = (encoderInput >= state.integrator) ? 1.0 : -1.0;
          
          const channelLagAlpha = lagAlpha * (1.0 - channelSpread * 0.15);
          state.comparatorState = state.comparatorState + channelLagAlpha * (bit - state.comparatorState);
          
          state.integrator = state.integrator * integratorLeak + state.comparatorState * currentStepSize;
        }
        
        if (isNaN(state.integrator)) {
          state.integrator = 0.0;
        } else {
          if (state.integrator > 3.0) state.integrator = 3.0;
          else if (state.integrator < -3.0) state.integrator = -3.0;
        }
        
        const channelReconCoeff = reconLpCoeff * (1.0 - channelSpread * 0.2);
        state.lpState = state.lpState + channelReconCoeff * (state.integrator - state.lpState);
        
        delayBuf[this.writePtr] = state.lpState;
        
        if (c === 0) {
          if (diagIntegrator) diagIntegrator[i] = (bypass >= 0.5) ? 0 : state.lpState; 
          if (diagStepSize) diagStepSize[i] = (bypass >= 0.5) ? 0 : currentStepSize * 10.0;
        }
        
        if (outputChannel) {
          if (bypass >= 0.5) {
            outputChannel[i] = inSample;
          } else {
            // Note: in freeze mode, output only the delayed signal
            if (isFreeze) {
               outputChannel[i] = state.lpState * mix;
            } else {
               outputChannel[i] = inSample * (1.0 - mix) + state.lpState * mix;
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
fs.writeFileSync('src/cvsd-worklet-code.ts', workletCode);
