const fs = require('fs');
let content = fs.readFileSync('src/App.tsx', 'utf8');
content = content.replace(/                  <\/p>\s*<\/motion\.div>\s*<\/div>\s*\)\}\s*<\/AnimatePresence>\s*<\/main>/, `                  </p>\n                </div>\n              </div>\n            </div>\n            </motion.div>\n          )}\n        </AnimatePresence>\n      </main>`);
fs.writeFileSync('src/App.tsx', content);
