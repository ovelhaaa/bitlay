export interface TapConfig {
  multiplier: number; // 0.1 - 2.0
  mix: number; // 0.0 - 1.0
  subdivision: '1/16' | '1/3T' | '1/8' | '1/8D' | '1/4' | '1/2D' | '1/2' | 'custom';
}

export interface CVSDDelayParams {
  circuitType: 'discrete' | 'companded';
  character: number; // 0.0 - 100.0
  delayTime: number; // ms (10 - 2000)
  feedback: number; // 0.0 - 1.0
  mix: number; // 0.0 - 1.0
  wobbleRate: number; // 0.01 - 20.0
  wobbleDepth: number; // 0.0 - 50.0
  feedbackTone: number; // 100 - 20000 (Hz)
  inputDrive: number; // 0.1 - 10.0
  dynamicResponse: number; // 0.0 - 1.0
  stereoSpread: number; // 0.0 - 100.0 (%)
  clockJitter: number; // 0.0 - 100.0
  integratorLag: number; // 1.0 - 50.0
  reconstructionCutoff: number; // Hz (100 - 20000)
  integratorLeak: number;
  envAttack: number;
  envRelease: number; // 0.8 - 1.0

  // Multi-tap config
  numTaps: number;
  tapDecay: boolean;
  taps: {
    1: TapConfig;
    2: TapConfig;
    3: TapConfig;
    4: TapConfig;
  };

  // Reverse mode config
  reverseMode: boolean;
  reverseChunkSize: number; // in ms, 50 - 1500
  reverseFeedback: boolean; // true = accumulative reverse, false = wet only reverse

  // Tap tempo & Subdivision sync
  bpm: number;
  bpmSync: boolean;
  wobbleSync: number; // 0.0 to 1.0
  mainSubdivision: '1/16' | '1/3T' | '1/8' | '1/8D' | '1/4' | '1/2D' | '1/2' | 'custom';

  discrete: {
    stepSize: number;
  };
  companded: {
    minStepSize: number;
    maxStepSize: number;
    syllabicTime: number;
  };
}

export const DEFAULT_PARAMS: CVSDDelayParams = {
  circuitType: 'discrete',
  character: 25,
  delayTime: 350,
  feedback: 0.6,
  mix: 0.5,
  wobbleRate: 1.5,
  wobbleDepth: 0.0,
  feedbackTone: 4000,
  inputDrive: 1.0,
  dynamicResponse: 0.0,
  stereoSpread: 0.0,
  clockJitter: 0.0,
  integratorLag: 1.0,
  reconstructionCutoff: 3500,
  integratorLeak: 0.995,
  envAttack: 0.005,
  envRelease: 0.05,

  numTaps: 2,
  tapDecay: true,
  taps: {
    1: { multiplier: 0.5, mix: 0.8, subdivision: '1/8' },
    2: { multiplier: 1.0, mix: 0.6, subdivision: '1/4' },
    3: { multiplier: 1.5, mix: 0.4, subdivision: '1/2D' },
    4: { multiplier: 2.0, mix: 0.2, subdivision: '1/2' },
  },

  reverseMode: false,
  reverseChunkSize: 350,
  reverseFeedback: false,

  // Default tap tempo properties
  bpm: 120,
  bpmSync: false,
  wobbleSync: 0.5,
  mainSubdivision: '1/4',

  discrete: {
    stepSize: 0.02,
  },
  companded: {
    minStepSize: 0.002,
    maxStepSize: 0.12,
    syllabicTime: 20,
  },
};
