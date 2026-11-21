// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// SpaceSVG.ts — SVG to native mesh renderer for Snap Spectacles
// Parses SVG XML and renders via MeshBuilder + RenderMeshVisual (no WebView).
//
// Usage:
//   1. Add this script to a SceneObject
//   2. Assign an unlit material (e.g., LegitUnlit) in the Inspector
//   3. Call loadSVG() with an SVG string
//
// Supports Phase 1: basic shapes, paths, groups, transforms, fill, stroke

// ─── Error Classes ───────────────────────────────────

class SpaceSVGParseError extends Error {
  constructor(msg: string, public tag?: string, public id?: string, public position?: number) {
    super(`[SpaceSVG Parse] ${msg}${tag ? ` <${tag}>` : ''}${id ? ` #${id}` : ''}${position != null ? ` @${position}` : ''}`);
    print(this.message);
  }
}

class SpaceSVGRenderError extends Error {
  constructor(msg: string, public tag?: string, public id?: string) {
    super(`[SpaceSVG Render] ${msg}${tag ? ` <${tag}>` : ''}${id ? ` #${id}` : ''}`);
    print(this.message);
  }
}

// ─── SVGNode ─────────────────────────────────────────

class SVGNode {
  tagName: string;
  attributes: Map<string, string>;
  children: SVGNode[];
  parent: SVGNode | null;
  textContent: string;

  constructor(tagName: string) {
    this.tagName = tagName;
    this.attributes = new Map();
    this.children = [];
    this.parent = null;
    this.textContent = '';
  }

  getAttribute(name: string): string | null {
    return this.attributes.get(name) ?? null;
  }

  setAttribute(name: string, value: string): void {
    this.attributes.set(name, value);
  }

  getElementById(id: string): SVGNode | null {
    if (this.getAttribute('id') === id) return this;
    for (const child of this.children) {
      const found = child.getElementById(id);
      if (found) return found;
    }
    return null;
  }

  getElementsByTagName(tag: string): SVGNode[] {
    const result: SVGNode[] = [];
    if (this.tagName === tag) result.push(this);
    for (const child of this.children) {
      result.push(...child.getElementsByTagName(tag));
    }
    return result;
  }

  clone(): SVGNode {
    const node = new SVGNode(this.tagName);
    this.attributes.forEach((v, k) => node.attributes.set(k, v));
    node.textContent = this.textContent;
    for (const child of this.children) {
      const cloned = child.clone();
      cloned.parent = node;
      node.children.push(cloned);
    }
    return node;
  }
}

// ─── XML Parser ──────────────────────────────────────

class SVGXMLParser {
  private pos: number = 0;
  private src: string = '';

  parse(svg: string): SVGNode {
    this.pos = 0;
    this.src = svg.trim();
    this.skipProlog();
    return this.parseElement();
  }

  private skipProlog(): void {
    while (this.pos < this.src.length) {
      this.skipWhitespace();
      if (this.src.startsWith('<?', this.pos)) {
        const end = this.src.indexOf('?>', this.pos);
        if (end === -1) throw new SpaceSVGParseError('Unclosed processing instruction', undefined, undefined, this.pos);
        this.pos = end + 2;
      } else if (this.src.startsWith('<!DOCTYPE', this.pos) || this.src.startsWith('<!doctype', this.pos)) {
        let depth = 0;
        while (this.pos < this.src.length) {
          if (this.src[this.pos] === '[') depth++;
          else if (this.src[this.pos] === ']') depth--;
          else if (this.src[this.pos] === '>' && depth === 0) { this.pos++; break; }
          this.pos++;
        }
      } else if (this.src.startsWith('<!--', this.pos)) {
        const end = this.src.indexOf('-->', this.pos);
        if (end === -1) throw new SpaceSVGParseError('Unclosed comment', undefined, undefined, this.pos);
        this.pos = end + 3;
      } else {
        break;
      }
    }
  }

  private parseElement(): SVGNode {
    this.skipWhitespace();
    if (this.src[this.pos] !== '<') {
      throw new SpaceSVGParseError('Expected <', undefined, undefined, this.pos);
    }
    this.pos++;

    const tagName = this.readName();
    const node = new SVGNode(tagName);

    // Parse attributes
    while (this.pos < this.src.length) {
      this.skipWhitespace();
      if (this.src[this.pos] === '/' && this.src[this.pos + 1] === '>') {
        this.pos += 2;
        return node;
      }
      if (this.src[this.pos] === '>') {
        this.pos++;
        break;
      }
      const attrName = this.readName();
      this.skipWhitespace();
      if (this.src[this.pos] !== '=') {
        node.setAttribute(attrName, '');
        continue;
      }
      this.pos++;
      this.skipWhitespace();
      const quote = this.src[this.pos];
      if (quote !== '"' && quote !== "'") {
        throw new SpaceSVGParseError(`Expected quote for attribute ${attrName}`, tagName, node.getAttribute('id') ?? undefined, this.pos);
      }
      this.pos++;
      const valueEnd = this.src.indexOf(quote, this.pos);
      if (valueEnd === -1) throw new SpaceSVGParseError(`Unclosed attribute ${attrName}`, tagName, undefined, this.pos);
      node.setAttribute(attrName, this.decodeEntities(this.src.slice(this.pos, valueEnd)));
      this.pos = valueEnd + 1;
    }

    // Parse children
    while (this.pos < this.src.length) {
      this.skipWhitespace();
      if (this.src.startsWith('</', this.pos)) {
        this.pos += 2;
        const closeName = this.readName();
        if (closeName !== tagName) {
          throw new SpaceSVGParseError(
            `Mismatched closing tag: expected </${tagName}>, got </${closeName}>`,
            tagName, node.getAttribute('id') ?? undefined, this.pos
          );
        }
        this.skipWhitespace();
        if (this.src[this.pos] === '>') this.pos++;
        return node;
      }
      if (this.src.startsWith('<!--', this.pos)) {
        const end = this.src.indexOf('-->', this.pos);
        if (end === -1) throw new SpaceSVGParseError('Unclosed comment', tagName, undefined, this.pos);
        this.pos = end + 3;
        continue;
      }
      if (this.src.startsWith('<![CDATA[', this.pos)) {
        const end = this.src.indexOf(']]>', this.pos);
        if (end === -1) throw new SpaceSVGParseError('Unclosed CDATA', tagName, undefined, this.pos);
        node.textContent += this.src.slice(this.pos + 9, end);
        this.pos = end + 3;
        continue;
      }
      if (this.src[this.pos] === '<') {
        const child = this.parseElement();
        child.parent = node;
        node.children.push(child);
      } else {
        const textEnd = this.src.indexOf('<', this.pos);
        if (textEnd === -1) throw new SpaceSVGParseError('Unexpected end of input', tagName, undefined, this.pos);
        node.textContent += this.decodeEntities(this.src.slice(this.pos, textEnd));
        this.pos = textEnd;
      }
    }

    throw new SpaceSVGParseError(`Unclosed tag <${tagName}>`, tagName, node.getAttribute('id') ?? undefined, this.pos);
  }

  private readName(): string {
    const start = this.pos;
    while (this.pos < this.src.length && /[a-zA-Z0-9_:\-.]/.test(this.src[this.pos])) {
      this.pos++;
    }
    if (this.pos === start) throw new SpaceSVGParseError('Expected name', undefined, undefined, this.pos);
    return this.src.slice(start, this.pos);
  }

