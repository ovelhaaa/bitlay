const fs = require('fs');
let content = fs.readFileSync('src/App.tsx', 'utf8');

content = content.replace(/<span>Ping-Pong\/Wide<\/span>[\s\S]*?{\/\* Group 4:/g, `<span>Ping-Pong/Wide</span>
                        </div>
                      </div>
                    </div>
                  </div>
                  {/* Group 4:`);

fs.writeFileSync('src/App.tsx', content);
