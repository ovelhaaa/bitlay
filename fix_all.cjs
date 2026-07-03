const fs = require('fs');
let content = fs.readFileSync('src/App.tsx', 'utf8');

// The easiest way is to rewrite everything below `Motor DSP Ativo` from a fresh correct state. 
// But we don't have it.

// Let's just fix the parameter blocks!
// A parameter block is:
/*
                      <div className="flex flex-col gap-1.5">
                        <div className="flex items-center justify-between text-xs font-mono">
                          <span ...>...</span>
                          <span ...>...</span>
                          ... (missing div?)
                        <input ... />
                </div>  <-- Wait! Where did this come from?
                        <div className="flex justify-between text-[8px] font-mono text-zinc-600">
                          <span>...</span>
                          <span>...</span>
                          ... missing div!
                          ... missing div!
*/

content = content.replace(/<\/span>\s*<input/g, '</span>\n                        </div>\n                        <input');
content = content.replace(/<\/span>\s*<input/g, '</span>\n                        </div>\n                        <input');
content = content.replace(/<\/span>\s*<input/g, '</span>\n                        </div>\n                        <input');

content = content.replace(/className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"\s*\/>\s*<\/div>/g, 'className="w-full accent-retro bg-border-dark h-1.5 rounded-lg appearance-none cursor-pointer"\n                        />');

content = content.replace(/<span>Wet \(100\%\)<\/span>\s*{\/\* Group 2:/, `<span>Wet (100%)</span>\n                        </div>\n                      </div>\n                    </div>\n                  </div>\n                  {/* Group 2:`);

content = content.replace(/<span>100% \(Sem Perda - DC Drift\)<\/span>\s*{\/\* Group 3:/, `<span>100% (Sem Perda - DC Drift)</span>\n                        </div>\n                      </div>\n                    </div>\n                  </div>\n                  {/* Group 3:`);

fs.writeFileSync('src/App.tsx', content);
