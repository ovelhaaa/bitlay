import { useState, useEffect, useRef, useCallback } from 'react';
import workletUrl from '../cvsd-worklet.ts?worker&url';
import { CVSDDelayParams } from '../types';

export function useAudioEngine(
  params: CVSDDelayParams,
  isBypassed: boolean,
  isCoupledMode: boolean,
  isFreeze: boolean,
) {
  const [audioContext, setAudioContext] = useState<AudioContext | null>(null);
  const [isInitialized, setIsInitialized] = useState(false);
  const [isLoading, setIsLoading] = useState(false);

  // Web Audio Nodes references
  const inputNodeRef = useRef<GainNode | null>(null);
  const cvsdDelayNodeRef = useRef<AudioWorkletNode | null>(null);
  const dryAnalyserRef = useRef<AnalyserNode | null>(null);
  const wetAnalyserRef = useRef<AnalyserNode | null>(null);
  const integratorAnalyserRef = useRef<AnalyserNode | null>(null);
  const stepSizeAnalyserRef = useRef<AnalyserNode | null>(null);

  useEffect(() => {
    return () => {
      if (audioContext) {
        audioContext.close();
      }
    };
  }, [audioContext]);

  useEffect(() => {
    const node = cvsdDelayNodeRef.current;
    const ctx = audioContext;
    if (!node || !ctx) return;

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

    let computedDrive = 1.0;
    const char = params.character;
    if (char <= 30) {
      computedDrive = 1.0 + (char / 30.0) * 0.5;
    } else if (char <= 70) {
      computedDrive = 1.5 + ((char - 30) / 40.0) * 3.5;
    } else {
      computedDrive = 5.0 + ((char - 70) / 30.0) * 5.0;
    }
    setAudioParam('inputDrive', computedDrive);
    setAudioParam('coupledMode', isCoupledMode ? 1.0 : 0.0);
    setAudioParam('freeze', isFreeze ? 1.0 : 0.0);
    setAudioParam('envAttack', params.envAttack);
    setAudioParam('envRelease', params.envRelease);

    setAudioParam('dynamicResponse', params.dynamicResponse);
    setAudioParam('stereoSpread', params.stereoSpread);

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

    setAudioParam('reverseMode', params.reverseMode ? 1.0 : 0.0);
    setAudioParam('reverseChunkSize', params.reverseChunkSize);
    setAudioParam('reverseFeedback', params.reverseFeedback ? 1.0 : 0.0);
    setAudioParam('wobbleSync', params.wobbleSync);
  }, [params, isBypassed, isCoupledMode, isFreeze, audioContext]);

  const initAudioSystem = useCallback(async () => {
    if (isInitialized || isLoading) return;
    setIsLoading(true);

    try {
      const ctx = new (window.AudioContext || (window as any).webkitAudioContext)();
      await ctx.audioWorklet.addModule(workletUrl);

      const inputNode = ctx.createGain();
      inputNode.gain.setValueAtTime(1.0, ctx.currentTime);

      const dryAnalyser = ctx.createAnalyser();
      dryAnalyser.fftSize = 1024;

      const wetAnalyser = ctx.createAnalyser();
      wetAnalyser.fftSize = 1024;

      const integratorAnalyser = ctx.createAnalyser();
      integratorAnalyser.fftSize = 1024;

      const stepSizeAnalyser = ctx.createAnalyser();
      stepSizeAnalyser.fftSize = 1024;

      const cvsdDelayNode = new AudioWorkletNode(ctx, 'cvsd-delay-processor', {
        numberOfInputs: 1,
        numberOfOutputs: 3,
        outputChannelCount: [2, 1, 1],
      });

      inputNode.connect(dryAnalyser);
      inputNode.connect(cvsdDelayNode, 0, 0);

      cvsdDelayNode.connect(wetAnalyser, 0, 0);
      cvsdDelayNode.connect(ctx.destination, 0, 0);
      cvsdDelayNode.connect(integratorAnalyser, 1, 0);
      cvsdDelayNode.connect(stepSizeAnalyser, 2, 0);

      inputNodeRef.current = inputNode;
      cvsdDelayNodeRef.current = cvsdDelayNode;
      dryAnalyserRef.current = dryAnalyser;
      wetAnalyserRef.current = wetAnalyser;
      integratorAnalyserRef.current = integratorAnalyser;
      stepSizeAnalyserRef.current = stepSizeAnalyser;

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
      setAudioParam('coupledMode', isCoupledMode ? 1.0 : 0.0);
      setAudioParam('freeze', isFreeze ? 1.0 : 0.0);
      setAudioParam('envAttack', params.envAttack);
      setAudioParam('envRelease', params.envRelease);
      setAudioParam('dynamicResponse', params.dynamicResponse);
      setAudioParam('stereoSpread', params.stereoSpread);
      setAudioParam('circuitType', params.circuitType === 'discrete' ? 0.0 : 1.0);
      setAudioParam('stepSize', params.discrete.stepSize);
      setAudioParam('minStepSize', params.companded.minStepSize);
      setAudioParam('maxStepSize', params.companded.maxStepSize);
      setAudioParam('syllabicTime', params.companded.syllabicTime);
      setAudioParam('clockJitter', params.clockJitter);
      setAudioParam('integratorLag', params.integratorLag);
      setAudioParam('reconstructionCutoff', params.reconstructionCutoff);
      setAudioParam('integratorLeak', params.integratorLeak);
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
  }, [isInitialized, isLoading, params, isCoupledMode, isFreeze]);

  return {
    audioContext,
    isInitialized,
    isLoading,
    initAudioSystem,
    inputNodeRef,
    cvsdDelayNodeRef,
    dryAnalyserRef,
    wetAnalyserRef,
    integratorAnalyserRef,
    stepSizeAnalyserRef,
  };
}