  private skipWhitespace(): void {
    while (this.pos < this.src.length && /\s/.test(this.src[this.pos])) this.pos++;
  }

  private decodeEntities(s: string): string {
    return s
      .replace(/&amp;/g, '&')
      .replace(/&lt;/g, '<')
      .replace(/&gt;/g, '>')
      .replace(/&quot;/g, '"')
      .replace(/&apos;/g, "'")
      .replace(/&#(\d+);/g, (_, n) => String.fromCharCode(parseInt(n)))
      .replace(/&#x([0-9a-fA-F]+);/g, (_, n) => String.fromCharCode(parseInt(n, 16)));
  }
}

// ─── Style Resolver ──────────────────────────────────

interface ResolvedStyle {
  fill: string;
  stroke: string;
  strokeWidth: number;
  opacity: number;
  fillOpacity: number;
  strokeOpacity: number;
  transform: number[]; // affine [a, b, c, d, e, f]
  display: string;
  visibility: string;
}

const DEFAULT_STYLE: ResolvedStyle = {
  fill: '#000000',
  stroke: 'none',
  strokeWidth: 1,
  opacity: 1,
  fillOpacity: 1,
  strokeOpacity: 1,
  transform: [1, 0, 0, 1, 0, 0],
  display: 'inline',
  visibility: 'visible',
};

class SVGStyleResolver {
  resolve(node: SVGNode, parentStyle?: ResolvedStyle): ResolvedStyle {
    const base = parentStyle
      ? { ...parentStyle, display: 'inline' as string, transform: [1, 0, 0, 1, 0, 0] }
      : { ...DEFAULT_STYLE };

    const styleAttr = node.getAttribute('style');
    const styleProps = new Map<string, string>();
    if (styleAttr) {
      for (const decl of styleAttr.split(';')) {
        const colon = decl.indexOf(':');
        if (colon === -1) continue;
        const k = decl.slice(0, colon).trim();
        const v = decl.slice(colon + 1).trim();
        if (k && v) styleProps.set(k, v);
      }
    }

    const get = (name: string): string | null => {
      return styleProps.get(name) ?? node.getAttribute(name);
    };

    const fill = get('fill');
    if (fill != null) base.fill = fill;

    const stroke = get('stroke');
    if (stroke != null) base.stroke = stroke;

    const sw = get('stroke-width');
    if (sw != null) base.strokeWidth = parseFloat(sw);

    const op = get('opacity');
    if (op != null) base.opacity = parseFloat(op);

    const fillOp = get('fill-opacity');
    if (fillOp != null) base.fillOpacity = parseFloat(fillOp);

    const strokeOp = get('stroke-opacity');
    if (strokeOp != null) base.strokeOpacity = parseFloat(strokeOp);

    const display = get('display');
    if (display != null) base.display = display;

    const vis = get('visibility');
    if (vis != null) base.visibility = vis;

    const transformAttr = node.getAttribute('transform');
    if (transformAttr) {
      const localTransform = this.parseTransform(transformAttr);
      base.transform = this.multiplyMatrix(
        parentStyle?.transform ?? [1, 0, 0, 1, 0, 0],
        localTransform
      );
    } else if (parentStyle) {
      base.transform = [...parentStyle.transform];
    }

    return base;
  }

  parseTransform(s: string): number[] {
    let result: number[] = [1, 0, 0, 1, 0, 0];
    const re = /(matrix|translate|scale|rotate|skewX|skewY)\s*\(([^)]+)\)/g;
    let m: RegExpExecArray | null;
    while ((m = re.exec(s)) !== null) {
      const args = m[2].split(/[\s,]+/).map(Number);
      let t: number[];
      switch (m[1]) {
        case 'matrix':
          t = [args[0], args[1], args[2], args[3], args[4], args[5]];
          break;
        case 'translate':
          t = [1, 0, 0, 1, args[0], args[1] || 0];
          break;
        case 'scale':
          t = [args[0], 0, 0, args[1] ?? args[0], 0, 0];
          break;
        case 'rotate': {
          const rad = args[0] * Math.PI / 180;
          const cos = Math.cos(rad), sin = Math.sin(rad);
          if (args.length === 3) {
            const cx = args[1], cy = args[2];
            t = [cos, sin, -sin, cos, cx - cos * cx + sin * cy, cy - sin * cx - cos * cy];
          } else {
            t = [cos, sin, -sin, cos, 0, 0];
          }
          break;
        }
        case 'skewX': {
          const tan = Math.tan(args[0] * Math.PI / 180);
          t = [1, 0, tan, 1, 0, 0];
          break;
        }
        case 'skewY': {
          const tan = Math.tan(args[0] * Math.PI / 180);
          t = [1, tan, 0, 1, 0, 0];
          break;
        }
        default:
          t = [1, 0, 0, 1, 0, 0];
      }
      result = this.multiplyMatrix(result, t);
    }
    return result;
  }

  multiplyMatrix(a: number[], b: number[]): number[] {
    return [
      a[0] * b[0] + a[2] * b[1],
      a[1] * b[0] + a[3] * b[1],
      a[0] * b[2] + a[2] * b[3],
      a[1] * b[2] + a[3] * b[3],
      a[0] * b[4] + a[2] * b[5] + a[4],
      a[1] * b[4] + a[3] * b[5] + a[5],
    ];
  }
}

// ─── Path Parser ─────────────────────────────────────

interface PathCommand {
  type: string;
  args: number[];
}

class SVGPathParser {
  parse(d: string): PathCommand[] {
    const commands: PathCommand[] = [];
    const cmdPositions: { cmd: string; start: number }[] = [];

    for (let i = 0; i < d.length; i++) {
      if (/[MmLlHhVvCcSsQqTtAaZz]/.test(d[i])) {
        cmdPositions.push({ cmd: d[i], start: i });
      }
    }

    for (let i = 0; i < cmdPositions.length; i++) {
      const { cmd } = cmdPositions[i];
      const start = cmdPositions[i].start + 1;
      const end = i + 1 < cmdPositions.length ? cmdPositions[i + 1].start : d.length;
      const argsStr = d.slice(start, end).trim();

      if (cmd === 'Z' || cmd === 'z') {
        commands.push({ type: 'Z', args: [] });
        continue;
      }

      const nums = this.parseNumbers(argsStr);
      const argCount = this.argCountFor(cmd);

      if (argCount === 0) {
        commands.push({ type: cmd, args: [] });
      } else {
        for (let j = 0; j < nums.length; j += argCount) {
          const args = nums.slice(j, j + argCount);
          if (args.length < argCount) break;
          if (j > 0 && (cmd === 'M' || cmd === 'm')) {
            commands.push({ type: cmd === 'M' ? 'L' : 'l', args });
          } else {
            commands.push({ type: cmd, args });
          }
        }
      }
    }

    return commands;
  }

  private parseNumbers(s: string): number[] {
    const nums: number[] = [];
    const re = /[+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?/g;
    let m: RegExpExecArray | null;
    while ((m = re.exec(s)) !== null) {
      nums.push(parseFloat(m[0]));
    }
    return nums;
  }

  private argCountFor(cmd: string): number {
    switch (cmd.toUpperCase()) {
      case 'M': case 'L': case 'T': return 2;
      case 'H': case 'V': return 1;
      case 'C': return 6;
      case 'S': case 'Q': return 4;
      case 'A': return 7;
      case 'Z': return 0;
      default: return 0;
    }
  }
}

// ─── Path Tessellation ───────────────────────────────

interface Point {
  x: number;
  y: number;
}

class SVGPathTessellator {
  private curveSegments: number = 8;

