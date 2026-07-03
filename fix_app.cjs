const fs = require('fs');
let code = fs.readFileSync('src/App.tsx', 'utf8');

// 1. Add state variables for freeze and coupled mode
code = code.replace(/const \[isBypassed, setIsBypassed\] = useState\(false\);/, 
  "const [isBypassed, setIsBypassed] = useState(false);\n  const [isCoupledMode, setIsCoupledMode] = useState(true);\n  const [isFreeze, setIsFreeze] = useState(false);");

// 2. Compute inputDrive from character and set it
code = code.replace(/setAudioParam\('inputDrive', params\.inputDrive\);/, `
    // Character Curve Mapping (3 zones)
    // 0-30%: 1.0 to 1.5 (Clean, slight rounding)
    // 30-70%: 1.5 to 5.0 (Rapid crunch growth)
    // 70-100%: 5.0 to 10.0 (Heavy saturation)
    let computedDrive = 1.0;
    const char = params.character;
    if (char <= 30) {
      // 0 to 30 -> 1.0 to 1.5
      computedDrive = 1.0 + (char / 30.0) * 0.5;
    } else if (char <= 70) {
      // 30 to 70 -> 1.5 to 5.0
      computedDrive = 1.5 + ((char - 30) / 40.0) * 3.5;
    } else {
      // 70 to 100 -> 5.0 to 10.0
      computedDrive = 5.0 + ((char - 70) / 30.0) * 5.0;
    }
    setAudioParam('inputDrive', computedDrive);
    setAudioParam('coupledMode', isCoupledMode ? 1.0 : 0.0);
    setAudioParam('freeze', isFreeze ? 1.0 : 0.0);
    setAudioParam('envAttack', params.envAttack);
    setAudioParam('envRelease', params.envRelease);
`);

// 3. Update the initAudioSystem param passing
code = code.replace(/setAudioParam\('inputDrive', params\.inputDrive\);/, `
      // We will rely on the useEffect to sync parameters, but for init:
      setAudioParam('coupledMode', isCoupledMode ? 1.0 : 0.0);
      setAudioParam('freeze', isFreeze ? 1.0 : 0.0);
      setAudioParam('envAttack', params.envAttack);
      setAudioParam('envRelease', params.envRelease);
`);

// 4. Update the presets
code = code.replace(/inputDrive:\s*[0-9.]+/g, (match) => {
  const val = parseFloat(match.split(':')[1]);
  // reverse map (approximate)
  let char = 25;
  if (val <= 1.5) char = (val - 1.0) * 2 * 30;
  else if (val <= 5.0) char = 30 + (val - 1.5) / 3.5 * 40;
  else char = 70 + (val - 5.0) / 5.0 * 30;
  return `character: ${Math.round(char)}`;
});

fs.writeFileSync('src/App.tsx', code);
