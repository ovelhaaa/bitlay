const fs = require('fs');
let code = fs.readFileSync('src/App.tsx', 'utf8');

const buttonsToAdd = `
                      <button
                        id="btn-freeze"
                        onClick={() => setIsFreeze(!isFreeze)}
                        className={\`flex items-center gap-1.5 px-3 py-1.5 text-xs font-mono rounded border transition-all cursor-pointer \${
                          isFreeze 
                            ? 'bg-blue-500/20 text-blue-400 border-blue-500/50' 
                            : 'bg-bg-dark hover:bg-border-dim text-zinc-300 border-border-dark hover:border-retro/40'
                        }\`}
                        title="Freeze the audio buffer (Infinite loop)"
                      >
                        {isFreeze ? 'Unfreeze' : 'Freeze'}
                      </button>
                      <button
                        id="btn-couple"
                        onClick={() => setIsCoupledMode(!isCoupledMode)}
                        className={\`flex items-center gap-1.5 px-3 py-1.5 text-xs font-mono rounded border transition-all cursor-pointer \${
                          isCoupledMode 
                            ? 'bg-retro/20 text-retro border-retro/50' 
                            : 'bg-bg-dark hover:bg-border-dim text-zinc-300 border-border-dark hover:border-retro/40'
                        }\`}
                        title="Ativar/Desativar Acoplamentos Musicais"
                      >
                        {isCoupledMode ? 'Acoplamentos: ON' : 'Acoplamentos: OFF'}
                      </button>`;

code = code.replace(/<button\s*id="btn-bypass"/, buttonsToAdd + '\n                      <button\n                        id="btn-bypass"');
fs.writeFileSync('src/App.tsx', code);
