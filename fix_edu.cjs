const fs = require('fs');
let content = fs.readFileSync('src/App.tsx', 'utf8');

content = content.replace(/\s*{\/\* 4\. Educational Details Box \*\//, `\n                  </div>\n                </div>\n                {/* 4. Educational Details Box */}`);

fs.writeFileSync('src/App.tsx', content);
