const fs = require('fs');
let content = fs.readFileSync('src/App.tsx', 'utf8');

content = content.replace(/\s*{\/\*\s*Right Column/g, `\n                    </div>\n                  </div>\n                </div>\n              </div>\n\n              {/* Right Column`);

fs.writeFileSync('src/App.tsx', content);