  tessellate(commands: PathCommand[]): Point[][] {
    const subpaths: Point[][] = [];
    let current: Point[] = [];
    let cx = 0, cy = 0;
    let sx = 0, sy = 0;
    let lastCx: number | null = null, lastCy: number | null = null;

    for (const cmd of commands) {
      const abs = cmd.type === cmd.type.toUpperCase();
      const a = cmd.args;

      switch (cmd.type.toUpperCase()) {
        case 'M': {
          if (current.length > 0) subpaths.push(current);
          const x = abs ? a[0] : cx + a[0];
          const y = abs ? a[1] : cy + a[1];
          current = [{ x, y }];
          cx = sx = x;
          cy = sy = y;
          lastCx = lastCy = null;
          break;
        }
        case 'L': {
          const x = abs ? a[0] : cx + a[0];
          const y = abs ? a[1] : cy + a[1];
          current.push({ x, y });
          cx = x; cy = y;
          lastCx = lastCy = null;
          break;
        }
        case 'H': {
          const x = abs ? a[0] : cx + a[0];
          current.push({ x, y: cy });
          cx = x;
          lastCx = lastCy = null;
          break;
        }
        case 'V': {
          const y = abs ? a[0] : cy + a[0];
          current.push({ x: cx, y });
          cy = y;
          lastCx = lastCy = null;
          break;
        }
        case 'C': {
          const x1 = abs ? a[0] : cx + a[0];
          const y1 = abs ? a[1] : cy + a[1];
          const x2 = abs ? a[2] : cx + a[2];
          const y2 = abs ? a[3] : cy + a[3];
          const x = abs ? a[4] : cx + a[4];
          const y = abs ? a[5] : cy + a[5];
          this.cubicBezier(current, cx, cy, x1, y1, x2, y2, x, y);
          lastCx = x2; lastCy = y2;
          cx = x; cy = y;
          break;
        }
        case 'S': {
          const x1 = lastCx != null ? 2 * cx - lastCx : cx;
          const y1 = lastCy != null ? 2 * cy - lastCy : cy;
          const x2 = abs ? a[0] : cx + a[0];
          const y2 = abs ? a[1] : cy + a[1];
          const x = abs ? a[2] : cx + a[2];
          const y = abs ? a[3] : cy + a[3];
          this.cubicBezier(current, cx, cy, x1, y1, x2, y2, x, y);
          lastCx = x2; lastCy = y2;
          cx = x; cy = y;
          break;
        }
        case 'Q': {
          const x1 = abs ? a[0] : cx + a[0];
          const y1 = abs ? a[1] : cy + a[1];
          const x = abs ? a[2] : cx + a[2];
          const y = abs ? a[3] : cy + a[3];
          this.quadBezier(current, cx, cy, x1, y1, x, y);
          lastCx = x1; lastCy = y1;
          cx = x; cy = y;
          break;
        }
        case 'T': {
          const x1 = lastCx != null ? 2 * cx - lastCx : cx;
          const y1 = lastCy != null ? 2 * cy - lastCy : cy;
          const x = abs ? a[0] : cx + a[0];
          const y = abs ? a[1] : cy + a[1];
          this.quadBezier(current, cx, cy, x1, y1, x, y);
          lastCx = x1; lastCy = y1;
          cx = x; cy = y;
          break;
        }
        case 'A': {
          const rx = a[0], ry = a[1], rotation = a[2];
          const largeArc = a[3] !== 0, sweep = a[4] !== 0;
          const x = abs ? a[5] : cx + a[5];
          const y = abs ? a[6] : cy + a[6];
          this.arcTo(current, cx, cy, rx, ry, rotation, largeArc, sweep, x, y);
          cx = x; cy = y;
          lastCx = lastCy = null;
          break;
        }
        case 'Z': {
          if (current.length > 0) {
            if (current[0].x !== cx || current[0].y !== cy) {
              current.push({ x: current[0].x, y: current[0].y });
            }
            subpaths.push(current);
            current = [];
          }
          cx = sx; cy = sy;
          lastCx = lastCy = null;
          break;
        }
      }
    }

    if (current.length > 0) subpaths.push(current);
    return subpaths;
  }

  private cubicBezier(pts: Point[], x0: number, y0: number,
    x1: number, y1: number, x2: number, y2: number, x3: number, y3: number): void {
    for (let i = 1; i <= this.curveSegments; i++) {
      const t = i / this.curveSegments;
      const t2 = t * t, t3 = t2 * t;
      const mt = 1 - t, mt2 = mt * mt, mt3 = mt2 * mt;
      pts.push({
        x: mt3 * x0 + 3 * mt2 * t * x1 + 3 * mt * t2 * x2 + t3 * x3,
        y: mt3 * y0 + 3 * mt2 * t * y1 + 3 * mt * t2 * y2 + t3 * y3,
      });
    }
  }

  private quadBezier(pts: Point[], x0: number, y0: number,
    x1: number, y1: number, x2: number, y2: number): void {
    for (let i = 1; i <= this.curveSegments; i++) {
      const t = i / this.curveSegments;
      const mt = 1 - t;
      pts.push({
        x: mt * mt * x0 + 2 * mt * t * x1 + t * t * x2,
        y: mt * mt * y0 + 2 * mt * t * y1 + t * t * y2,
      });
    }
  }

