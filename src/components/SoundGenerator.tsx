import React, { useState, useEffect, useRef } from 'react';
import { Play, Pause, Music, Mic, Upload, Volume2, Square, Sparkles } from 'lucide-react';

interface SoundGeneratorProps {
  audioContext: AudioContext | null;
  destinationNode: AudioNode | null;
  isPlayingSource: boolean;
  setIsPlayingSource: (playing: boolean) => void;
}

type SourceType = 'synth' | 'mic' | 'file';
type SynthPreset = 'pluck' | 'beat' | 'click';

export default function SoundGenerator({
  audioContext,
  destinationNode,
  isPlayingSource,
  setIsPlayingSource,
}: SoundGeneratorProps) {
  const [sourceType, setSourceType] = useState<SourceType>('synth');
  const [synthPreset, setSynthPreset] = useState<SynthPreset>('pluck');
  const [micActive, setMicActive] = useState(false);
  const [fileName, setFileName] = useState<string | null>(null);
  const [fileLoading, setFileLoading] = useState(false);

  // Audio references
  const schedulerTimerRef = useRef<number | null>(null);
  const nextNoteTimeRef = useRef<number>(0);
  const currentStepRef = useRef<number>(0);
  const micStreamRef = useRef<MediaStream | null>(null);
  const micNodeRef = useRef<MediaStreamAudioSourceNode | null>(null);
  const fileBufferRef = useRef<AudioBuffer | null>(null);
  const fileSourceNodeRef = useRef<AudioBufferSourceNode | null>(null);

  // Track state for clean cleanup
  const destinationRef = useRef<AudioNode | null>(null);
  destinationRef.current = destinationNode;

  const contextRef = useRef<AudioContext | null>(null);
  contextRef.current = audioContext;

  const bpm = 120;
  const stepDuration = 60 / bpm / 4; // 16th note duration (~125ms)

  // Handle source type change
  useEffect(() => {
    stopAll();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [sourceType]);

  // Cleanup on unmount
  useEffect(() => {
    return () => {
      stopAll();
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const stopAll = () => {
    setIsPlayingSource(false);
    
    // Stop synth scheduler
    if (schedulerTimerRef.current) {
      window.clearInterval(schedulerTimerRef.current);
      schedulerTimerRef.current = null;
    }

    // Stop mic stream
    if (micStreamRef.current) {
      micStreamRef.current.getTracks().forEach(track => track.stop());
      micStreamRef.current = null;
    }
    if (micNodeRef.current) {
      micNodeRef.current.disconnect();
      micNodeRef.current = null;
    }
    setMicActive(false);

    // Stop file source
    if (fileSourceNodeRef.current) {
      try {
        fileSourceNodeRef.current.stop();
      } catch (e) {
        // ignore
      }
      fileSourceNodeRef.current.disconnect();
      fileSourceNodeRef.current = null;
    }
  };

  // --- SYNTHESIZER SEQUENCER ENGINE ---
  const startSynthSequencer = () => {
    if (!contextRef.current || !destinationRef.current) return;
    
    if (contextRef.current.state === 'suspended') {
      contextRef.current.resume();
    }

    setIsPlayingSource(true);
    nextNoteTimeRef.current = contextRef.current.currentTime + 0.05;
    currentStepRef.current = 0;

    // Run scheduler every 25ms
    schedulerTimerRef.current = window.setInterval(() => {
      schedulerLoop();
    }, 25);
  };

  const schedulerLoop = () => {
    const ctx = contextRef.current;
    if (!ctx) return;

    // Schedule events that fall within the next 40ms
    while (nextNoteTimeRef.current < ctx.currentTime + 0.04) {
      scheduleStep(currentStepRef.current, nextNoteTimeRef.current);
      nextNoteTimeRef.current += stepDuration;
      currentStepRef.current = (currentStepRef.current + 1) % 16;
    }
  };

  const scheduleStep = (step: number, time: number) => {
    const ctx = contextRef.current;
    const dest = destinationRef.current;
    if (!ctx || !dest) return;

    if (synthPreset === 'pluck') {
      // Elegant arpeggio sequence (A minor)
      const pitches = [57, 60, 64, 67, 69, 72, 76, 72, 69, 67, 64, 60, 57, 57, 64, 67];
      const midiNote = pitches[step];
      
      // Pluck every 2nd 16th note, or syncopated
      const playStep = step % 2 === 0 || step === 7 || step === 11;
      if (playStep && midiNote) {
        const freq = Math.pow(2, (midiNote - 69) / 12) * 440;
        triggerPluck(ctx, dest, freq, time);
      }
    } else if (synthPreset === 'beat') {
      // Techno Drum Beat
      // Kick on 1, 5, 9, 13
      if (step % 4 === 0) {
        triggerKick(ctx, dest, time);
      }
      // Snare/Noise-clap on 5, 13 (with small offset/delay)
      if (step === 4 || step === 12) {
        triggerSnare(ctx, dest, time);
      }
      // Hihat on offbeats: 2, 4, 6, 8, 10, 12, 14, 16
      if (step % 2 === 2 || step % 4 === 2) {
        triggerHihat(ctx, dest, time);
      }
    } else if (synthPreset === 'click') {
      // Single Click / Sharp Impulse
      // play click every beat (1, 5, 9, 13)
      if (step % 4 === 0) {
        triggerImpulse(ctx, dest, time);
      }
    }
  };

  // --- SYNTH VOICE TRIGGERS ---
  const triggerPluck = (ctx: AudioContext, dest: AudioNode, freq: number, time: number) => {
    // Synth oscillator: Sawtooth blended with square
    const osc = ctx.createOscillator();
    osc.type = 'sawtooth';
    osc.frequency.setValueAtTime(freq, time);

    // Sub oscillator (octave below) for fullness
    const subOsc = ctx.createOscillator();
    subOsc.type = 'triangle';
    subOsc.frequency.setValueAtTime(freq / 2, time);

    // Filter
    const filter = ctx.createBiquadFilter();
    filter.type = 'lowpass';
    filter.Q.setValueAtTime(4, time);
    
    // Filter envelope: quick decay for pluckiness
    filter.frequency.setValueAtTime(200, time);
    filter.frequency.exponentialRampToValueAtTime(3000, time + 0.005);
    filter.frequency.exponentialRampToValueAtTime(350, time + 0.15);

    // Gain envelope
    const gainNode = ctx.createGain();
    gainNode.gain.setValueAtTime(0, time);
    gainNode.gain.linearRampToValueAtTime(0.25, time + 0.005); // quick attack
    gainNode.gain.exponentialRampToValueAtTime(0.001, time + 0.35); // decay

    // Connections
    osc.connect(filter);
    subOsc.connect(filter);
    filter.connect(gainNode);
    gainNode.connect(dest);

    osc.start(time);
    subOsc.start(time);
    osc.stop(time + 0.4);
    subOsc.stop(time + 0.4);
  };

  const triggerKick = (ctx: AudioContext, dest: AudioNode, time: number) => {
    const osc = ctx.createOscillator();
    const gainNode = ctx.createGain();

    osc.frequency.setValueAtTime(150, time);
    osc.frequency.exponentialRampToValueAtTime(45, time + 0.08);

    gainNode.gain.setValueAtTime(0, time);
    gainNode.gain.linearRampToValueAtTime(0.6, time + 0.002);
    gainNode.gain.exponentialRampToValueAtTime(0.001, time + 0.18);

    osc.connect(gainNode);
    gainNode.connect(dest);

    osc.start(time);
    osc.stop(time + 0.2);
  };

  const triggerSnare = (ctx: AudioContext, dest: AudioNode, time: number) => {
    // Noise buffer for snappy snare
    const bufferSize = ctx.sampleRate * 0.15; // 150ms
    const buffer = ctx.createBuffer(1, bufferSize, ctx.sampleRate);
    const data = buffer.getChannelData(0);
    for (let i = 0; i < bufferSize; i++) {
      data[i] = Math.random() * 2 - 1;
    }

    const noise = ctx.createBufferSource();
    noise.buffer = buffer;

    const noiseFilter = ctx.createBiquadFilter();
    noiseFilter.type = 'highpass';
    noiseFilter.frequency.setValueAtTime(1000, time);

    const gainNode = ctx.createGain();
    gainNode.gain.setValueAtTime(0, time);
    gainNode.gain.linearRampToValueAtTime(0.2, time + 0.002);
    gainNode.gain.exponentialRampToValueAtTime(0.001, time + 0.12);

    // Snare tone body
    const tone = ctx.createOscillator();
    tone.type = 'triangle';
    tone.frequency.setValueAtTime(180, time);
    
    const toneGain = ctx.createGain();
    toneGain.gain.setValueAtTime(0, time);
    toneGain.gain.linearRampToValueAtTime(0.25, time + 0.002);
    toneGain.gain.exponentialRampToValueAtTime(0.001, time + 0.08);

    noise.connect(noiseFilter);
    noiseFilter.connect(gainNode);
    gainNode.connect(dest);

    tone.connect(toneGain);
    toneGain.connect(dest);

    noise.start(time);
    tone.start(time);
    noise.stop(time + 0.2);
    tone.stop(time + 0.2);
  };

  const triggerHihat = (ctx: AudioContext, dest: AudioNode, time: number) => {
    const bufferSize = ctx.sampleRate * 0.04; // 40ms
    const buffer = ctx.createBuffer(1, bufferSize, ctx.sampleRate);
    const data = buffer.getChannelData(0);
    for (let i = 0; i < bufferSize; i++) {
      data[i] = Math.random() * 2 - 1;
    }

    const noise = ctx.createBufferSource();
    noise.buffer = buffer;

    const filter = ctx.createBiquadFilter();
    filter.type = 'bandpass';
    filter.frequency.setValueAtTime(8000, time);

    const gainNode = ctx.createGain();
    gainNode.gain.setValueAtTime(0, time);
    gainNode.gain.linearRampToValueAtTime(0.08, time + 0.001);
    gainNode.gain.exponentialRampToValueAtTime(0.001, time + 0.03);

    noise.connect(filter);
    filter.connect(gainNode);
    gainNode.connect(dest);

    noise.start(time);
    noise.stop(time + 0.05);
  };

  const triggerImpulse = (ctx: AudioContext, dest: AudioNode, time: number) => {
    // Short click or impulse (1ms oscillator sweep from high to low)
    const osc = ctx.createOscillator();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(6000, time);
    osc.frequency.exponentialRampToValueAtTime(100, time + 0.004);

    const gainNode = ctx.createGain();
    gainNode.gain.setValueAtTime(0, time);
    gainNode.gain.linearRampToValueAtTime(0.6, time + 0.0005);
    gainNode.gain.exponentialRampToValueAtTime(0.001, time + 0.008);

    osc.connect(gainNode);
    gainNode.connect(dest);

    osc.start(time);
    osc.stop(time + 0.02);
  };

  // --- MICROPHONE SOURCE ENGINE ---
  const toggleMic = async () => {
    if (micActive) {
      stopAll();
      return;
    }

    const ctx = contextRef.current;
    const dest = destinationRef.current;
    if (!ctx || !dest) return;

    try {
      if (ctx.state === 'suspended') {
        await ctx.resume();
      }

      const stream = await navigator.mediaDevices.getUserMedia({
        audio: {
          echoCancellation: false,
          noiseSuppression: false,
          autoGainControl: false,
        },
      });

      const sourceNode = ctx.createMediaStreamSource(stream);
      sourceNode.connect(dest);

      micStreamRef.current = stream;
      micNodeRef.current = sourceNode;
      setMicActive(true);
      setIsPlayingSource(true);
    } catch (err) {
      console.error('Error accessing microphone:', err);
      alert('Não foi possível acessar o microfone. Verifique as permissões de áudio.');
    }
  };

  // --- AUDIO FILE ENGINE ---
  const handleFileUpload = async (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file || !contextRef.current) return;

    setFileLoading(true);
    setFileName(file.name);
    stopAll();

    try {
      const arrayBuffer = await file.arrayBuffer();
      // Decode audio data safely
      const audioBuffer = await contextRef.current.decodeAudioData(arrayBuffer);
      fileBufferRef.current = audioBuffer;
    } catch (err) {
      console.error('Error decoding file:', err);
      alert('Erro ao processar o arquivo de áudio. Use arquivos WAV/MP3 padrão.');
      setFileName(null);
    } finally {
      setFileLoading(false);
    }
  };

  const startFilePlayback = () => {
    const ctx = contextRef.current;
    const dest = destinationRef.current;
    const buffer = fileBufferRef.current;
    if (!ctx || !dest || !buffer) return;

    if (ctx.state === 'suspended') {
      ctx.resume();
    }

    // Stop current if active
    if (fileSourceNodeRef.current) {
      try {
        fileSourceNodeRef.current.stop();
      } catch (e) {
        // ignore
      }
      fileSourceNodeRef.current.disconnect();
    }

    const source = ctx.createBufferSource();
    source.buffer = buffer;
    source.loop = true;
    source.connect(dest);

    source.start(0);
    fileSourceNodeRef.current = source;
    setIsPlayingSource(true);
  };

  const stopFilePlayback = () => {
    if (fileSourceNodeRef.current) {
      try {
        fileSourceNodeRef.current.stop();
      } catch (e) {}
      fileSourceNodeRef.current.disconnect();
      fileSourceNodeRef.current = null;
    }
    setIsPlayingSource(false);
  };

  const handlePlayStopToggle = () => {
    if (isPlayingSource) {
      stopAll();
    } else {
      if (sourceType === 'synth') {
        startSynthSequencer();
      } else if (sourceType === 'file') {
        if (fileBufferRef.current) {
          startFilePlayback();
        } else {
          alert('Por favor, carregue um arquivo de áudio primeiro.');
        }
      }
    }
  };

  return (
    <div id="sound-generator-card" className="bg-panel border border-border-dark rounded-xl p-5 flex flex-col gap-4 shadow-lg">
      <div className="flex items-center justify-between border-b border-border-dark pb-3">
        <div className="flex items-center gap-2">
          <Volume2 className="w-5 h-5 text-retro" />
          <h2 className="font-sans font-semibold text-sm tracking-wide text-zinc-100 uppercase">
            Fonte de Áudio (Sinal de Teste)
          </h2>
        </div>
        {isPlayingSource && (
          <div className="flex items-center gap-1.5 px-2 py-0.5 bg-retro/10 border border-retro/20 text-retro text-[10px] font-mono uppercase tracking-widest rounded-full animate-pulse font-bold">
            <span className="w-1.5 h-1.5 bg-retro rounded-full"></span>
            Ativo
          </div>
        )}
      </div>

      {/* Tabs */}
      <div className="grid grid-cols-3 gap-1 p-1 bg-bg-dark rounded-lg">
        <button
          id="btn-source-synth"
          onClick={() => setSourceType('synth')}
          className={`flex items-center justify-center gap-2 py-2 text-xs font-medium rounded-md transition-all cursor-pointer ${
            sourceType === 'synth'
              ? 'bg-panel text-retro shadow-sm font-semibold'
              : 'text-zinc-400 hover:text-zinc-200 hover:bg-panel/50'
          }`}
        >
          <Music className="w-3.5 h-3.5" />
          Sintetizador
        </button>
        <button
          id="btn-source-mic"
          onClick={() => setSourceType('mic')}
          className={`flex items-center justify-center gap-2 py-2 text-xs font-medium rounded-md transition-all cursor-pointer ${
            sourceType === 'mic'
              ? 'bg-panel text-retro shadow-sm font-semibold'
              : 'text-zinc-400 hover:text-zinc-200 hover:bg-panel/50'
          }`}
        >
          <Mic className="w-3.5 h-3.5" />
          Microfone
        </button>
        <button
          id="btn-source-file"
          onClick={() => setSourceType('file')}
          className={`flex items-center justify-center gap-2 py-2 text-xs font-medium rounded-md transition-all cursor-pointer ${
            sourceType === 'file'
              ? 'bg-panel text-retro shadow-sm font-semibold'
              : 'text-zinc-400 hover:text-zinc-200 hover:bg-panel/50'
          }`}
        >
          <Upload className="w-3.5 h-3.5" />
          Arquivo
        </button>
      </div>

      {/* Content Areas */}
      <div className="flex-1 flex flex-col justify-center min-h-[120px] p-3 bg-bg-dark/60 rounded-lg border border-border-dark/40">
        {sourceType === 'synth' && (
          <div className="flex flex-col gap-4">
            <div className="flex flex-col gap-1.5">
              <label className="text-[10px] font-mono text-zinc-500 uppercase tracking-wider">
                Preset do Sequenciador
              </label>
              <div className="grid grid-cols-3 gap-1.5">
                {(['pluck', 'beat', 'click'] as SynthPreset[]).map(preset => (
                  <button
                    key={preset}
                    id={`btn-preset-${preset}`}
                    onClick={() => {
                      setSynthPreset(preset);
                      if (isPlayingSource) {
                        stopAll();
                        // Restart sequencer with new preset after tiny delay
                        setTimeout(() => startSynthSequencer(), 50);
                      }
                    }}
                    className={`py-1.5 px-2 text-xs font-mono rounded border capitalize transition-all cursor-pointer ${
                      synthPreset === preset
                        ? 'border-retro/40 bg-retro/5 text-retro font-bold'
                        : 'border-border-dark bg-panel/40 text-zinc-400 hover:bg-border-dim hover:text-zinc-300'
                    }`}
                  >
                    {preset === 'pluck' ? 'Pluck' : preset === 'beat' ? 'Beat' : 'Click/Metr.'}
                  </button>
                ))}
              </div>
            </div>

            <div className="flex justify-center">
              <button
                id="btn-play-synth"
                onClick={handlePlayStopToggle}
                className={`flex items-center gap-2 py-2.5 px-6 rounded-lg text-xs font-semibold uppercase tracking-wider transition-all duration-200 cursor-pointer ${
                  isPlayingSource
                    ? 'bg-red-600/15 border border-red-500/30 text-red-400 hover:bg-red-600/25'
                    : 'bg-retro text-black hover:bg-retro/90 font-bold shadow-md shadow-retro/10'
                }`}
              >
                {isPlayingSource ? (
                  <>
                    <Pause className="w-4 h-4 fill-current" /> Parar Sequência
                  </>
                ) : (
                  <>
                    <Play className="w-4 h-4 fill-current" /> Iniciar Sequência
                  </>
                )}
              </button>
            </div>
          </div>
        )}

        {sourceType === 'mic' && (
          <div className="flex flex-col items-center justify-center gap-3 text-center">
            <p className="text-xs text-zinc-400 max-w-[280px]">
              Fale ou bata palmas no microfone para ouvir a degradação e o feedback do delay delta.
            </p>
            <button
              id="btn-toggle-mic"
              onClick={toggleMic}
              className={`flex items-center gap-2 py-2.5 px-6 rounded-lg text-xs font-semibold uppercase tracking-wider transition-all duration-200 cursor-pointer ${
                micActive
                  ? 'bg-red-600/15 border border-red-500/30 text-red-400 hover:bg-red-600/25'
                  : 'bg-retro text-black hover:bg-retro/90 font-bold shadow-md shadow-retro/10'
              }`}
            >
              {micActive ? (
                <>
                  <Square className="w-3.5 h-3.5 fill-current" /> Desativar Microfone
                </>
              ) : (
                <>
                  <Mic className="w-3.5 h-3.5" /> Ativar Microfone
                </>
              )}
            </button>
            <span className="text-[10px] font-mono text-zinc-600 uppercase tracking-widest">
              Recomendado: usar fones de ouvido para evitar feedback de loop acústico!
            </span>
          </div>
        )}

        {sourceType === 'file' && (
          <div className="flex flex-col gap-3.5">
            <div className="flex items-center justify-between gap-2 border border-dashed border-border-dark hover:border-border-dark/80 bg-panel/20 rounded-lg p-3 cursor-pointer relative group transition-all">
              <input
                id="file-upload-input"
                type="file"
                accept="audio/*"
                onChange={handleFileUpload}
                className="absolute inset-0 w-full h-full opacity-0 cursor-pointer"
              />
              <div className="flex items-center gap-3">
                <div className="p-2 bg-bg-dark rounded-lg text-zinc-400 group-hover:text-retro transition-all">
                  <Upload className="w-4 h-4" />
                </div>
                <div className="flex flex-col text-left">
                  <span className="text-xs font-medium text-zinc-300 group-hover:text-zinc-100 transition-all">
                    {fileName ? fileName : 'Escolha um arquivo de áudio...'}
                  </span>
                  <span className="text-[10px] font-mono text-zinc-500 uppercase">
                    {fileLoading ? 'Lendo...' : fileBufferRef.current ? 'Carregado com sucesso' : 'MP3, WAV, OGG, M4A'}
                  </span>
                </div>
              </div>
            </div>

            <div className="flex justify-center">
              <button
                id="btn-play-file"
                disabled={!fileBufferRef.current}
                onClick={handlePlayStopToggle}
                className={`flex items-center gap-2 py-2.5 px-6 rounded-lg text-xs font-semibold uppercase tracking-wider transition-all duration-200 cursor-pointer ${
                  !fileBufferRef.current
                    ? 'bg-border-dark text-zinc-600 border border-border-dark/40 cursor-not-allowed'
                    : isPlayingSource
                    ? 'bg-red-600/15 border border-red-500/30 text-red-400 hover:bg-red-600/25'
                    : 'bg-retro text-black hover:bg-retro/90 font-bold shadow-md shadow-retro/10'
                }`}
              >
                {isPlayingSource ? (
                  <>
                    <Pause className="w-4 h-4 fill-current" /> Pausar Áudio
                  </>
                ) : (
                  <>
                    <Play className="w-4 h-4 fill-current" /> Reproduzir Áudio
                  </>
                )}
              </button>
            </div>
          </div>
        )}
      </div>

      <div className="flex items-center gap-1.5 p-2 bg-bg-dark rounded border border-border-dim">
        <Sparkles className="w-3.5 h-3.5 text-zinc-500 shrink-0" />
        <span className="text-[10px] font-mono text-zinc-500 leading-normal">
          Dica: Use o preset <strong className="text-zinc-400">Pluck</strong> ou <strong className="text-zinc-400">Click</strong> para testar a resposta rápida de transientes da modulação CVSD.
        </span>
      </div>
    </div>
  );
}
