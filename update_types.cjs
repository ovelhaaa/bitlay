const fs = require('fs');
let code = fs.readFileSync('src/types.ts', 'utf8');

code = code.replace(
  /export interface CVSDDelayParams \{/,
  `export interface CVSDDelayParams {
  character: number;          // 0.0 - 100.0`,
);

code = code.replace(
  /export const DEFAULT_PARAMS: CVSDDelayParams = \{/,
  `export const DEFAULT_PARAMS: CVSDDelayParams = {
  character: 25,`,
);

fs.writeFileSync('src/types.ts', code);
