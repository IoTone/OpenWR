// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// SpaceSVGDemo.ts — Demo exercising SpaceSVG Phase 1 features
//
// Setup in Lens Studio:
//   1. Add this script to a SceneObject
//   2. Assign an unlit material (e.g., LegitUnlit) to the "material" input
//   3. Deploy to Spectacles (MeshBuilder rendering must be verified on device)

import { SVGXMLParser, SpaceSVGMeshBackend } from './SpaceSVG';

// ─── Test SVGs ───────────────────────────────────────

const SVG_BASIC_SHAPES = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <rect x="10" y="10" width="60" height="40" fill="red"/>
  <circle cx="150" cy="30" r="25" fill="blue"/>
  <ellipse cx="60" cy="100" rx="40" ry="20" fill="green"/>
  <rect x="120" y="80" width="50" height="50" rx="10" fill="orange"/>
</svg>`;

const SVG_POLYGON_POLYLINE = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <polygon points="100,10 140,80 60,80" fill="crimson"/>
  <polyline points="20,150 60,120 100,160 140,130 180,170"
    stroke="navy" stroke-width="4" fill="none"/>
</svg>`;

const SVG_PATH_LINES = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <path d="M 10 80 L 50 10 L 90 80 Z" fill="purple"/>
  <path d="M 110 80 H 190 V 10 H 110 Z" fill="teal"/>
  <path d="M 10 120 l 40 -30 l 40 60 Z" fill="coral"/>
</svg>`;

const SVG_PATH_CURVES = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <path d="M 10 100 C 10 10, 90 10, 90 100 S 170 190, 170 100"
    stroke="dodgerblue" stroke-width="3" fill="none"/>
  <path d="M 10 150 Q 50 100, 100 150 T 190 150"
    stroke="hotpink" stroke-width="3" fill="none"/>
</svg>`;

const SVG_PATH_ARCS = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <path d="M 30 100 A 40 40 0 0 1 110 100 A 40 40 0 0 1 30 100 Z"
    fill="gold"/>
  <path d="M 130 60 A 30 50 30 1 0 170 140"
    stroke="indigo" stroke-width="3" fill="none"/>
</svg>`;

const SVG_TRANSFORMS = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <rect x="0" y="0" width="40" height="40" fill="red"
    transform="translate(20,20)"/>
  <rect x="0" y="0" width="40" height="40" fill="blue"
    transform="translate(100,20) rotate(45,20,20)"/>
  <rect x="0" y="0" width="40" height="40" fill="green"
    transform="translate(20,120) scale(1.5)"/>
  <rect x="0" y="0" width="40" height="40" fill="orange"
    transform="translate(120,120) skewX(20)"/>
</svg>`;

const SVG_GROUPS = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <g fill="royalblue" transform="translate(10,10)">
    <rect x="0" y="0" width="30" height="30"/>
    <rect x="40" y="0" width="30" height="30"/>
    <rect x="80" y="0" width="30" height="30"/>
  </g>
  <g fill="tomato" opacity="0.7" transform="translate(10,60)">
    <circle cx="15" cy="15" r="15"/>
    <circle cx="55" cy="15" r="15"/>
    <circle cx="95" cy="15" r="15"/>
  </g>
</svg>`;

const SVG_DEFS_USE = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <circle id="dot" cx="0" cy="0" r="10"/>
  </defs>
  <use href="#dot" x="30" y="30" fill="red"/>
  <use href="#dot" x="80" y="30" fill="green"/>
  <use href="#dot" x="130" y="30" fill="blue"/>
  <use href="#dot" x="80" y="80" fill="purple"/>
</svg>`;

const SVG_STROKE = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <rect x="20" y="20" width="60" height="60"
    fill="lightyellow" stroke="black" stroke-width="3"/>
  <circle cx="150" cy="50" r="30"
    fill="none" stroke="darkred" stroke-width="4"/>
  <line x1="20" y1="130" x2="180" y2="130"
    stroke="steelblue" stroke-width="5"/>
  <path d="M 20 170 Q 100 120, 180 170"
    fill="none" stroke="forestgreen" stroke-width="3"/>
</svg>`;

const SVG_OPACITY = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <rect x="20" y="20" width="100" height="100" fill="red"/>
  <rect x="80" y="80" width="100" height="100" fill="blue" opacity="0.5"/>
  <circle cx="100" cy="100" r="40" fill="green" fill-opacity="0.6"/>
</svg>`;

const SVG_STYLE_ATTR = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <rect x="20" y="20" width="60" height="60"
    style="fill:salmon;stroke:darkslategray;stroke-width:2"/>
  <circle cx="150" cy="50" r="30"
    style="fill:mediumpurple;opacity:0.8"/>
</svg>`;

const SVG_HAVE_A_NICE_DAY_LOGO = `
<svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg">
  <circle cx="50" cy="40" r="25" fill="#FFFC00"/>
  <circle cx="42" cy="35" r="3" fill="black"/>
  <circle cx="58" cy="35" r="3" fill="black"/>
  <path d="M 38 48 Q 50 60, 62 48" stroke="black" stroke-width="2" fill="none"/>
  <polygon points="30,65 50,85 70,65" fill="#FFFC00"/>
</svg>`;

// ─── Fancy SVG Path Text: "SVG" ──────────────────────
// Letters drawn with cubic bezier paths in a decorative style

const SVG_FANCY_TEXT = `
<svg viewBox="0 0 400 200" xmlns="http://www.w3.org/2000/svg">
  <!-- Background panel -->
  <rect x="5" y="5" width="390" height="190" rx="15" fill="#1a1a2e"/>
  <rect x="10" y="10" width="380" height="180" rx="12" fill="none" stroke="#e94560" stroke-width="2"/>

  <!-- "S" letter -->
  <path d="M 70 55 C 30 55, 30 95, 65 100 C 100 105, 100 145, 60 145"
    stroke="#e94560" stroke-width="8" fill="none"/>
  <!-- "S" decorative dots -->
  <circle cx="70" cy="50" r="4" fill="#0f3460"/>
  <circle cx="55" cy="150" r="4" fill="#0f3460"/>

  <!-- "V" letter -->
  <path d="M 120 55 L 155 145 L 190 55"
    stroke="#e94560" stroke-width="8" fill="none"/>
  <!-- "V" accent -->
  <circle cx="155" cy="150" r="5" fill="#16c79a"/>

  <!-- "G" letter -->
  <path d="M 270 55 C 220 45, 210 100, 210 105 C 210 115, 220 150, 270 145 L 270 110 L 245 110"
    stroke="#e94560" stroke-width="8" fill="none"/>

  <!-- Decorative underline -->
  <path d="M 40 170 Q 100 155, 200 170 Q 300 185, 360 170"
    stroke="#16c79a" stroke-width="3" fill="none"/>

  <!-- Corner accents -->
  <path d="M 20 30 L 20 20 L 30 20" stroke="#0f3460" stroke-width="3" fill="none"/>
  <path d="M 370 30 L 380 20 L 380 30" stroke="#0f3460" stroke-width="3" fill="none" transform="rotate(0)"/>
  <path d="M 20 170 L 20 180 L 30 180" stroke="#0f3460" stroke-width="3" fill="none"/>
  <path d="M 370 170 L 380 180 L 370 180" stroke="#0f3460" stroke-width="3" fill="none"/>

  <!-- Sparkle dots -->
  <circle cx="100" cy="30" r="2" fill="#16c79a"/>
  <circle cx="300" cy="35" r="2" fill="#16c79a"/>
  <circle cx="340" cy="80" r="2" fill="#16c79a"/>
  <circle cx="50" cy="120" r="2" fill="#16c79a"/>
  <circle cx="320" cy="160" r="2" fill="#16c79a"/>
</svg>`;

// ─── Geometric "HELLO" in block letters ──────────────

const SVG_BLOCK_TEXT = `
<svg viewBox="0 0 500 120" xmlns="http://www.w3.org/2000/svg">
  <!-- H -->
  <rect x="10" y="10" width="12" height="100" fill="#ff6b6b"/>
  <rect x="10" y="50" width="50" height="12" fill="#ff6b6b"/>
  <rect x="48" y="10" width="12" height="100" fill="#ff6b6b"/>

  <!-- E -->
  <rect x="80" y="10" width="12" height="100" fill="#feca57"/>
  <rect x="80" y="10" width="45" height="12" fill="#feca57"/>
  <rect x="80" y="50" width="35" height="12" fill="#feca57"/>
  <rect x="80" y="98" width="45" height="12" fill="#feca57"/>

  <!-- L -->
  <rect x="150" y="10" width="12" height="100" fill="#48dbfb"/>
  <rect x="150" y="98" width="45" height="12" fill="#48dbfb"/>

  <!-- L -->
  <rect x="220" y="10" width="12" height="100" fill="#ff9ff3"/>
  <rect x="220" y="98" width="45" height="12" fill="#ff9ff3"/>

  <!-- O -->
  <path d="M 310 10 L 350 10 A 5 5 0 0 1 355 15 L 355 105 A 5 5 0 0 1 350 110
           L 310 110 A 5 5 0 0 1 305 105 L 305 15 A 5 5 0 0 1 310 10 Z"
    fill="none" stroke="#54a0ff" stroke-width="12"/>

  <!-- Decorative shadow rectangles -->
  <rect x="14" y="14" width="12" height="100" fill="#c44569" opacity="0.3"/>
  <rect x="84" y="14" width="12" height="100" fill="#f6b93b" opacity="0.3"/>
  <rect x="154" y="14" width="12" height="100" fill="#0abde3" opacity="0.3"/>
  <rect x="224" y="14" width="12" height="100" fill="#f368e0" opacity="0.3"/>
</svg>`;

// ─── Animated Clock Generator ────────────────────────

function buildClockSVG(hours: number, minutes: number, seconds: number): string {
  const cx = 100, cy = 100, r = 90;

  // Angles (12 o'clock = -90 degrees, clockwise)
  const secAngle = seconds * 6 - 90;
  const minAngle = minutes * 6 + seconds * 0.1 - 90;
  const hrAngle = (hours % 12) * 30 + minutes * 0.5 - 90;

  // Hour markers
  let markers = '';
  for (let i = 0; i < 12; i++) {
    const angle = i * 30 * Math.PI / 180;
    const isQuarter = i % 3 === 0;
    const innerR = isQuarter ? 70 : 75;
    const outerR = 85;
    const x1 = cx + innerR * Math.sin(angle);
    const y1 = cy - innerR * Math.cos(angle);
    const x2 = cx + outerR * Math.sin(angle);
    const y2 = cy - outerR * Math.cos(angle);
    const w = isQuarter ? 3 : 1.5;
    markers += `  <line x1="${x1.toFixed(1)}" y1="${y1.toFixed(1)}" x2="${x2.toFixed(1)}" y2="${y2.toFixed(1)}" stroke="silver" stroke-width="${w}"/>\n`;
  }

  // Minute tick marks
  let ticks = '';
  for (let i = 0; i < 60; i++) {
    if (i % 5 === 0) continue; // skip hour positions
    const angle = i * 6 * Math.PI / 180;
    const x1 = cx + 82 * Math.sin(angle);
    const y1 = cy - 82 * Math.cos(angle);
    const x2 = cx + 85 * Math.sin(angle);
    const y2 = cy - 85 * Math.cos(angle);
    ticks += `  <line x1="${x1.toFixed(1)}" y1="${y1.toFixed(1)}" x2="${x2.toFixed(1)}" y2="${y2.toFixed(1)}" stroke="gray" stroke-width="0.5"/>\n`;
  }

  // Hand endpoints
  const secRad = secAngle * Math.PI / 180;
  const minRad = minAngle * Math.PI / 180;
  const hrRad = hrAngle * Math.PI / 180;

  const secLen = 72, minLen = 60, hrLen = 42;
  const secX = cx + secLen * Math.cos(secRad);
  const secY = cy + secLen * Math.sin(secRad);
  const minX = cx + minLen * Math.cos(minRad);
  const minY = cy + minLen * Math.sin(minRad);
  const hrX = cx + hrLen * Math.cos(hrRad);
  const hrY = cy + hrLen * Math.sin(hrRad);

  // Tail ends (short lines opposite the hand)
  const secTailX = cx - 15 * Math.cos(secRad);
  const secTailY = cy - 15 * Math.sin(secRad);

  return `<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <!-- Clock face -->
  <circle cx="${cx}" cy="${cy}" r="${r}" fill="#1a1a2e"/>
  <circle cx="${cx}" cy="${cy}" r="${r}" fill="none" stroke="#e94560" stroke-width="3"/>
  <circle cx="${cx}" cy="${cy}" r="87" fill="none" stroke="#0f3460" stroke-width="1"/>

${markers}
${ticks}

  <!-- Hour hand -->
  <line x1="${cx}" y1="${cy}" x2="${hrX.toFixed(1)}" y2="${hrY.toFixed(1)}"
    stroke="white" stroke-width="5"/>

  <!-- Minute hand -->
  <line x1="${cx}" y1="${cy}" x2="${minX.toFixed(1)}" y2="${minY.toFixed(1)}"
    stroke="silver" stroke-width="3"/>

  <!-- Second hand -->
  <line x1="${secTailX.toFixed(1)}" y1="${secTailY.toFixed(1)}" x2="${secX.toFixed(1)}" y2="${secY.toFixed(1)}"
    stroke="#e94560" stroke-width="1.5"/>

  <!-- Center cap -->
  <circle cx="${cx}" cy="${cy}" r="5" fill="#e94560"/>
  <circle cx="${cx}" cy="${cy}" r="2.5" fill="white"/>