  private arcTo(pts: Point[], x1: number, y1: number,
    rx: number, ry: number, phi: number, fA: boolean, fS: boolean,
    x2: number, y2: number): void {
    if (rx === 0 || ry === 0) {
      pts.push({ x: x2, y: y2 });
      return;
    }

    rx = Math.abs(rx);
    ry = Math.abs(ry);
    const phiRad = phi * Math.PI / 180;
    const cosPhi = Math.cos(phiRad), sinPhi = Math.sin(phiRad);

    const dx = (x1 - x2) / 2, dy = (y1 - y2) / 2;
    const x1p = cosPhi * dx + sinPhi * dy;
    const y1p = -sinPhi * dx + cosPhi * dy;

    let lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
    if (lambda > 1) {
      const sqrtL = Math.sqrt(lambda);
      rx *= sqrtL;
      ry *= sqrtL;
    }

    const rxSq = rx * rx, rySq = ry * ry;
    const x1pSq = x1p * x1p, y1pSq = y1p * y1p;

    let num = Math.max(0, rxSq * rySq - rxSq * y1pSq - rySq * x1pSq);
    let den = rxSq * y1pSq + rySq * x1pSq;
    let sq = Math.sqrt(num / den);
    if (fA === fS) sq = -sq;

    const cxp = sq * rx * y1p / ry;
    const cyp = -sq * ry * x1p / rx;

    const cxCenter = cosPhi * cxp - sinPhi * cyp + (x1 + x2) / 2;
    const cyCenter = sinPhi * cxp + cosPhi * cyp + (y1 + y2) / 2;

    const theta1 = Math.atan2((y1p - cyp) / ry, (x1p - cxp) / rx);
    let dtheta = Math.atan2((-y1p - cyp) / ry, (-x1p - cxp) / rx) - theta1;

    if (fS && dtheta < 0) dtheta += 2 * Math.PI;
    if (!fS && dtheta > 0) dtheta -= 2 * Math.PI;

    const segments = Math.max(1, Math.ceil(Math.abs(dtheta) / (Math.PI / 4)) * 4);
    for (let i = 1; i <= segments; i++) {
      const t = theta1 + (i / segments) * dtheta;
      const cosT = Math.cos(t), sinT = Math.sin(t);
      pts.push({
        x: cosPhi * rx * cosT - sinPhi * ry * sinT + cxCenter,
        y: sinPhi * rx * cosT + cosPhi * ry * sinT + cyCenter,
      });
    }
  }
}

// ─── Triangulator (Ear Clipping) ─────────────────────

class SVGTriangulator {
  // Triangulate a polygon that may be self-intersecting (e.g., a 5-point
  // pentagram drawn as a single subpath). Detects intersections, swaps the
  // edge connections at each one to untangle the polygon into one or more
  // simple sub-polygons, then ear-clips each. Returns expanded vertex list
  // (original + intersection points) and indices into it.
  triangulateFill(polygon: Point[]): { vertices: Point[]; indices: number[] } {
    if (polygon.length < 3) return { vertices: polygon, indices: [] };

    const inters = this.findSelfIntersections(polygon);
    if (inters.length === 0) {
      return { vertices: polygon, indices: this.triangulate(polygon) };
    }

    // Build expanded sequence: each original vertex, then any intersections
    // along its outgoing edge (sorted by parameter). Track each intersection's
    // two visit positions so we can swap them.
    type Entry = { pt: Point; globalIdx: number; interIdx: number };
    const entries: Entry[] = [];
    const visits: Map<number, number[]> = new Map();
    const allVerts: Point[] = [...polygon];
    const interGlobalIdx: number[] = [];
    for (let i = 0; i < inters.length; i++) {
      interGlobalIdx.push(allVerts.length);
      allVerts.push(inters[i].pt);
    }

    for (let e = 0; e < polygon.length; e++) {
      entries.push({ pt: polygon[e], globalIdx: e, interIdx: -1 });
      const onEdge = inters
        .map((it, idx) => ({ idx, t: it.edgeA === e ? it.tA : (it.edgeB === e ? it.tB : -1) }))
        .filter(x => x.t >= 0)
        .sort((a, b) => a.t - b.t);
      for (const oe of onEdge) {
        const pos = entries.length;
        entries.push({ pt: inters[oe.idx].pt, globalIdx: interGlobalIdx[oe.idx], interIdx: oe.idx });
        if (!visits.has(oe.idx)) visits.set(oe.idx, []);
        visits.get(oe.idx)!.push(pos);
      }
    }

    // next[i] = position to walk to from position i; default = sequential cycle
    const total = entries.length;
    const next: number[] = new Array(total);
    for (let i = 0; i < total; i++) next[i] = (i + 1) % total;
    // Untangle: at each intersection, swap the outgoing connection of its two visits
    for (const v of visits.values()) {
      if (v.length !== 2) continue;
      const tmp = next[v[0]];
      next[v[0]] = next[v[1]];
      next[v[1]] = tmp;
    }

    // Walk all cycles. Each cycle becomes a simple sub-polygon to ear-clip.
    const visited = new Array<boolean>(total).fill(false);
    const indices: number[] = [];
    for (let s = 0; s < total; s++) {
      if (visited[s]) continue;
      const cycle: number[] = [];
      const cyclePts: Point[] = [];
      let cur = s;
      let safety = total + 1;
      while (!visited[cur] && safety-- > 0) {
        visited[cur] = true;
        cycle.push(entries[cur].globalIdx);
        cyclePts.push(entries[cur].pt);
        cur = next[cur];
      }
      if (cyclePts.length < 3) continue;
      const subIdx = this.triangulate(cyclePts);
      for (const i of subIdx) indices.push(cycle[i]);
    }

    return { vertices: allVerts, indices };
  }

  // Return the list of self-intersections between non-adjacent edges of the
  // polygon (treated as closed). Each entry has the two edge indices, the
  // intersection point, and the parameter on each edge.
  private findSelfIntersections(poly: Point[]): Array<{ edgeA: number; edgeB: number; pt: Point; tA: number; tB: number }> {
    const out: Array<{ edgeA: number; edgeB: number; pt: Point; tA: number; tB: number }> = [];
    const n = poly.length;
    const eps = 1e-6;
    for (let i = 0; i < n; i++) {
      const a1 = poly[i];
      const a2 = poly[(i + 1) % n];
      for (let j = i + 2; j < n; j++) {
        if (i === 0 && j === n - 1) continue; // adjacent (wrap)
        const b1 = poly[j];
        const b2 = poly[(j + 1) % n];
        const dx1 = a2.x - a1.x, dy1 = a2.y - a1.y;
        const dx2 = b2.x - b1.x, dy2 = b2.y - b1.y;
        const denom = dx1 * dy2 - dy1 * dx2;
        if (Math.abs(denom) < 1e-9) continue;
        const tA = ((b1.x - a1.x) * dy2 - (b1.y - a1.y) * dx2) / denom;
        const tB = ((b1.x - a1.x) * dy1 - (b1.y - a1.y) * dx1) / denom;
        if (tA <= eps || tA >= 1 - eps || tB <= eps || tB >= 1 - eps) continue;
        out.push({
          edgeA: i, edgeB: j,
          pt: { x: a1.x + dx1 * tA, y: a1.y + dy1 * tA },
          tA, tB,
        });
      }
    }
    return out;
  }

  triangulate(polygon: Point[]): number[] {
    const n = polygon.length;
    if (n < 3) return [];
    if (n === 3) return [0, 1, 2];

    const area = this.signedArea(polygon);
    const verts = Array.from({ length: n }, (_, i) => i);
    if (area < 0) verts.reverse();

    let remaining = [...verts];
    const indices: number[] = [];
    let safety = remaining.length * 2;

    while (remaining.length > 3 && safety-- > 0) {
      let earFound = false;
      for (let i = 0; i < remaining.length; i++) {
        const prev = remaining[(i - 1 + remaining.length) % remaining.length];
        const curr = remaining[i];
        const next = remaining[(i + 1) % remaining.length];

        if (!this.isConvex(polygon[prev], polygon[curr], polygon[next])) continue;

        let inside = false;
        for (let j = 0; j < remaining.length; j++) {
          const vi = remaining[j];
          if (vi === prev || vi === curr || vi === next) continue;
          if (this.pointInTriangle(polygon[vi], polygon[prev], polygon[curr], polygon[next])) {
            inside = true;
            break;
          }
        }

        if (!inside) {
          indices.push(prev, curr, next);
          remaining.splice(i, 1);
          earFound = true;
          break;
        }
      }
      if (!earFound) break;
    }

    if (remaining.length === 3) {
      indices.push(remaining[0], remaining[1], remaining[2]);
    } else if (remaining.length > 3) {
      // Ear-clipping deadlocked. Fall back to a triangle fan from the first
      // remaining vertex so the polygon at least gets covered. Some triangles
      // may overlap or extend slightly outside the polygon, but a fan over a
      // ~near-convex remaining region is usually visually acceptable and
      // strictly better than leaving a hole.
      print(`[SpaceSVG] Ear-clipping deadlocked with ${remaining.length} of ${polygon.length} vertices unclipped — falling back to fan triangulation`);
      const a = remaining[0];
      for (let k = 1; k < remaining.length - 1; k++) {
        indices.push(a, remaining[k], remaining[k + 1]);
      }
    }

    return indices;
  }

