const fs = require('fs');
let code = fs.readFileSync('src/cvsd-worklet-code.ts', 'utf8');

if (!code.includes('export const CVSD_WORKLET_CODE')) {
  code = `export const CVSD_WORKLET_CODE = \`
${code}
\`;`;
  fs.writeFileSync('src/cvsd-worklet-code.ts', code);
}
