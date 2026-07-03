const fs = require('fs');
const content = fs.readFileSync('src/App.tsx', 'utf8');

const lines = content.split('\n');
let tags = [];
let insideReturn = false;
for (let i = 0; i < lines.length; i++) {
    if (lines[i].includes('return (')) insideReturn = true;
    if (!insideReturn) continue;
    
    const line = lines[i];
    // naive parser
    let m;
    const re = /<(\/?[a-zA-Z0-9\.]+)(?:\s+[^>]*?)?\/?>/g;
    while ((m = re.exec(line)) !== null) {
        let tag = m[1];
        if (m[0].endsWith('/>')) continue;
        if (tag === 'br' || tag === 'hr' || tag === 'input' || tag === 'img') continue;
        if (line.includes('//')) continue; // naive ignore comments
        
        if (tag.startsWith('/')) {
            let last = tags.pop();
            if (last && last.tag !== tag.substring(1)) {
                console.log(`Mismatch on line ${i+1}: expected </${last.tag}> (opened on ${last.line}) but found <${tag}>`);
                console.log(`Last few open tags: ${tags.slice(-5).map(t=>t.tag).join(', ')}`);
                process.exit(1);
            }
        } else {
            tags.push({tag, line: i+1});
        }
    }
}
console.log("No mismatches found!");