  // Generates stroke geometry as one rectangle quad per edge plus a bevel
  // triangle per corner. Per-edge normals avoid the pinch/bowtie artifacts
  // that the averaged-normal-per-point approach produces at sharp turns.
  triangulateStroke(points: Point[], width: number): { vertices: Point[]; indices: number[] } {
    if (points.length < 2) return { vertices: [], indices: [] };
    const hw = width / 2;

    const closed = points.length > 2 &&
      Math.abs(points[0].x - points[points.length - 1].x) < 0.001 &&
      Math.abs(points[0].y - points[points.length - 1].y) < 0.001;

    const pts = closed ? points.slice(0, -1) : points;
    const n = pts.length;
    if (n < 2) return { vertices: [], indices: [] };

    const edgeCount = closed ? n : n - 1;
    const normals: { nx: number; ny: number }[] = [];
    for (let e = 0; e < edgeCount; e++) {
      const dx = pts[(e + 1) % n].x - pts[e].x;
      const dy = pts[(e + 1) % n].y - pts[e].y;
      const len = Math.sqrt(dx * dx + dy * dy);
      if (len < 0.0001) {
        // Degenerate edge — reuse previous normal so corner math stays sane
        normals.push(normals.length > 0 ? normals[normals.length - 1] : { nx: 0, ny: 1 });
      } else {
        normals.push({ nx: -dy / len, ny: dx / len });
      }
    }

    const vertices: Point[] = [];
    const indices: number[] = [];

    // One rectangle quad per edge — CCW order matches existing fill convention
    for (let e = 0; e < edgeCount; e++) {
      const i = e;
      const j = (e + 1) % n;
      const ne = normals[e];
      const baseIdx = vertices.length;
      vertices.push(
        { x: pts[i].x + ne.nx * hw, y: pts[i].y + ne.ny * hw }, // leftStart
        { x: pts[i].x - ne.nx * hw, y: pts[i].y - ne.ny * hw }, // rightStart
        { x: pts[j].x + ne.nx * hw, y: pts[j].y + ne.ny * hw }, // leftEnd
        { x: pts[j].x - ne.nx * hw, y: pts[j].y - ne.ny * hw }, // rightEnd
      );
      indices.push(baseIdx, baseIdx + 1, baseIdx + 2);
      indices.push(baseIdx + 1, baseIdx + 3, baseIdx + 2);
    }

    // Bevel triangle at each corner, on the outer side (where adjacent edges'
    // offset vertices spread apart). Inner side gets a small overlap which
    // renders fine since strokes are opaque single-color.
    const cornerCount = closed ? edgeCount : edgeCount - 1;
    for (let c = 0; c < cornerCount; c++) {
      const e1 = c;
      const e2 = (c + 1) % edgeCount;
      const cornerPt = pts[(c + 1) % n];
      const n1 = normals[e1];
      const n2 = normals[e2];
      const cross = n1.nx * n2.ny - n1.ny * n2.nx;
      if (Math.abs(cross) < 0.001) continue; // collinear — no bevel needed

      const baseIdx = vertices.length;
      if (cross > 0) {
        // Outer side is "left": fill gap between leftEnd(e1), leftStart(e2), corner
        vertices.push(
          { x: cornerPt.x + n1.nx * hw, y: cornerPt.y + n1.ny * hw },
          { x: cornerPt.x + n2.nx * hw, y: cornerPt.y + n2.ny * hw },
          { x: cornerPt.x, y: cornerPt.y },
        );
      } else {
        // Outer side is "right": same triangle on the other side, opposite winding
        vertices.push(
          { x: cornerPt.x - n1.nx * hw, y: cornerPt.y - n1.ny * hw },
          { x: cornerPt.x, y: cornerPt.y },
          { x: cornerPt.x - n2.nx * hw, y: cornerPt.y - n2.ny * hw },
        );
      }
      indices.push(baseIdx, baseIdx + 1, baseIdx + 2);
    }

    return { vertices, indices };
  }

  private signedArea(poly: Point[]): number {
    let area = 0;
    for (let i = 0; i < poly.length; i++) {
      const j = (i + 1) % poly.length;
      area += poly[i].x * poly[j].y - poly[j].x * poly[i].y;
    }
    return area / 2;
  }

  private isConvex(a: Point, b: Point, c: Point): boolean {
    // Tolerate near-collinear corners (cross ≈ 0) as convex. Strict > 0 lets
    // floating-point noise from dense curve tessellation classify a perfectly-
    // valid ear as concave, which can deadlock the ear-clipping loop.
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) > -1e-9;
  }

  private pointInTriangle(p: Point, a: Point, b: Point, c: Point): boolean {
    // Strict-interior test: a vertex sitting exactly on (or numerically next
    // to) one of the ear triangle's edges shouldn't count as "inside" — it
    // doesn't actually block the ear, and treating it as a blocker is what
    // causes complex fills (like the pig body at curveSegments=8) to leave
    // black holes.
    const eps = 1e-9;
    const d1 = this.cross(a, b, p);
    const d2 = this.cross(b, c, p);
    const d3 = this.cross(c, a, p);
    return (d1 > eps && d2 > eps && d3 > eps) || (d1 < -eps && d2 < -eps && d3 < -eps);
  }

