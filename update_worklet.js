const fs = require('fs');
let code = fs.readFileSync('src/cvsd-worklet-code.ts', 'utf8');

// I will insert parameters and modify the process method.
code = code.replace(/static get parameterDescriptors\(\) \{[\s\S]*?return \[/, `static get parameterDescriptors() {
    return [
      { name: 'freeze', defaultValue: 0, minValue: 0, maxValue: 1 },
      { name: 'coupledMode', defaultValue: 1, minValue: 0, maxValue: 1 },
      { name: 'envAttack', defaultValue: 0.005, minValue: 0.001, maxValue: 0.1 },
      { name: 'envRelease', defaultValue: 0.050, minValue: 0.010, maxValue: 0.5 },`);

code = code.replace(/this\.channels\.push\(this\.createChannelState\(\)\);/, `this.channels.push(Object.assign(this.createChannelState(), { hfEnvelope: 0.0 }));`);

fs.writeFileSync('src/cvsd-worklet-code.ts', code);