</svg>`;
}

// ─── Sinc Plot Chart ─────────────────────────────────
// Based on matplotlib sinc(x) output — curve data extracted from real SVG

const SVG_SINC_CHART = `
<svg viewBox="0 0 432 288" xmlns="http://www.w3.org/2000/svg">
  <!-- Plot background -->
  <rect x="54" y="34" width="335" height="218" fill="white"/>

  <!-- Grid lines -->
  <g stroke="#e0e0e0" stroke-width="0.5">
    <line x1="54" y1="43" x2="389" y2="43"/>
    <line x1="54" y1="76" x2="389" y2="76"/>
    <line x1="54" y1="109" x2="389" y2="109"/>
    <line x1="54" y1="141" x2="389" y2="141"/>
    <line x1="54" y1="174" x2="389" y2="174"/>
    <line x1="54" y1="207" x2="389" y2="207"/>
    <line x1="54" y1="239" x2="389" y2="239"/>
    <line x1="100" y1="34" x2="100" y2="252"/>
    <line x1="161" y1="34" x2="161" y2="252"/>
    <line x1="221" y1="34" x2="221" y2="252"/>
    <line x1="282" y1="34" x2="282" y2="252"/>
    <line x1="343" y1="34" x2="343" y2="252"/>
  </g>

  <!-- Sinc curve -->
  <path d="M 69 207 L 72 204 L 75 200 L 78 198 L 82 196 L 85 195
    L 88 196 L 91 197 L 94 200 L 97 203 L 100 207 L 103 211
    L 106 215 L 109 219 L 112 221 L 115 222 L 118 221 L 121 219
    L 125 216 L 128 211 L 131 206 L 134 200 L 137 195 L 140 190
    L 143 187 L 146 186 L 149 187 L 152 190 L 155 195 L 158 201
    L 161 209 L 165 218 L 168 226 L 171 234 L 174 239 L 177 242
    L 180 242 L 183 237 L 186 229 L 189 216 L 192 200 L 195 181
    L 198 160 L 201 137 L 205 115 L 208 94 L 211 75 L 214 60
    L 217 50 L 220 44 L 223 44 L 226 50 L 229 60 L 232 75
    L 235 94 L 238 115 L 241 137 L 245 160 L 248 181 L 251 200
    L 254 216 L 257 229 L 260 237 L 263 242 L 266 242 L 269 239
    L 272 234 L 275 226 L 278 218 L 281 209 L 284 201 L 288 195
    L 291 190 L 294 187 L 297 186 L 300 187 L 303 190 L 306 195
    L 309 200 L 312 206 L 315 211 L 318 216 L 321 219 L 324 221
    L 327 222 L 331 221 L 334 219 L 337 215 L 340 211 L 343 207
    L 346 203 L 349 200 L 352 197 L 355 196 L 358 195 L 361 196
    L 364 198 L 367 200 L 371 204 L 374 207"
    fill="none" stroke="#1f77b4" stroke-width="2"/>

  <!-- Axes -->
  <line x1="54" y1="252" x2="389" y2="252" stroke="black" stroke-width="1"/>
  <line x1="54" y1="34" x2="54" y2="252" stroke="black" stroke-width="1"/>
  <line x1="389" y1="34" x2="389" y2="252" stroke="black" stroke-width="1"/>
  <line x1="54" y1="34" x2="389" y2="34" stroke="black" stroke-width="1"/>

  <!-- X-axis tick marks -->
  <g stroke="black" stroke-width="1">
    <line x1="100" y1="252" x2="100" y2="256"/>
    <line x1="161" y1="252" x2="161" y2="256"/>
    <line x1="221" y1="252" x2="221" y2="256"/>
    <line x1="282" y1="252" x2="282" y2="256"/>
    <line x1="343" y1="252" x2="343" y2="256"/>
  </g>

  <!-- Y-axis tick marks -->
  <g stroke="black" stroke-width="1">
    <line x1="54" y1="239" x2="50" y2="239"/>
    <line x1="54" y1="207" x2="50" y2="207"/>
    <line x1="54" y1="174" x2="50" y2="174"/>
    <line x1="54" y1="141" x2="50" y2="141"/>
    <line x1="54" y1="109" x2="50" y2="109"/>
    <line x1="54" y1="76" x2="50" y2="76"/>
    <line x1="54" y1="43" x2="50" y2="43"/>
  </g>

  <!-- Zero line -->
  <line x1="54" y1="207" x2="389" y2="207" stroke="#999999" stroke-width="0.5"/>
</svg>`;

// ─── Heatmap (10x10 grid) ────────────────────────────

function buildHeatmapSVG(): string {
  const cols = 10, rows = 10;
  const cellW = 30, cellH = 30;
  const pad = 10;
  const w = cols * cellW + pad * 2;
  const h = rows * cellH + pad * 2;

  let cells = '';
  for (let r = 0; r < rows; r++) {
    for (let c = 0; c < cols; c++) {
      // Generate a smooth 2D pattern: radial gradient from center
      const dx = (c - cols / 2 + 0.5) / (cols / 2);
      const dy = (r - rows / 2 + 0.5) / (rows / 2);
      const v = Math.max(0, Math.min(1, 1 - Math.sqrt(dx * dx + dy * dy)));

      // Blue (cold) → Cyan → Green → Yellow → Red (hot)
      let red: number, green: number, blue: number;
      if (v < 0.25) {
        const t = v / 0.25;
        red = 0; green = Math.round(t * 128); blue = Math.round(128 + t * 127);
      } else if (v < 0.5) {
        const t = (v - 0.25) / 0.25;
        red = 0; green = Math.round(128 + t * 127); blue = Math.round(255 - t * 255);
      } else if (v < 0.75) {
        const t = (v - 0.5) / 0.25;
        red = Math.round(t * 255); green = 255; blue = 0;
      } else {
        const t = (v - 0.75) / 0.25;
        red = 255; green = Math.round(255 - t * 255); blue = 0;
      }

      const hex = '#' + [red, green, blue].map(x =>
        x.toString(16).padStart(2, '0')
      ).join('');
      const x = pad + c * cellW;
      const y = pad + r * cellH;
      cells += `  <rect x="${x}" y="${y}" width="${cellW}" height="${cellH}" fill="${hex}"/>\n`;
    }
  }

  // Color scale bar
  let legend = '';
  const barX = w + 5;
  const barH = rows * cellH;
  for (let i = 0; i < 20; i++) {
    const v = 1 - i / 19;
    let red: number, green: number, blue: number;
    if (v < 0.25) {
      const t = v / 0.25;
      red = 0; green = Math.round(t * 128); blue = Math.round(128 + t * 127);
    } else if (v < 0.5) {
      const t = (v - 0.25) / 0.25;
      red = 0; green = Math.round(128 + t * 127); blue = Math.round(255 - t * 255);
    } else if (v < 0.75) {
      const t = (v - 0.5) / 0.25;
      red = Math.round(t * 255); green = 255; blue = 0;
    } else {
      const t = (v - 0.75) / 0.25;
      red = 255; green = Math.round(255 - t * 255); blue = 0;
    }
    const hex = '#' + [red, green, blue].map(x =>
      x.toString(16).padStart(2, '0')
    ).join('');
    const sy = pad + (i / 19) * (barH - barH / 20);
    legend += `  <rect x="${barX}" y="${sy.toFixed(1)}" width="15" height="${(barH / 19 + 1).toFixed(1)}" fill="${hex}"/>\n`;
  }

  return `<svg viewBox="0 0 ${w + 30} ${h}" xmlns="http://www.w3.org/2000/svg">
  <rect x="0" y="0" width="${w + 30}" height="${h}" fill="#222"/>
${cells}
  <!-- Grid lines -->
  <rect x="${pad}" y="${pad}" width="${cols * cellW}" height="${rows * cellH}" fill="none" stroke="#444" stroke-width="1"/>
  <!-- Color legend -->
${legend}
  <rect x="${barX}" y="${pad}" width="15" height="${barH}" fill="none" stroke="#666" stroke-width="1"/>
</svg>`;
}

// ─── Stylized Tiger Face ─────────────────────────────

const SVG_TIGER = `
<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
  <!-- Background -->
  <circle cx="100" cy="100" r="98" fill="#1a1a1a"/>

  <!-- Head shape -->
  <ellipse cx="100" cy="105" rx="70" ry="65" fill="#e8820c"/>

  <!-- Dark forehead stripes -->
  <path d="M 60 60 Q 70 50, 80 55 Q 85 65, 80 75" fill="#2d1600"/>
  <path d="M 90 48 Q 100 40, 110 48 Q 108 60, 100 65 Q 92 60, 90 48" fill="#2d1600"/>
  <path d="M 120 55 Q 130 50, 140 60 Q 135 70, 120 75" fill="#2d1600"/>

  <!-- Cheek stripes -->
  <path d="M 40 85 Q 50 82, 55 90 Q 48 95, 38 92" fill="#2d1600"/>
  <path d="M 38 100 Q 50 97, 56 105 Q 48 112, 35 108" fill="#2d1600"/>
  <path d="M 145 90 Q 150 82, 160 85 Q 162 92, 152 95" fill="#2d1600"/>
  <path d="M 144 105 Q 150 97, 162 100 Q 165 108, 152 112" fill="#2d1600"/>

  <!-- White face patches -->
  <ellipse cx="80" cy="95" rx="20" ry="15" fill="#ffecd2"/>
  <ellipse cx="120" cy="95" rx="20" ry="15" fill="#ffecd2"/>

  <!-- Eyes -->
  <ellipse cx="80" cy="90" rx="12" ry="9" fill="white"/>
  <ellipse cx="120" cy="90" rx="12" ry="9" fill="white"/>
  <ellipse cx="80" cy="90" rx="7" ry="8" fill="#4a7c2f"/>
  <ellipse cx="120" cy="90" rx="7" ry="8" fill="#4a7c2f"/>
  <circle cx="80" cy="89" r="4" fill="black"/>
  <circle cx="120" cy="89" r="4" fill="black"/>
  <circle cx="82" cy="87" r="1.5" fill="white"/>
  <circle cx="122" cy="87" r="1.5" fill="white"/>

  <!-- Eye liner -->
  <path d="M 68 90 Q 62 85, 58 82" stroke="black" stroke-width="2" fill="none"/>
  <path d="M 132 90 Q 138 85, 142 82" stroke="black" stroke-width="2" fill="none"/>

  <!-- Nose -->
  <path d="M 93 108 Q 100 103, 107 108 Q 107 115, 100 118 Q 93 115, 93 108 Z" fill="#e85d75"/>

  <!-- White muzzle area -->
  <ellipse cx="100" cy="125" rx="25" ry="18" fill="#ffecd2"/>

  <!-- Mouth -->
  <line x1="100" y1="118" x2="100" y2="128" stroke="#2d1600" stroke-width="1.5"/>
  <path d="M 88 130 Q 94 135, 100 128 Q 106 135, 112 130" stroke="#2d1600" stroke-width="1.5" fill="none"/>

  <!-- Whisker dots -->
  <g fill="#2d1600">
    <circle cx="80" cy="115" r="1"/>
    <circle cx="85" cy="118" r="1"/>
    <circle cx="82" cy="121" r="1"/>
    <circle cx="120" cy="115" r="1"/>
    <circle cx="115" cy="118" r="1"/>
    <circle cx="118" cy="121" r="1"/>
  </g>

  <!-- Whiskers -->
  <g stroke="white" stroke-width="0.8" fill="none">
    <line x1="70" y1="112" x2="35" y2="108"/>
    <line x1="70" y1="118" x2="32" y2="120"/>
    <line x1="70" y1="124" x2="35" y2="130"/>
    <line x1="130" y1="112" x2="165" y2="108"/>
    <line x1="130" y1="118" x2="168" y2="120"/>
    <line x1="130" y1="124" x2="165" y2="130"/>
  </g>

  <!-- Ears -->
  <path d="M 45 55 Q 35 25, 55 40 Q 60 50, 50 60 Z" fill="#e8820c"/>
  <path d="M 48 50 Q 42 35, 53 43" fill="#ffc0cb"/>
  <path d="M 155 55 Q 165 25, 145 40 Q 140 50, 150 60 Z" fill="#e8820c"/>
  <path d="M 152 50 Q 158 35, 147 43" fill="#ffc0cb"/>

  <!-- Chin shadow -->
  <ellipse cx="100" cy="145" rx="20" ry="8" fill="#c06800" opacity="0.4"/>
