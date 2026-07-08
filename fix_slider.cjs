const fs = require('fs');
let code = fs.readFileSync('src/App.tsx', 'utf8');

code = code.replace(/{\/\* inputDrive \*\/}[\s\S]*?<span>10\.0x \(Saturado\)<\/span>\s*<\/div>\s*<\/div>/, `{/* character */}
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span className="text-zinc-500 uppercase text-[10px] tracking-wide" title="Saturação do OTA e acoplamentos não-lineares">Character</span>
                          <span className="text-retro font-bold">{params.character.toFixed(0)}%</span>
                        </div>
                        <input
                          id="slider-character"
                          type="range"
                          min="0"
                          max="100"
                          step="1"
                          value={params.character}
                          onChange={(e) => handleParamChange('character', parseFloat(e.target.value))}
                          className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"
                        />
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>Limpo (0%)</span>
                          <span>Bitcrush (100%)</span>
                        </div>
                      </div>`);

fs.writeFileSync('src/App.tsx', code);
