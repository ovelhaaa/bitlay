import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Mock do ambiente do AudioWorklet
global.sampleRate = 44100;
global.AudioWorkletProcessor = class AudioWorkletProcessor {};

let ProcessorClass = null;
global.registerProcessor = (name, cls) => {
    ProcessorClass = cls;
};

// Lê e avalia o código original
const workletCodePath = path.resolve(__dirname, 'cvsd-worklet.js');
let code = fs.readFileSync(workletCodePath, 'utf8');

try {
    eval(code);
} catch (e) {
    console.error("Falha ao avaliar o worklet JS:", e);
    process.exit(1);
}

if (!ProcessorClass) {
    console.error("ProcessorClass não foi registrada.");
    process.exit(1);
}

// Gera sinais
const lengthSec = 2; // 2 seconds
const numSamples = global.sampleRate * lengthSec;

// 1. Impulse
const impulse = new Float32Array(numSamples);
impulse[0] = 1.0;

// 2. Chirp (20Hz a 20kHz log sweep)
const chirp = new Float32Array(numSamples);
for (let i = 0; i < numSamples; i++) {
    const t = i / global.sampleRate;
    const f0 = 20.0;
    const f1 = 20000.0;
    const k = Math.exp(Math.log(f1 / f0) / lengthSec);
    const phase = 2 * Math.PI * f0 * ((Math.pow(k, t) - 1) / Math.log(k));
    chirp[i] = Math.sin(phase) * 0.5;
}

// 3. Noise
const noise = new Float32Array(numSamples);
for (let i = 0; i < numSamples; i++) {
    noise[i] = Math.random() * 2.0 - 1.0;
}

const signals = { impulse, chirp, noise };

// Salva os inputs também para o C++ ler
const vectorsDir = path.resolve(__dirname, 'vectors');
if (!fs.existsSync(vectorsDir)) fs.mkdirSync(vectorsDir, { recursive: true });

for (const [name, arr] of Object.entries(signals)) {
    fs.writeFileSync(path.resolve(vectorsDir, `input_${name}.raw`), new Uint8Array(arr.buffer));
}

// Cenários
const scenarios = [
    {
        name: 'default',
        params: { feedback: 0.0, mix: 1.0, clockJitter: 0.0 }
    },
    {
        name: 'companded_wobble',
        params: { feedback: 0.6, wobbleDepth: 10.0, circuitType: 1.0, clockJitter: 0.0 }
    },
    {
        name: 'reverse',
        params: { reverseMode: 1.0, clockJitter: 0.0 }
    }
];

function runScenario(signalName, scenario, inputData) {
    const proc = new ProcessorClass();
    
    // Constrói array de inputs/outputs do jeito que o worklet espera
    // inputs[channel][sample]
    const inputs = [[inputData]];
    const outData = new Float32Array(numSamples);
    const outputs = [[outData]];
    
    // Constrói params (constantes, não a-rate, para simplicidade)
    const parameters = {};
    for (const desc of ProcessorClass.parameterDescriptors) {
        const val = scenario.params[desc.name] !== undefined ? scenario.params[desc.name] : desc.defaultValue;
        parameters[desc.name] = new Float32Array([val]);
    }

    // Processa em blocos de 128 (simulando Web Audio API)
    const blockSize = 128;
    for (let i = 0; i < numSamples; i += blockSize) {
        const blockIn = new Float32Array(blockSize);
        const blockOut = new Float32Array(blockSize);
        
        for (let j = 0; j < blockSize && (i + j) < numSamples; j++) {
            blockIn[j] = inputData[i + j];
        }
        
        proc.process([[blockIn]], [[blockOut]], parameters);
        
        for (let j = 0; j < blockSize && (i + j) < numSamples; j++) {
            outData[i + j] = blockOut[j];
        }
    }
    
    // Salva o arquivo de saída
    const vectorsDir = path.resolve(__dirname, 'vectors');
    if (!fs.existsSync(vectorsDir)) fs.mkdirSync(vectorsDir, { recursive: true });
    
    const outPath = path.resolve(vectorsDir, `js_out_${signalName}_${scenario.name}.raw`);
    fs.writeFileSync(outPath, new Uint8Array(outData.buffer));
    console.log(`Salvo: ${outPath}`);
}

for (const [signalName, signalData] of Object.entries(signals)) {
    for (const scenario of scenarios) {
        runScenario(signalName, scenario, signalData);
    }
}