</svg>`;

const SVG_LOGO = `
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="395" height="125" viewBox="0 0 395 125" version="1.1">
    <!-- Generator: Sketch 46.2 (44496) - http://www.bohemiancoding.com/sketch -->
    <title>Logo</title>
    <desc>Created with Sketch.</desc>
    <defs>
        <polygon id="path-1" points="124.858 124.8408 124.858 0.3378 0.3554 0.3378 0.3554 124.8408"/>
        <polygon id="path-3" points="124.5213 125 0.0003 125 0.0003 0.0351 124.5213 0.0351"/>
    </defs>
    <g id="Page-1" stroke="none" stroke-width="1" fill="none" fill-rule="evenodd">
        <g id="Logo" transform="translate(0.000000, -1.000000)">
            <g id="Group-3" transform="translate(270.000000, 0.965000)">
                <mask id="mask-2" fill="white">
                    <use xlink:href="#path-1"/>
                </mask>
                <g id="Clip-2"/>
                <path d="M124.8584,113.4668 C124.8584,115.0888 124.5684,116.5928 123.9914,117.9828 C123.4114,119.3708 122.6144,120.5728 121.6024,121.5838 C120.5884,122.5978 119.3884,123.3948 117.9994,123.9728 C116.6104,124.5518 115.1044,124.8408 113.4854,124.8408 L32.0454,124.8408 C29.9624,124.8408 27.7174,124.5958 25.3164,124.1038 C22.9144,123.6118 20.5414,122.8438 18.1974,121.8028 C15.8524,120.7598 13.6094,119.4148 11.4684,117.7648 C9.3264,116.1148 7.4314,114.1178 5.7814,111.7738 C4.1314,109.4298 2.8144,106.7098 1.8314,103.6128 C0.8454,100.5168 0.3554,97.0288 0.3554,93.1508 L0.3554,32.0288 C0.3554,29.9438 0.6004,27.7018 1.0934,25.2988 C1.5844,22.8978 2.3514,20.5238 3.3934,18.1798 C4.4354,15.8358 5.7954,13.5928 7.4744,11.4508 C9.1524,9.3108 11.1644,7.4138 13.5084,5.7638 C15.8524,4.1138 18.5584,2.7978 21.6274,1.8138 C24.6944,0.8298 28.1674,0.3378 32.0454,0.3378 L112.6244,0.3378 L112.6244,22.9108 L32.0454,22.9108 C29.0924,22.9108 26.8364,23.6928 25.2734,25.2558 C23.7114,26.8188 22.9294,29.1338 22.9294,32.2008 L22.9294,93.1508 C22.9294,96.0458 23.7244,98.2878 25.3164,99.8788 C26.9084,101.4718 29.1504,102.2668 32.0454,102.2668 L102.2854,102.2668 L102.2854,73.9638 L51.1944,73.9638 L51.1944,51.2158 L113.4854,51.2158 C115.1044,51.2158 116.6104,51.5198 117.9994,52.1278 C119.3884,52.7348 120.5884,53.5598 121.6024,54.6018 C122.6144,55.6428 123.4114,56.8448 123.9914,58.2048 C124.5684,59.5648 124.8584,61.0258 124.8584,62.5898 L124.8584,113.4668 Z" id="Fill-1" fill="#050505" mask="url(#mask-2)"/>
            </g>
            <g id="Group-6" transform="translate(0.000000, 0.965000)">
                <mask id="mask-4" fill="white">
                    <use xlink:href="#path-3"/>
                </mask>
                <g id="Clip-5"/>
                <path d="M123.7773,76.3271 C123.2823,73.9061 122.5073,71.5131 121.4573,69.1491 C120.4063,66.7851 119.0373,64.5241 117.3423,62.3641 C115.6503,60.2061 113.6233,58.2931 111.2583,56.6301 C108.8943,54.9671 106.1673,53.6401 103.0733,52.6471 C99.9803,51.6551 96.4783,51.1591 92.5683,51.1591 L31.9533,51.3631 C28.9763,51.3631 26.7013,50.3581 25.1253,48.7821 C23.5493,47.2061 22.7613,44.8711 22.7613,41.8811 L22.7933,32.2661 C22.7933,29.0711 23.5813,26.8391 25.1563,25.2621 C26.7323,23.6871 29.0073,22.8981 31.9843,22.8981 L112.0543,22.8981 L112.0543,0.0351 L31.9843,0.0351 C28.0743,0.0351 24.5723,0.5311 21.4803,1.5231 C18.3853,2.5161 15.6583,3.8431 13.2943,5.5071 C10.9293,7.1701 8.9023,9.0821 7.2103,11.2401 C5.5163,13.4001 4.1463,15.6611 3.0953,18.0251 C2.0453,20.3891 1.2703,22.7821 0.7753,25.2031 C0.2793,27.6261 0.0313,29.8871 0.0313,31.9891 L0.0003,41.9981 C0.0003,44.1001 0.2483,46.3611 0.7443,48.7841 C1.2393,51.2051 2.0133,53.5981 3.0643,55.9621 C4.1153,58.3261 5.4853,60.5871 7.1773,62.7471 C8.8713,64.9051 10.8983,66.8181 13.2623,68.4801 C15.6273,70.1441 18.3543,71.4711 21.4493,72.4641 C24.5413,73.4561 28.0433,73.9521 31.9533,73.9521 L92.5683,73.9651 C95.5453,73.9651 97.8203,74.7531 99.3963,76.3281 C100.9723,77.9051 101.7593,80.2401 101.7593,83.3321 L101.7503,92.8721 C101.7503,95.9651 100.9633,98.2991 99.3863,99.8761 C97.8103,101.4511 95.5353,102.4961 92.5583,102.4961 L12.1463,102.4961 L12.1463,125.0001 L92.5583,125.0001 C96.4683,125.0001 99.9703,124.5041 103.0633,123.5121 C106.1573,122.5191 108.8843,121.1921 111.2493,119.5281 C113.6133,117.8661 115.6403,115.9521 117.3343,113.7951 C119.0273,111.6351 120.3963,109.3741 121.4473,107.0101 C122.4983,104.6461 123.2723,102.2531 123.7673,99.8311 C124.2633,97.4091 124.5113,95.1481 124.5113,93.0471 L124.5213,83.1121 C124.5213,81.0101 124.2733,78.7501 123.7773,76.3271" id="Fill-4" fill="#050505" mask="url(#mask-4)"/>
            </g>
            <polygon id="Fill-7" fill="#E02832" points="186.2031 125.9455 135.0171 1.0835 160.1441 1.0835 196.6891 94.5795 233.7871 1.1255 259.7751 1.1255 207.1741 125.9455"/>
        </g>
    </g>
</svg>`;

const SVG_LATEX1 = `
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink"
    style="width: 17.241ex; height: 6.621ex; vertical-align: -3.034ex; margin: 1px 0px;"
    viewBox="0 -1597.1581423917737 7394.666666666666 2841.2945139861167">
    <rect x="0" y="-1598" width="7395" height="2842" fill="#1a1a2e"/>
    <g stroke="white" fill="grey" stroke-width="0" transform="matrix(1 0 0 -1 0 0)">
        <use xlink:href="#MJSZ2-2211" xmlns:xlink="http://www.w3.org/1999/xlink" />
        <g transform="translate(145,-1105)">
            <use transform="scale(0.7071067811865476)" xlink:href="#MJMATHI-69"
                xmlns:xlink="http://www.w3.org/1999/xlink" />
            <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-3D"
                xmlns:xlink="http://www.w3.org/1999/xlink" x="350" y="0" />
            <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-30"
                xmlns:xlink="http://www.w3.org/1999/xlink" x="1133" y="0" />
        </g>
        <use transform="scale(0.7071067811865476)" xlink:href="#MJMATHI-6E"
            xmlns:xlink="http://www.w3.org/1999/xlink" x="722" y="1640" />
        <use xlink:href="#MJMATHI-69" xmlns:xlink="http://www.w3.org/1999/xlink" x="1615" y="0" />
        <use xlink:href="#MJMAIN-3D" xmlns:xlink="http://www.w3.org/1999/xlink" x="2243" y="0" />
        <g transform="translate(3026,0)">
            <g transform="translate(397,0)">
                <rect stroke="none" width="3850" height="60" x="0" y="220" />
                <g transform="translate(60,748)">
                    <use xlink:href="#MJMATHI-6E" xmlns:xlink="http://www.w3.org/1999/xlink" />
                    <use xlink:href="#MJMAIN-28" xmlns:xlink="http://www.w3.org/1999/xlink" x="605"
                        y="0" />
                    <use xlink:href="#MJMATHI-6E" xmlns:xlink="http://www.w3.org/1999/xlink" x="999"
                        y="0" />
                    <use xlink:href="#MJMAIN-2B" xmlns:xlink="http://www.w3.org/1999/xlink" x="1826"
                        y="0" />
                    <use xlink:href="#MJMAIN-31" xmlns:xlink="http://www.w3.org/1999/xlink" x="2831"
                        y="0" />
                    <use xlink:href="#MJMAIN-29" xmlns:xlink="http://www.w3.org/1999/xlink" x="3336"
                        y="0" />
                </g>
                <use xlink:href="#MJMAIN-32" xmlns:xlink="http://www.w3.org/1999/xlink" x="1672"
                    y="-686" />
            </g>
        </g>
    </g>
    <defs id="MathJax_SVG_glyphs">
        <path id="MJSZ2-2211" stroke-width="10"
            d="M60 948Q63 950 665 950H1267L1325 815Q1384 677 1388 669H1348L1341 683Q1320 724 1285 761Q1235 809 1174 838T1033 881T882 898T699 902H574H543H251L259 891Q722 258 724 252Q725 250 724 246Q721 243 460 -56L196 -356Q196 -357 407 -357Q459 -357 548 -357T676 -358Q812 -358 896 -353T1063 -332T1204 -283T1307 -196Q1328 -170 1348 -124H1388Q1388 -125 1381 -145T1356 -210T1325 -294L1267 -449L666 -450Q64 -450 61 -448Q55 -446 55 -439Q55 -437 57 -433L590 177Q590 178 557 222T452 366T322 544L56 909L55 924Q55 945 60 948Z" />
        <path id="MJMATHI-69" stroke-width="10"
            d="M184 600Q184 624 203 642T247 661Q265 661 277 649T290 619Q290 596 270 577T226 557Q211 557 198 567T184 600ZM21 287Q21 295 30 318T54 369T98 420T158 442Q197 442 223 419T250 357Q250 340 236 301T196 196T154 83Q149 61 149 51Q149 26 166 26Q175 26 185 29T208 43T235 78T260 137Q263 149 265 151T282 153Q302 153 302 143Q302 135 293 112T268 61T223 11T161 -11Q129 -11 102 10T74 74Q74 91 79 106T122 220Q160 321 166 341T173 380Q173 404 156 404H154Q124 404 99 371T61 287Q60 286 59 284T58 281T56 279T53 278T49 278T41 278H27Q21 284 21 287Z" />
        <path id="MJMAIN-3D" stroke-width="10"
            d="M56 347Q56 360 70 367H707Q722 359 722 347Q722 336 708 328L390 327H72Q56 332 56 347ZM56 153Q56 168 72 173H708Q722 163 722 153Q722 140 707 133H70Q56 140 56 153Z" />
        <path id="MJMAIN-30" stroke-width="10"
            d="M96 585Q152 666 249 666Q297 666 345 640T423 548Q460 465 460 320Q460 165 417 83Q397 41 362 16T301 -15T250 -22Q224 -22 198 -16T137 16T82 83Q39 165 39 320Q39 494 96 585ZM321 597Q291 629 250 629Q208 629 178 597Q153 571 145 525T137 333Q137 175 145 125T181 46Q209 16 250 16Q290 16 318 46Q347 76 354 130T362 333Q362 478 354 524T321 597Z" />
        <path id="MJMATHI-6E" stroke-width="10"
            d="M21 287Q22 293 24 303T36 341T56 388T89 425T135 442Q171 442 195 424T225 390T231 369Q231 367 232 367L243 378Q304 442 382 442Q436 442 469 415T503 336T465 179T427 52Q427 26 444 26Q450 26 453 27Q482 32 505 65T540 145Q542 153 560 153Q580 153 580 145Q580 144 576 130Q568 101 554 73T508 17T439 -10Q392 -10 371 17T350 73Q350 92 386 193T423 345Q423 404 379 404H374Q288 404 229 303L222 291L189 157Q156 26 151 16Q138 -11 108 -11Q95 -11 87 -5T76 7T74 17Q74 30 112 180T152 343Q153 348 153 366Q153 405 129 405Q91 405 66 305Q60 285 60 284Q58 278 41 278H27Q21 284 21 287Z" />
        <path id="MJMAIN-28" stroke-width="10"
            d="M94 250Q94 319 104 381T127 488T164 576T202 643T244 695T277 729T302 750H315H319Q333 750 333 741Q333 738 316 720T275 667T226 581T184 443T167 250T184 58T225 -81T274 -167T316 -220T333 -241Q333 -250 318 -250H315H302L274 -226Q180 -141 137 -14T94 250Z" />
        <path id="MJMAIN-2B" stroke-width="10"
            d="M56 237T56 250T70 270H369V420L370 570Q380 583 389 583Q402 583 409 568V270H707Q722 262 722 250T707 230H409V-68Q401 -82 391 -82H389H387Q375 -82 369 -68V230H70Q56 237 56 250Z" />
        <path id="MJMAIN-31" stroke-width="10"
            d="M213 578L200 573Q186 568 160 563T102 556H83V602H102Q149 604 189 617T245 641T273 663Q275 666 285 666Q294 666 302 660V361L303 61Q310 54 315 52T339 48T401 46H427V0H416Q395 3 257 3Q121 3 100 0H88V46H114Q136 46 152 46T177 47T193 50T201 52T207 57T213 61V578Z" />
        <path id="MJMAIN-29" stroke-width="10"
            d="M60 749L64 750Q69 750 74 750H86L114 726Q208 641 251 514T294 250Q294 182 284 119T261 12T224 -76T186 -143T145 -194T113 -227T90 -246Q87 -249 86 -250H74Q66 -250 63 -250T58 -247T55 -238Q56 -237 66 -225Q221 -64 221 250T66 725Q56 737 55 738Q55 746 60 749Z" />
        <path id="MJMAIN-32" stroke-width="10"
            d="M109 429Q82 429 66 447T50 491Q50 562 103 614T235 666Q326 666 387 610T449 465Q449 422 429 383T381 315T301 241Q265 210 201 149L142 93L218 92Q375 92 385 97Q392 99 409 186V189H449V186Q448 183 436 95T421 3V0H50V19V31Q50 38 56 46T86 81Q115 113 136 137Q145 147 170 174T204 211T233 244T261 278T284 308T305 340T320 369T333 401T340 431T343 464Q343 527 309 573T212 619Q179 619 154 602T119 569T109 550Q109 549 114 549Q132 549 151 535T170 489Q170 464 154 447T109 429Z" />
    </defs>