  private cross(a: Point, b: Point, p: Point): number {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
  }
}

// ─── Color Parser ────────────────────────────────────

const NAMED_COLORS: Record<string, string> = {
  black: '#000000', white: '#ffffff', red: '#ff0000', green: '#008000',
  blue: '#0000ff', yellow: '#ffff00', cyan: '#00ffff', magenta: '#ff00ff',
  orange: '#ffa500', purple: '#800080', gray: '#808080', grey: '#808080',
  lime: '#00ff00', maroon: '#800000', navy: '#000080', olive: '#808000',
  teal: '#008080', silver: '#c0c0c0', aqua: '#00ffff', fuchsia: '#ff00ff',
  pink: '#ffc0cb', brown: '#a52a2a', coral: '#ff7f50', gold: '#ffd700',
  indigo: '#4b0082', violet: '#ee82ee', crimson: '#dc143c', tomato: '#ff6347',
  salmon: '#fa8072', khaki: '#f0e68c', plum: '#dda0dd', orchid: '#da70d6',
  tan: '#d2b48c', wheat: '#f5deb3', ivory: '#fffff0', lavender: '#e6e6fa',
  beige: '#f5f5dc', linen: '#faf0e6', peru: '#cd853f', sienna: '#a0522d',
  chocolate: '#d2691e', firebrick: '#b22222', darkred: '#8b0000',
  darkgreen: '#006400', darkblue: '#00008b', darkcyan: '#008b8b',
  darkmagenta: '#8b008b', darkorange: '#ff8c00', darkviolet: '#9400d3',
  deeppink: '#ff1493', deepskyblue: '#00bfff', dodgerblue: '#1e90ff',
  forestgreen: '#228b22', hotpink: '#ff69b4', indianred: '#cd5c5c',
  lightblue: '#add8e6', lightcoral: '#f08080', lightcyan: '#e0ffff',
  lightgray: '#d3d3d3', lightgreen: '#90ee90', lightpink: '#ffb6c1',
  lightyellow: '#ffffe0', limegreen: '#32cd32', mediumblue: '#0000cd',
  midnightblue: '#191970', royalblue: '#4169e1', seagreen: '#2e8b57',
  skyblue: '#87ceeb', slateblue: '#6a5acd', slategray: '#708090',
  springgreen: '#00ff7f', steelblue: '#4682b4', turquoise: '#40e0d0',
  yellowgreen: '#9acd32',
};

function parseColor(color: string): [number, number, number, number] | null {
  if (!color || color === 'none' || color === 'transparent') return null;

  color = color.trim().toLowerCase();
  if (NAMED_COLORS[color]) color = NAMED_COLORS[color];

  if (color[0] === '#') {
    let hex = color.slice(1);
    if (hex.length === 3) hex = hex[0] + hex[0] + hex[1] + hex[1] + hex[2] + hex[2];
    if (hex.length === 6) hex += 'ff';
    return [
      parseInt(hex.slice(0, 2), 16) / 255,
      parseInt(hex.slice(2, 4), 16) / 255,
      parseInt(hex.slice(4, 6), 16) / 255,
      parseInt(hex.slice(6, 8), 16) / 255,
    ];
  }

  const rgbMatch = color.match(/rgba?\(\s*(\d+)[,\s]+(\d+)[,\s]+(\d+)(?:[,\s/]+([0-9.]+))?\s*\)/);
  if (rgbMatch) {
    return [
      parseInt(rgbMatch[1]) / 255,
      parseInt(rgbMatch[2]) / 255,
      parseInt(rgbMatch[3]) / 255,
      rgbMatch[4] != null ? parseFloat(rgbMatch[4]) : 1,
    ];
  }

  return null;
}

// ─── Shape to Points ─────────────────────────────────

function shapeToPoints(node: SVGNode): Point[][] | null {
  switch (node.tagName) {
    case 'rect': {
      const x = parseFloat(node.getAttribute('x') || '0');
      const y = parseFloat(node.getAttribute('y') || '0');
      const w = parseFloat(node.getAttribute('width') || '0');
      const h = parseFloat(node.getAttribute('height') || '0');
      let rx = parseFloat(node.getAttribute('rx') || '0');
      let ry = parseFloat(node.getAttribute('ry') || rx.toString());
      if (w <= 0 || h <= 0) return null;
      if (rx > 0 || ry > 0) {
        rx = Math.min(rx, w / 2);
        ry = Math.min(ry, h / 2);
        const pts: Point[] = [];
        const segs = 8;
        for (let i = 0; i <= segs; i++) {
          const a = -Math.PI / 2 + (Math.PI / 2) * (i / segs);
          pts.push({ x: x + w - rx + rx * Math.cos(a), y: y + ry + ry * Math.sin(a) });
        }
        for (let i = 0; i <= segs; i++) {
          const a = (Math.PI / 2) * (i / segs);
          pts.push({ x: x + w - rx + rx * Math.cos(a), y: y + h - ry + ry * Math.sin(a) });
        }
        for (let i = 0; i <= segs; i++) {
          const a = Math.PI / 2 + (Math.PI / 2) * (i / segs);
          pts.push({ x: x + rx + rx * Math.cos(a), y: y + h - ry + ry * Math.sin(a) });
        }
        for (let i = 0; i <= segs; i++) {
          const a = Math.PI + (Math.PI / 2) * (i / segs);
          pts.push({ x: x + rx + rx * Math.cos(a), y: y + ry + ry * Math.sin(a) });
        }
        // Close: duplicate first point so stroke detects closed path
        pts.push({ x: pts[0].x, y: pts[0].y });
        return [pts];
      }
      // Close rect
      return [[
        { x, y },
        { x: x + w, y },
        { x: x + w, y: y + h },
        { x, y: y + h },
        { x, y }, // closing point
      ]];
    }
    case 'circle': {
      const cx = parseFloat(node.getAttribute('cx') || '0');
      const cy = parseFloat(node.getAttribute('cy') || '0');
      const r = parseFloat(node.getAttribute('r') || '0');
      if (r <= 0) return null;
      const segs = 32;
      const pts: Point[] = [];
      for (let i = 0; i < segs; i++) {
        const a = (2 * Math.PI * i) / segs;
        pts.push({ x: cx + r * Math.cos(a), y: cy + r * Math.sin(a) });
      }
      pts.push({ x: pts[0].x, y: pts[0].y }); // closing point
      return [pts];
    }
    case 'ellipse': {
      const cx = parseFloat(node.getAttribute('cx') || '0');
      const cy = parseFloat(node.getAttribute('cy') || '0');
      const rx = parseFloat(node.getAttribute('rx') || '0');
      const ry = parseFloat(node.getAttribute('ry') || '0');
      if (rx <= 0 || ry <= 0) return null;
      const segs = 32;
      const pts: Point[] = [];
      for (let i = 0; i < segs; i++) {
        const a = (2 * Math.PI * i) / segs;
        pts.push({ x: cx + rx * Math.cos(a), y: cy + ry * Math.sin(a) });
      }
      pts.push({ x: pts[0].x, y: pts[0].y }); // closing point
      return [pts];
    }
    case 'line': {
      const x1 = parseFloat(node.getAttribute('x1') || '0');
      const y1 = parseFloat(node.getAttribute('y1') || '0');
      const x2 = parseFloat(node.getAttribute('x2') || '0');
      const y2 = parseFloat(node.getAttribute('y2') || '0');
      return [[{ x: x1, y: y1 }, { x: x2, y: y2 }]];
    }
    case 'polyline':
    case 'polygon': {
      const pointsAttr = node.getAttribute('points');
      if (!pointsAttr) return null;
      const nums = pointsAttr.trim().split(/[\s,]+/).map(Number);
      const pts: Point[] = [];
      for (let i = 0; i + 1 < nums.length; i += 2) {
        pts.push({ x: nums[i], y: nums[i + 1] });
      }
      if (pts.length < 2) return null;
      // Close polygon (not polyline)
      if (node.tagName === 'polygon' && pts.length > 2) {
        pts.push({ x: pts[0].x, y: pts[0].y });
      }
      return [pts];
    }
    default:
      return null;
  }
}

// ─── ViewBox ─────────────────────────────────────────

interface ViewBox {
  minX: number;
  minY: number;
  width: number;
  height: number;
}

function parseViewBox(s: string | null): ViewBox | null {
  if (!s) return null;
  const parts = s.trim().split(/[\s,]+/).map(Number);
  if (parts.length !== 4) return null;
  return { minX: parts[0], minY: parts[1], width: parts[2], height: parts[3] };
}

// ─── Tag Classification ─────────────────────────────

// Tags that are valid containers (process children, produce no own geometry)
const SVG_CONTAINER_TAGS = new Set(['svg', 'g', 'a', 'symbol', 'switch']);
// Tags that are metadata/non-visual (safe to ignore silently)
const SVG_METADATA_TAGS = new Set(['title', 'desc', 'metadata']);

// ─── Mesh Backend ────────────────────────────────────

interface MeshGroup {
  vertices: number[];
  indices: number[];
  color: [number, number, number, number];
}

class SpaceSVGMeshBackend {
  private pathParser: SVGPathParser = new SVGPathParser();
  private tessellator: SVGPathTessellator = new SVGPathTessellator();
  private triangulator: SVGTriangulator = new SVGTriangulator();
  private styleResolver: SVGStyleResolver = new SVGStyleResolver();
  private strokeScale: number = 1;
  private skipGroupIds: Set<string> = new Set();

