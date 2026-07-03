import React, { useState, useEffect, useRef } from 'react';
import { motion, AnimatePresence } from 'motion/react';
import {
  Sliders,
  Settings,
  HelpCircle,
  Radio,
  Volume2,
  Zap,
  Info,
  RotateCcw,
  Power,
  Play,
  Pause,
  SlidersHorizontal,
  FolderOpen,
  Timer,
  Music,
} from 'lucide-react';
import workletUrl from './cvsd-worklet.ts?worker&url';
import { DEFAULT_PARAMS, CVSDDelayParams } from './types';
import SoundGenerator from './components/SoundGenerator';
import Visualizer from './components/Visualizer';

export const SUBDIVISIONS = {
  '1/16': 0.25,
  '1/3T': 0.3333333333333333,
  '1/8': 0.5,
  '1/8D': 0.75,
  '1/4': 1.0,
  '1/2D': 1.5,
  '1/2': 2.0,
  custom: 1.0,
} as const;

export default function App() {
  const [audioContext, setAudioContext] = useState<AudioContext | null>(null);
  const [isInitialized, setIsInitialized] = useState(false);
  const [isLoading, setIsLoading] = useState(false);
  const [isPlayingSource, setIsPlayingSource] = useState(false);
  const [isBypassed, setIsBypassed] = useState(false);
  const [isCoupledMode, setIsCoupledMode] = useState(true);
  const [isFreeze, setIsFreeze] = useState(false);

  // Params state
  const [params, setParams] = useState<CVSDDelayParams>(DEFAULT_PARAMS);
  const [currentPreset, setCurrentPreset] = useState<string>('default');

  // Tap tempo and sync states
  const [tapTimes, setTapTimes] = useState<number[]>([]);
  const [pulse, setPulse] = useState(false);

  // Web Audio Nodes references
  const inputNodeRef = useRef<GainNode | null>(null);
  const cvsdDelayNodeRef = useRef<AudioWorkletNode | null>(null);
  const dryAnalyserRef = useRef<AnalyserNode | null>(null);
  const wetAnalyserRef = useRef<AnalyserNode | null>(null);
  const integratorAnalyserRef = useRef<AnalyserNode | null>(null);
  const stepSizeAnalyserRef = useRef<AnalyserNode | null>(null);

  // Clean up on unmount
  useEffect(() => {
    return () => {
      if (audioContext) {
        audioContext.close();
      }
    };
  }, [audioContext]);

  // Sync React params with AudioWorklet params whenever they change
  useEffect(() => {
    const node = cvsdDelayNodeRef.current;
    const ctx = audioContext;
    if (!node || !ctx) return;

    // Apply values smoothly
    const setAudioParam = (name: string, value: number) => {
      const p = node.parameters.get(name);
      if (p) {
        p.setValueAtTime(value, ctx.currentTime);
      }
    };

    setAudioParam('delayTime', params.delayTime);
    setAudioParam('feedback', params.feedback);
    setAudioParam('mix', params.mix);
    setAudioParam('wobbleRate', params.wobbleRate);
    setAudioParam('wobbleDepth', params.wobbleDepth);
    setAudioParam('feedbackTone', params.feedbackTone);

    // Character Curve Mapping (3 zones)
    // 0-30%: 1.0 to 1.5 (Clean, slight rounding)
    // 30-70%: 1.5 to 5.0 (Rapid crunch growth)
    // 70-100%: 5.0 to 10.0 (Heavy saturation)
    let computedDrive = 1.0;
    const char = params.character;
    if (char <= 30) {
      // 0 to 30 -> 1.0 to 1.5
      computedDrive = 1.0 + (char / 30.0) * 0.5;
    } else if (char <= 70) {
      // 30 to 70 -> 1.5 to 5.0
      computedDrive = 1.5 + ((char - 30) / 40.0) * 3.5;
    } else {
      // 70 to 100 -> 5.0 to 10.0
      computedDrive = 5.0 + ((char - 70) / 30.0) * 5.0;
    }
    setAudioParam('inputDrive', computedDrive);
    setAudioParam('coupledMode', isCoupledMode ? 1.0 : 0.0);
    setAudioParam('freeze', isFreeze ? 1.0 : 0.0);
    setAudioParam('envAttack', params.envAttack);
    setAudioParam('envRelease', params.envRelease);

    setAudioParam('dynamicResponse', params.dynamicResponse);
    setAudioParam('stereoSpread', params.stereoSpread);

    // Circuit Type & Namespaced Parameters
    setAudioParam('circuitType', params.circuitType === 'discrete' ? 0.0 : 1.0);
    setAudioParam('stepSize', params.discrete.stepSize);
    setAudioParam('minStepSize', params.companded.minStepSize);
    setAudioParam('maxStepSize', params.companded.maxStepSize);
    setAudioParam('syllabicTime', params.companded.syllabicTime);

    setAudioParam('clockJitter', params.clockJitter);
    setAudioParam('integratorLag', params.integratorLag);
    setAudioParam('reconstructionCutoff', params.reconstructionCutoff);
    setAudioParam('integratorLeak', params.integratorLeak);
    setAudioParam('bypass', isBypassed ? 1.0 : 0.0);

    // Multi-tap parameters
    setAudioParam('numTaps', params.numTaps);
    setAudioParam('tapDecay', params.tapDecay ? 1.0 : 0.0);
    setAudioParam('tap1_mult', params.taps[1].multiplier);
    setAudioParam('tap1_mix', params.taps[1].mix);
    setAudioParam('tap2_mult', params.taps[2].multiplier);
    setAudioParam('tap2_mix', params.taps[2].mix);
    setAudioParam('tap3_mult', params.taps[3].multiplier);
    setAudioParam('tap3_mix', params.taps[3].mix);
    setAudioParam('tap4_mult', params.taps[4].multiplier);
    setAudioParam('tap4_mix', params.taps[4].mix);

    // Reverse parameters
    setAudioParam('reverseMode', params.reverseMode ? 1.0 : 0.0);
    setAudioParam('reverseChunkSize', params.reverseChunkSize);
    setAudioParam('reverseFeedback', params.reverseFeedback ? 1.0 : 0.0);

    // Wobble Sync
    setAudioParam('wobbleSync', params.wobbleSync);
  }, [params, isBypassed, isCoupledMode, isFreeze, audioContext]);

  // Initializing Audio System
  const initAudioSystem = async () => {
    if (isInitialized || isLoading) return;
    setIsLoading(true);

    try {
      const ctx = new (window.AudioContext || (window as any).webkitAudioContext)();

      // Load the worklet
      await ctx.audioWorklet.addModule(workletUrl);

      // Create nodes
      const inputNode = ctx.createGain();
      inputNode.gain.setValueAtTime(1.0, ctx.currentTime);

      // Analyser nodes
      const dryAnalyser = ctx.createAnalyser();
      dryAnalyser.fftSize = 1024;

      const wetAnalyser = ctx.createAnalyser();
      wetAnalyser.fftSize = 1024;

      const integratorAnalyser = ctx.createAnalyser();
      integratorAnalyser.fftSize = 1024;

      const stepSizeAnalyser = ctx.createAnalyser();
      stepSizeAnalyser.fftSize = 1024;

      // Create AudioWorkletNode
      // It has 1 input, and 3 outputs:
      // Output 0: Stereo audio output (dry/wet mixed)
      // Output 1: Mono integrator signal (diagnostics)
      // Output 2: Mono step size signal (diagnostics)
      const cvsdDelayNode = new AudioWorkletNode(ctx, 'cvsd-delay-processor', {
        numberOfInputs: 1,
        numberOfOutputs: 3,
        outputChannelCount: [2, 1, 1],
      });

      // Connections
      // Input Source -> Input Gain Node
      // Input Gain Node -> Dry Analyser
      // Input Gain Node -> CVSD Delay Node (Input 0)
      inputNode.connect(dryAnalyser);
      inputNode.connect(cvsdDelayNode, 0, 0);

      // CVSD Delay Node Output 0 (Audio) -> Wet Analyser & Main Destination
      cvsdDelayNode.connect(wetAnalyser, 0, 0);
      cvsdDelayNode.connect(ctx.destination, 0, 0);

      // CVSD Delay Node Output 1 (Integrator) -> Integrator Analyser
      cvsdDelayNode.connect(integratorAnalyser, 1, 0);

      // CVSD Delay Node Output 2 (Step Size) -> Step Size Analyser
      cvsdDelayNode.connect(stepSizeAnalyser, 2, 0);

      // Store references
      inputNodeRef.current = inputNode;
      cvsdDelayNodeRef.current = cvsdDelayNode;
      dryAnalyserRef.current = dryAnalyser;
      wetAnalyserRef.current = wetAnalyser;
      integratorAnalyserRef.current = integratorAnalyser;
      stepSizeAnalyserRef.current = stepSizeAnalyser;

      // Set initial params in worklet
      const setAudioParam = (name: string, value: number) => {
        const p = cvsdDelayNode.parameters.get(name);
        if (p) {
          p.setValueAtTime(value, ctx.currentTime);
        }
      };

      setAudioParam('delayTime', params.delayTime);
      setAudioParam('feedback', params.feedback);
      setAudioParam('mix', params.mix);
      setAudioParam('wobbleRate', params.wobbleRate);
      setAudioParam('wobbleDepth', params.wobbleDepth);
      setAudioParam('feedbackTone', params.feedbackTone);

      // We will rely on the useEffect to sync parameters, but for init:
      setAudioParam('coupledMode', isCoupledMode ? 1.0 : 0.0);
      setAudioParam('freeze', isFreeze ? 1.0 : 0.0);
      setAudioParam('envAttack', params.envAttack);
      setAudioParam('envRelease', params.envRelease);

      setAudioParam('dynamicResponse', params.dynamicResponse);
      setAudioParam('stereoSpread', params.stereoSpread);

      // Circuit Type & Namespaced Parameters
      setAudioParam('circuitType', params.circuitType === 'discrete' ? 0.0 : 1.0);
      setAudioParam('stepSize', params.discrete.stepSize);
      setAudioParam('minStepSize', params.companded.minStepSize);
      setAudioParam('maxStepSize', params.companded.maxStepSize);
      setAudioParam('syllabicTime', params.companded.syllabicTime);

      setAudioParam('clockJitter', params.clockJitter);
      setAudioParam('integratorLag', params.integratorLag);
      setAudioParam('reconstructionCutoff', params.reconstructionCutoff);
      setAudioParam('integratorLeak', params.integratorLeak);

      // Multi-tap parameters
      setAudioParam('numTaps', params.numTaps);
      setAudioParam('tapDecay', params.tapDecay ? 1.0 : 0.0);
      setAudioParam('tap1_mult', params.taps[1].multiplier);
      setAudioParam('tap1_mix', params.taps[1].mix);
      setAudioParam('tap2_mult', params.taps[2].multiplier);
      setAudioParam('tap2_mix', params.taps[2].mix);
      setAudioParam('tap3_mult', params.taps[3].multiplier);
      setAudioParam('tap3_mix', params.taps[3].mix);
      setAudioParam('tap4_mult', params.taps[4].multiplier);
      setAudioParam('tap4_mix', params.taps[4].mix);

      // Reverse parameters
      setAudioParam('reverseMode', params.reverseMode ? 1.0 : 0.0);
      setAudioParam('reverseChunkSize', params.reverseChunkSize);
      setAudioParam('reverseFeedback', params.reverseFeedback ? 1.0 : 0.0);
      setAudioParam('wobbleSync', params.wobbleSync);

      setAudioContext(ctx);
      setIsInitialized(true);
    } catch (err) {
      console.error('Failed to initialize AudioWorklet:', err);
      alert('Seu navegador não suporta WebAudio/AudioWorklets modernos.');
    } finally {
      setIsLoading(false);
    }
  };
  // Preset loading handler
  const loadPreset = (name: string) => {
    setCurrentPreset(name);
    let newParams = { ...DEFAULT_PARAMS };
    switch (name) {
      case 'default':
        newParams = {
          ...DEFAULT_PARAMS,
          circuitType: 'discrete',
          delayTime: 350,
          feedback: 0.6,
          mix: 0.5,
          wobbleRate: 1.5,
          wobbleDepth: 0.5,
          feedbackTone: 4000,
          character: 0,
          dynamicResponse: 0.2,
          stereoSpread: 0.0,
          discrete: { stepSize: 0.02 },
          clockJitter: 0.0,
          integratorLag: 1.0,
        };
        break;
      case 'crunch-overload':
        newParams = {
          ...DEFAULT_PARAMS,
          circuitType: 'discrete',
          delayTime: 280,
          feedback: 0.55,
          mix: 0.6,
          character: 38,
          discrete: { stepSize: 0.005 },
          integratorLag: 12.0,
          clockJitter: 25.0,
          reconstructionCutoff: 2200,
          wobbleRate: 2.5,
          wobbleDepth: 2.0,
          feedbackTone: 3000,
          dynamicResponse: 0.5,
          stereoSpread: 30.0,
        };
        break;
      case 'dark-tape':
        newParams = {
          ...DEFAULT_PARAMS,
          circuitType: 'discrete',
          delayTime: 500,
          feedback: 0.75,
          mix: 0.45,
          character: 12,
          discrete: { stepSize: 0.015 },
          integratorLag: 5.0,
          clockJitter: 15.0,
          reconstructionCutoff: 1200,
          integratorLeak: 0.992,
          wobbleRate: 0.5,
          wobbleDepth: 4.5,
          feedbackTone: 1200,
          dynamicResponse: 0.1,
          stereoSpread: 50.0,
        };
        break;
      case 'one-bit-chaos':
        newParams = {
          ...DEFAULT_PARAMS,
          circuitType: 'discrete',
          delayTime: 180,
          feedback: 0.7,
          mix: 0.65,
          character: 36,
          discrete: { stepSize: 0.05 },
          integratorLag: 1.0,
          clockJitter: 85.0,
          reconstructionCutoff: 4500,
          wobbleRate: 8.0,
          wobbleDepth: 12.0,
          feedbackTone: 8000,
          dynamicResponse: 0.8,
          stereoSpread: 80.0,
        };
        break;
      case 'clean-reference':
        newParams = {
          ...DEFAULT_PARAMS,
          circuitType: 'discrete',
          delayTime: 400,
          feedback: 0.5,
          mix: 0.5,
          character: -12,
          discrete: { stepSize: 0.005 },
          clockJitter: 0.0,
          integratorLag: 1.0,
          reconstructionCutoff: 15000,
          integratorLeak: 0.999,
          wobbleRate: 1.0,
          wobbleDepth: 0.0,
          feedbackTone: 20000,
          dynamicResponse: 1.0,
          stereoSpread: 0.0,
        };
        break;
      case 'vintage-cvsd':
        newParams = {
          ...DEFAULT_PARAMS,
          circuitType: 'companded',
          delayTime: 320,
          feedback: 0.65,
          mix: 0.5,
          character: 15,
          companded: {
            minStepSize: 0.001,
            maxStepSize: 0.15,
            syllabicTime: 12, // fast syllabic adaptation (classic 12ms syllabic time)
          },
          reconstructionCutoff: 3000,
          integratorLeak: 0.99,
          dynamicResponse: 0.0,
          clockJitter: 10.0,
          integratorLag: 1.2,
          wobbleRate: 1.0,
          wobbleDepth: 0.5,
        };
        break;
      case 'reverse-space':
        newParams = {
          ...DEFAULT_PARAMS,
          circuitType: 'discrete',
          delayTime: 400,
          feedback: 0.65,
          mix: 0.6,
          character: 15,
          discrete: { stepSize: 0.015 },
          wobbleRate: 1.2,
          wobbleDepth: 2.5,
          feedbackTone: 3500,
          stereoSpread: 60.0,
          reverseMode: true,
          reverseChunkSize: 450,
          reverseFeedback: false,
        };
        break;
      case 'reverse-granular':
        newParams = {
          ...DEFAULT_PARAMS,
          circuitType: 'discrete',
          delayTime: 250,
          feedback: 0.7,
          mix: 0.65,
          character: 35,
          discrete: { stepSize: 0.03 },
          wobbleRate: 3.5,
          wobbleDepth: 4.0,
          feedbackTone: 5000,
          stereoSpread: 40.0,
          reverseMode: true,
          reverseChunkSize: 85,
          reverseFeedback: true,
        };
        break;
    }
    setParams(newParams);
  };

  // Synchronized pulse indicator
  useEffect(() => {
    const intervalTime = Math.max(50, params.delayTime);
    const interval = setInterval(() => {
      setPulse(true);
      const timeout = setTimeout(() => {
        setPulse(false);
      }, 95); // flash for 95ms
      return () => clearTimeout(timeout);
    }, intervalTime);

    return () => clearInterval(interval);
  }, [params.delayTime]);

  const updateBpmAndDelay = (
    newBpm: number,
    isSynced: boolean,
    subdiv: '1/16' | '1/3T' | '1/8' | '1/8D' | '1/4' | '1/2D' | '1/2' | 'custom',
  ) => {
    setParams((prev) => {
      const updated = { ...prev, bpm: newBpm, bpmSync: isSynced, mainSubdivision: subdiv };

      if (isSynced && subdiv !== 'custom') {
        const mult = SUBDIVISIONS[subdiv] || 1.0;
        const calculatedDelay = Math.round((60000 / newBpm) * mult);
        updated.delayTime = Math.max(10, Math.min(2000, calculatedDelay));

        // Update relative multipliers of taps to maintain their specific subdivisions
        const newTaps = { ...prev.taps };
        for (let t = 1; t <= 4; t++) {
          const tapId = t as 1 | 2 | 3 | 4;
          const tapSubdiv = prev.taps[tapId].subdivision;
          if (tapSubdiv && tapSubdiv !== 'custom') {
            const tapMult = SUBDIVISIONS[tapSubdiv] / mult;
            newTaps[tapId] = {
              ...prev.taps[tapId],
              multiplier: Math.max(0.1, Math.min(2.0, parseFloat(tapMult.toFixed(2)))),
            };
          }
        }
        updated.taps = newTaps;
      }

      return updated;
    });
  };

  const handleTapTempo = () => {
    const now = performance.now();
    setTapTimes((prev) => {
      // If last tap was more than 2.5 seconds ago, reset the chain
      if (prev.length > 0 && now - prev[prev.length - 1] > 2500) {
        return [now];
      }
      const newTimes = [...prev, now];
      const sliced = newTimes.slice(-4);

      if (sliced.length >= 2) {
        const intervals: number[] = [];
        for (let i = 1; i < sliced.length; i++) {
          intervals.push(sliced[i] - sliced[i - 1]);
        }
        const avgInterval = intervals.reduce((sum, val) => sum + val, 0) / intervals.length;
        const calculatedBpm = Math.round(60000 / avgInterval);

        if (calculatedBpm >= 30 && calculatedBpm <= 300) {
          updateBpmAndDelay(calculatedBpm, true, params.mainSubdivision);
        }
      }
      return sliced;
    });
  };

  const handleTapSubdivisionChange = (
    tapId: 1 | 2 | 3 | 4,
    subdiv: '1/16' | '1/3T' | '1/8' | '1/8D' | '1/4' | '1/2D' | '1/2' | 'custom',
  ) => {
    setCurrentPreset('custom');
    setParams((prev) => {
      const updatedTaps = { ...prev.taps };
      const currentTap = updatedTaps[tapId];

      let newMultiplier = currentTap.multiplier;
      if (subdiv !== 'custom') {
        const mainMult = SUBDIVISIONS[prev.mainSubdivision] || 1.0;
        const tapMult = SUBDIVISIONS[subdiv] || 1.0;
        newMultiplier = Math.max(0.1, Math.min(2.0, parseFloat((tapMult / mainMult).toFixed(2))));
      }

      updatedTaps[tapId] = {
        ...currentTap,
        subdivision: subdiv,
        multiplier: newMultiplier,
      };

      return {
        ...prev,
        taps: updatedTaps,
      };
    });
  };

  const handleParamChange = (key: keyof CVSDDelayParams, value: any) => {
    setCurrentPreset('custom');
    if (key === 'delayTime') {
      setParams((prev) => ({
        ...prev,
        delayTime: value,
        mainSubdivision: 'custom',
        bpmSync: false,
      }));
    } else {
      setParams((prev) => ({
        ...prev,
        [key]: value,
      }));
    }
  };

  const handleNestedParamChange = (mode: 'discrete' | 'companded', key: string, value: number) => {
    setCurrentPreset('custom');
    setParams((prev) => ({
      ...prev,
      [mode]: {
        ...prev[mode],
        [key]: value,
      },
    }));
  };

  const handleTapParamChange = (
    tapId: 1 | 2 | 3 | 4,
    field: 'multiplier' | 'mix',
    value: number,
  ) => {
    setCurrentPreset('custom');
    setParams((prev) => {
      const updatedTaps = { ...prev.taps };
      if (field === 'multiplier') {
        updatedTaps[tapId] = {
          ...prev.taps[tapId],
          multiplier: value,
          subdivision: 'custom',
        };
      } else {
        updatedTaps[tapId] = {
          ...prev.taps[tapId],
          mix: value,
        };
      }
      return {
        ...prev,
        taps: updatedTaps,
      };
    });
  };

  const handleReset = () => {
    loadPreset('default');
  };

  return (
    <div className="min-h-screen bg-bg-dark text-zinc-100 flex flex-col font-sans select-none">
      {/* Top Banner / Header */}
      <header className="bg-panel border-b border-border-dark py-4 px-6 shrink-0 flex items-center justify-between">
        <div className="flex items-center gap-3">
          <div className="w-10 h-10 bg-retro rounded-lg flex items-center justify-center shadow-lg shadow-retro/10">
            <svg
              className="w-6 h-6 text-black"
              viewBox="0 0 24 24"
              fill="none"
              stroke="currentColor"
              strokeWidth="2"
            >
              <line
                x1="4"
                y1="4"
                x2="4"
                y2="20"
                stroke="currentColor"
                strokeWidth="1.5"
                opacity="0.4"
              />
              <line
                x1="7"
                y1="4"
                x2="7"
                y2="20"
                stroke="currentColor"
                strokeWidth="1.5"
                opacity="0.4"
              />
              <line
                x1="10"
                y1="4"
                x2="10"
                y2="20"
                stroke="currentColor"
                strokeWidth="1.5"
                opacity="0.4"
              />
              <line
                x1="13"
                y1="4"
                x2="13"
                y2="20"
                stroke="currentColor"
                strokeWidth="1.5"
                opacity="0.4"
              />
              <line
                x1="16"
                y1="4"
                x2="16"
                y2="20"
                stroke="currentColor"
                strokeWidth="1.5"
                opacity="0.4"
              />
              <line
                x1="19"
                y1="4"
                x2="19"
                y2="20"
                stroke="currentColor"
                strokeWidth="1.5"
                opacity="0.4"
              />
              <line
                x1="22"
                y1="4"
                x2="22"
                y2="20"
                stroke="currentColor"
                strokeWidth="1.5"
                opacity="0.4"
              />
              <path
                d="M2 12 Q 7 3, 12 12 T 22 12"
                stroke="currentColor"
                strokeWidth="2.5"
                strokeLinecap="round"
                strokeLinejoin="round"
              />
            </svg>
          </div>
          <div>
            <h1 className="font-sans font-bold text-base tracking-wide text-zinc-100 uppercase">
              Bitlay{' '}
              <span className="text-retro text-xs font-mono lowercase bg-retro/10 px-2 py-0.5 rounded-full border border-retro/20 ml-2 font-bold">
                v1.1 (CVSD Engine)
              </span>
            </h1>
            <p className="text-xs text-zinc-400">
              Delay digital de 1-bit via modulação delta adaptativa com tap tempo e reverse.
            </p>
          </div>
        </div>
        {isInitialized ? (
          <div className="flex items-center gap-3 bg-bg-dark px-4 py-2 rounded-lg border border-border-dark">
            <span className="w-2.5 h-2.5 bg-emerald-500 rounded-full animate-pulse shrink-0"></span>
            <span className="text-xs font-mono text-zinc-300 uppercase tracking-widest">
              Motor DSP Ativo ({audioContext?.sampleRate}Hz)
            </span>
          </div>
        ) : (
          <div className="flex items-center gap-3 bg-bg-dark px-4 py-2 rounded-lg border border-border-dark">
            <span className="w-2.5 h-2.5 bg-red-500 rounded-full shrink-0"></span>
            <span className="text-xs font-mono text-zinc-500 uppercase tracking-widest">
              Aguardando Inicialização
            </span>
          </div>
        )}
      </header>

      {/* Main workspace */}
      <main className="flex-1 overflow-auto p-6 max-w-7xl w-full mx-auto flex flex-col gap-6">
        <AnimatePresence mode="wait">
          {!isInitialized ? (
            // Landing screen to bypass autoplay blockages
            <motion.div
              key="landing"
              initial={{ opacity: 0, y: 15 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -15 }}
              className="flex-1 flex flex-col items-center justify-center text-center max-w-sm mx-auto my-16 p-10 bg-panel/40 border border-border-dark/60 rounded-2xl shadow-2xl relative overflow-hidden"
            >
              {/* Glowing decorative background aura */}
              <div className="absolute -top-24 -left-24 w-48 h-48 bg-retro/5 rounded-full blur-3xl pointer-events-none"></div>
              <div className="absolute -bottom-24 -right-24 w-48 h-48 bg-retro/5 rounded-full blur-3xl pointer-events-none"></div>

              {/* Custom barcode + reconstructed wave visual SVG */}
              <div className="relative mb-10 drop-shadow-[0_0_20px_rgba(212,255,0,0.25)] animate-pulse">
                <svg
                  className="w-24 h-24 text-retro"
                  viewBox="0 0 24 24"
                  fill="none"
                  stroke="currentColor"
                  strokeWidth="2"
                >
                  <line
                    x1="3"
                    y1="3"
                    x2="3"
                    y2="21"
                    stroke="currentColor"
                    strokeWidth="1.2"
                    opacity="0.3"
                  />
                  <line
                    x1="6"
                    y1="3"
                    x2="6"
                    y2="21"
                    stroke="currentColor"
                    strokeWidth="1.2"
                    opacity="0.3"
                  />
                  <line
                    x1="9"
                    y1="3"
                    x2="9"
                    y2="21"
                    stroke="currentColor"
                    strokeWidth="1.2"
                    opacity="0.3"
                  />
                  <line
                    x1="12"
                    y1="3"
                    x2="12"
                    y2="21"
                    stroke="currentColor"
                    strokeWidth="1.2"
                    opacity="0.3"
                  />
                  <line
                    x1="15"
                    y1="3"
                    x2="15"
                    y2="21"
                    stroke="currentColor"
                    strokeWidth="1.2"
                    opacity="0.3"
                  />
                  <line
                    x1="18"
                    y1="3"
                    x2="18"
                    y2="21"
                    stroke="currentColor"
                    strokeWidth="1.2"
                    opacity="0.3"
                  />
                  <line
                    x1="21"
                    y1="3"
                    x2="21"
                    y2="21"
                    stroke="currentColor"
                    strokeWidth="1.2"
                    opacity="0.3"
                  />
                  <path
                    d="M1 12 C 5 2, 9 22, 12 12 C 15 2, 19 22, 23 12"
                    stroke="currentColor"
                    strokeWidth="2.5"
                    strokeLinecap="round"
                    strokeLinejoin="round"
                  />
                </svg>
              </div>

              <button
                id="btn-init-audio"
                disabled={isLoading}
                onClick={initAudioSystem}
                className="w-full py-5 px-8 bg-retro hover:bg-retro/90 text-black font-black uppercase tracking-widest rounded-xl transition-all duration-200 transform hover:scale-[1.03] active:scale-[0.97] shadow-xl shadow-retro/10 hover:shadow-retro/25 disabled:opacity-50 disabled:cursor-not-allowed cursor-pointer text-xs"
              >
                {isLoading ? 'Iniciando Motor...' : 'Ativar Motor de Áudio'}
              </button>
            </motion.div>
          ) : (
            // Core Workspace Grid
            <motion.div
              key="workspace"
              initial={{ opacity: 0 }}
              animate={{ opacity: 1 }}
              className="grid grid-cols-1 lg:grid-cols-12 gap-6 items-stretch"
            >
              {/* Left Column - DSP Rack Controls (lg:col-span-7) */}
              <div className="lg:col-span-7 flex flex-col gap-6">
                {/* Vintage Rack Panel Faceplate */}
                <div
                  id="rack-faceplate"
                  className="bg-panel border border-border-dark rounded-xl p-6 shadow-xl relative overflow-hidden flex flex-col gap-6"
                >
                  {/* Rack design handle highlights */}
                  <div className="absolute top-0 left-0 w-1.5 h-full bg-retro/40"></div>
                  <div className="absolute top-0 right-0 w-1.5 h-full bg-retro/40"></div>
                  <div className="flex items-center justify-between border-b border-border-dark pb-4">
                    <div className="flex items-center gap-2">
                      <Sliders className="w-5 h-5 text-retro" />
                      <h2 className="font-sans font-semibold text-sm tracking-wide text-zinc-100 uppercase">
                        CONTROLES
                      </h2>
                    </div>

                    <div className="flex items-center gap-3">
                      <button
                        id="btn-freeze"
                        onClick={() => setIsFreeze(!isFreeze)}
                        className={`flex items-center gap-1.5 px-3 py-1.5 text-xs font-mono rounded border transition-all cursor-pointer ${
                          isFreeze
                            ? 'bg-blue-500/20 text-blue-400 border-blue-500/50'
                            : 'bg-bg-dark hover:bg-border-dim text-zinc-300 border-border-dark hover:border-retro/40'
                        }`}
                        title="Freeze the audio buffer (Infinite loop)"
                      >
                        {isFreeze ? 'Unfreeze' : 'Freeze'}
                      </button>
                      <button
                        id="btn-couple"
                        onClick={() => setIsCoupledMode(!isCoupledMode)}
                        className={`flex items-center gap-1.5 px-3 py-1.5 text-xs font-mono rounded border transition-all cursor-pointer ${
                          isCoupledMode
                            ? 'bg-retro/20 text-retro border-retro/50'
                            : 'bg-bg-dark hover:bg-border-dim text-zinc-300 border-border-dark hover:border-retro/40'
                        }`}
                        title="Ativar/Desativar Acoplamentos Musicais"
                      >
                        {isCoupledMode ? 'Acoplamentos: ON' : 'Acoplamentos: OFF'}
                      </button>
                      <button
                        id="btn-bypass"
                        onClick={() => setIsBypassed(!isBypassed)}
                        className={`flex items-center gap-1.5 px-3 py-1.5 text-xs font-mono rounded border transition-all cursor-pointer ${
                          isBypassed
                            ? 'bg-red-500/20 text-red-400 border-red-500/50'
                            : 'bg-bg-dark hover:bg-border-dim text-zinc-300 border-border-dark hover:border-retro/40'
                        }`}
                        title="Ativar/Desativar Bypass (Sinal Seco)"
                      >
                        <Power className="w-3.5 h-3.5" />
                        {isBypassed ? 'Bypassed' : 'Bypass'}
                      </button>
                      <button
                        id="btn-reset-params"
                        onClick={handleReset}
                        className="flex items-center gap-1.5 px-2.5 py-1.5 bg-bg-dark hover:bg-border-dim text-zinc-300 text-xs font-mono rounded border border-border-dark hover:border-retro/40 transition-all cursor-pointer"
                        title="Redefinir para os parâmetros padrão"
                      >
                        <RotateCcw className="w-3.5 h-3.5" />
                        Resetar
                      </button>
                    </div>
                  </div>
                  {/* Group 1: Standard Delay Params */}
                  <div className="flex flex-col gap-4">
                    <div className="flex items-center gap-1.5">
                      <span className="text-retro text-xs font-mono font-semibold">[01]</span>
                      <h3 className="text-xs font-bold font-sans uppercase tracking-wider text-zinc-400">
                        Linha de Atraso (Delay)
                      </h3>
                    </div>

                    <div className="grid grid-cols-1 md:grid-cols-3 gap-5 bg-bg-dark p-4 rounded-lg border border-border-dark/60">
                      {/* Delay Time */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span className="text-zinc-500 uppercase text-[10px] tracking-wide">
                            Tempo (ms)
                          </span>
                          <span className="text-retro font-bold">{params.delayTime} ms</span>
                        </div>
                        <input
                          id="slider-delayTime"
                          type="range"
                          min="10"
                          max="2000"
                          step="1"
                          value={params.delayTime}
                          onChange={(e) =>
                            handleParamChange('delayTime', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>10ms</span>
                          <span>2000ms</span>
                        </div>
                      </div>
                      {/* Feedback */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span className="text-zinc-500 uppercase text-[10px] tracking-wide">
                            Feedback
                          </span>
                          <span className="text-retro font-bold">
                            {Math.round(params.feedback * 100)}%
                          </span>
                        </div>
                        <input
                          id="slider-feedback"
                          type="range"
                          min="0.0"
                          max="1.0"
                          step="0.01"
                          value={params.feedback}
                          onChange={(e) =>
                            handleParamChange('feedback', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>0%</span>
                          <span>100%</span>
                        </div>
                      </div>
                      {/* Mix */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span className="text-zinc-500 uppercase text-[10px] tracking-wide">
                            Mix (Dry/Wet)
                          </span>
                          <span className="text-retro font-bold">
                            {Math.round(params.mix * 100)}%
                          </span>
                        </div>
                        <input
                          id="slider-mix"
                          type="range"
                          min="0.0"
                          max="1.0"
                          step="0.01"
                          value={params.mix}
                          onChange={(e) => handleParamChange('mix', parseFloat(e.target.value))}
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>Dry (0%)</span>
                          <span>Wet (100%)</span>
                        </div>
                      </div>
                    </div>
                  </div>

                  {/* Group 1B: Tap Tempo & Rhythmic Sync */}
                  <div className="flex flex-col gap-4">
                    <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 border-b border-border-dark/60 pb-3">
                      <div className="flex items-center gap-1.5">
                        <span className="text-retro text-xs font-mono font-semibold">[01B]</span>
                        <h3 className="text-xs font-bold font-sans uppercase tracking-wider text-zinc-400">
                          Sincronização Rítmica & Tap Tempo
                        </h3>
                      </div>

                      {/* Main Sync Toggle */}
                      <button
                        id="btn-bpmSync"
                        onClick={() =>
                          updateBpmAndDelay(params.bpm, !params.bpmSync, params.mainSubdivision)
                        }
                        className={`flex items-center gap-1.5 px-3 py-1.5 text-xs font-mono rounded border transition-all cursor-pointer ${
                          params.bpmSync
                            ? 'bg-retro/20 text-retro border-retro/50 font-bold'
                            : 'bg-bg-dark hover:bg-border-dim text-zinc-300 border-border-dark hover:border-retro/40'
                        }`}
                      >
                        {params.bpmSync ? 'BPM Sync: ATIVO' : 'BPM Sync: DESATIVADO'}
                      </button>
                    </div>

                    <div className="grid grid-cols-1 md:grid-cols-12 gap-5 bg-bg-dark p-5 rounded-lg border border-border-dark/60">
                      {/* Interactive Stompbox Tap Tempo Trigger (4 cols) */}
                      <div className="md:col-span-4 flex flex-col gap-3 justify-between bg-panel/30 p-4 rounded-lg border border-border-dark/40">
                        <div className="flex items-center justify-between">
                          <span className="text-zinc-400 uppercase text-[9px] font-mono tracking-wider">
                            TAP TEMPO
                          </span>
                          {/* Flashing Pulse LED Indicator */}
                          <div className="flex items-center gap-1.5">
                            <span className="text-[8px] font-mono text-zinc-500 uppercase">
                              LED Sync:
                            </span>
                            <span
                              className={`w-3.5 h-3.5 rounded-full transition-all duration-75 ${
                                pulse
                                  ? 'bg-retro shadow-[0_0_12px_rgba(212,255,0,0.8)] border border-retro'
                                  : 'bg-zinc-800 border border-zinc-700'
                              }`}
                            ></span>
                          </div>
                        </div>

                        {/* Stomp Button */}
                        <button
                          id="btn-tapTempo-stomp"
                          onClick={handleTapTempo}
                          className="w-full py-6 px-4 bg-zinc-900 border-2 border-zinc-700 active:border-retro hover:border-zinc-500 text-zinc-300 font-mono text-center font-bold text-sm tracking-wider uppercase rounded-xl transition-all duration-75 shadow-md active:bg-zinc-800 hover:text-white cursor-pointer select-none active:scale-95"
                        >
                          TAP TEMPO
                        </button>

                        {/* Status detail */}
                        <div className="flex items-center justify-between text-[10px] font-mono border-t border-border-dark/30 pt-2">
                          <span className="text-zinc-500">Detector:</span>
                          <span className="text-zinc-300 font-semibold">
                            {tapTimes.length >= 2
                              ? `${tapTimes.length} Taps`
                              : 'Aguardando taps...'}
                          </span>
                        </div>
                      </div>

                      {/* Subdivision Selector & BPM Dial (8 cols) */}
                      <div className="md:col-span-8 flex flex-col gap-4">
                        {/* BPM control dial & status */}
                        <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 bg-panel/20 p-3 rounded border border-border-dark/30">
                          <div className="flex flex-col gap-0.5">
                            <span className="text-zinc-400 text-xs font-bold uppercase font-sans tracking-wide">
                              Tempo do Delay
                            </span>
                            <span className="text-[10px] text-zinc-500 font-mono">
                              BPM sincronizado diretamente na linha do tempo
                            </span>
                          </div>

                          {/* Interactive BPM Input Slider */}
                          <div className="flex items-center gap-3">
                            <div className="flex flex-col items-end">
                              <span className="text-retro font-mono font-black text-xl tracking-wide">
                                {params.bpm} BPM
                              </span>
                              <span className="text-[8px] text-zinc-500 font-mono uppercase">
                                Calculado / Ajustado
                              </span>
                            </div>
                            <input
                              id="slider-bpm"
                              type="range"
                              min="30"
                              max="300"
                              step="1"
                              value={params.bpm}
                              onChange={(e) =>
                                updateBpmAndDelay(
                                  parseInt(e.target.value),
                                  params.bpmSync,
                                  params.mainSubdivision,
                                )
                              }
                              className="accent-retro bg-border-dark w-24 h-1 rounded appearance-none cursor-pointer"
                            />
                          </div>
                        </div>

                        {/* Subdivision select grid */}
                        <div className="flex flex-col gap-1.5">
                          <span className="text-zinc-500 uppercase text-[9px] font-mono tracking-wider">
                            Subdivisão Principal (delayTime)
                          </span>
                          <div className="grid grid-cols-4 sm:grid-cols-7 gap-1 bg-panel/30 p-1.5 rounded border border-border-dark/40">
                            {(['1/16', '1/3T', '1/8', '1/8D', '1/4', '1/2D', '1/2'] as const).map(
                              (sub) => {
                                const isActive = params.mainSubdivision === sub;
                                return (
                                  <button
                                    key={sub}
                                    id={`btn-mainSub-${sub}`}
                                    onClick={() => updateBpmAndDelay(params.bpm, true, sub)}
                                    className={`py-2 px-1.5 text-xs font-mono rounded border text-center transition-all cursor-pointer ${
                                      isActive && params.bpmSync
                                        ? 'border-retro bg-retro/15 text-retro font-bold'
                                        : 'border-transparent text-zinc-400 hover:text-zinc-200 hover:bg-border-dim/50'
                                    }`}
                                  >
                                    {sub}
                                  </button>
                                );
                              },
                            )}
                          </div>
                        </div>

                        {/* Wobble Sync Slider */}
                        <div className="flex flex-col gap-2 bg-panel/20 p-3 rounded-lg border border-border-dark/30">
                          <div className="flex items-center justify-between text-xs font-mono">
                            <span className="text-zinc-400 uppercase text-[9px] tracking-wide">
                              Wobble Sync (Ajuste de Modulação Proporcional)
                            </span>
                            <span className="text-retro font-bold">
                              {Math.round(params.wobbleSync * 100)}%
                            </span>
                          </div>
                          <input
                            id="slider-wobbleSync"
                            type="range"
                            min="0.0"
                            max="1.0"
                            step="0.01"
                            value={params.wobbleSync}
                            onChange={(e) =>
                              handleParamChange('wobbleSync', parseFloat(e.target.value))
                            }
                            className="w-full accent-retro bg-border-dark h-1 rounded appearance-none cursor-pointer"
                          />
                          <div className="flex justify-between text-[8px] font-mono text-zinc-500 leading-relaxed">
                            <span>0% (Velocidade Estática)</span>
                            <span className="text-center italic text-retro">
                              {params.wobbleSync === 0
                                ? 'Modulação independente'
                                : params.wobbleSync < 0.4
                                  ? 'Leve aceleração em tempos rápidos'
                                  : 'Aceleração totalmente proporcional'}
                            </span>
                            <span>100% (Sincronizado)</span>
                          </div>
                        </div>
                      </div>
                    </div>
                  </div>

                  {/* Group 5: Multi-Tap & Independent Degradation */}
                  <div className="flex flex-col gap-4">
                    <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 border-b border-border-dark/60 pb-3">
                      <div className="flex items-center gap-1.5">
                        <span className="text-retro text-xs font-mono font-semibold">[05]</span>
                        <h3 className="text-xs font-bold font-sans uppercase tracking-wider text-zinc-400">
                          Multi-Tap & Degradação Independente
                        </h3>
                      </div>

                      {/* Toggle tapDecay */}
                      <button
                        id="btn-tapDecay"
                        onClick={() => handleParamChange('tapDecay', !params.tapDecay)}
                        className={`flex items-center gap-1.5 px-3 py-1.5 text-xs font-mono rounded border transition-all cursor-pointer ${
                          params.tapDecay
                            ? 'bg-retro/20 text-retro border-retro/50'
                            : 'bg-bg-dark hover:bg-border-dim text-zinc-400 border-border-dark hover:border-retro/40'
                        }`}
                        title="Taps mais distantes sofrem maior degradação cumulativa"
                      >
                        {params.tapDecay
                          ? 'Degradação Dinâmica: ATIVA'
                          : 'Degradação Dinâmica: UNIFORME'}
                      </button>
                    </div>

                    <div className="flex flex-col gap-5 bg-bg-dark p-5 rounded-lg border border-border-dark/60">
                      {/* Selector for numTaps */}
                      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3">
                        <div className="flex flex-col">
                          <span className="text-zinc-400 text-xs font-bold font-sans uppercase tracking-wider">
                            Quantidade de Taps
                          </span>
                          <span className="text-[10px] text-zinc-500 font-mono">
                            Defina o número de repetições em paralelo (2-4)
                          </span>
                        </div>
                        <div className="flex items-center gap-1 bg-panel p-1 rounded-lg border border-border-dark">
                          {[2, 3, 4].map((num) => (
                            <button
                              key={num}
                              id={`btn-taps-${num}`}
                              onClick={() => handleParamChange('numTaps', num)}
                              className={`px-4 py-1.5 text-xs font-mono rounded transition-all cursor-pointer ${
                                params.numTaps === num
                                  ? 'bg-retro text-black font-bold shadow-sm'
                                  : 'text-zinc-400 hover:text-zinc-200 hover:bg-border-dim/50'
                              }`}
                            >
                              {num} Taps
                            </button>
                          ))}
                        </div>
                      </div>

                      {/* Timeline Horizontal dos Taps */}
                      <div className="flex flex-col gap-2 bg-panel/30 p-4 rounded-lg border border-border-dark/40">
                        <span className="text-zinc-500 uppercase text-[9px] tracking-widest font-mono">
                          Linha do Tempo dos Ecos (Timeline)
                        </span>

                        {/* The Timeline Ruler */}
                        <div className="relative h-14 bg-bg-dark/85 rounded-md border border-border-dark/80 flex items-center px-4 overflow-visible mt-2">
                          {/* Grid line markers */}
                          <div className="absolute inset-0 flex justify-between px-4 pointer-events-none opacity-20">
                            {[0, 0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 2.0].map((tick) => (
                              <div
                                key={tick}
                                className="flex flex-col items-center justify-between h-full py-1"
                              >
                                <div className="w-[1px] h-2 bg-retro"></div>
                                <span className="text-[7px] font-mono text-retro">{tick}x</span>
                              </div>
                            ))}
                          </div>

                          {/* Interactive/Visual Tap Markers */}
                          {Array.from({ length: params.numTaps }).map((_, idx) => {
                            const tapId = (idx + 1) as 1 | 2 | 3 | 4;
                            const tap = params.taps[tapId];
                            const percentage = (tap.multiplier / 2.0) * 100; // max multiplier is 2.0
                            const delayMs = Math.round(params.delayTime * tap.multiplier);

                            // Calculate color based on decay
                            const decayFactor = params.tapDecay
                              ? 1.0 + (tap.multiplier - 0.5) * 1.5
                              : 1.0;
                            let badgeColor =
                              'border-emerald-500/50 text-emerald-400 bg-emerald-500/10';
                            let stateLabel = 'Suave';
                            if (decayFactor > 2.2) {
                              badgeColor =
                                'border-red-500/60 text-red-400 bg-red-500/15 animate-pulse';
                              stateLabel = 'Fragmentado';
                            } else if (decayFactor > 1.3) {
                              badgeColor = 'border-amber-500/50 text-amber-400 bg-amber-500/10';
                              stateLabel = 'Degradado';
                            }

                            return (
                              <div
                                key={tapId}
                                className="absolute top-1/2 -translate-y-1/2 -translate-x-1/2 transition-all duration-300 z-10"
                                style={{ left: `${Math.min(95, Math.max(5, percentage))}%` }}
                              >
                                <div
                                  className={`flex flex-col items-center px-2 py-0.5 rounded border text-[9px] font-mono shadow-md min-w-[70px] ${badgeColor}`}
                                >
                                  <div className="font-bold flex items-center gap-1">
                                    <span>T{tapId}</span>
                                    <span>•</span>
                                    <span>{(tap.mix * 100).toFixed(0)}%</span>
                                  </div>
                                  <div className="text-[7px] opacity-80">{delayMs}ms</div>
                                  <div className="text-[7px] font-semibold tracking-tighter uppercase opacity-90">
                                    {stateLabel}
                                  </div>
                                </div>
                              </div>
                            );
                          })}
                        </div>
                      </div>

                      {/* Sliders for each Tap */}
                      <div className="grid grid-cols-1 md:grid-cols-2 gap-4 mt-2">
                        {Array.from({ length: params.numTaps }).map((_, idx) => {
                          const tapId = (idx + 1) as 1 | 2 | 3 | 4;
                          const tap = params.taps[tapId];
                          const delayMs = Math.round(params.delayTime * tap.multiplier);
                          const decayFactor = params.tapDecay
                            ? 1.0 + (tap.multiplier - 0.5) * 1.5
                            : 1.0;

                          return (
                            <div
                              key={tapId}
                              className="flex flex-col gap-3 p-3 bg-panel/20 rounded-lg border border-border-dark/40 hover:border-border-dark transition-all"
                            >
                              <div className="flex items-center justify-between border-b border-border-dark/30 pb-1.5">
                                <span className="text-xs font-mono font-bold text-retro">
                                  TAP {tapId}
                                </span>
                                <div className="flex items-center gap-2">
                                  <span className="text-[9px] font-mono text-zinc-500">
                                    Degradação:
                                  </span>
                                  <span className="text-[10px] font-mono font-bold text-zinc-300">
                                    {decayFactor.toFixed(2)}x
                                  </span>
                                </div>
                              </div>

                              {/* Multiplier Slider */}
                              <div className="flex flex-col gap-1">
                                <div className="flex justify-between text-[10px] font-mono">
                                  <span className="text-zinc-500 uppercase text-[9px]">
                                    Atraso Relativo
                                  </span>
                                  <span className="text-retro font-bold">
                                    {tap.multiplier.toFixed(2)}x ({delayMs} ms)
                                  </span>
                                </div>
                                <input
                                  type="range"
                                  min="0.1"
                                  max="2.0"
                                  step="0.05"
                                  value={tap.multiplier}
                                  onChange={(e) =>
                                    handleTapParamChange(
                                      tapId,
                                      'multiplier',
                                      parseFloat(e.target.value),
                                    )
                                  }
                                  className="w-full accent-retro bg-border-dark h-1 rounded appearance-none cursor-pointer"
                                />
                              </div>

                              {/* Mix Slider */}
                              <div className="flex flex-col gap-1">
                                <div className="flex justify-between text-[10px] font-mono">
                                  <span className="text-zinc-500 uppercase text-[9px]">
                                    Volume / Mix
                                  </span>
                                  <span className="text-retro font-bold">
                                    {Math.round(tap.mix * 100)}%
                                  </span>
                                </div>
                                <input
                                  type="range"
                                  min="0.0"
                                  max="1.0"
                                  step="0.01"
                                  value={tap.mix}
                                  onChange={(e) =>
                                    handleTapParamChange(tapId, 'mix', parseFloat(e.target.value))
                                  }
                                  className="w-full accent-retro bg-border-dark h-1 rounded appearance-none cursor-pointer"
                                />
                              </div>

                              {/* Subdivision Selector for Tap */}
                              <div className="flex flex-col gap-1 mt-1">
                                <span className="text-zinc-500 uppercase text-[9px] font-mono tracking-wider">
                                  Subdivisão Sync
                                </span>
                                <div className="grid grid-cols-4 gap-1 bg-bg-dark/60 p-1 rounded border border-border-dark/60">
                                  {(
                                    [
                                      '1/16',
                                      '1/3T',
                                      '1/8',
                                      '1/8D',
                                      '1/4',
                                      '1/2D',
                                      '1/2',
                                      'custom',
                                    ] as const
                                  ).map((sub) => {
                                    const isActive = tap.subdivision === sub;
                                    return (
                                      <button
                                        key={sub}
                                        id={`btn-tap-${tapId}-sub-${sub}`}
                                        onClick={() => handleTapSubdivisionChange(tapId, sub)}
                                        className={`py-1 px-1 text-[9px] font-mono rounded border text-center cursor-pointer transition-all ${
                                          isActive
                                            ? 'border-retro bg-retro/10 text-retro font-bold'
                                            : 'border-transparent text-zinc-500 hover:text-zinc-300 hover:bg-border-dim/20'
                                        }`}
                                      >
                                        {sub === 'custom' ? 'Custom' : sub}
                                      </button>
                                    );
                                  })}
                                </div>
                              </div>
                            </div>
                          );
                        })}
                      </div>
                    </div>
                  </div>

                  {/* Group 6: Modo Reverse */}
                  <div className="flex flex-col gap-4">
                    <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 border-b border-border-dark/60 pb-3">
                      <div className="flex items-center gap-1.5">
                        <span className="text-retro text-xs font-mono font-semibold">[06]</span>
                        <h3 className="text-xs font-bold font-sans uppercase tracking-wider text-zinc-400">
                          Rebobinamento de Fita & Modo Reverse
                        </h3>
                      </div>

                      {/* Main Reverse Toggle */}
                      <button
                        id="btn-reverseMode"
                        onClick={() => handleParamChange('reverseMode', !params.reverseMode)}
                        className={`flex items-center gap-1.5 px-3 py-1.5 text-xs font-mono rounded border transition-all cursor-pointer ${
                          params.reverseMode
                            ? 'bg-retro/20 text-retro border-retro/50 font-bold'
                            : 'bg-bg-dark hover:bg-border-dim text-zinc-400 border-border-dark hover:border-retro/40'
                        }`}
                      >
                        {params.reverseMode ? 'Modo Reverse: ATIVO' : 'Modo Reverse: DESATIVADO'}
                      </button>
                    </div>

                    <div className="flex flex-col gap-5 bg-bg-dark p-5 rounded-lg border border-border-dark/60">
                      {/* Slider for reverseChunkSize */}
                      <div className="flex flex-col gap-2">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span className="text-zinc-400 uppercase text-[10px] tracking-wide">
                            Tamanho do Segmento (reverseChunkSize)
                          </span>
                          <span className="text-retro font-bold">
                            {params.reverseChunkSize} ms —{' '}
                            <span className="underline">
                              {params.reverseChunkSize < 150
                                ? 'Granular / Glitch'
                                : params.reverseChunkSize < 300
                                  ? 'Pulsante / Rítmico'
                                  : 'Fita Rebobinando'}
                            </span>
                          </span>
                        </div>
                        <input
                          id="slider-reverseChunkSize"
                          type="range"
                          min="50"
                          max="1500"
                          step="10"
                          value={params.reverseChunkSize}
                          onChange={(e) =>
                            handleParamChange('reverseChunkSize', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>50ms (Curto/Glitch)</span>
                          <span>1500ms (Longo/Tape Rewind)</span>
                        </div>
                      </div>

                      {/* Toggle for reverseFeedback */}
                      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 bg-panel/20 p-3 rounded-lg border border-border-dark/40">
                        <div className="flex flex-col">
                          <span className="text-zinc-400 text-xs font-bold font-sans uppercase tracking-wider">
                            Tipo de Realimentação (Feedback)
                          </span>
                          <span className="text-[10px] text-zinc-500 font-mono">
                            Defina se o reverse entra no loop ou afeta apenas o áudio wet
                          </span>
                        </div>
                        <button
                          id="btn-reverseFeedback"
                          onClick={() =>
                            handleParamChange('reverseFeedback', !params.reverseFeedback)
                          }
                          className={`px-4 py-2 text-xs font-mono rounded border transition-all cursor-pointer font-bold ${
                            params.reverseFeedback
                              ? 'bg-amber-500/10 text-amber-400 border-amber-500/40 hover:bg-amber-500/20'
                              : 'bg-emerald-500/10 text-emerald-400 border-emerald-500/40 hover:bg-emerald-500/20'
                          }`}
                          title={
                            params.reverseFeedback
                              ? 'Cada eco é revertido em cima do anterior'
                              : 'Loop forward limpo, com saída final revertida'
                          }
                        >
                          {params.reverseFeedback
                            ? 'LOOP ACUMULATIVO (Reverso do Reverso)'
                            : 'Apenas Wet (Loop Forward Estável)'}
                        </button>
                      </div>

                      <p className="text-[10px] text-zinc-500 font-mono italic leading-relaxed">
                        * Nota: O reverse processa o sinal que já passou pelas imperfeições
                        analógicas do modulador delta de 1-bit, mantendo todo o ruído de quantização
                        e o slope overload íntegros.
                      </p>
                    </div>
                  </div>

                  {/* Group 2: Modulator Core Params */}
                  <div className="flex flex-col gap-4">
                    <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 border-b border-border-dark/60 pb-3">
                      <div className="flex items-center gap-1.5">
                        <span className="text-retro text-xs font-mono font-semibold">[02]</span>
                        <h3 className="text-xs font-bold font-sans uppercase tracking-wider text-zinc-400">
                          Modulador Delta 1-Bit (
                          {params.circuitType === 'discrete' ? 'Discrete' : 'Companded'})
                        </h3>
                      </div>

                      {/* Circuit Type Selector */}
                      <div className="flex items-center gap-1 bg-bg-dark p-1 rounded-lg border border-border-dark">
                        <button
                          id="btn-circuit-discrete"
                          onClick={() => handleParamChange('circuitType', 'discrete')}
                          className={`px-3 py-1 text-xs font-mono rounded transition-all cursor-pointer ${
                            params.circuitType === 'discrete'
                              ? 'bg-retro text-black font-bold'
                              : 'text-zinc-400 hover:text-zinc-200 hover:bg-border-dim/50'
                          }`}
                        >
                          Discrete
                        </button>
                        <button
                          id="btn-circuit-companded"
                          onClick={() => handleParamChange('circuitType', 'companded')}
                          className={`px-3 py-1 text-xs font-mono rounded transition-all cursor-pointer ${
                            params.circuitType === 'companded'
                              ? 'bg-retro text-black font-bold'
                              : 'text-zinc-400 hover:text-zinc-200 hover:bg-border-dim/50'
                          }`}
                        >
                          Companded
                        </button>
                      </div>
                    </div>

                    <div className="grid grid-cols-1 md:grid-cols-2 gap-5 bg-bg-dark p-4 rounded-lg border border-border-dark/60">
                      {/* Conditionally render parameters based on active Circuit Type */}
                      {params.circuitType === 'discrete' ? (
                        <div className="flex flex-col gap-1.5 md:col-span-2">
                          <div className="flex items-center justify-between text-xs font-mono">
                            <span
                              className="text-zinc-500 uppercase text-[10px] tracking-wide"
                              title="Tamanho do passo fixo"
                            >
                              Tamanho do Passo (stepSize)
                            </span>
                            <span className="text-retro font-bold">
                              {params.discrete.stepSize.toFixed(4)}
                            </span>
                          </div>
                          <input
                            id="slider-stepSize"
                            type="range"
                            min="0.001"
                            max="0.2"
                            step="0.001"
                            value={params.discrete.stepSize}
                            onChange={(e) =>
                              handleNestedParamChange(
                                'discrete',
                                'stepSize',
                                parseFloat(e.target.value),
                              )
                            }
                            className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                          />
                          <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                            <span>0.001 (Mais limpo/Lento)</span>
                            <span>0.200 (Ruidoso/Rápido)</span>
                          </div>
                        </div>
                      ) : (
                        <>
                          {/* minStepSize slider */}
                          <div className="flex flex-col gap-1.5">
                            <div className="flex items-center justify-between text-xs font-mono">
                              <span
                                className="text-zinc-500 uppercase text-[10px] tracking-wide"
                                title="Passo mínimo quando não há sobrecarga (sinal baixo)"
                              >
                                Passo Mínimo (minStep)
                              </span>
                              <span className="text-retro font-bold">
                                {params.companded.minStepSize.toFixed(4)}
                              </span>
                            </div>
                            <input
                              id="slider-minStepSize"
                              type="range"
                              min="0.001"
                              max="0.2"
                              step="0.001"
                              value={params.companded.minStepSize}
                              onChange={(e) =>
                                handleNestedParamChange(
                                  'companded',
                                  'minStepSize',
                                  parseFloat(e.target.value),
                                )
                              }
                              className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                            />
                            <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                              <span>0.001 (Limpo)</span>
                              <span>0.200 (Ruidoso)</span>
                            </div>
                          </div>

                          {/* maxStepSize slider */}
                          <div className="flex flex-col gap-1.5">
                            <div className="flex items-center justify-between text-xs font-mono">
                              <span
                                className="text-zinc-500 uppercase text-[10px] tracking-wide"
                                title="Passo máximo sob sobrecarga (sinais fortes)"
                              >
                                Passo Máximo (maxStep)
                              </span>
                              <span className="text-retro font-bold">
                                {params.companded.maxStepSize.toFixed(4)}
                              </span>
                            </div>
                            <input
                              id="slider-maxStepSize"
                              type="range"
                              min="0.01"
                              max="0.5"
                              step="0.001"
                              value={params.companded.maxStepSize}
                              onChange={(e) =>
                                handleNestedParamChange(
                                  'companded',
                                  'maxStepSize',
                                  parseFloat(e.target.value),
                                )
                              }
                              className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                            />
                            <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                              <span>0.010 (Suave)</span>
                              <span>0.500 (Agressivo)</span>
                            </div>
                          </div>

                          {/* syllabicTime slider */}
                          <div className="flex flex-col gap-1.5 md:col-span-2">
                            <div className="flex items-center justify-between text-xs font-mono">
                              <span
                                className="text-zinc-500 uppercase text-[10px] tracking-wide"
                                title="Constante de tempo de integração do detector de sobrecarga"
                              >
                                Tempo Silábico (syllabicTime)
                              </span>
                              <span className="text-retro font-bold">
                                {params.companded.syllabicTime} ms
                              </span>
                            </div>
                            <input
                              id="slider-syllabicTime"
                              type="range"
                              min="5"
                              max="100"
                              step="1"
                              value={params.companded.syllabicTime}
                              onChange={(e) =>
                                handleNestedParamChange(
                                  'companded',
                                  'syllabicTime',
                                  parseFloat(e.target.value),
                                )
                              }
                              className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                            />
                            <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                              <span>5 ms (Rápido)</span>
                              <span>100 ms (Suave/Fita)</span>
                            </div>
                          </div>
                        </>
                      )}

                      {/* integratorLag */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span
                            className="text-zinc-500 uppercase text-[10px] tracking-wide"
                            title="Simula a inércia do OTA (slew rate) no integrador em amostras"
                          >
                            Inércia do OTA (Lag)
                          </span>
                          <span className="text-retro font-bold">
                            {params.integratorLag.toFixed(1)}
                          </span>
                        </div>
                        <input
                          id="slider-integratorLag"
                          type="range"
                          min="1.0"
                          max="50.0"
                          step="0.1"
                          value={params.integratorLag}
                          onChange={(e) =>
                            handleParamChange('integratorLag', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>1.0 (Ideal)</span>
                          <span>50.0 (Lento/Overload)</span>
                        </div>
                      </div>

                      {/* clockJitter */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span
                            className="text-zinc-500 uppercase text-[10px] tracking-wide"
                            title="Ruído na taxa de amostragem interna"
                          >
                            Clock Jitter (%)
                          </span>
                          <span className="text-retro font-bold">
                            {params.clockJitter.toFixed(1)}%
                          </span>
                        </div>
                        <input
                          id="slider-clockJitter"
                          type="range"
                          min="0.0"
                          max="100.0"
                          step="1.0"
                          value={params.clockJitter}
                          onChange={(e) =>
                            handleParamChange('clockJitter', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>0% (Cristal)</span>
                          <span>100% (Osc. RC Instável)</span>
                        </div>
                      </div>

                      {/* integratorLeak */}
                      <div className="flex flex-col gap-1.5 md:col-span-2">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span
                            className="text-zinc-500 uppercase text-[10px] tracking-wide"
                            title="Fator de vazamento do integrador para evitar offset DC"
                          >
                            Vazamento do Integrador
                          </span>
                          <span className="text-retro font-bold">
                            {(params.integratorLeak * 100).toFixed(1)}%
                          </span>
                        </div>
                        <input
                          id="slider-integratorLeak"
                          type="range"
                          min="0.90"
                          max="1.0"
                          step="0.001"
                          value={params.integratorLeak}
                          onChange={(e) =>
                            handleParamChange('integratorLeak', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>90% (Vaza Rápido)</span>
                          <span>100% (Sem Perda - DC Drift)</span>
                        </div>
                      </div>
                    </div>
                  </div>
                  {/* Group 3: Input & Output */}
                  <div className="flex flex-col gap-4">
                    <div className="flex items-center gap-1.5">
                      <span className="text-retro text-xs font-mono font-semibold">[03]</span>
                      <h3 className="text-xs font-bold font-sans uppercase tracking-wider text-zinc-400">
                        Entrada, Dinâmica & Saída
                      </h3>
                    </div>

                    <div className="grid grid-cols-1 md:grid-cols-2 gap-5 bg-bg-dark p-4 rounded-lg border border-border-dark/60">
                      {/* character */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span
                            className="text-zinc-500 uppercase text-[10px] tracking-wide"
                            title="Saturação do OTA e acoplamentos não-lineares"
                          >
                            Character
                          </span>
                          <span className="text-retro font-bold">
                            {params.character.toFixed(0)}%
                          </span>
                        </div>
                        <input
                          id="slider-character"
                          type="range"
                          min="0"
                          max="100"
                          step="1"
                          value={params.character}
                          onChange={(e) =>
                            handleParamChange('character', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>Limpo (0%)</span>
                          <span>Bitcrush (100%)</span>
                        </div>
                      </div>
                      {/* dynamicResponse */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span
                            className="text-zinc-500 uppercase text-[10px] tracking-wide"
                            title="Resposta dinâmica ao transiente"
                          >
                            Ducking Dinâmico
                          </span>
                          <span className="text-retro font-bold">
                            {params.dynamicResponse.toFixed(2)}
                          </span>
                        </div>
                        <input
                          id="slider-dynamicResponse"
                          type="range"
                          min="0.0"
                          max="1.0"
                          step="0.05"
                          value={params.dynamicResponse}
                          onChange={(e) =>
                            handleParamChange('dynamicResponse', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>0 (Estático)</span>
                          <span>1 (Reativo)</span>
                        </div>
                      </div>
                      {/* reconstructionCutoff */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span
                            className="text-zinc-500 uppercase text-[10px] tracking-wide"
                            title="Filtro de reconstrução de saída para suavizar o ruído de quantização"
                          >
                            Corte do Filtro RC (Hz)
                          </span>
                          <span className="text-retro font-bold">
                            {params.reconstructionCutoff} Hz
                          </span>
                        </div>
                        <input
                          id="slider-reconstructionCutoff"
                          type="range"
                          min="200"
                          max="20000"
                          step="50"
                          value={params.reconstructionCutoff}
                          onChange={(e) =>
                            handleParamChange('reconstructionCutoff', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>200Hz</span>
                          <span>20kHz</span>
                        </div>
                      </div>
                      {/* stereoSpread */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span
                            className="text-zinc-500 uppercase text-[10px] tracking-wide"
                            title="Desvio de circuito entre canais L/R"
                          >
                            Spread Estéreo (%)
                          </span>
                          <span className="text-retro font-bold">
                            {params.stereoSpread.toFixed(1)}%
                          </span>
                        </div>
                        <input
                          id="slider-stereoSpread"
                          type="range"
                          min="0.0"
                          max="100.0"
                          step="1.0"
                          value={params.stereoSpread}
                          onChange={(e) =>
                            handleParamChange('stereoSpread', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>Mono</span>
                          <span>Ping-Pong/Wide</span>
                        </div>
                      </div>
                    </div>
                  </div>
                  {/* Group 4: Vintage Analog Character */}
                  <div className="flex flex-col gap-4">
                    <div className="flex items-center gap-1.5">
                      <span className="text-retro text-xs font-mono font-semibold">[04]</span>
                      <h3 className="text-xs font-bold font-sans uppercase tracking-wider text-zinc-400">
                        Caráter Analógico Vintage (Mod & Feedback)
                      </h3>
                    </div>

                    <div className="grid grid-cols-1 md:grid-cols-2 gap-5 bg-bg-dark p-4 rounded-lg border border-border-dark/60">
                      {/* wobbleDepth */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span
                            className="text-zinc-500 uppercase text-[10px] tracking-wide"
                            title="Profundidade da modulação LFO de tempo e relógio"
                          >
                            Profundidade do Wobble
                          </span>
                          <span className="text-retro font-bold">
                            {params.wobbleDepth.toFixed(1)}
                          </span>
                        </div>
                        <input
                          id="slider-wobbleDepth"
                          type="range"
                          min="0.0"
                          max="50.0"
                          step="0.1"
                          value={params.wobbleDepth}
                          onChange={(e) =>
                            handleParamChange('wobbleDepth', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>0 (Estável)</span>
                          <span>50 (Chorusing)</span>
                        </div>
                      </div>

                      {/* wobbleRate */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span
                            className="text-zinc-500 uppercase text-[10px] tracking-wide"
                            title="Velocidade da modulação LFO"
                          >
                            Velocidade do Wobble
                          </span>
                          <span className="text-retro font-bold">
                            {params.wobbleRate.toFixed(2)} Hz
                          </span>
                        </div>
                        <input
                          id="slider-wobbleRate"
                          type="range"
                          min="0.01"
                          max="20.0"
                          step="0.05"
                          value={params.wobbleRate}
                          onChange={(e) =>
                            handleParamChange('wobbleRate', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>0.01Hz (Wow)</span>
                          <span>20Hz (Flutter)</span>
                        </div>
                      </div>

                      {/* feedbackTone */}
                      <div className="flex flex-col gap-1.5 md:col-span-2">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span
                            className="text-zinc-500 uppercase text-[10px] tracking-wide"
                            title="Corte de agudos no loop de feedback simulando limite de banda"
                          >
                            Tom das Repetições (LPF)
                          </span>
                          <span className="text-retro font-bold">{params.feedbackTone} Hz</span>
                        </div>
                        <input
                          id="slider-feedbackTone"
                          type="range"
                          min="100"
                          max="20000"
                          step="100"
                          value={params.feedbackTone}
                          onChange={(e) =>
                            handleParamChange('feedbackTone', parseFloat(e.target.value))
                          }
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>100Hz (Muito Escuro)</span>
                          <span>20kHz (Fita Nova)</span>
                        </div>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
              {/* Right Column - Sound Gen, Visualizers & Presets (lg:col-span-5) */}
              <div className="lg:col-span-5 flex flex-col gap-6">
                {/* 1. Sound Generator */}
                <SoundGenerator
                  audioContext={audioContext}
                  destinationNode={inputNodeRef.current}
                  isPlayingSource={isPlayingSource}
                  setIsPlayingSource={setIsPlayingSource}
                />

                {/* 2. Visualizer */}
                <div className="flex-1 min-h-[300px]">
                  <Visualizer
                    dryAnalyser={dryAnalyserRef.current}
                    wetAnalyser={wetAnalyserRef.current}
                    integratorAnalyser={integratorAnalyserRef.current}
                    stepSizeAnalyser={stepSizeAnalyserRef.current}
                    isPlaying={isPlayingSource}
                  />
                </div>

                {/* 3. Presets Quick Select */}
                <div
                  id="presets-panel"
                  className="bg-panel border border-border-dark rounded-xl p-5 flex flex-col gap-3 shadow-lg"
                >
                  <div className="flex items-center gap-2 border-b border-border-dark pb-2">
                    <FolderOpen className="w-4 h-4 text-retro" />
                    <h2 className="font-sans font-semibold text-xs tracking-wide text-zinc-100 uppercase">
                      Presets de Calibração
                    </h2>
                  </div>

                  <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-2 xl:grid-cols-3 gap-2">
                    <button
                      id="btn-preset-vintage-cvsd"
                      onClick={() => loadPreset('vintage-cvsd')}
                      className={`py-2 px-2.5 text-xs font-mono rounded border text-left transition-all cursor-pointer ${
                        currentPreset === 'vintage-cvsd'
                          ? 'border-retro bg-retro/5 text-retro'
                          : 'border-border-dark bg-bg-dark/40 text-zinc-400 hover:bg-border-dim hover:text-zinc-300 hover:border-border-dark'
                      }`}
                    >
                      <span className="block font-bold">Vintage CVSD</span>
                      <span className="text-[9px] text-zinc-500 block">Sílaba Adaptativa</span>
                    </button>

                    <button
                      id="btn-preset-default"
                      onClick={() => loadPreset('default')}
                      className={`py-2 px-2.5 text-xs font-mono rounded border text-left transition-all cursor-pointer ${
                        currentPreset === 'default'
                          ? 'border-retro bg-retro/5 text-retro'
                          : 'border-border-dark bg-bg-dark/40 text-zinc-400 hover:bg-border-dim hover:text-zinc-300 hover:border-border-dark'
                      }`}
                    >
                      <span className="block font-bold">Standard Delay</span>
                      <span className="text-[9px] text-zinc-500 block">Equilibrado</span>
                    </button>

                    <button
                      id="btn-preset-crunch"
                      onClick={() => loadPreset('crunch-overload')}
                      className={`py-2 px-2.5 text-xs font-mono rounded border text-left transition-all cursor-pointer ${
                        currentPreset === 'crunch-overload'
                          ? 'border-retro bg-retro/5 text-retro'
                          : 'border-border-dark bg-bg-dark/40 text-zinc-400 hover:bg-border-dim hover:text-zinc-300 hover:border-border-dark'
                      }`}
                    >
                      <span className="block font-bold">Slope Overload</span>
                      <span className="text-[9px] text-zinc-500 block">Distorção Rápida</span>
                    </button>

                    <button
                      id="btn-preset-tape"
                      onClick={() => loadPreset('dark-tape')}
                      className={`py-2 px-2.5 text-xs font-mono rounded border text-left transition-all cursor-pointer ${
                        currentPreset === 'dark-tape'
                          ? 'border-retro bg-retro/5 text-retro'
                          : 'border-border-dark bg-bg-dark/40 text-zinc-400 hover:bg-border-dim hover:text-zinc-300 hover:border-border-dark'
                      }`}
                    >
                      <span className="block font-bold">Dark Tape Echo</span>
                      <span className="text-[9px] text-zinc-500 block">Ecos Quentes</span>
                    </button>

                    <button
                      id="btn-preset-chaos"
                      onClick={() => loadPreset('one-bit-chaos')}
                      className={`py-2 px-2.5 text-xs font-mono rounded border text-left transition-all cursor-pointer ${
                        currentPreset === 'one-bit-chaos'
                          ? 'border-retro bg-retro/5 text-retro'
                          : 'border-border-dark bg-bg-dark/40 text-zinc-400 hover:bg-border-dim hover:text-zinc-300 hover:border-border-dark'
                      }`}
                    >
                      <span className="block font-bold">1-Bit Chaos</span>
                      <span className="text-[9px] text-zinc-500 block">Glitch / Lo-fi</span>
                    </button>

                    <button
                      id="btn-preset-clean"
                      onClick={() => loadPreset('clean-reference')}
                      className={`py-2 px-2.5 text-xs font-mono rounded border text-left transition-all cursor-pointer ${
                        currentPreset === 'clean-reference'
                          ? 'border-retro bg-retro/5 text-retro'
                          : 'border-border-dark bg-bg-dark/40 text-zinc-400 hover:bg-border-dim hover:text-zinc-300 hover:border-border-dark'
                      }`}
                    >
                      <span className="block font-bold">Linear PCM</span>
                      <span className="text-[9px] text-zinc-500 block">Referência Limpa</span>
                    </button>

                    <button
                      id="btn-preset-reverse-space"
                      onClick={() => loadPreset('reverse-space')}
                      className={`py-2 px-2.5 text-xs font-mono rounded border text-left transition-all cursor-pointer ${
                        currentPreset === 'reverse-space'
                          ? 'border-retro bg-retro/5 text-retro'
                          : 'border-border-dark bg-bg-dark/40 text-zinc-400 hover:bg-border-dim hover:text-zinc-300 hover:border-border-dark'
                      }`}
                    >
                      <span className="block font-bold">Reverse Space</span>
                      <span className="text-[9px] text-zinc-500 block">Eco Espacial</span>
                    </button>

                    <button
                      id="btn-preset-reverse-granular"
                      onClick={() => loadPreset('reverse-granular')}
                      className={`py-2 px-2.5 text-xs font-mono rounded border text-left transition-all cursor-pointer ${
                        currentPreset === 'reverse-granular'
                          ? 'border-retro bg-retro/5 text-retro'
                          : 'border-border-dark bg-bg-dark/40 text-zinc-400 hover:bg-border-dim hover:text-zinc-300 hover:border-border-dark'
                      }`}
                    >
                      <span className="block font-bold">Glitch Reverso</span>
                      <span className="text-[9px] text-zinc-500 block">Caos Granular</span>
                    </button>
                  </div>
                </div>
                {/* 4. Educational Details Box */}
                <div
                  id="educational-info"
                  className="bg-panel/40 border border-border-dark/80 rounded-xl p-5 flex flex-col gap-3"
                >
                  <div className="flex items-center gap-2">
                    <Info className="w-4 h-4 text-zinc-500" />
                    <span className="font-sans font-bold text-xs uppercase tracking-wider text-zinc-400">
                      Arquitetura de Delay Primitivo (Delta 1-Bit de Passo Fixo)
                    </span>

                    <p className="text-[11px] text-zinc-400 leading-relaxed font-mono">
                      Este delay emula uma arquitetura digital muito primitiva baseada em
                      componentes discretos (como um integrador OTA CA3080 e shift registers CMOS).
                      Utiliza um <strong>Modulador Delta de Passo Fixo</strong> para codificar o
                      áudio em apenas <strong>1 bit por ciclo</strong>. Ao invés do CVSD adaptativo
                      moderno, o passo fixo causa forte "Slope Overload" (distorção em transientes
                      rápidos) e "Granular Noise" (ruído de quantização no silêncio).
                    </p>
                    <p className="text-[11px] text-zinc-400 leading-relaxed font-mono">
                      <strong>Imperfeições Analógicas:</strong> Incluímos a{' '}
                      <strong>Inércia do OTA (Lag)</strong> que torna o integrador menos ideal,
                      limitando a velocidade de subida e acentuando o overload. Adicionamos{' '}
                      <strong>Clock Jitter</strong> no sample rate interno do modulador para simular
                      a ausência de um cristal preciso. O feedback analógico inclui modulação LFO de
                      tempo, saturação pesada, e filtros RC passa-alta/passa-baixa, recriando as
                      limitações dos primeiros delays digitais rack.
                    </p>
                  </div>
                </div>
              </div>
            </motion.div>
          )}
        </AnimatePresence>
      </main>

      {/* Footer */}
      <footer className="bg-bg-dark border-t border-border-dim py-4 px-6 text-center text-zinc-600 text-[10px] font-mono shrink-0">
        PROTÓTIPO DE PORTABILIDADE DSP C++ — PROJETADO PARA EMBARCADOS ESP32 / RP2350
      </footer>
    </div>
  );
}