</svg>`;

// Builds animated countdown SVG with two rows of MathJax glyphs:
//   Row 1: "a ≠ 0"  (while a > 0)  or  "a = 0"  (when a reaches 0)
//   Row 2: "a = {value}"  showing the current value of a
// The instruction text ("Solve the expression...") uses a native Text component (see instructionText).
function buildLatexCountdownSVG(value: number): string {
  const digits = value.toString();
  const opX = 811;
  const rhsX = 1872;
  const digitAdvance = 500;
  const mathRowGap = 950; // vertical gap between math rows (glyph-space, pre-flip)

  // Row 1: assertion — use ≠ while a>0, = when a==0
  const operatorId = value === 0 ? 'MJMAIN-3D' : 'MJMAIN-2260';
  const topColor = value === 0 ? '#44cc44' : 'white';

  // Row 2: "a = {value}" — build digit <use> elements
  let valueUses = '';
  for (let i = 0; i < digits.length; i++) {
    const hex = '3' + digits[i];
    const x = rhsX + i * digitAdvance;
    valueUses += `            <use xlink:href="#MJMAIN-${hex}" xmlns:xlink="http://www.w3.org/1999/xlink" x="${x}" y="0" />\n`;
  }

  // Fixed width — use widest case (value=10, 2 digits) so layout doesn't shift
  const totalWidth = rhsX + 2 * digitAdvance;

  const vbMinY = -750;
  const vbHeight = mathRowGap + 800;

  return `<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink"
    viewBox="0 ${vbMinY} ${totalWidth} ${vbHeight}">
    <rect x="0" y="${vbMinY}" width="${totalWidth}" height="${vbHeight}" fill="#1a1a2e"/>
    <g stroke-width="0" transform="matrix(1 0 0 -1 0 0)">
        <g stroke="${topColor}" fill="grey">
            <use xlink:href="#MJMATHI-61" xmlns:xlink="http://www.w3.org/1999/xlink" />
            <use xlink:href="#${operatorId}" xmlns:xlink="http://www.w3.org/1999/xlink" x="${opX}" y="0" />
            <use xlink:href="#MJMAIN-30" xmlns:xlink="http://www.w3.org/1999/xlink" x="${rhsX}" y="0" />
        </g>
        <g stroke="grey" fill="grey" transform="translate(0, -${mathRowGap})">
            <use xlink:href="#MJMATHI-61" xmlns:xlink="http://www.w3.org/1999/xlink" />
            <use xlink:href="#MJMAIN-3D" xmlns:xlink="http://www.w3.org/1999/xlink" x="${opX}" y="0" />
${valueUses}        </g>
    </g>
    <defs>
        <path id="MJMATHI-61" stroke-width="10"
            d="M33 157Q33 258 109 349T280 441Q331 441 370 392Q386 422 416 422Q429 422 439 414T449 394Q449 381 412 234T374 68Q374 43 381 35T402 26Q411 27 422 35Q443 55 463 131Q469 151 473 152Q475 153 483 153H487Q506 153 506 144Q506 138 501 117T481 63T449 13Q436 0 417 -8Q409 -10 393 -10Q359 -10 336 5T306 36L300 51Q299 52 296 50Q294 48 292 46Q233 -10 172 -10Q117 -10 75 30T33 157ZM351 328Q351 334 346 350T323 385T277 405Q242 405 210 374T160 293Q131 214 119 129Q119 126 119 118T118 106Q118 61 136 44T179 26Q217 26 254 59T298 110Q300 114 325 217T351 328Z" />
        <path id="MJMAIN-3D" stroke-width="10"
            d="M56 347Q56 360 70 367H707Q722 359 722 347Q722 336 708 328L390 327H72Q56 332 56 347ZM56 153Q56 168 72 173H708Q722 163 722 153Q722 140 707 133H70Q56 140 56 153Z" />
        <path id="MJMAIN-2260" stroke-width="10"
            d="M166 -215T159 -215T147 -212T141 -204T139 -197Q139 -190 144 -183L306 133H70Q56 140 56 153Q56 168 72 173H327L406 327H72Q56 332 56 347Q56 360 70 367H426Q597 702 602 707Q605 716 618 716Q625 716 630 712T636 703T638 696Q638 692 471 367H707Q722 359 722 347Q722 336 708 328L451 327L371 173H708Q722 163 722 153Q722 140 707 133H351Q175 -210 170 -212Q166 -215 159 -215Z" />
        <path id="MJMAIN-30" stroke-width="10"
            d="M96 585Q152 666 249 666Q297 666 345 640T423 548Q460 465 460 320Q460 165 417 83Q397 41 362 16T301 -15T250 -22Q224 -22 198 -16T137 16T82 83Q39 165 39 320Q39 494 96 585ZM321 597Q291 629 250 629Q208 629 178 597Q153 571 145 525T137 333Q137 175 145 125T181 46Q209 16 250 16Q290 16 318 46Q347 76 354 130T362 333Q362 478 354 524T321 597Z" />
        <path id="MJMAIN-31" stroke-width="10"
            d="M213 578L200 573Q186 568 160 563T102 556H83V602H102Q149 604 189 617T245 641T273 663Q275 666 285 666Q294 666 302 660V361L303 61Q310 54 315 52T339 48T401 46H427V0H416Q395 3 257 3Q121 3 100 0H88V46H114Q136 46 152 46T177 47T193 50T201 52T207 57T213 61V578Z" />
        <path id="MJMAIN-32" stroke-width="10"
            d="M109 429Q82 429 66 447T50 491Q50 562 103 614T235 666Q326 666 387 610T449 465Q449 422 429 383T381 315T301 241Q265 210 201 149L142 93L218 92Q375 92 385 97Q392 99 409 186V189H449V186Q448 183 436 95T421 3V0H50V19V31Q50 38 56 46T86 81Q115 113 136 137Q145 147 170 174T204 211T233 244T261 278T284 308T305 340T320 369T333 401T340 431T343 464Q343 527 309 573T212 619Q179 619 154 602T119 569T109 550Q109 549 114 549Q132 549 151 535T170 489Q170 464 154 447T109 429Z" />
        <path id="MJMAIN-33" stroke-width="10"
            d="M127 463Q100 463 85 480T69 524Q69 579 117 622T233 665Q268 665 277 664Q351 652 390 611T430 522Q430 470 396 421T302 350L299 348Q299 347 308 345T337 336T375 315Q457 262 457 175Q457 96 395 37T238 -22Q158 -22 100 21T42 130Q42 158 60 175T105 193Q133 193 151 175T169 130Q169 119 166 110T159 94T148 82T136 74T126 70T118 67L114 66Q165 21 238 21Q293 21 321 74Q338 107 338 175V195Q338 290 274 322Q259 328 213 329L171 330L168 332Q166 335 166 348Q166 366 174 366Q202 366 232 371Q266 376 294 413T322 525V533Q322 590 287 612Q265 626 240 626Q208 626 181 615T143 592T132 580H135Q138 579 143 578T153 573T165 566T175 555T183 540T186 520Q186 498 172 481T127 463Z" />
        <path id="MJMAIN-34" stroke-width="10"
            d="M462 0Q444 3 333 3Q217 3 199 0H190V46H221Q241 46 248 46T265 48T279 53T286 61Q287 63 287 115V165H28V211L179 442Q332 674 334 675Q336 677 355 677H373L379 671V211H471V165H379V114Q379 73 379 66T385 54Q393 47 442 46H471V0H462ZM293 211V545L74 212L183 211H293Z" />
        <path id="MJMAIN-35" stroke-width="10"
            d="M164 157Q164 133 148 117T109 101H102Q148 22 224 22Q294 22 326 82Q345 115 345 210Q345 313 318 349Q292 382 260 382H254Q176 382 136 314Q132 307 129 306T114 304Q97 304 95 310Q93 314 93 485V614Q93 664 98 664Q100 666 102 666Q103 666 123 658T178 642T253 634Q324 634 389 662Q397 666 402 666Q410 666 410 648V635Q328 538 205 538Q174 538 149 544L139 546V374Q158 388 169 396T205 412T256 420Q337 420 393 355T449 201Q449 109 385 44T229 -22Q148 -22 99 32T50 154Q50 178 61 192T84 210T107 214Q132 214 148 197T164 157Z" />
        <path id="MJMAIN-36" stroke-width="10"
            d="M42 313Q42 476 123 571T303 666Q372 666 402 630T432 550Q432 525 418 510T379 495Q356 495 341 509T326 548Q326 592 373 601Q351 623 311 626Q240 626 194 566Q147 500 147 364L148 360Q153 366 156 373Q197 433 263 433H267Q313 433 348 414Q372 400 396 374T435 317Q456 268 456 210V192Q456 169 451 149Q440 90 387 34T253 -22Q225 -22 199 -14T143 16T92 75T56 172T42 313ZM257 397Q227 397 205 380T171 335T154 278T148 216Q148 133 160 97T198 39Q222 21 251 21Q302 21 329 59Q342 77 347 104T352 209Q352 289 347 316T329 361Q302 397 257 397Z" />
        <path id="MJMAIN-37" stroke-width="10"
            d="M55 458Q56 460 72 567L88 674Q88 676 108 676H128V672Q128 662 143 655T195 646T364 644H485V605L417 512Q408 500 387 472T360 435T339 403T319 367T305 330T292 284T284 230T278 162T275 80Q275 66 275 52T274 28V19Q270 2 255 -10T221 -22Q210 -22 200 -19T179 0T168 40Q168 198 265 368Q285 400 349 489L395 552H302Q128 552 119 546Q113 543 108 522T98 479L95 458V455H55V458Z" />
        <path id="MJMAIN-38" stroke-width="10"
            d="M70 417T70 494T124 618T248 666Q319 666 374 624T429 515Q429 485 418 459T392 417T361 389T335 371T324 363L338 354Q352 344 366 334T382 323Q457 264 457 174Q457 95 399 37T249 -22Q159 -22 101 29T43 155Q43 263 172 335L154 348Q133 361 127 368Q70 417 70 494ZM286 386L292 390Q298 394 301 396T311 403T323 413T334 425T345 438T355 454T364 471T369 491T371 513Q371 556 342 586T275 624Q268 625 242 625Q201 625 165 599T128 534Q128 511 141 492T167 463T217 431Q224 426 228 424L286 386ZM250 21Q308 21 350 55T392 137Q392 154 387 169T375 194T353 216T330 234T301 253T274 270Q260 279 244 289T218 306L210 311Q204 311 181 294T133 239T107 157Q107 98 150 60T250 21Z" />
        <path id="MJMAIN-39" stroke-width="10"
            d="M352 287Q304 211 232 211Q154 211 104 270T44 396Q42 412 42 436V444Q42 537 111 606Q171 666 243 666Q245 666 249 666T257 665H261Q273 665 286 663T323 651T370 619T413 560Q456 472 456 334Q456 194 396 97Q361 41 312 10T208 -22Q147 -22 108 7T68 93T121 149Q143 149 158 135T173 96Q173 78 164 65T148 49T135 44L131 43Q131 41 138 37T164 27T206 22H212Q272 22 313 86Q352 142 352 280V287ZM244 248Q292 248 321 297T351 430Q351 508 343 542Q341 552 337 562T323 588T293 615T246 625Q208 625 181 598Q160 576 154 546T147 441Q147 358 152 329T172 282Q197 248 244 248Z" />
    </defs>
