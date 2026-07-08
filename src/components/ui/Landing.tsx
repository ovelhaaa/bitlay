import React from 'react';
import { motion } from 'motion/react';

export function Landing({
  isLoading,
  initAudioSystem,
}: {
  isLoading: boolean;
  initAudioSystem: () => void;
}) {
  return (
    <motion.div
      key="landing"
      initial={{ opacity: 0, y: 15 }}
      animate={{ opacity: 1, y: 0 }}
      exit={{ opacity: 0, y: -15 }}
      className="flex-1 flex flex-col items-center justify-center text-center max-w-sm mx-auto my-16 p-10 bg-panel/40 border border-border-dark/60 rounded-2xl shadow-2xl relative overflow-hidden"
    >
      <div className="absolute -top-24 -left-24 w-48 h-48 bg-retro/5 rounded-full blur-3xl pointer-events-none"></div>
      <div className="absolute -bottom-24 -right-24 w-48 h-48 bg-retro/5 rounded-full blur-3xl pointer-events-none"></div>

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
  );
}
