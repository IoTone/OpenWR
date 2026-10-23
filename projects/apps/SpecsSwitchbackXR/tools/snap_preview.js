// snap_preview.js — run the lens's camera-to-maplet conversion on a photo.
//
//     node tools/snap_preview.js <typescript.js> in.rgba W H out.pbm [frameW frameH]
//
// in.rgba is raw RGBA, top-down (e.g. from Pillow: Image.tobytes()). The
// target is the centred box at the frame's aspect, 60 % of the width,
// tilted 4 degrees to exercise the corner mapping. Writes the 1-bit frame
// as a PBM (1 = black, as the maplet shows it).
const fs = require("fs"), path = require("path");
const [tsPath, inPath, W, H, out, fw = "212", fh = "104"] = process.argv.slice(2);
const ts = require(tsPath);
const src = fs.readFileSync(path.join(__dirname, "..", "Assets", "ProjectScripts", "snap", "Dither.ts"), "utf8");
const js = ts.transpileModule(src, { compilerOptions: { target: ts.ScriptTarget.ES2021, module: ts.ModuleKind.CommonJS } }).outputText;
const m = { exports: {} };
new Function("module", "exports", js)(m, m.exports);
const w = +W, h = +H, ow = +fw, oh = +fh;
const bw = w * 0.6, bh = bw * oh / ow, a = 4 * Math.PI / 180, cx = w / 2, cy = h / 2;
const corners = [[-1, -1], [1, -1], [1, 1], [-1, 1]].map(([sx, sy]) => {
  const x = sx * bw / 2, y = sy * bh / 2;
  return [cx + x * Math.cos(a) - y * Math.sin(a), cy + x * Math.sin(a) + y * Math.cos(a)];
});
const t = Date.now();
const frame = m.exports.toFrame({ rgba: fs.readFileSync(inPath), w, h, bottomUp: false }, corners, ow, oh);
console.log(frame.length + " bytes in " + (Date.now() - t) + " ms; hex line 0: " + m.exports.toHex(frame, 0, 8) + "...");
fs.writeFileSync(out, Buffer.concat([Buffer.from("P4\n" + ow + " " + oh + "\n"), Buffer.from(frame)]));