</svg>`;
}

const SVG_LATEX3 = `
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink"
    style="width: 60.138ex; height: 6.621ex; vertical-align: -4.552ex; margin: 1px 0px;"
    viewBox="0 -936.1162627448118 25891.748939099285 2868.381097357651">
    <g stroke="white" fill="white" stroke-width="0" transform="matrix(1 0 0 -1 0 0)">
        <use xlink:href="#MJMAIN-33" xmlns:xlink="http://www.w3.org/1999/xlink" />
        <g transform="translate(505,0)">
            <use xlink:href="#MJMAIN-42" xmlns:xlink="http://www.w3.org/1999/xlink" />
            <use xlink:href="#MJMAIN-61" xmlns:xlink="http://www.w3.org/1999/xlink" x="713" y="0" />
            <g transform="translate(1218,419)">
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-32"
                    xmlns:xlink="http://www.w3.org/1999/xlink" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-2B"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="505" y="0" />
            </g>
        </g>
        <use xlink:href="#MJMAIN-2B" xmlns:xlink="http://www.w3.org/1999/xlink" x="2955" y="0" />
        <use xlink:href="#MJMAIN-36" xmlns:xlink="http://www.w3.org/1999/xlink" x="3961" y="0" />
        <g transform="translate(4466,0)">
            <use xlink:href="#MJMAIN-4F" xmlns:xlink="http://www.w3.org/1999/xlink" />
            <use xlink:href="#MJMAIN-48" xmlns:xlink="http://www.w3.org/1999/xlink" x="783" y="0" />
            <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-2212"
                xmlns:xlink="http://www.w3.org/1999/xlink" x="2175" y="625" />
        </g>
        <use xlink:href="#MJMAIN-2B" xmlns:xlink="http://www.w3.org/1999/xlink" x="6880" y="0" />
        <use xlink:href="#MJMAIN-36" xmlns:xlink="http://www.w3.org/1999/xlink" x="7885" y="0" />
        <g transform="translate(8390,0)">
            <use xlink:href="#MJMAIN-48" xmlns:xlink="http://www.w3.org/1999/xlink" />
            <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-2B"
                xmlns:xlink="http://www.w3.org/1999/xlink" x="1067" y="593" />
        </g>
        <use xlink:href="#MJMAIN-2B" xmlns:xlink="http://www.w3.org/1999/xlink" x="10021" y="0" />
        <g transform="translate(11026,0)">
            <g transform="translate(97,0)">
                <use xlink:href="#MJMAIN-32" xmlns:xlink="http://www.w3.org/1999/xlink" />
                <g transform="translate(505,0)">
                    <use xlink:href="#MJMAIN-50" xmlns:xlink="http://www.w3.org/1999/xlink" />
                    <use xlink:href="#MJMAIN-4F" xmlns:xlink="http://www.w3.org/1999/xlink" x="686"
                        y="0" />
                    <g transform="translate(1469,441)">
                        <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-33"
                            xmlns:xlink="http://www.w3.org/1999/xlink" />
                        <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-2212"
                            xmlns:xlink="http://www.w3.org/1999/xlink" x="505" y="0" />
                    </g>
                    <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-34"
                        xmlns:xlink="http://www.w3.org/1999/xlink" x="2077" y="-350" />
                </g>
                <g transform="translate(12,-778)">
                    <use xlink:href="#MJSZ4-E152" xmlns:xlink="http://www.w3.org/1999/xlink" x="19"
                        y="0" />
                    <g transform="translate(490.58781865576356,0) scale(1.3175637311527062,1)">
                        <use xlink:href="#MJSZ4-E154" xmlns:xlink="http://www.w3.org/1999/xlink" />
                    </g>
                    <g transform="translate(1037,0)">
                        <use xlink:href="#MJSZ4-E151" xmlns:xlink="http://www.w3.org/1999/xlink" />
                        <use xlink:href="#MJSZ4-E150" xmlns:xlink="http://www.w3.org/1999/xlink"
                            x="455" y="0" />
                    </g>
                    <g transform="translate(1953.9645857399003,0) scale(1.3175637311527062,1)">
                        <use xlink:href="#MJSZ4-E154" xmlns:xlink="http://www.w3.org/1999/xlink" />
                    </g>
                    <use xlink:href="#MJSZ4-E153" xmlns:xlink="http://www.w3.org/1999/xlink"
                        x="2505" y="0" />
                </g>
            </g>
            <g transform="translate(0,-1666)">
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-70"
                    xmlns:xlink="http://www.w3.org/1999/xlink" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-68"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="561" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-6F"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="1122" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-73"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="1627" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-70"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="2025" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-68"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="2587" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-61"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="3148" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-74"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="3653" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-65"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="4047" y="0" />
            </g>
        </g>
        <use xlink:href="#MJMAIN-27F6" xmlns:xlink="http://www.w3.org/1999/xlink" x="14483" y="0" />
        <use xlink:href="#MJMAIN-36" xmlns:xlink="http://www.w3.org/1999/xlink" x="16404" y="0" />
        <g transform="translate(16909,0)">
            <use xlink:href="#MJMAIN-48" xmlns:xlink="http://www.w3.org/1999/xlink" />
            <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-32"
                xmlns:xlink="http://www.w3.org/1999/xlink" x="1067" y="-213" />
        </g>
        <use xlink:href="#MJMAIN-4F" xmlns:xlink="http://www.w3.org/1999/xlink" x="18121" y="0" />
        <use xlink:href="#MJMAIN-2B" xmlns:xlink="http://www.w3.org/1999/xlink" x="19126" y="0" />
        <g transform="translate(20131,0)">
            <g transform="translate(65,0)">
                <use xlink:href="#MJMAIN-42" xmlns:xlink="http://www.w3.org/1999/xlink" />
                <use xlink:href="#MJMAIN-61" xmlns:xlink="http://www.w3.org/1999/xlink" x="713"
                    y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-33"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="1722" y="-213" />
                <use xlink:href="#MJMAIN-28" xmlns:xlink="http://www.w3.org/1999/xlink" x="1675"
                    y="0" />
                <g transform="translate(2069,0)">
                    <use xlink:href="#MJMAIN-50" xmlns:xlink="http://www.w3.org/1999/xlink" />
                    <use xlink:href="#MJMAIN-4F" xmlns:xlink="http://www.w3.org/1999/xlink" x="686"
                        y="0" />
                    <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-34"
                        xmlns:xlink="http://www.w3.org/1999/xlink" x="2077" y="-213" />
                </g>
                <g transform="translate(3995,0)">
                    <use xlink:href="#MJMAIN-29" xmlns:xlink="http://www.w3.org/1999/xlink" />
                    <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-32"
                        xmlns:xlink="http://www.w3.org/1999/xlink" x="557" y="-213" />
                </g>
                <use xlink:href="#MJMAIN-2193" xmlns:xlink="http://www.w3.org/1999/xlink" x="5124"
                    y="0" />
                <g transform="translate(12,-783)">
                    <use xlink:href="#MJSZ4-E152" xmlns:xlink="http://www.w3.org/1999/xlink" x="19"
                        y="0" />
                    <g transform="translate(506.327646138544,0) scale(4.465529227708807,1)">
                        <use xlink:href="#MJSZ4-E154" xmlns:xlink="http://www.w3.org/1999/xlink" />
                    </g>
                    <g transform="translate(2359,0)">
                        <use xlink:href="#MJSZ4-E151" xmlns:xlink="http://www.w3.org/1999/xlink" />
                        <use xlink:href="#MJSZ4-E150" xmlns:xlink="http://www.w3.org/1999/xlink"
                            x="455" y="0" />
                    </g>
                    <g transform="translate(3291.849921776243,0) scale(4.465529227708807,1)">
                        <use xlink:href="#MJSZ4-E154" xmlns:xlink="http://www.w3.org/1999/xlink" />
                    </g>
                    <use xlink:href="#MJSZ4-E153" xmlns:xlink="http://www.w3.org/1999/xlink"
                        x="5150" y="0" />
                </g>
            </g>
            <g transform="translate(0,-1672)">
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-62"
                    xmlns:xlink="http://www.w3.org/1999/xlink" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-61"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="561" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-72"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="1066" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-69"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="1463" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-75"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="1746" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-6D"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="2307" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-7E"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="3145" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-70"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="3650" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-68"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="4211" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-6F"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="4772" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-73"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="5277" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-70"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="5676" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-68"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="6237" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-61"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="6798" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-74"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="7303" y="0" />
                <use transform="scale(0.7071067811865476)" xlink:href="#MJMAIN-65"
                    xmlns:xlink="http://www.w3.org/1999/xlink" x="7697" y="0" />
            </g>
        </g>
    </g>
    <defs id="MathJax_SVG_glyphs">
        <path id="MJSZ2-2211" stroke-width="10"
            d="M60 948Q63 950 665 950H1267L1325 815Q1384 677 1388 669H1348L1341 683Q1320 724 1285 761Q1235 809 1174 838T1033 881T882 898T699 902H574H543H251L259 891Q722 258 724 252Q725 250 724 246Q721 243 460 -56L196 -356Q196 -357 407 -357Q459 -357 548 -357T676 -358Q812 -358 896 -353T1063 -332T1204 -283T1307 -196Q1328 -170 1348 -124H1388Q1388 -125 1381 -145T1356 -210T1325 -294L1267 -449L666 -450Q64 -450 61 -448Q55 -446 55 -439Q55 -437 57 -433L590 177Q590 178 557 222T452 366T322 544L56 909L55 924Q55 945 60 948Z" />
        <path id="MJMATHI-69" stroke-width="10"
            d="M184 600Q184 624 203 642T247 661Q265 661 277 649T290 619Q290 596 270 577T226 557Q211 557 198 567T184 600ZM21 287Q21 295 30 318T54 369T98 420T158 442Q197 442 223 419T250 357Q250 340 236 301T196 196T154 83Q149 61 149 51Q149 26 166 26Q175 26 185 29T208 43T235 78T260 137Q263 149 265 151T282 153Q302 153 302 143Q302 135 293 112T268 61T223 11T161 -11Q129 -11 102 10T74 74Q74 91 79 106T122 220Q160 321 166 341T173 380Q173 404 156 404H154Q124 404 99 371T61 287Q60 286 59 284T58 281T56 279T53 278T49 278T41 278H27Q21 284 21 287Z" />
        <path id="MJMAIN-3D" stroke-width="10"
            d="M56 347Q56 360 70 367H707Q722 359 722 347Q722 336 708 328L390 327H72Q56 332 56 347ZM56 153Q56 168 72 173H708Q722 163 722 153Q722 140 707 133H70Q56 140 56 153Z" />
        <path id="MJMAIN-30" stroke-width="10"
            d="M96 585Q152 666 249 666Q297 666 345 640T423 548Q460 465 460 320Q460 165 417 83Q397 41 362 16T301 -15T250 -22Q224 -22 198 -16T137 16T82 83Q39 165 39 320Q39 494 96 585ZM321 597Q291 629 250 629Q208 629 178 597Q153 571 145 525T137 333Q137 175 145 125T181 46Q209 16 250 16Q290 16 318 46Q347 76 354 130T362 333Q362 478 354 524T321 597Z" />
        <path id="MJMATHI-6E" stroke-width="10"
            d="M21 287Q22 293 24 303T36 341T56 388T89 425T135 442Q171 442 195 424T225 390T231 369Q231 367 232 367L243 378Q304 442 382 442Q436 442 469 415T503 336T465 179T427 52Q427 26 444 26Q450 26 453 27Q482 32 505 65T540 145Q542 153 560 153Q580 153 580 145Q580 144 576 130Q568 101 554 73T508 17T439 -10Q392 -10 371 17T350 73Q350 92 386 193T423 345Q423 404 379 404H374Q288 404 229 303L222 291L189 157Q156 26 151 16Q138 -11 108 -11Q95 -11 87 -5T76 7T74 17Q74 30 112 180T152 343Q153 348 153 366Q153 405 129 405Q91 405 66 305Q60 285 60 284Q58 278 41 278H27Q21 284 21 287Z" />
        <path id="MJMAIN-28" stroke-width="10"
            d="M94 250Q94 319 104 381T127 488T164 576T202 643T244 695T277 729T302 750H315H319Q333 750 333 741Q333 738 316 720T275 667T226 581T184 443T167 250T184 58T225 -81T274 -167T316 -220T333 -241Q333 -250 318 -250H315H302L274 -226Q180 -141 137 -14T94 250Z" />
        <path id="MJMAIN-2B" stroke-width="10"
            d="M56 237T56 250T70 270H369V420L370 570Q380 583 389 583Q402 583 409 568V270H707Q722 262 722 250T707 230H409V-68Q401 -82 391 -82H389H387Q375 -82 369 -68V230H70Q56 237 56 250Z" />
        <path id="MJMAIN-31" stroke-width="10"
            d="M213 578L200 573Q186 568 160 563T102 556H83V602H102Q149 604 189 617T245 641T273 663Q275 666 285 666Q294 666 302 660V361L303 61Q310 54 315 52T339 48T401 46H427V0H416Q395 3 257 3Q121 3 100 0H88V46H114Q136 46 152 46T177 47T193 50T201 52T207 57T213 61V578Z" />
        <path id="MJMAIN-29" stroke-width="10"
            d="M60 749L64 750Q69 750 74 750H86L114 726Q208 641 251 514T294 250Q294 182 284 119T261 12T224 -76T186 -143T145 -194T113 -227T90 -246Q87 -249 86 -250H74Q66 -250 63 -250T58 -247T55 -238Q56 -237 66 -225Q221 -64 221 250T66 725Q56 737 55 738Q55 746 60 749Z" />
        <path id="MJMAIN-32" stroke-width="10"
            d="M109 429Q82 429 66 447T50 491Q50 562 103 614T235 666Q326 666 387 610T449 465Q449 422 429 383T381 315T301 241Q265 210 201 149L142 93L218 92Q375 92 385 97Q392 99 409 186V189H449V186Q448 183 436 95T421 3V0H50V19V31Q50 38 56 46T86 81Q115 113 136 137Q145 147 170 174T204 211T233 244T261 278T284 308T305 340T320 369T333 401T340 431T343 464Q343 527 309 573T212 619Q179 619 154 602T119 569T109 550Q109 549 114 549Q132 549 151 535T170 489Q170 464 154 447T109 429Z" />
        <path id="MJMATHI-61" stroke-width="10"
            d="M33 157Q33 258 109 349T280 441Q331 441 370 392Q386 422 416 422Q429 422 439 414T449 394Q449 381 412 234T374 68Q374 43 381 35T402 26Q411 27 422 35Q443 55 463 131Q469 151 473 152Q475 153 483 153H487Q506 153 506 144Q506 138 501 117T481 63T449 13Q436 0 417 -8Q409 -10 393 -10Q359 -10 336 5T306 36L300 51Q299 52 296 50Q294 48 292 46Q233 -10 172 -10Q117 -10 75 30T33 157ZM351 328Q351 334 346 350T323 385T277 405Q242 405 210 374T160 293Q131 214 119 129Q119 126 119 118T118 106Q118 61 136 44T179 26Q217 26 254 59T298 110Q300 114 325 217T351 328Z" />
        <path id="MJMAIN-2260" stroke-width="10"
            d="M166 -215T159 -215T147 -212T141 -204T139 -197Q139 -190 144 -183L306 133H70Q56 140 56 153Q56 168 72 173H327L406 327H72Q56 332 56 347Q56 360 70 367H426Q597 702 602 707Q605 716 618 716Q625 716 630 712T636 703T638 696Q638 692 471 367H707Q722 359 722 347Q722 336 708 328L451 327L371 173H708Q722 163 722 153Q722 140 707 133H351Q175 -210 170 -212Q166 -215 159 -215Z" />
        <path id="MJMATHI-78" stroke-width="10"
            d="M52 289Q59 331 106 386T222 442Q257 442 286 424T329 379Q371 442 430 442Q467 442 494 420T522 361Q522 332 508 314T481 292T458 288Q439 288 427 299T415 328Q415 374 465 391Q454 404 425 404Q412 404 406 402Q368 386 350 336Q290 115 290 78Q290 50 306 38T341 26Q378 26 414 59T463 140Q466 150 469 151T485 153H489Q504 153 504 145Q504 144 502 134Q486 77 440 33T333 -11Q263 -11 227 52Q186 -10 133 -10H127Q78 -10 57 16T35 71Q35 103 54 123T99 143Q142 143 142 101Q142 81 130 66T107 46T94 41L91 40Q91 39 97 36T113 29T132 26Q168 26 194 71Q203 87 217 139T245 247T261 313Q266 340 266 352Q266 380 251 392T217 404Q177 404 142 372T93 290Q91 281 88 280T72 278H58Q52 284 52 289Z" />
        <path id="MJMATHI-62" stroke-width="10"
            d="M73 647Q73 657 77 670T89 683Q90 683 161 688T234 694Q246 694 246 685T212 542Q204 508 195 472T180 418L176 399Q176 396 182 402Q231 442 283 442Q345 442 383 396T422 280Q422 169 343 79T173 -11Q123 -11 82 27T40 150V159Q40 180 48 217T97 414Q147 611 147 623T109 637Q104 637 101 637H96Q86 637 83 637T76 640T73 647ZM336 325V331Q336 405 275 405Q258 405 240 397T207 376T181 352T163 330L157 322L136 236Q114 150 114 114Q114 66 138 42Q154 26 178 26Q211 26 245 58Q270 81 285 114T318 219Q336 291 336 325Z" />
        <path id="MJMATHI-63" stroke-width="10"
            d="M34 159Q34 268 120 355T306 442Q362 442 394 418T427 355Q427 326 408 306T360 285Q341 285 330 295T319 325T330 359T352 380T366 386H367Q367 388 361 392T340 400T306 404Q276 404 249 390Q228 381 206 359Q162 315 142 235T121 119Q121 73 147 50Q169 26 205 26H209Q321 26 394 111Q403 121 406 121Q410 121 419 112T429 98T420 83T391 55T346 25T282 0T202 -11Q127 -11 81 37T34 159Z" />
        <path id="MJMAIN-2212" stroke-width="10"
            d="M84 237T84 250T98 270H679Q694 262 694 250T679 230H98Q84 237 84 250Z" />
        <path id="MJMAIN-B1" stroke-width="10"
            d="M56 320T56 333T70 353H369V502Q369 651 371 655Q376 666 388 666Q402 666 405 654T409 596V500V353H707Q722 345 722 333Q722 320 707 313H409V40H707Q722 32 722 20T707 0H70Q56 7 56 20T70 40H369V313H70Q56 320 56 333Z" />
        <path id="MJMAIN-34" stroke-width="10"
            d="M462 0Q444 3 333 3Q217 3 199 0H190V46H221Q241 46 248 46T265 48T279 53T286 61Q287 63 287 115V165H28V211L179 442Q332 674 334 675Q336 677 355 677H373L379 671V211H471V165H379V114Q379 73 379 66T385 54Q393 47 442 46H471V0H462ZM293 211V545L74 212L183 211H293Z" />
        <path id="MJMAIN-221A" stroke-width="10"
            d="M95 178Q89 178 81 186T72 200T103 230T169 280T207 309Q209 311 212 311H213Q219 311 227 294T281 177Q300 134 312 108L397 -77Q398 -77 501 136T707 565T814 786Q820 800 834 800Q841 800 846 794T853 782V776L620 293L385 -193Q381 -200 366 -200Q357 -200 354 -197Q352 -195 256 15L160 225L144 214Q129 202 113 190T95 178Z" />
        <path id="MJMAIN-2E" stroke-width="10"
            d="M78 60Q78 84 95 102T138 120Q162 120 180 104T199 61Q199 36 182 18T139 0T96 17T78 60Z" />
        <path id="MJMAIN-43" stroke-width="10"
            d="M56 342Q56 428 89 500T174 615T283 681T391 705Q394 705 400 705T408 704Q499 704 569 636L582 624L612 663Q639 700 643 704Q644 704 647 704T653 705H657Q660 705 666 699V419L660 413H626Q620 419 619 430Q610 512 571 572T476 651Q457 658 426 658Q322 658 252 588Q173 509 173 342Q173 221 211 151Q232 111 263 84T328 45T384 29T428 24Q517 24 571 93T626 244Q626 251 632 257H660L666 251V236Q661 133 590 56T403 -21Q262 -21 159 83T56 342Z" />
        <path id="MJMAIN-48" stroke-width="10"
            d="M128 622Q121 629 117 631T101 634T58 637H25V683H36Q57 680 180 680Q315 680 324 683H335V637H302Q262 636 251 634T233 622L232 500V378H517V622Q510 629 506 631T490 634T447 637H414V683H425Q446 680 569 680Q704 680 713 683H724V637H691Q651 636 640 634T622 622V61Q628 51 639 49T691 46H724V0H713Q692 3 569 3Q434 3 425 0H414V46H447Q489 47 498 49T517 61V332H232V197L233 61Q239 51 250 49T302 46H335V0H324Q303 3 180 3Q45 3 36 0H25V46H58Q100 47 109 49T128 61V622Z" />
        <path id="MJMAIN-4F" stroke-width="10"
            d="M56 340Q56 423 86 494T164 610T270 680T388 705Q521 705 621 601T722 341Q722 260 693 191T617 75T510 4T388 -22T267 3T160 74T85 189T56 340ZM467 647Q426 665 388 665Q360 665 331 654T269 620T213 549T179 439Q174 411 174 354Q174 144 277 61Q327 20 385 20H389H391Q474 20 537 99Q603 188 603 354Q603 411 598 439Q577 592 467 647Z" />
        <path id="MJMAIN-27F6" stroke-width="10"
            d="M84 237T84 250T98 270H1444Q1328 357 1301 493Q1301 494 1301 496T1300 499Q1300 511 1317 511H1320Q1329 511 1332 510T1338 506T1341 497T1344 481T1352 456Q1374 389 1425 336T1544 261Q1553 258 1553 250Q1553 244 1548 241T1524 231T1486 212Q1445 186 1415 152T1370 85T1349 35T1341 4Q1339 -6 1336 -8T1320 -11Q1300 -11 1300 0Q1300 7 1305 25Q1337 151 1444 230H98Q84 237 84 250Z" />
        <path id="MJMAIN-33" stroke-width="10"
            d="M127 463Q100 463 85 480T69 524Q69 579 117 622T233 665Q268 665 277 664Q351 652 390 611T430 522Q430 470 396 421T302 350L299 348Q299 347 308 345T337 336T375 315Q457 262 457 175Q457 96 395 37T238 -22Q158 -22 100 21T42 130Q42 158 60 175T105 193Q133 193 151 175T169 130Q169 119 166 110T159 94T148 82T136 74T126 70T118 67L114 66Q165 21 238 21Q293 21 321 74Q338 107 338 175V195Q338 290 274 322Q259 328 213 329L171 330L168 332Q166 335 166 348Q166 366 174 366Q202 366 232 371Q266 376 294 413T322 525V533Q322 590 287 612Q265 626 240 626Q208 626 181 615T143 592T132 580H135Q138 579 143 578T153 573T165 566T175 555T183 540T186 520Q186 498 172 481T127 463Z" />
        <path id="MJMAIN-42" stroke-width="10"
            d="M131 622Q124 629 120 631T104 634T61 637H28V683H229H267H346Q423 683 459 678T531 651Q574 627 599 590T624 512Q624 461 583 419T476 360L466 357Q539 348 595 302T651 187Q651 119 600 67T469 3Q456 1 242 0H28V46H61Q103 47 112 49T131 61V622ZM511 513Q511 560 485 594T416 636Q415 636 403 636T371 636T333 637Q266 637 251 636T232 628Q229 624 229 499V374H312L396 375L406 377Q410 378 417 380T442 393T474 417T499 456T511 513ZM537 188Q537 239 509 282T430 336L329 337H229V200V116Q229 57 234 52Q240 47 334 47H383Q425 47 443 53Q486 67 511 104T537 188Z" />
        <path id="MJMAIN-61" stroke-width="10"
            d="M137 305T115 305T78 320T63 359Q63 394 97 421T218 448Q291 448 336 416T396 340Q401 326 401 309T402 194V124Q402 76 407 58T428 40Q443 40 448 56T453 109V145H493V106Q492 66 490 59Q481 29 455 12T400 -6T353 12T329 54V58L327 55Q325 52 322 49T314 40T302 29T287 17T269 6T247 -2T221 -8T190 -11Q130 -11 82 20T34 107Q34 128 41 147T68 188T116 225T194 253T304 268H318V290Q318 324 312 340Q290 411 215 411Q197 411 181 410T156 406T148 403Q170 388 170 359Q170 334 154 320ZM126 106Q126 75 150 51T209 26Q247 26 276 49T315 109Q317 116 318 175Q318 233 317 233Q309 233 296 232T251 223T193 203T147 166T126 106Z" />
        <path id="MJMAIN-36" stroke-width="10"
            d="M42 313Q42 476 123 571T303 666Q372 666 402 630T432 550Q432 525 418 510T379 495Q356 495 341 509T326 548Q326 592 373 601Q351 623 311 626Q240 626 194 566Q147 500 147 364L148 360Q153 366 156 373Q197 433 263 433H267Q313 433 348 414Q372 400 396 374T435 317Q456 268 456 210V192Q456 169 451 149Q440 90 387 34T253 -22Q225 -22 199 -14T143 16T92 75T56 172T42 313ZM257 397Q227 397 205 380T171 335T154 278T148 216Q148 133 160 97T198 39Q222 21 251 21Q302 21 329 59Q342 77 347 104T352 209Q352 289 347 316T329 361Q302 397 257 397Z" />
        <path id="MJMAIN-50" stroke-width="10"
            d="M130 622Q123 629 119 631T103 634T60 637H27V683H214Q237 683 276 683T331 684Q419 684 471 671T567 616Q624 563 624 489Q624 421 573 372T451 307Q429 302 328 301H234V181Q234 62 237 58Q245 47 304 46H337V0H326Q305 3 182 3Q47 3 38 0H27V46H60Q102 47 111 49T130 61V622ZM507 488Q507 514 506 528T500 564T483 597T450 620T397 635Q385 637 307 637H286Q237 637 234 628Q231 624 231 483V342H302H339Q390 342 423 349T481 382Q507 411 507 488Z" />
        <path id="MJSZ4-E152" stroke-width="10"
            d="M-24 327L-18 333H-1Q11 333 15 333T22 329T27 322T35 308T54 284Q115 203 225 162T441 120Q454 120 457 117T460 95V60V28Q460 8 457 4T442 0Q355 0 260 36Q75 118 -16 278L-24 292V327Z" />
        <path id="MJSZ4-E153" stroke-width="10"
            d="M-10 60V95Q-10 113 -7 116T9 120Q151 120 250 171T396 284Q404 293 412 305T424 324T431 331Q433 333 451 333H468L474 327V292L466 278Q375 118 190 36Q95 0 8 0Q-5 0 -7 3T-10 24V60Z" />
        <path id="MJSZ4-E151" stroke-width="10"
            d="M-10 60Q-10 104 -10 111T-5 118Q-1 120 10 120Q96 120 190 84Q375 2 466 -158L474 -172V-207L468 -213H451H447Q437 -213 434 -213T428 -209T423 -202T414 -187T396 -163Q331 -82 224 -41T9 0Q-4 0 -7 3T-10 25V60Z" />
        <path id="MJSZ4-E150" stroke-width="10"
            d="M-18 -213L-24 -207V-172L-16 -158Q75 2 260 84Q334 113 415 119Q418 119 427 119T440 120Q454 120 457 117T460 98V60V25Q460 7 457 4T441 0Q308 0 193 -55T25 -205Q21 -211 18 -212T-1 -213H-18Z" />
        <path id="MJSZ4-E154" stroke-width="10" d="M-10 0V120H410V0H-10Z" />
        <path id="MJMAIN-70" stroke-width="10"
            d="M36 -148H50Q89 -148 97 -134V-126Q97 -119 97 -107T97 -77T98 -38T98 6T98 55T98 106Q98 140 98 177T98 243T98 296T97 335T97 351Q94 370 83 376T38 385H20V408Q20 431 22 431L32 432Q42 433 61 434T98 436Q115 437 135 438T165 441T176 442H179V416L180 390L188 397Q247 441 326 441Q407 441 464 377T522 216Q522 115 457 52T310 -11Q242 -11 190 33L182 40V-45V-101Q182 -128 184 -134T195 -145Q216 -148 244 -148H260V-194H252L228 -193Q205 -192 178 -192T140 -191Q37 -191 28 -194H20V-148H36ZM424 218Q424 292 390 347T305 402Q234 402 182 337V98Q222 26 294 26Q345 26 384 80T424 218Z" />
        <path id="MJMAIN-68" stroke-width="10"
            d="M41 46H55Q94 46 102 60V68Q102 77 102 91T102 124T102 167T103 217T103 272T103 329Q103 366 103 407T103 482T102 542T102 586T102 603Q99 622 88 628T43 637H25V660Q25 683 27 683L37 684Q47 685 66 686T103 688Q120 689 140 690T170 693T181 694H184V367Q244 442 328 442Q451 442 463 329Q464 322 464 190V104Q464 66 466 59T477 49Q498 46 526 46H542V0H534L510 1Q487 2 460 2T422 3Q319 3 310 0H302V46H318Q379 46 379 62Q380 64 380 200Q379 335 378 343Q372 371 358 385T334 402T308 404Q263 404 229 370Q202 343 195 315T187 232V168V108Q187 78 188 68T191 55T200 49Q221 46 249 46H265V0H257L234 1Q210 2 183 2T145 3Q42 3 33 0H25V46H41Z" />
        <path id="MJMAIN-6F" stroke-width="10"
            d="M28 214Q28 309 93 378T250 448Q340 448 405 380T471 215Q471 120 407 55T250 -10Q153 -10 91 57T28 214ZM250 30Q372 30 372 193V225V250Q372 272 371 288T364 326T348 362T317 390T268 410Q263 411 252 411Q222 411 195 399Q152 377 139 338T126 246V226Q126 130 145 91Q177 30 250 30Z" />
        <path id="MJMAIN-73" stroke-width="10"
            d="M295 316Q295 356 268 385T190 414Q154 414 128 401Q98 382 98 349Q97 344 98 336T114 312T157 287Q175 282 201 278T245 269T277 256Q294 248 310 236T342 195T359 133Q359 71 321 31T198 -10H190Q138 -10 94 26L86 19L77 10Q71 4 65 -1L54 -11H46H42Q39 -11 33 -5V74V132Q33 153 35 157T45 162H54Q66 162 70 158T75 146T82 119T101 77Q136 26 198 26Q295 26 295 104Q295 133 277 151Q257 175 194 187T111 210Q75 227 54 256T33 318Q33 357 50 384T93 424T143 442T187 447H198Q238 447 268 432L283 424L292 431Q302 440 314 448H322H326Q329 448 335 442V310L329 304H301Q295 310 295 316Z" />
        <path id="MJMAIN-74" stroke-width="10"
            d="M27 422Q80 426 109 478T141 600V615H181V431H316V385H181V241Q182 116 182 100T189 68Q203 29 238 29Q282 29 292 100Q293 108 293 146V181H333V146V134Q333 57 291 17Q264 -10 221 -10Q187 -10 162 2T124 33T105 68T98 100Q97 107 97 248V385H18V422H27Z" />
        <path id="MJMAIN-65" stroke-width="10"
            d="M28 218Q28 273 48 318T98 391T163 433T229 448Q282 448 320 430T378 380T406 316T415 245Q415 238 408 231H126V216Q126 68 226 36Q246 30 270 30Q312 30 342 62Q359 79 369 104L379 128Q382 131 395 131H398Q415 131 415 121Q415 117 412 108Q393 53 349 21T250 -11Q155 -11 92 58T28 218ZM333 275Q322 403 238 411H236Q228 411 220 410T195 402T166 381T143 340T127 274V267H333V275Z" />
        <path id="MJMAIN-2193" stroke-width="10"
            d="M473 86Q483 86 483 67Q483 63 483 61T483 56T481 53T480 50T478 48T474 47T470 46T464 44Q428 35 391 14T316 -55T264 -168Q264 -170 263 -173T262 -180T261 -184Q259 -194 251 -194Q242 -194 238 -176T221 -121T180 -49Q169 -34 155 -21T125 2T95 20T67 33T44 42T27 47L21 49Q17 53 17 67Q17 87 28 87Q33 87 42 84Q158 52 223 -45L230 -55V312Q230 391 230 482T229 591Q229 662 231 676T243 693Q244 694 251 694Q264 692 270 679V-55L277 -45Q307 1 353 33T430 76T473 86Z" />
        <path id="MJMAIN-62" stroke-width="10"
            d="M307 -11Q234 -11 168 55L158 37Q156 34 153 28T147 17T143 10L138 1L118 0H98V298Q98 599 97 603Q94 622 83 628T38 637H20V660Q20 683 22 683L32 684Q42 685 61 686T98 688Q115 689 135 690T165 693T176 694H179V543Q179 391 180 391L183 394Q186 397 192 401T207 411T228 421T254 431T286 439T323 442Q401 442 461 379T522 216Q522 115 458 52T307 -11ZM182 98Q182 97 187 90T196 79T206 67T218 55T233 44T250 35T271 29T295 26Q330 26 363 46T412 113Q424 148 424 212Q424 287 412 323Q385 405 300 405Q270 405 239 390T188 347L182 339V98Z" />
        <path id="MJMAIN-72" stroke-width="10"
            d="M36 46H50Q89 46 97 60V68Q97 77 97 91T98 122T98 161T98 203Q98 234 98 269T98 328L97 351Q94 370 83 376T38 385H20V408Q20 431 22 431L32 432Q42 433 60 434T96 436Q112 437 131 438T160 441T171 442H174V373Q213 441 271 441H277Q322 441 343 419T364 373Q364 352 351 337T313 322Q288 322 276 338T263 372Q263 381 265 388T270 400T273 405Q271 407 250 401Q234 393 226 386Q179 341 179 207V154Q179 141 179 127T179 101T180 81T180 66V61Q181 59 183 57T188 54T193 51T200 49T207 48T216 47T225 47T235 46T245 46H276V0H267Q249 3 140 3Q37 3 28 0H20V46H36Z" />
        <path id="MJMAIN-69" stroke-width="10"
            d="M69 609Q69 637 87 653T131 669Q154 667 171 652T188 609Q188 579 171 564T129 549Q104 549 87 564T69 609ZM247 0Q232 3 143 3Q132 3 106 3T56 1L34 0H26V46H42Q70 46 91 49Q100 53 102 60T104 102V205V293Q104 345 102 359T88 378Q74 385 41 385H30V408Q30 431 32 431L42 432Q52 433 70 434T106 436Q123 437 142 438T171 441T182 442H185V62Q190 52 197 50T232 46H255V0H247Z" />
        <path id="MJMAIN-75" stroke-width="10"
            d="M383 58Q327 -10 256 -10H249Q124 -10 105 89Q104 96 103 226Q102 335 102 348T96 369Q86 385 36 385H25V408Q25 431 27 431L38 432Q48 433 67 434T105 436Q122 437 142 438T172 441T184 442H187V261Q188 77 190 64Q193 49 204 40Q224 26 264 26Q290 26 311 35T343 58T363 90T375 120T379 144Q379 145 379 161T380 201T380 248V315Q380 361 370 372T320 385H302V431Q304 431 378 436T457 442H464V264Q464 84 465 81Q468 61 479 55T524 46H542V0Q540 0 467 -5T390 -11H383V58Z" />
        <path id="MJMAIN-6D" stroke-width="10"
            d="M41 46H55Q94 46 102 60V68Q102 77 102 91T102 122T103 161T103 203Q103 234 103 269T102 328V351Q99 370 88 376T43 385H25V408Q25 431 27 431L37 432Q47 433 65 434T102 436Q119 437 138 438T167 441T178 442H181V402Q181 364 182 364T187 369T199 384T218 402T247 421T285 437Q305 442 336 442Q351 442 364 440T387 434T406 426T421 417T432 406T441 395T448 384T452 374T455 366L457 361L460 365Q463 369 466 373T475 384T488 397T503 410T523 422T546 432T572 439T603 442Q729 442 740 329Q741 322 741 190V104Q741 66 743 59T754 49Q775 46 803 46H819V0H811L788 1Q764 2 737 2T699 3Q596 3 587 0H579V46H595Q656 46 656 62Q657 64 657 200Q656 335 655 343Q649 371 635 385T611 402T585 404Q540 404 506 370Q479 343 472 315T464 232V168V108Q464 78 465 68T468 55T477 49Q498 46 526 46H542V0H534L510 1Q487 2 460 2T422 3Q319 3 310 0H302V46H318Q379 46 379 62Q380 64 380 200Q379 335 378 343Q372 371 358 385T334 402T308 404Q263 404 229 370Q202 343 195 315T187 232V168V108Q187 78 188 68T191 55T200 49Q221 46 249 46H265V0H257L234 1Q210 2 183 2T145 3Q42 3 33 0H25V46H41Z" />
        <path id="MJMAIN-7E" stroke-width="10"
            d="M179 251Q164 251 151 245T131 234T111 215L97 227L83 238Q83 239 95 253T121 283T142 304Q165 318 187 318T253 300T320 282Q335 282 348 288T368 299T388 318L402 306L416 295Q375 236 344 222Q330 215 313 215Q292 215 248 233T179 251Z" />
    </defs>
