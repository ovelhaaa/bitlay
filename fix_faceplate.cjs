const fs = require('fs');
let content = fs.readFileSync('src/App.tsx', 'utf8');

content = content.replace(/                      <\/button>\s*{\/\* Group 1:/g, `                      </button>\n                    </div>\n                  </div>\n                  {/* Group 1:`);

fs.writeFileSync('src/App.tsx', content);
