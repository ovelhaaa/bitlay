const fs = require('fs');
let code = fs.readFileSync('src/App.tsx', 'utf8');

code = code.replace(/}, \[params, isBypassed, audioContext\]\);/, "}, [params, isBypassed, isCoupledMode, isFreeze, audioContext]);");

fs.writeFileSync('src/App.tsx', code);