</svg>
`;
// ─── Demo List ───────────────────────────────────────

const DEMOS: { name: string; svg?: string; animated?: boolean; generator?: () => string }[] = [
  { name: 'Basic Shapes', svg: SVG_BASIC_SHAPES },
  { name: 'Polygon & Polyline', svg: SVG_POLYGON_POLYLINE },
  { name: 'Path Lines', svg: SVG_PATH_LINES },
  { name: 'Path Curves', svg: SVG_PATH_CURVES },
  { name: 'Path Arcs', svg: SVG_PATH_ARCS },
  { name: 'Transforms', svg: SVG_TRANSFORMS },
  { name: 'Groups', svg: SVG_GROUPS },
  { name: 'Defs & Use', svg: SVG_DEFS_USE },
  { name: 'Stroke', svg: SVG_STROKE },
  { name: 'Opacity', svg: SVG_OPACITY },
  { name: 'Style Attribute', svg: SVG_STYLE_ATTR },
  { name: 'Have a Nice Day Logo', svg: SVG_HAVE_A_NICE_DAY_LOGO },
  { name: 'Fancy Text SVG', svg: SVG_FANCY_TEXT },
  { name: 'Block Text HELLO', svg: SVG_BLOCK_TEXT },
  { name: 'Analog Clock', animated: true },
  { name: 'Sinc Plot Chart', svg: SVG_SINC_CHART },
  { name: 'Heatmap', generator: buildHeatmapSVG },
  { name: 'Tiger Face', svg: SVG_TIGER },
  { name: 'SVG Logo', svg: SVG_LOGO },
  { name: 'SVG LATEX1', svg: SVG_LATEX1 },
  { name: 'SVG LATEX2', animated: true },
  { name: 'SVG LATEX3', svg: SVG_LATEX3 },
];

// ─── Demo Component ──────────────────────────────────

@component
export default class SpaceSVGDemo extends BaseScriptComponent {

  @input
  @hint('Unlit material for mesh rendering (e.g., LegitUnlit)')
  material!: Material;

  @input
  @hint('World-space width')
  worldWidth: number = 20;

  @input
  @hint('World-space height')
  worldHeight: number = 20;

  @input
  @hint('Which demo to show (0-based index)')
  demoIndex: number = 0;

  @input
  @hint('Seconds between demos when cycling (0 to disable)')
  cycleInterval: number = 0;

  @input
  @hint('Optional: Button prefab from SpectaclesUIKit (e.g., FrameButton) for prev/next navigation')
  buttonPrefab: ObjectPrefab;

  private parser = new SVGXMLParser();
  private backend = new SpaceSVGMeshBackend();
  private meshObjects: SceneObject[] = [];
  private currentDemo: number = 0;
  private timer: number = 0;
  private animTimer: number = 0;
  private isAnimatedDemo: boolean = false;
  private countdownValue: number = 10;
  private countdownDone: boolean = false;
  private countdownFrames: SceneObject[][] = [];
  private currentCountdownFrame: number = -1;
  private labelText: Text | null = null;
  private instructionText: Text | null = null;
  private instructionObj: SceneObject | null = null;
  private navObjects: SceneObject[] = [];

  onAwake() {
    print('[SpaceSVGDemo] onAwake');

    if (!this.material) {
      print('[SpaceSVGDemo] ERROR: No material assigned.');
      return;
    }

    this.currentDemo = Math.max(0, Math.min(this.demoIndex, DEMOS.length - 1));
    this.renderDemo(this.currentDemo);
    this.setupNavigation();

    this.createEvent('UpdateEvent').bind((ev: UpdateEvent) => {
      const dt = ev.getDeltaTime();
      this.timer += dt;

      // Handle animated demos — tick once per second via elapsed time
      if (this.isAnimatedDemo) {
        this.animTimer += dt;
        if (this.animTimer >= 1.0) {
          this.animTimer -= 1.0;
          this.renderAnimatedFrame();
        }
      }

      // Handle demo cycling
      if (this.cycleInterval > 0 && this.timer >= this.cycleInterval) {
        this.timer = 0;
        this.currentDemo = (this.currentDemo + 1) % DEMOS.length;
        this.renderDemo(this.currentDemo);
        this.updateLabel();
      }
    });
  }

  private renderDemo(index: number): void {
    const demo = DEMOS[index % DEMOS.length];
    print(`[SpaceSVGDemo] Rendering: ${demo.name}`);
    this.clearMeshes();
    this.isAnimatedDemo = !!demo.animated;

    if (demo.animated) {
      this.animTimer = 0;
      if (demo.name === 'SVG LATEX2') {
        this.countdownValue = 10;
        this.countdownDone = false;
        this.showCountdownFrame(this.countdownValue);
        this.showInstruction(this.countdownValue);
        return;
      }
      this.renderAnimatedFrame();
    } else {
      this.hideInstruction();
      if (demo.generator) {
        this.renderSVG(demo.generator(), demo.name);
      } else if (demo.svg) {
        this.renderSVG(demo.svg, demo.name);
      }
    }
  }

  private renderAnimatedFrame(): void {
    const demo = DEMOS[this.currentDemo];

    if (demo.name === 'Analog Clock') {
      this.hideInstruction();
      this.clearMeshes();
      const now = new Date();
      const svg = buildClockSVG(now.getHours(), now.getMinutes(), now.getSeconds());
      this.renderSVG(svg, 'Clock');
    } else if (demo.name === 'SVG LATEX2') {
      if (this.countdownDone) return;

      if (this.countdownValue > 0) {
        this.countdownValue--;
        this.showCountdownFrame(this.countdownValue);
        this.showInstruction(this.countdownValue);
      } else {
        this.countdownDone = true;
        this.isAnimatedDemo = false;
        print('[SpaceSVGDemo] LATEX2 countdown complete');
      }
    }
  }

  // ─── Instruction Text (native Text component) ──────

  private showInstruction(value: number): void {
    const sentence = `Solve the expression for a when a equals ${value}`;
    if (!this.instructionObj) {
      this.instructionObj = global.scene.createSceneObject('InstructionText');
      this.instructionObj.setParent(this.getSceneObject());
      // Position above the SVG content
      this.instructionObj.getTransform().setLocalPosition(new vec3(0, this.worldHeight / 2 + 2, 0));
      const textComp = this.instructionObj.createComponent('Component.Text') as Text;
      try { (textComp as any).textFill.color = new vec4(0.6, 0.6, 0.6, 1); } catch (e) { /* ok */ }
      this.instructionText = textComp;
    }
    if (this.instructionText) {
      this.instructionText.text = sentence;
    }
    this.instructionObj.enabled = true;
  }

  private hideInstruction(): void {
    if (this.instructionObj) {
      this.instructionObj.enabled = false;
    }
  }

  // ─── Lazy Countdown Frames (built one per tick) ─────

  // Build a single frame on demand and cache it. Returns the frame's scene objects.
  private buildCountdownFrame(value: number): SceneObject[] {
    const frameIdx = 10 - value;
    // Return cached frame if already built
    if (this.countdownFrames[frameIdx]) return this.countdownFrames[frameIdx];

    try {
      const svg = buildLatexCountdownSVG(value);
      const tree = this.parser.parse(svg);
      const groups = this.backend.buildMeshes(tree, this.worldWidth, this.worldHeight);
      const frameObjects: SceneObject[] = [];
      for (let i = 0; i < groups.length; i++) {
        const obj = this.buildMeshObject(groups[i], i);
        if (obj) {
          obj.enabled = false;
          frameObjects.push(obj);
        }
      }
      this.countdownFrames[frameIdx] = frameObjects;
      return frameObjects;
    } catch (e: any) {
      print(`[SpaceSVGDemo] Error building countdown frame ${value}: ${e.message || e}`);
      this.countdownFrames[frameIdx] = [];
      return [];
    }
  }

  private showCountdownFrame(value: number): void {
    const frameIdx = 10 - value;

    // Hide previous frame
    if (this.currentCountdownFrame >= 0 && this.countdownFrames[this.currentCountdownFrame]) {
      for (const obj of this.countdownFrames[this.currentCountdownFrame]) {
        obj.enabled = false;
      }
    }

    // Build (if needed) and show new frame
    const objects = this.buildCountdownFrame(value);
    for (const obj of objects) {
      obj.enabled = true;
    }
    this.currentCountdownFrame = frameIdx;
  }

  private clearCountdownFrames(): void {
    for (const frame of this.countdownFrames) {
      if (!frame) continue;
      for (const obj of frame) obj.destroy();
    }
    this.countdownFrames = [];
    this.currentCountdownFrame = -1;
  }

  private renderSVG(svg: string, label: string): void {
    try {
      const tree = this.parser.parse(svg);
      const groups = this.backend.buildMeshes(tree, this.worldWidth, this.worldHeight);
      print(`[SpaceSVGDemo] ${label}: ${groups.length} mesh groups`);

      for (let i = 0; i < groups.length; i++) {
        this.createMeshFromGroup(groups[i], i);
      }
    } catch (e: any) {
      print(`[SpaceSVGDemo] Error: ${e.message || e}`);
    }
  }

  private buildMeshObject(group: { vertices: number[]; indices: number[]; color: number[] }, index: number): SceneObject | null {
    const builder = new MeshBuilder([
      { name: 'position', components: 3 },
    ]);
    builder.topology = MeshTopology.Triangles;
    builder.indexType = MeshIndexType.UInt16;

    builder.appendVerticesInterleaved(group.vertices);
    builder.appendIndices(group.indices);

    if (!builder.isValid()) {
      print(`[SpaceSVGDemo] Warning: invalid mesh group ${index}`);
      return null;
    }

    builder.updateMesh();
    const mesh = builder.getMesh();

    const obj = global.scene.createSceneObject(`SVGDemo_${index}`);
    obj.setParent(this.getSceneObject());
    obj.getTransform().setLocalPosition(new vec3(0, 0, index * 0.01));

    const visual = obj.createComponent('Component.RenderMeshVisual') as RenderMeshVisual;
    visual.mesh = mesh;
    visual.mainMaterial = this.material;

    const overrides = visual.mainPassOverrides as any;
    overrides.baseColor = new vec4(
      group.color[0], group.color[1], group.color[2], group.color[3]
    );

    return obj;
  }

  private createMeshFromGroup(group: { vertices: number[]; indices: number[]; color: number[] }, index: number): void {
    const obj = this.buildMeshObject(group, index);
    if (obj) this.meshObjects.push(obj);
  }

  // ─── Navigation UI ──────────────────────────────────

  private setupNavigation(): void {
    if (!this.buttonPrefab) {
      print('[SpaceSVGDemo] No buttonPrefab assigned — use demoIndex input or cycleInterval for navigation');
      return;
    }

    const navRoot = global.scene.createSceneObject('NavUI');
    navRoot.setParent(this.getSceneObject());
    navRoot.getTransform().setLocalPosition(new vec3(0, -this.worldHeight / 2 - 3, 0));
    this.navObjects.push(navRoot);

    // Prev button (left side)
    const prevObj = this.buttonPrefab.instantiate(navRoot);
    prevObj.name = 'PrevButton';
    prevObj.getTransform().setLocalPosition(new vec3(-8, 0, 0));
    prevObj.getTransform().setLocalScale(new vec3(1.0, 1.0, 1.0));
    this.navObjects.push(prevObj);
    this.setButtonLabel(prevObj, '\u25C0');
    this.hookButtonEvent(prevObj, () => this.navigateDemo(-1));

    // Demo name label (center) — offset Z forward so it's in front of button surfaces
    const labelObj = global.scene.createSceneObject('DemoLabel');
    labelObj.setParent(navRoot);
    labelObj.getTransform().setLocalPosition(new vec3(0, 0, 0.5));
    const textComp = labelObj.createComponent('Component.Text') as Text;
    textComp.text = this.getDemoLabel();
    try { (textComp as any).textFill.color = new vec4(1, 1, 1, 1); } catch (e) { /* textFill may not be set yet */ }
    this.labelText = textComp;
    this.navObjects.push(labelObj);

    // Next button (right side)
    const nextObj = this.buttonPrefab.instantiate(navRoot);
    nextObj.name = 'NextButton';
    nextObj.getTransform().setLocalPosition(new vec3(8, 0, 0));
    nextObj.getTransform().setLocalScale(new vec3(1.0, 1.0, 1.0));
    this.navObjects.push(nextObj);
    this.setButtonLabel(nextObj, '\u25B6');
    this.hookButtonEvent(nextObj, () => this.navigateDemo(1));

    print(`[SpaceSVGDemo] Navigation buttons created — ${DEMOS.length} demos available`);
  }

  private setButtonLabel(obj: SceneObject, label: string): void {
    // Try immediately first
    const textComp = this.findTextComponent(obj);
    if (textComp) {
      this.applyLabelStyle(textComp, label);
      print(`[SpaceSVGDemo] Set label "${label}" on ${obj.name}`);
      return;
    }
    // Prefab components may not be ready yet — defer to next frame
    print(`[SpaceSVGDemo] Text not found on ${obj.name} yet, deferring...`);
    this.createEvent('UpdateEvent').bind((ev: UpdateEvent) => {
      const deferred = this.findTextComponent(obj);
      if (deferred) {
        this.applyLabelStyle(deferred, label);
        print(`[SpaceSVGDemo] Deferred label "${label}" set on ${obj.name}`);
      } else {
        // Log hierarchy for debugging
        this.logHierarchy(obj, 0);
        print(`[SpaceSVGDemo] Warning: Still no Text component on ${obj.name}`);
      }
    });
  }

  private applyLabelStyle(textComp: Text, label: string): void {
    textComp.text = label;
    // Ensure text is white and visible
    try { (textComp as any).textFill.color = new vec4(1, 1, 1, 1); } catch (e) { /* textFill may not be set yet */ }
    // Push the text's SceneObject forward in Z so it's in front of the button surface
    const textObj = textComp.getSceneObject();
    const pos = textObj.getTransform().getLocalPosition();
    textObj.getTransform().setLocalPosition(new vec3(pos.x, pos.y, pos.z + 0.5));
  }

  private findTextComponent(obj: SceneObject): Text | null {
    const text = obj.getComponent('Component.Text');
    if (text) return text;
    for (let i = 0; i < obj.getChildrenCount(); i++) {
      const result = this.findTextComponent(obj.getChild(i));
      if (result) return result;
    }
    return null;
  }

  private logHierarchy(obj: SceneObject, depth: number): void {
    const indent = '  '.repeat(depth);
    const compCount = obj.getComponentCount ? (obj as any).getComponentCount() : '?';
    print(`${indent}[Hierarchy] "${obj.name}" children=${obj.getChildrenCount()}`);
    // Log component types if available
    try {
      const scripts = obj.getComponents('Component.ScriptComponent');
      if (scripts.length > 0) print(`${indent}  scripts: ${scripts.length}`);
    } catch (e) { /* ignore */ }
    for (let i = 0; i < obj.getChildrenCount(); i++) {
      this.logHierarchy(obj.getChild(i), depth + 1);
    }
  }

  private hookButtonEvent(root: SceneObject, callback: () => void): void {
    const api = this.findButtonApi(root);
    if (api) {
      api.onTriggerUp.add(callback);
      print(`[SpaceSVGDemo] Hooked button event: ${root.name}`);
    } else {
      print(`[SpaceSVGDemo] Warning: Could not find button API on ${root.name}`);
    }
  }

  private findButtonApi(obj: SceneObject): any {
    // Search script components on this object for UIKit button API (onTriggerUp event)
    const scripts = obj.getComponents('Component.ScriptComponent');
    for (let i = 0; i < scripts.length; i++) {
      const s = scripts[i] as any;
      if (s.api && s.api.onTriggerUp) return s.api;
      // Some UIKit components expose events directly
      if (s.onTriggerUp) return s;
    }
    // Recurse into children (prefab may have nested button component)
    for (let i = 0; i < obj.getChildrenCount(); i++) {
      const result = this.findButtonApi(obj.getChild(i));
      if (result) return result;
    }
    return null;
  }

  private navigateDemo(direction: number): void {
    this.currentDemo = (this.currentDemo + direction + DEMOS.length) % DEMOS.length;
    this.renderDemo(this.currentDemo);
    this.updateLabel();
    print(`[SpaceSVGDemo] Navigated to: ${DEMOS[this.currentDemo].name}`);
  }

  private getDemoLabel(): string {
    return `${this.currentDemo + 1}/${DEMOS.length} ${DEMOS[this.currentDemo].name}`;
  }

  private updateLabel(): void {
    if (this.labelText) {
      this.labelText.text = this.getDemoLabel();
    }
  }

  private clearMeshes(): void {
    for (const obj of this.meshObjects) {
      obj.destroy();
    }
    this.meshObjects = [];
    this.clearCountdownFrames();
  }
}
