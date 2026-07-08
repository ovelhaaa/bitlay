const fs = require('fs');

const file = fs.readFileSync('src/App.tsx', 'utf8');
const lines = file.split('\n');

// Find the start of the header
const headerStartIndex = lines.findIndex(line => line.includes('<header className="bg-panel border-b border-border-dark py-4 px-6 shrink-0 flex items-center justify-between">'));
const headerEndIndex = lines.findIndex(line => line.includes('</header>'));

const newHeader = `      <header className="bg-panel border-b border-border-dark py-4 px-6 shrink-0 flex items-center justify-between">
        <div className="flex items-center gap-3">
          <div className="w-10 h-10 bg-retro rounded-lg flex items-center justify-center shadow-lg shadow-retro/10">
            <Radio className="w-6 h-6 text-black font-bold" />
          </div>
          <div>
            <h1 className="font-sans font-bold text-base tracking-wide text-zinc-100 uppercase">
              Delta Delay CVSD <span className="text-retro text-xs font-mono lowercase bg-retro/10 px-2 py-0.5 rounded-full border border-retro/20 ml-2 font-bold">v1.0 (DSP Lab)</span>
            </h1>
            <p className="text-xs text-zinc-400">
              Emulação de atraso digital vintage de 1-bit via modulação delta adaptativa
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
      </header>`;

lines.splice(headerStartIndex, headerEndIndex - headerStartIndex + 1, newHeader);
fs.writeFileSync('src/App.tsx', lines.join('\n'));
