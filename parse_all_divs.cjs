const fs = require('fs');
const content = fs.readFileSync('src/App.tsx', 'utf8');

let divs = 0;
const lines = content.split('\n');
for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    
    // ignore comments that are entire line
    if (line.trim().startsWith('//')) continue;
    
    const openDivs = (line.match(/<div/g) || []).length;
    let closeDivs = (line.match(/<\/div>/g) || []).length;
    // motion.div
    const openMotion = (line.match(/<motion\.div/g) || []).length;
    const closeMotion = (line.match(/<\/motion\.div>/g) || []).length;
    
    divs += openDivs - closeDivs + openMotion - closeMotion;
    
    if (openDivs > 0 || closeDivs > 0 || openMotion > 0 || closeMotion > 0) {
        // console.log(`L${i+1}: divs=${divs} | +${openDivs+openMotion} -${closeDivs+closeMotion} | ${line.trim().substring(0, 40)}`);
    }
    
    // Print lines where divs jump unusually or aren't closed properly at end of blocks
    if (line.trim().startsWith('{/*') && divs > 0) {
        console.log(`L${i+1} START BLOCK: current open divs = ${divs}`);
    }
}
console.log(`Final open divs in file: ${divs}`);
