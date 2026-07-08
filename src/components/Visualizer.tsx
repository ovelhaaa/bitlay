import React, { useRef, useEffect, useState } from 'react';
import { Activity, Zap, TrendingUp, Sliders } from 'lucide-react';

interface VisualizerProps {
  dryAnalyser: AnalyserNode | null;
  wetAnalyser: AnalyserNode | null;
  integratorAnalyser: AnalyserNode | null;
  stepSizeAnalyser: AnalyserNode | null;
  isPlaying: boolean;
}

type VisualizerMode = 'delta' | 'stepSize' | 'spectrum';

export default function Visualizer({
  dryAnalyser,
  wetAnalyser,
  integratorAnalyser,
  stepSizeAnalyser,
  isPlaying,
}: VisualizerProps) {
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const [mode, setMode] = useState<VisualizerMode>('delta');
  const animationRef = useRef<number | null>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const bufferLength = 512;
    const dryData = new Float32Array(bufferLength);
    const wetData = new Float32Array(bufferLength);
    const intData = new Float32Array(bufferLength);
    const stepData = new Float32Array(bufferLength);

    const resizeCanvas = () => {
      const rect = canvas.getBoundingClientRect();
      // Account for device pixel ratio for sharp rendering
      const dpr = window.devicePixelRatio || 1;
      canvas.width = rect.width * dpr;
      canvas.height = rect.height * dpr;
      ctx.scale(dpr, dpr);
    };

    resizeCanvas();
    window.addEventListener('resize', resizeCanvas);

    // Render loop
    const render = () => {
      const width = canvas.width / (window.devicePixelRatio || 1);
      const height = canvas.height / (window.devicePixelRatio || 1);

      // Draw background
      ctx.fillStyle = '#0A0A0B'; // Sophisticated Dark bg-dark
      ctx.fillRect(0, 0, width, height);

      // Draw grid lines
      ctx.strokeStyle = '#121214'; // Sophisticated Dark panel bg
      ctx.lineWidth = 1;
      
      // Horizontal grid lines
      const gridRows = 6;
      for (let i = 1; i < gridRows; i++) {
        const y = (height / gridRows) * i;
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(width, y);
        ctx.stroke();
      }

      // Vertical grid lines
      const gridCols = 8;
      for (let i = 1; i < gridCols; i++) {
        const x = (width / gridCols) * i;
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, height);
        ctx.stroke();
      }

      // Draw center line
      ctx.strokeStyle = '#1F1F22'; // Sophisticated Dark border-dark
      ctx.lineWidth = 1.5;
      ctx.beginPath();
      ctx.moveTo(0, height / 2);
      ctx.lineTo(width, height / 2);
      ctx.stroke();

      // Read real-time audio data if available
      let hasData = false;
      if (dryAnalyser) {
        dryAnalyser.getFloatTimeDomainData(dryData);
        hasData = true;
      }
      if (wetAnalyser) {
        wetAnalyser.getFloatTimeDomainData(wetData);
      }
      if (integratorAnalyser) {
        integratorAnalyser.getFloatTimeDomainData(intData);
      }
      if (stepSizeAnalyser) {
        stepSizeAnalyser.getFloatTimeDomainData(stepData);
      }

      if (hasData && isPlaying) {
        if (mode === 'delta') {
          // --- DELTA OSCILLOSCOPE (Dry vs Integrator) ---
          
          // Plot original Dry input signal in cool gray
          ctx.beginPath();
          ctx.strokeStyle = 'rgba(161, 161, 170, 0.45)'; // zinc-400
          ctx.lineWidth = 1.5;
          for (let i = 0; i < bufferLength; i++) {
            const x = (width / bufferLength) * i;
            const y = (height / 2) - (dryData[i] * (height * 0.42));
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();

          // Plot CVSD Integrator signal in glowing retro amber/orange
          // This shows the slope overload lagging behind!
          ctx.beginPath();
          ctx.strokeStyle = '#F27D26'; // Brand retro orange
          ctx.lineWidth = 2.5;
          // Add drop shadow / glow effect
          ctx.shadowBlur = 8;
          ctx.shadowColor = 'rgba(242, 125, 38, 0.5)';
          for (let i = 0; i < bufferLength; i++) {
            const x = (width / bufferLength) * i;
            const y = (height / 2) - (intData[i] * (height * 0.42));
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();
          ctx.shadowBlur = 0; // reset shadow

          // Draw legend labels
          ctx.font = '10px monospace';
          ctx.fillStyle = '#a1a1aa';
          ctx.fillText('Entrada (Dry)', 12, 18);
          ctx.fillStyle = '#F27D26';
          ctx.fillText('Modulador CVSD (Reconstruído)', 12, 32);

        } else if (mode === 'stepSize') {
          // --- STEP SIZE DYNAMICS ---
          
          // Plot original Dry wave in background for context
          ctx.beginPath();
          ctx.strokeStyle = 'rgba(63, 63, 70, 0.5)'; // zinc-700
          ctx.lineWidth = 1;
          for (let i = 0; i < bufferLength; i++) {
            const x = (width / bufferLength) * i;
            const y = (height * 0.7) - (dryData[i] * (height * 0.25));
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();

          // Plot the step size delta (glowing cyan)
          // Showing step size expansion in transients
          ctx.beginPath();
          ctx.strokeStyle = '#06b6d4'; // cyan-500
          ctx.lineWidth = 2;
          ctx.shadowBlur = 8;
          ctx.shadowColor = 'rgba(6, 182, 212, 0.4)';
          for (let i = 0; i < bufferLength; i++) {
            const x = (width / bufferLength) * i;
            // Step size is positive, so we draw it from bottom up
            const stepVal = Math.min(stepData[i], 1.5); // Cap for drawing
            const y = (height - 12) - (stepVal * (height * 0.5));
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();
          ctx.shadowBlur = 0;

          // Draw labels
          ctx.font = '10px monospace';
          ctx.fillStyle = '#71717a';
          ctx.fillText('Sinal (Fundo)', 12, 18);
          ctx.fillStyle = '#06b6d4';
          ctx.fillText('Step Size Adaptativo (Compansão Silábica)', 12, 32);

        } else if (mode === 'spectrum') {
          // --- SPECTRUM COMPARISON (Dry vs Wet with cumulative LP filter) ---
          
          // Plot Dry waveform
          ctx.beginPath();
          ctx.strokeStyle = '#71717a'; // zinc-500
          ctx.lineWidth = 1.5;
          for (let i = 0; i < bufferLength; i++) {
            const x = (width / bufferLength) * i;
            const y = (height / 2) - (dryData[i] * (height * 0.42));
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();

          // Plot Wet output (glowing lime/emerald, showing filtered/warm delay)
          ctx.beginPath();
          ctx.strokeStyle = '#10b981'; // emerald-500
          ctx.lineWidth = 2;
          ctx.shadowBlur = 8;
          ctx.shadowColor = 'rgba(16, 185, 129, 0.4)';
          for (let i = 0; i < bufferLength; i++) {
            const x = (width / bufferLength) * i;
            const y = (height / 2) - (wetData[i] * (height * 0.42));
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();
          ctx.shadowBlur = 0;

          // Draw labels
          ctx.font = '10px monospace';
          ctx.fillStyle = '#71717a';
          ctx.fillText('Entrada Direta (Dry)', 12, 18);
          ctx.fillStyle = '#10b981';
          ctx.fillText('Saída do Delay (Wet + Filtro Vintage)', 12, 32);
        }
      } else {
        // Draw steady state flat line with noise-like indicator
        ctx.beginPath();
        ctx.strokeStyle = '#1F1F22'; // Sophisticated Dark border-dark
        ctx.lineWidth = 1.5;
        ctx.moveTo(0, height / 2);
        ctx.lineTo(width, height / 2);
        ctx.stroke();

        ctx.font = '11px monospace';
        ctx.fillStyle = '#52525b';
        ctx.textAlign = 'center';
        ctx.fillText('OSCILOSCÓPIO ESPERANDO SINAL...', width / 2, height / 2 - 12);
        ctx.fillText('(Inicie o sinal de teste acima)', width / 2, height / 2 + 18);
        ctx.textAlign = 'left';
      }

      animationRef.current = requestAnimationFrame(render);
    };

    render();

    return () => {
      window.removeEventListener('resize', resizeCanvas);
      if (animationRef.current) {
        cancelAnimationFrame(animationRef.current);
      }
    };
  }, [mode, dryAnalyser, wetAnalyser, integratorAnalyser, stepSizeAnalyser, isPlaying]);

  return (
    <div id="visualizer-card" className="bg-panel border border-border-dark rounded-xl p-5 flex flex-col gap-4 shadow-lg h-full">
      <div className="flex items-center justify-between border-b border-border-dark pb-3">
        <div className="flex items-center gap-2">
          <Activity className="w-5 h-5 text-retro" />
          <h2 className="font-sans font-semibold text-sm tracking-wide text-zinc-100 uppercase">
            Visualizador de Sinais DSP
          </h2>
        </div>
        
        {/* Mode Selector */}
        <div className="flex gap-1 bg-bg-dark p-0.5 rounded-lg border border-border-dark">
          <button
            id="btn-vis-mode-delta"
            onClick={() => setMode('delta')}
            className={`flex items-center gap-1.5 px-2.5 py-1 text-[10px] font-mono rounded-md transition-all cursor-pointer ${
              mode === 'delta'
                ? 'bg-retro/10 border border-retro/20 text-retro font-bold'
                : 'text-zinc-500 hover:text-zinc-300'
            }`}
          >
            <Zap className="w-3 h-3" />
            Modulação Delta
          </button>
          <button
            id="btn-vis-mode-step"
            onClick={() => setMode('stepSize')}
            className={`flex items-center gap-1.5 px-2.5 py-1 text-[10px] font-mono rounded-md transition-all cursor-pointer ${
              mode === 'stepSize'
                ? 'bg-cyan-500/10 border border-cyan-500/20 text-cyan-400'
                : 'text-zinc-500 hover:text-zinc-300'
            }`}
          >
            <TrendingUp className="w-3 h-3" />
            Step Size (Comp.)
          </button>
          <button
            id="btn-vis-mode-spectrum"
            onClick={() => setMode('spectrum')}
            className={`flex items-center gap-1.5 px-2.5 py-1 text-[10px] font-mono rounded-md transition-all cursor-pointer ${
              mode === 'spectrum'
                ? 'bg-emerald-500/10 border border-emerald-500/20 text-emerald-400'
                : 'text-zinc-500 hover:text-zinc-300'
            }`}
          >
            <Sliders className="w-3 h-3" />
            Dry vs Wet
          </button>
        </div>
      </div>

      {/* Screen Container */}
      <div className="relative flex-1 bg-bg-dark rounded-lg border border-border-dark overflow-hidden min-h-[220px]">
        <canvas ref={canvasRef} className="w-full h-full block" />
        
        {/* Retro screen overlay grid lines */}
        <div className="absolute inset-0 pointer-events-none bg-[radial-gradient(circle_at_center,transparent_40%,rgba(0,0,0,0.45)_100%)] mix-blend-overlay"></div>
        <div className="absolute inset-0 pointer-events-none bg-[linear-gradient(rgba(18,16,16,0)_50%,rgba(0,0,0,0.25)_50%),linear-gradient(90deg,rgba(255,0,0,0.06),rgba(0,255,0,0.02),rgba(0,0,255,0.06))] bg-[size:100%_4px,3px_100%]"></div>
      </div>

      <div className="text-[10px] font-mono text-zinc-500 flex justify-between px-1">
        <span>MODE: {mode.toUpperCase()} MODE</span>
        <span>SAMPLE RATE: {dryAnalyser ? dryAnalyser.context.sampleRate : 44100} Hz</span>
        <span>RESOLUTION: 1-BIT SIGMA-DELTA</span>
      </div>
    </div>
  );
}
