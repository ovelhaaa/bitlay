const fs = require('fs');
let content = fs.readFileSync('src/App.tsx', 'utf8');

const regex = /<span>20kHz \(Fita Nova\)<\/span>\s*<\/div>\s*<\/div>\s*<\/div>\s*<\/div>\s*<\/div>\s*<\/div>\s*<\/div>/;

if(regex.test(content)) {
  content = content.replace(regex, `<span>20kHz (Fita Nova)</span>
                        </div>
                      </div>
                    </div>
                  </div>
                </div>
              </div>`);
} else {
  // try broader regex
  content = content.replace(/<span>20kHz \(Fita Nova\)<\/span>[\s\S]*?{\/\* Right Column/g, `<span>20kHz (Fita Nova)</span>
                        </div>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
              {/* Right Column`);
}

fs.writeFileSync('src/App.tsx', content);
