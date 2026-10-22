// pin_preview.js — draw the pin board's plate in Node, as the lens does.
//
//     node tools/pin_preview.js <typescript.js> out.ppm
//
// Transpiles Assets/ProjectScripts/ui/Raster.ts and pins/PinMap.ts with the
// TypeScript API and runs PinMap's drawing on the lens's starting state.
// Prints the time the contours take; convert the PPM with Pillow.
const fs = require("fs"), path = require("path");
const [tsPath, out] = process.argv.slice(2);
const ts = require(tsPath);
const root = path.join(__dirname, "..", "Assets", "ProjectScripts");
const mods = {};
function load(rel) {
  if (mods[rel]) return mods[rel].exports;
  const src = fs.readFileSync(path.join(root, rel + ".ts"), "utf8");
  const js = ts.transpileModule(src, { compilerOptions: { target: ts.ScriptTarget.ES2021, module: ts.ModuleKind.CommonJS } }).outputText;
  const m = mods[rel] = { exports: {} };
  const req = (p) => load(path.join(path.dirname(rel), p));
  new Function("module", "exports", "require", js)(m, m.exports, req);
  return m.exports;
}
const map = load("pins/PinMap"), { Raster } = load("ui/Raster");
let t = Date.now();
const z = map.terrain(map.MAP_W, map.MAP_H, 11), base = map.contours(z);
console.log("contours " + (Date.now() - t) + " ms");
const c = new Raster(map.MAP_W, map.MAP_H);
t = Date.now();
map.drawMap(c, base, {
  you: [320, 150],
  track: [[40, 270], [90, 240], [130, 246], [180, 205], [230, 196], [270, 160], [320, 150]],
  pins: [{ name: "Creek", x: 180, y: 205, kind: 0 }, { name: "Saddle", x: 440, y: 108, kind: 0 },
         { name: "Wildwood TH", x: 560, y: 62, kind: 1 }, { name: "14:05", x: 270, y: 160, kind: 3 }],
  dest: 2,
});
console.log("draw " + (Date.now() - t) + " ms");
const rgb = Buffer.alloc(c.w * c.h * 3);
for (let i = 0; i < c.w * c.h; i++) for (let j = 0; j < 3; j++) rgb[i * 3 + j] = c.px[i * 4 + j];
fs.writeFileSync(out, Buffer.concat([Buffer.from("P6\n" + c.w + " " + c.h + "\n255\n"), rgb]));
