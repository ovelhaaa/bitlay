const fs = require('fs');
let code = fs.readFileSync('src/types.ts', 'utf8');

if(!code.includes('envAttack')) {
  code = code.replace(/integratorLeak: number;/, `integratorLeak: number;
  envAttack: number;
  envRelease: number;`);
  
  code = code.replace(/integratorLeak: 0\.995,/, `integratorLeak: 0.995,
  envAttack: 0.005,
  envRelease: 0.050,`);
}

fs.writeFileSync('src/types.ts', code);
