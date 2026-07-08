const fs = require('fs');
let content = fs.readFileSync('src/App.tsx', 'utf8');

// The end of a parameter is two spans inside the label div:
content = content.replace(/<span>(.*?)<\/span>\s*<span>(.*?)<\/span>\s*({)/g, '<span>$1</span>\n                          <span>$2</span>\n                        </div>\n                      </div>\n                      $3');

// Oh wait, some parameters have a comment after them.
content = content.replace(/<span>(.*?)<\/span>\s*<span>(.*?)<\/span>\s*(?!\s*<\/div>)(?!.*<\/span>)(?=\s*<div|\s*{|\s*<input)/g, '<span>$1</span>\n                          <span>$2</span>\n                        </div>\n                      </div>\n');

// Actually let's just match the specific known strings for the bottom spans!
const bottomSpans = [
  "Wet (100%)",
  "100% (Sem Perda - DC Drift)",
  "10ms (Rápido)", "2000ms (Longo)",
  "0.001 (Fino)", "0.2 (Grosseiro)",
  "0% (Relógio Estável)", "100% (Modulador Caótico)",
  "100Hz", "10000Hz (Banda Larga)",
  "0.90", "1.0",
  "1 (Ideal)", "20 (Lento/Overload)",
  "0 (Estável)", "50 (Chorusing)",
  "0.01Hz (Wow)", "20Hz (Flutter)",
  "100Hz (Muito Escuro)", "20kHz (Fita Nova)",
  "1.0 (Limpo)", "10.0 (Saturado)"
];

for (const text of bottomSpans) {
    // If it's a second span, replace `<span>text</span>` with `<span>text</span>\n</div>\n</div>` if it's not already followed by `</div>`
    const regex = new RegExp(`<span>${text.replace(/[.*+?^$\/()|[\\]{}\\\\]/g, '\\$&')}<\\/span>\\s*(?!<\\/div>)`, 'g');
    content = content.replace(regex, `<span>${text}</span>\n                        </div>\n                      </div>\n`);
}

fs.writeFileSync('src/App.tsx', content);
