// ring_geometry.js — run ZoneRing.ts's pure geometry in Node and print it as JSON.
//
//     node tools/ring_geometry.js <typescript.js> <radius> <thickness> <extra> <zone>...
//
// Transpiles Assets/ProjectScripts/pulse/ZoneRing.ts with the TypeScript API
// (the @component class is cut off; only ringSectors and zoneColor run), so
// the preview draws exactly the triangles the lens builds.
const fs = require("fs"), path = require("path");
const [tsPath, radius, thickness, extra, ...zones] = process.argv.slice(2);
const ts = require(tsPath);
const src = fs.readFileSync(path.join(__dirname, "..", "Assets", "ProjectScripts", "pulse", "ZoneRing.ts"), "utf8");
const core = src.slice(0, src.indexOf("@component"));
const js = ts.transpileModule(core, { compilerOptions: { target: ts.ScriptTarget.ES2021, module: ts.ModuleKind.CommonJS } }).outputText;
const m = { exports: {} };
new Function("module", "exports", js)(m, m.exports);
const out = {};
for (const z of zones.map(Number)) {
  out[z] = m.exports.ringSectors(+radius, +thickness, +extra, z).map((s, i) => ({ ...s, color: m.exports.zoneColor(i, z) }));
}
process.stdout.write(JSON.stringify(out));