  buildMeshes(root: SVGNode, worldWidth: number, worldHeight: number, options?: { strokeScale?: number; skipGroupIds?: string[] }): MeshGroup[] {
    const viewBox = parseViewBox(root.getAttribute('viewBox'));
    const svgW = viewBox?.width ?? parseFloat(root.getAttribute('width') || '100');
    const svgH = viewBox?.height ?? parseFloat(root.getAttribute('height') || '100');
    const vbX = viewBox?.minX ?? 0;
    const vbY = viewBox?.minY ?? 0;

    const scaleX = worldWidth / svgW;
    const scaleY = worldHeight / svgH;
    const scale = Math.min(scaleX, scaleY);

    this.strokeScale = options?.strokeScale ?? 1;
    this.skipGroupIds = new Set(options?.skipGroupIds ?? []);

    const groups: MeshGroup[] = [];
    this.processNode(root, groups, { ...DEFAULT_STYLE }, vbX, vbY, scale, worldWidth, worldHeight);
    return groups;
  }

  private processNode(
    node: SVGNode, groups: MeshGroup[], parentStyle: ResolvedStyle,
    vbX: number, vbY: number, scale: number,
    worldWidth: number, worldHeight: number,
  ): void {
    const style = this.styleResolver.resolve(node, parentStyle);

    if (style.display === 'none' || style.visibility === 'hidden') return;
    if (node.tagName === 'defs') return;

    // Skip groups (and their entire subtree) whose id is in the skip set —
    // useful to drop 3D-export "MeshIntersection" / "Crease" interior paths
    // that aren't part of the artistic outline.
    if (node.tagName === 'g' && this.skipGroupIds.size > 0) {
      const gid = node.getAttribute('id');
      if (gid && this.skipGroupIds.has(gid)) return;
    }

    if (node.tagName === 'use') {
      const href = node.getAttribute('href') || node.getAttribute('xlink:href');
      if (href && href.startsWith('#')) {
        const root = this.findRoot(node);
        const target = root?.getElementById(href.slice(1));
        if (target) {
          const clone = target.clone();
          const useX = parseFloat(node.getAttribute('x') || '0');
          const useY = parseFloat(node.getAttribute('y') || '0');
          if (useX || useY) {
            const existing = clone.getAttribute('transform') || '';
            clone.setAttribute('transform', `translate(${useX},${useY}) ${existing}`);
          }
          this.processNode(clone, groups, style, vbX, vbY, scale, worldWidth, worldHeight);
        } else {
          print(`[SpaceSVG] Error: <use> target "${href}" not found in document`);
        }
      }
      return;
    }

    let subpaths: Point[][] | null = null;

    if (node.tagName === 'path') {
      const d = node.getAttribute('d');
      if (d) {
        const cmds = this.pathParser.parse(d);
        subpaths = this.tessellator.tessellate(cmds);
      } else {
        print(`[SpaceSVG] Warning: <path> element has no 'd' attribute`);
      }
    } else {
      subpaths = shapeToPoints(node);
      if (!subpaths && !SVG_CONTAINER_TAGS.has(node.tagName) && !SVG_METADATA_TAGS.has(node.tagName)) {
        const id = node.getAttribute('id');
        print(`[SpaceSVG] Warning: unhandled SVG element <${node.tagName}>${id ? ` id="${id}"` : ''}`);
      }
    }

    if (subpaths && subpaths.length > 0) {
      const tf = style.transform;
      for (const sp of subpaths) {
        for (let i = 0; i < sp.length; i++) {
          const { x, y } = sp[i];
          sp[i] = {
            x: tf[0] * x + tf[2] * y + tf[4],
            y: tf[1] * x + tf[3] * y + tf[5],
          };
        }
      }

      // Fill — bridge holes into outers, merge into one mesh group
      const fillColor = parseColor(style.fill);
      if (fillColor) {
        fillColor[3] *= style.opacity * style.fillOpacity;
        const polygons = this.preparePolygonsWithHoles(subpaths);
        const mergedVerts: number[] = [];
        const mergedIndices: number[] = [];
        for (const poly of polygons) {
          // triangulateFill detects self-intersection (e.g., pentagrams) and
          // returns possibly-expanded vertex list with indices into it.
          const tri = this.triangulator.triangulateFill(poly);
          const triIndices = tri.indices;
          const triVerts = tri.vertices;
          if (triIndices.length === 0) {
            print(`[SpaceSVG] Warning: fill triangulation failed for <${node.tagName}> (${poly.length} points)`);
            continue;
          }

          const baseVertex = mergedVerts.length / 3;
          for (const p of triVerts) {
            const wx = (p.x - vbX) * scale - worldWidth / 2;
            const wy = worldHeight / 2 - (p.y - vbY) * scale;
            mergedVerts.push(wx, wy, 0);
          }

          // Y-flip reverses winding order — swap each triangle to fix back-face culling
          for (let ti = 0; ti < triIndices.length; ti += 3) {
            const tmp = triIndices[ti + 1];
            triIndices[ti + 1] = triIndices[ti + 2];
            triIndices[ti + 2] = tmp;
          }

          for (const idx of triIndices) {
            mergedIndices.push(idx + baseVertex);
          }
        }
        if (mergedIndices.length > 0) {
          groups.push({
            vertices: mergedVerts,
            indices: mergedIndices,
            color: [fillColor[0], fillColor[1], fillColor[2], fillColor[3]],
          });
        }
      }

      // Stroke — merge all subpaths into one mesh group to minimize draw calls
      const strokeColor = parseColor(style.stroke);
      if (strokeColor && style.strokeWidth > 0) {
        strokeColor[3] *= style.opacity * style.strokeOpacity;
        const effectiveStrokeWidth = style.strokeWidth * this.strokeScale;
        const mergedVerts: number[] = [];
        const mergedIndices: number[] = [];
        for (const sp of subpaths) {
          if (sp.length < 2) continue;
          const stroke = this.triangulator.triangulateStroke(sp, effectiveStrokeWidth);
          if (stroke.vertices.length < 2 || stroke.indices.length < 3) continue;

          const baseVertex = mergedVerts.length / 3;
          for (const p of stroke.vertices) {
            const wx = (p.x - vbX) * scale - worldWidth / 2;
            const wy = worldHeight / 2 - (p.y - vbY) * scale;
            mergedVerts.push(wx, wy, 0);
          }

          const triIndices = stroke.indices;

          // Y-flip reverses winding order — swap each triangle to fix back-face culling
          for (let ti = 0; ti < triIndices.length; ti += 3) {
            const tmp = triIndices[ti + 1];
            triIndices[ti + 1] = triIndices[ti + 2];
            triIndices[ti + 2] = tmp;
          }

          for (const idx of triIndices) {
            mergedIndices.push(idx + baseVertex);
          }
        }
        if (mergedIndices.length > 0) {
          groups.push({
            vertices: mergedVerts,
            indices: mergedIndices,
            color: [strokeColor[0], strokeColor[1], strokeColor[2], strokeColor[3]],
          });
        }
      }
    }

    for (const child of node.children) {
      this.processNode(child, groups, style, vbX, vbY, scale, worldWidth, worldHeight);
    }
  }

  // ─── Hole-aware polygon preparation ───────────────

