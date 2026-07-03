const fs = require('fs');

let content = fs.readFileSync('src/App.tsx', 'utf-8');
const lines = content.split('\n');

// 1. Add imports
const importIndex = lines.findIndex((line) =>
  line.includes(`import Visualizer from './components/Visualizer';`),
);
lines.splice(
  importIndex + 1,
  0,
  `import { Header } from './components/ui/Header';`,
  `import { Landing } from './components/ui/Landing';`,
);

// 2. Replace Header
const headerStartIdx = lines.findIndex((line) =>
  line.includes(`      {/* Top Banner / Header */}`),
);
const headerEndIdx = lines.findIndex(
  (line, i) => i > headerStartIdx && line.includes(`      </header>`),
);
if (headerStartIdx !== -1 && headerEndIdx !== -1) {
  lines.splice(
    headerStartIdx,
    headerEndIdx - headerStartIdx + 1,
    `      <Header isInitialized={isInitialized} sampleRate={audioContext?.sampleRate} />`,
  );
}

// 3. Replace Landing
const landingStartIdx = lines.findIndex((line) =>
  line.includes(`            // Landing screen to bypass autoplay blockages`),
);
// Find the <motion.div key="landing" BEFORE the landing screen comment
const motionStartIdx = lines.lastIndexOf(`<motion.div`, landingStartIdx);
let realMotionStartIdx = motionStartIdx;
while (realMotionStartIdx >= 0 && !lines[realMotionStartIdx].includes(`<motion.div`)) {
  realMotionStartIdx--;
}

// Find the corresponding closing </motion.div>
let depth = 0;
let motionEndIdx = -1;
for (let i = realMotionStartIdx; i < lines.length; i++) {
  if (lines[i].includes(`<motion.div`)) depth++;
  if (lines[i].includes(`</motion.div>`)) {
    depth--;
    if (depth === 0) {
      motionEndIdx = i;
      break;
    }
  }
}

if (realMotionStartIdx !== -1 && motionEndIdx !== -1) {
  lines.splice(
    realMotionStartIdx,
    motionEndIdx - realMotionStartIdx + 1,
    `            <Landing isLoading={isLoading} initAudioSystem={initAudioSystem} />`,
  );
}

fs.writeFileSync('src/App.tsx', lines.join('\n'));
console.log('Done replacement safely');
