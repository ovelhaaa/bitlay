import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const vectorsDir = path.resolve(__dirname, 'vectors');

const signals = ['impulse', 'chirp', 'noise'];
const scenarios = ['default', 'companded_wobble', 'reverse'];

let allPassed = true;

for (const signal of signals) {
    for (const scenario of scenarios) {
        const jsFile = path.join(vectorsDir, `js_out_${signal}_${scenario}.raw`);
        const cppFile = path.join(vectorsDir, `cpp_out_${signal}_${scenario}.raw`);

        if (!fs.existsSync(jsFile) || !fs.existsSync(cppFile)) {
            console.error(`Missing file(s) for ${signal} - ${scenario}`);
            allPassed = false;
            continue;
        }

        const jsBuf = fs.readFileSync(jsFile);
        const cppBuf = fs.readFileSync(cppFile);

        const jsArr = new Float32Array(jsBuf.buffer, jsBuf.byteOffset, jsBuf.byteLength / 4);
        const cppArr = new Float32Array(cppBuf.buffer, cppBuf.byteOffset, cppBuf.byteLength / 4);

        if (jsArr.length !== cppArr.length) {
            console.error(`Size mismatch for ${signal} - ${scenario}`);
            allPassed = false;
            continue;
        }

        let maxErr = 0;
        let sumSqErr = 0;

        for (let i = 0; i < jsArr.length; i++) {
            const err = Math.abs(jsArr[i] - cppArr[i]);
            if (err > maxErr) maxErr = err;
            sumSqErr += err * err;
        }

        const rmsErr = Math.sqrt(sumSqErr / jsArr.length);

        console.log(`[${signal} - ${scenario}] Max Err: ${maxErr.toExponential(3)}, RMS Err: ${rmsErr.toExponential(3)}`);

        if (maxErr > 1e-5 || rmsErr > 1e-6) {
            console.error(`  => ERROR TOO HIGH!`);
            allPassed = false;
        } else {
            console.log(`  => PASS`);
        }
    }
}

if (allPassed) {
    console.log("\nALL TESTS PASSED.");
} else {
    console.log("\nSOME TESTS FAILED.");
    process.exit(1);
}