  // Groups subpaths into outer contours with bridged holes.
  // Holes are detected by opposite winding direction + point-in-polygon containment.
  // Each hole is bridged into its parent outer via a zero-width cut, creating a
  // single polygon that ear-clipping can triangulate with the hole left empty.
  private preparePolygonsWithHoles(subpaths: Point[][]): Point[][] {
    const polys: { pts: Point[]; area: number }[] = [];
    for (const sp of subpaths) {
      if (sp.length < 3) continue;
      const pts = [...sp];
      if (pts.length > 1 &&
        Math.abs(pts[0].x - pts[pts.length - 1].x) < 0.001 &&
        Math.abs(pts[0].y - pts[pts.length - 1].y) < 0.001) {
        pts.pop();
      }
      if (pts.length < 3) continue;
      const area = this.polySignedArea(pts);
      polys.push({ pts, area });
    }

    if (polys.length <= 1) return polys.map(p => p.pts);

    // Largest absolute area first — these are outer contours
    polys.sort((a, b) => Math.abs(b.area) - Math.abs(a.area));

    const result: Point[][] = [];
    const assigned = new Set<number>();

    for (let i = 0; i < polys.length; i++) {
      if (assigned.has(i)) continue;
      let outer = polys[i].pts;
      const outerArea = polys[i].area;

      // Find holes: smaller subpaths with opposite winding contained in this outer
      const holes: Point[][] = [];
      for (let j = i + 1; j < polys.length; j++) {
        if (assigned.has(j)) continue;
        if (outerArea * polys[j].area < 0 &&
          this.isPointInPoly(polys[j].pts[0], outer)) {
          holes.push(polys[j].pts);
          assigned.add(j);
        }
      }

      if (holes.length > 0) {
        // Process rightmost holes first for correct bridge ordering
        holes.sort((a, b) => {
          const maxA = a.reduce((m, p) => Math.max(m, p.x), -Infinity);
          const maxB = b.reduce((m, p) => Math.max(m, p.x), -Infinity);
          return maxB - maxA;
        });
        for (const hole of holes) {
          outer = this.bridgeHole(outer, hole);
        }
      }

      result.push(outer);
    }

    return result;
  }

  private polySignedArea(poly: Point[]): number {
    let area = 0;
    for (let i = 0; i < poly.length; i++) {
      const j = (i + 1) % poly.length;
      area += poly[i].x * poly[j].y - poly[j].x * poly[i].y;
    }
    return area / 2;
  }

  private isPointInPoly(p: Point, poly: Point[]): boolean {
    let inside = false;
    for (let i = 0, j = poly.length - 1; i < poly.length; j = i++) {
      if ((poly[i].y > p.y) !== (poly[j].y > p.y) &&
        p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x) {
        inside = !inside;
      }
    }
    return inside;
  }

  private bridgeHole(outer: Point[], hole: Point[]): Point[] {
    // Find rightmost point of the hole
    let hIdx = 0;
    for (let i = 1; i < hole.length; i++) {
      if (hole[i].x > hole[hIdx].x) hIdx = i;
    }
    const hp = hole[hIdx];

    // Cast horizontal ray right from hp, find nearest intersecting outer edge
    let bestIdx = 0;
    let bestDist = Infinity;
    let foundEdge = false;

    for (let i = 0; i < outer.length; i++) {
      const j = (i + 1) % outer.length;
      const a = outer[i], b = outer[j];
      if (a.y === b.y) continue;
      if ((a.y - hp.y) * (b.y - hp.y) > 0) continue;
      const t = (hp.y - a.y) / (b.y - a.y);
      if (t < 0 || t > 1) continue;
      const ix = a.x + t * (b.x - a.x);
      if (ix >= hp.x && ix < bestDist) {
        bestDist = ix;
        // Pick the endpoint of this edge closest to hp
        const dI = Math.hypot(a.x - hp.x, a.y - hp.y);
        const dJ = Math.hypot(b.x - hp.x, b.y - hp.y);
        bestIdx = dI <= dJ ? i : j;
        foundEdge = true;
      }
    }

    if (!foundEdge) {
      // Fallback: nearest outer vertex
      bestDist = Infinity;
      for (let i = 0; i < outer.length; i++) {
        const d = Math.hypot(outer[i].x - hp.x, outer[i].y - hp.y);
        if (d < bestDist) { bestDist = d; bestIdx = i; }
      }
    }

    // Build combined polygon: outer[0..bestIdx] + hole walk + bridge back + outer[bestIdx+1..end]
    const result: Point[] = [];
    for (let i = 0; i <= bestIdx; i++) result.push(outer[i]);
    for (let i = 0; i < hole.length; i++) result.push(hole[(hIdx + i) % hole.length]);
    // Bridge return — tiny offset to avoid degenerate zero-length edges in ear-clipping
    result.push({ x: hole[hIdx].x, y: hole[hIdx].y + 0.01 });
    result.push({ x: outer[bestIdx].x + 0.01, y: outer[bestIdx].y });
    for (let i = bestIdx + 1; i < outer.length; i++) result.push(outer[i]);

    return result;
  }

  private findRoot(node: SVGNode): SVGNode | null {
    let current: SVGNode | null = node;
    while (current?.parent) current = current.parent;
    return current;
  }
}

// ─── SpaceSVG Public API ─────────────────────────────

@component
export class SpaceSVG extends BaseScriptComponent {
  @input
  @hint('Unlit material for mesh rendering (e.g., LegitUnlit)')
  material!: Material;

  @input
  @hint('World-space width of the SVG output')
  worldWidth: number = 20;

  @input
  @hint('World-space height of the SVG output')
  worldHeight: number = 20;

  private backend: SpaceSVGMeshBackend = new SpaceSVGMeshBackend();
  private xmlParser: SVGXMLParser = new SVGXMLParser();
  private meshObjects: SceneObject[] = [];

  parse(svgString: string): SVGNode {
    return this.xmlParser.parse(svgString);
  }

  loadSVG(svgString: string): void {
    const tree = this.parse(svgString);
    this.render(tree);
  }

  render(tree: SVGNode): void {
    this.clear();

    if (!this.material) {
      throw new SpaceSVGRenderError('No material assigned. Assign an unlit material in the Inspector.');
    }

    const groups = this.backend.buildMeshes(tree, this.worldWidth, this.worldHeight);
    print(`[SpaceSVG] Rendering ${groups.length} mesh groups`);

    for (let i = 0; i < groups.length; i++) {
      this.createMesh(groups[i], i);
    }
  }

  clear(): void {
    for (const obj of this.meshObjects) {
      obj.destroy();
    }
    this.meshObjects = [];
  }

  private createMesh(group: MeshGroup, index: number): void {
    const builder = new MeshBuilder([
      { name: 'position', components: 3 },
    ]);
    builder.topology = MeshTopology.Triangles;
    builder.indexType = MeshIndexType.UInt16;

    builder.appendVerticesInterleaved(group.vertices);
    builder.appendIndices(group.indices);

    if (!builder.isValid()) {
      print(`[SpaceSVG] Warning: invalid mesh group ${index}, skipping`);
      return;
    }

    builder.updateMesh();
    const mesh = builder.getMesh();

    const obj = global.scene.createSceneObject(`SpaceSVG_${index}`);
    obj.setParent(this.getSceneObject());

    const visual = obj.createComponent('Component.RenderMeshVisual') as RenderMeshVisual;
    visual.mesh = mesh;
    visual.mainMaterial = this.material;

    // Per-instance color override — doesn't affect other visuals sharing the material
    const overrides = visual.mainPassOverrides as any;
    overrides.baseColor = new vec4(
      group.color[0], group.color[1], group.color[2], group.color[3]
    );

    this.meshObjects.push(obj);
  }
}

// Export internals for testing and advanced usage
export {
  SVGNode,
  SVGXMLParser,
  SVGPathParser,
  SVGPathTessellator,
  SVGTriangulator,
  SVGStyleResolver,
  SpaceSVGMeshBackend,
  SpaceSVGParseError,
  SpaceSVGRenderError,
  parseColor,
  parseViewBox,
};
