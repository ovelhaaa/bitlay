// src/cvsd-worklet.test.ts

(global as any).sampleRate = 48000;
(global as any).AudioWorkletProcessor = class AudioWorkletProcessor {
  port = { postMessage: () => {} };
};
let ProcessorClass: any = null;
(global as any).registerProcessor = (name: string, cls: any) => {
  ProcessorClass = cls;
};

await import('./cvsd-worklet.ts');

const processor = new ProcessorClass();

const bufferSize = 1024;
const input = new Float32Array(bufferSize);
const output = new Float32Array(bufferSize);

input[0] = 1.0;
input[1] = 1.0;
input[2] = 1.0;

const params: Record<string, Float32Array> = {};
ProcessorClass.parameterDescriptors.forEach((desc: any) => {
  params[desc.name] = new Float32Array([desc.defaultValue]);
});

params.feedback[0] = 0;
params.mix[0] = 1;
params.delayTime[0] = 10;
params.stepSize[0] = 0.5;
params.inputDrive[0] = 10.0;
params.tap1_mult[0] = 1.0;
params.numTaps[0] = 1;

processor.process([[input]], [[output]], params);

let maxEnergy = 0;
let maxIndex = 0;
for (let i = 0; i < bufferSize; i++) {
  if (Math.abs(output[i]) > maxEnergy) {
    maxEnergy = Math.abs(output[i]);
    maxIndex = i;
  }
}

console.log(`Max energy ${maxEnergy.toFixed(4)} found at index ${maxIndex}`);
if (maxIndex > 450 && maxIndex < 520) {
  console.log('SUCCESS: Impulse was delayed correctly!');
  process.exit(0);
} else {
  console.error('FAILED: Impulse not found at the expected delay position.');
  for (let i = 0; i < bufferSize; i++) {
    if (Math.abs(output[i]) > 0.01) {
      console.log(`Energy starts at ${i}: ${output.slice(i, i + 10).join(', ')}`);
      break;
    }
  }
  process.exit(1);
}
