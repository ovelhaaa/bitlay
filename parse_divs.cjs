const fs = require('fs');
const content = fs.readFileSync('dump_group2.txt', 'utf8');

let divs = 0;
let spans = 0;
const lines = content.split('\n');
for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const openDivs = (line.match(/<div/g) || []).length;
    const closeDivs = (line.match(/<\/div>/g) || []).length;
    divs += openDivs - closeDivs;
    
    const openSpans = (line.match(/<span/g) || []).length;
    const closeSpans = (line.match(/<\/span>/g) || []).length;
    spans += openSpans - closeSpans;
    
    // console.log(`L${i+1}: divs=${divs}, spans=${spans} ` + line.trim().substring(0, 40));
}
console.log(`Final open divs in block: ${divs}`);
console.log(`Final open spans in block: ${spans}`);
