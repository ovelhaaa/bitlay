import React from 'react';

export function Header({
  isInitialized,
  sampleRate,
}: {
  isInitialized: boolean;
  sampleRate?: number;
}) {
  return (
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
            Motor DSP Ativo ({sampleRate}Hz)
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
  );
}
