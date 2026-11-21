// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// SpaceSVGTestSuite.ts — Conformance tests for SpaceSVG Phase 1
//
// Runs parser, style resolver, path parser, and triangulator tests at startup.
// Results are logged via print() to the Lens Studio console.

import {
  SVGNode,
  SVGXMLParser,
  SVGPathParser,
  SVGPathTessellator,
  SVGTriangulator,
  SVGStyleResolver,
  SpaceSVGMeshBackend,
  SpaceSVGParseError,
  parseColor,
  parseViewBox,
} from './SpaceSVG';

// ─── Test Runner ─────────────────────────────────────

let passed = 0;
let failed = 0;
let errors: string[] = [];

function assert(condition: boolean, name: string): void {
  if (condition) {
    passed++;
  } else {
    failed++;
    errors.push(name);
    print(`  FAIL: ${name}`);
  }
}

function assertApprox(a: number, b: number, name: string, epsilon: number = 0.01): void {
  assert(Math.abs(a - b) < epsilon, name);
}

// ─── XML Parser Tests ────────────────────────────────

function testXMLParser(): void {
  print('[TestSuite] XML Parser');
  const parser = new SVGXMLParser();

  // Basic element
  const n1 = parser.parse('<svg></svg>');
  assert(n1.tagName === 'svg', 'parse svg tag');

  // Attributes
  const n2 = parser.parse('<svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg"></svg>');
  assert(n2.getAttribute('viewBox') === '0 0 100 100', 'parse viewBox attr');

  // Self-closing
  const n3 = parser.parse('<svg><rect x="10" y="20"/></svg>');
  assert(n3.children.length === 1, 'self-closing child count');
  assert(n3.children[0].tagName === 'rect', 'self-closing tag name');
  assert(n3.children[0].getAttribute('x') === '10', 'self-closing attr');

  // Nested elements
  const n4 = parser.parse('<svg><g><rect/><circle/></g></svg>');
  assert(n4.children[0].tagName === 'g', 'nested g');
  assert(n4.children[0].children.length === 2, 'nested children count');

  // Entity decoding
  const n5 = parser.parse('<svg attr="a&amp;b&lt;c"></svg>');
  assert(n5.getAttribute('attr') === 'a&b<c', 'entity decode');

  // Comments
  const n6 = parser.parse('<svg><!-- comment --><rect/></svg>');
  assert(n6.children.length === 1, 'skip comments');

  // Text content
  const n7 = parser.parse('<svg><text>Hello</text></svg>');
  assert(n7.children[0].textContent === 'Hello', 'text content');

  // getElementById
  const n8 = parser.parse('<svg><rect id="r1"/><circle id="c1"/></svg>');
  assert(n8.getElementById('c1')?.tagName === 'circle', 'getElementById');
  assert(n8.getElementById('missing') === null, 'getElementById missing');

  // getElementsByTagName
  const n9 = parser.parse('<svg><g><rect/></g><rect/></svg>');
  assert(n9.getElementsByTagName('rect').length === 2, 'getElementsByTagName');

  // Error: mismatched tags
  let parseErr = false;
  try { parser.parse('<svg></div>'); } catch (e) { parseErr = true; }
  assert(parseErr, 'mismatched tag error');

  // Error: unclosed tag
  let unclosedErr = false;
  try { parser.parse('<svg><rect>'); } catch (e) { unclosedErr = true; }
  assert(unclosedErr, 'unclosed tag error');

  // XML prolog
  const n10 = parser.parse('<?xml version="1.0"?><svg></svg>');
  assert(n10.tagName === 'svg', 'skip xml prolog');

  // Clone
  const original = parser.parse('<svg><rect id="r"/></svg>');
  const cloned = original.clone();
  cloned.children[0].setAttribute('id', 'changed');
  assert(original.children[0].getAttribute('id') === 'r', 'clone independence');
}

// ─── Color Parser Tests ──────────────────────────────

function testColorParser(): void {
  print('[TestSuite] Color Parser');

  const red = parseColor('#ff0000');
  assert(red !== null, 'parse hex red');
  assertApprox(red![0], 1, 'red R');
  assertApprox(red![1], 0, 'red G');
  assertApprox(red![2], 0, 'red B');

  const short = parseColor('#0f0');
  assert(short !== null, 'parse short hex');
  assertApprox(short![1], 1, 'short hex G');

  const named = parseColor('blue');
  assert(named !== null, 'parse named blue');
  assertApprox(named![2], 1, 'named blue B');

  const rgb = parseColor('rgb(128, 64, 32)');
  assert(rgb !== null, 'parse rgb()');
  assertApprox(rgb![0], 128 / 255, 'rgb R');

  const rgba = parseColor('rgba(255, 0, 0, 0.5)');
  assert(rgba !== null, 'parse rgba()');
  assertApprox(rgba![3], 0.5, 'rgba A');

  assert(parseColor('none') === null, 'none returns null');
  assert(parseColor('transparent') === null, 'transparent returns null');
  assert(parseColor('') === null, 'empty returns null');
}

// ─── ViewBox Tests ───────────────────────────────────

function testViewBox(): void {
  print('[TestSuite] ViewBox');

  const vb = parseViewBox('0 0 100 200');
  assert(vb !== null, 'parse viewBox');
  assert(vb!.minX === 0, 'vb minX');
  assert(vb!.width === 100, 'vb width');
  assert(vb!.height === 200, 'vb height');

  const vbComma = parseViewBox('10,20,300,400');
  assert(vbComma !== null, 'parse viewBox with commas');
  assert(vbComma!.minX === 10, 'vbComma minX');

  assert(parseViewBox(null) === null, 'null viewBox');
  assert(parseViewBox('invalid') === null, 'invalid viewBox');
}

// ─── Path Parser Tests ───────────────────────────────

function testPathParser(): void {
  print('[TestSuite] Path Parser');
  const parser = new SVGPathParser();

  // Basic moveto/lineto
  const cmds1 = parser.parse('M 10 20 L 30 40');
  assert(cmds1.length === 2, 'M L count');
  assert(cmds1[0].type === 'M', 'first is M');
  assert(cmds1[0].args[0] === 10 && cmds1[0].args[1] === 20, 'M args');
  assert(cmds1[1].type === 'L', 'second is L');

  // Implicit lineto after M
  const cmds2 = parser.parse('M 0 0 10 20 30 40');
  assert(cmds2.length === 3, 'implicit L count');
  assert(cmds2[1].type === 'L', 'implicit L type');

  // H and V
  const cmds3 = parser.parse('M 0 0 H 50 V 50');
  assert(cmds3.length === 3, 'H V count');
  assert(cmds3[1].type === 'H' && cmds3[1].args[0] === 50, 'H args');
  assert(cmds3[2].type === 'V' && cmds3[2].args[0] === 50, 'V args');

  // Close path
  const cmds4 = parser.parse('M 0 0 L 10 0 L 10 10 Z');
  assert(cmds4[cmds4.length - 1].type === 'Z', 'Z command');

  // Cubic bezier
  const cmds5 = parser.parse('M 0 0 C 10 20 30 40 50 60');
  assert(cmds5[1].type === 'C', 'C command');
  assert(cmds5[1].args.length === 6, 'C args count');

  // Quadratic bezier
  const cmds6 = parser.parse('M 0 0 Q 25 50 50 0');
  assert(cmds6[1].type === 'Q', 'Q command');
  assert(cmds6[1].args.length === 4, 'Q args count');

  // Arc
  const cmds7 = parser.parse('M 0 0 A 25 25 0 0 1 50 50');
  assert(cmds7[1].type === 'A', 'A command');
  assert(cmds7[1].args.length === 7, 'A args count');

  // Relative commands
  const cmds8 = parser.parse('m 10 20 l 5 5');
  assert(cmds8[0].type === 'm', 'relative m');
  assert(cmds8[1].type === 'l', 'relative l');

  // Compact notation (no spaces)
  const cmds9 = parser.parse('M10,20L30,40');
  assert(cmds9.length === 2, 'compact notation');
  assert(cmds9[0].args[0] === 10, 'compact M x');

  // Negative numbers
  const cmds10 = parser.parse('M -10 -20 L -30 40');
  assert(cmds10[0].args[0] === -10, 'negative x');

  // Multiple shapes (repeated M)
  const cmds11 = parser.parse('M 0 0 L 10 10 M 20 20 L 30 30');
  assert(cmds11.length === 4, 'multiple M commands');
}

// ─── Tessellator Tests ───────────────────────────────

function testTessellator(): void {
  print('[TestSuite] Path Tessellator');
  const parser = new SVGPathParser();
  const tess = new SVGPathTessellator();

  // Triangle path
  const tri = tess.tessellate(parser.parse('M 0 0 L 10 0 L 5 10 Z'));
  assert(tri.length === 1, 'triangle subpath count');
  assert(tri[0].length >= 3, 'triangle has points');

  // Multiple subpaths
  const multi = tess.tessellate(parser.parse('M 0 0 L 10 0 Z M 20 20 L 30 20 Z'));
  assert(multi.length === 2, 'multiple subpaths');

  // Cubic bezier generates points
  const cubic = tess.tessellate(parser.parse('M 0 0 C 0 10 10 10 10 0'));
  assert(cubic.length === 1, 'cubic subpath');
  assert(cubic[0].length > 2, 'cubic has tessellated points');

  // Arc generates points
  const arc = tess.tessellate(parser.parse('M 0 0 A 10 10 0 0 1 20 0'));
  assert(arc.length === 1, 'arc subpath');
  assert(arc[0].length > 2, 'arc has tessellated points');
}

// ─── Triangulator Tests ──────────────────────────────

function testTriangulator(): void {
  print('[TestSuite] Triangulator');
  const tri = new SVGTriangulator();

  // Simple triangle
  const t1 = tri.triangulate([{ x: 0, y: 0 }, { x: 10, y: 0 }, { x: 5, y: 10 }]);
  assert(t1.length === 3, 'triangle indices count');

  // Quad
  const t2 = tri.triangulate([
    { x: 0, y: 0 }, { x: 10, y: 0 }, { x: 10, y: 10 }, { x: 0, y: 10 },
  ]);
  assert(t2.length === 6, 'quad indices count (2 triangles)');

  // Pentagon
  const pent = [];
  for (let i = 0; i < 5; i++) {
    const a = (2 * Math.PI * i) / 5 - Math.PI / 2;
    pent.push({ x: Math.cos(a) * 10, y: Math.sin(a) * 10 });
  }
  const t3 = tri.triangulate(pent);
  assert(t3.length === 9, 'pentagon indices count (3 triangles)');

  // Degenerate (< 3 points)
  assert(tri.triangulate([]).length === 0, 'empty polygon');
  assert(tri.triangulate([{ x: 0, y: 0 }]).length === 0, 'single point');
  assert(tri.triangulate([{ x: 0, y: 0 }, { x: 1, y: 1 }]).length === 0, 'two points');

  // Stroke triangulation
  const stroke = tri.triangulateStroke([
    { x: 0, y: 0 }, { x: 10, y: 0 },
  ], 2);
  assert(stroke.vertices.length === 4, 'stroke vertices (2 pairs)');
  assert(stroke.indices.length === 6, 'stroke indices (2 triangles)');
}

// ─── Style Resolver Tests ────────────────────────────

function testStyleResolver(): void {
  print('[TestSuite] Style Resolver');
  const resolver = new SVGStyleResolver();
  const parser = new SVGXMLParser();

  // Default fill
  const n1 = new SVGNode('rect');
  const s1 = resolver.resolve(n1);
  assert(s1.fill === '#000000', 'default fill');
  assert(s1.stroke === 'none', 'default stroke');

  // Attribute override
  const n2 = new SVGNode('rect');
  n2.setAttribute('fill', 'red');
  n2.setAttribute('stroke', 'blue');
  n2.setAttribute('stroke-width', '3');
  const s2 = resolver.resolve(n2);
  assert(s2.fill === 'red', 'fill attr');
  assert(s2.stroke === 'blue', 'stroke attr');
  assert(s2.strokeWidth === 3, 'stroke-width attr');

  // Style attribute overrides presentation attr
  const n3 = new SVGNode('rect');
  n3.setAttribute('fill', 'red');
  n3.setAttribute('style', 'fill:blue');
  const s3 = resolver.resolve(n3);
  assert(s3.fill === 'blue', 'style overrides attr');

  // Inheritance
  const tree = parser.parse('<svg fill="green"><rect/></svg>');
  const parentStyle = resolver.resolve(tree);
  const childStyle = resolver.resolve(tree.children[0], parentStyle);
  assert(childStyle.fill === 'green', 'inherited fill');

  // Transform parsing
  const t1 = resolver.parseTransform('translate(10, 20)');
  assertApprox(t1[4], 10, 'translate x');
  assertApprox(t1[5], 20, 'translate y');

  const t2 = resolver.parseTransform('scale(2)');
  assertApprox(t2[0], 2, 'scale x');
  assertApprox(t2[3], 2, 'scale y');

  const t3 = resolver.parseTransform('rotate(90)');
  assertApprox(t3[0], 0, 'rotate cos', 0.001);
  assertApprox(t3[1], 1, 'rotate sin', 0.001);

  // Composed transforms
  const t4 = resolver.parseTransform('translate(10,0) scale(2)');
  assertApprox(t4[0], 2, 'composed scale');
  assertApprox(t4[4], 10, 'composed translate');
}

// ─── Integration Tests ───────────────────────────────

function testIntegration(): void {
  print('[TestSuite] Integration');
  const parser = new SVGXMLParser();
  const backend = new SpaceSVGMeshBackend();

  // Basic rect
  const groups1 = backend.buildMeshes(
    parser.parse('<svg viewBox="0 0 100 100"><rect x="10" y="10" width="80" height="80" fill="red"/></svg>'),
    10, 10
  );
  assert(groups1.length >= 1, 'rect produces mesh groups');
  assert(groups1[0].vertices.length > 0, 'rect has vertices');
  assert(groups1[0].indices.length > 0, 'rect has indices');
  assertApprox(groups1[0].color[0], 1, 'rect color R');

  // Circle
  const groups2 = backend.buildMeshes(
    parser.parse('<svg viewBox="0 0 100 100"><circle cx="50" cy="50" r="30" fill="blue"/></svg>'),
    10, 10
  );
  assert(groups2.length >= 1, 'circle produces groups');

  // Path
  const groups3 = backend.buildMeshes(
    parser.parse('<svg viewBox="0 0 100 100"><path d="M 10 10 L 90 10 L 50 90 Z" fill="green"/></svg>'),
    10, 10
  );
  assert(groups3.length >= 1, 'path triangle produces groups');

  // Group with transform
  const groups4 = backend.buildMeshes(
    parser.parse('<svg viewBox="0 0 100 100"><g transform="translate(10,10)"><rect x="0" y="0" width="20" height="20" fill="red"/></g></svg>'),
    10, 10
  );
  assert(groups4.length >= 1, 'group transform produces groups');

  // Defs/use
  const groups5 = backend.buildMeshes(
    parser.parse('<svg viewBox="0 0 100 100"><defs><rect id="r" width="10" height="10"/></defs><use href="#r" x="20" y="20" fill="blue"/></svg>'),
    10, 10
  );
  assert(groups5.length >= 1, 'defs/use produces groups');

  // Stroke-only
  const groups6 = backend.buildMeshes(
    parser.parse('<svg viewBox="0 0 100 100"><line x1="10" y1="50" x2="90" y2="50" stroke="black" stroke-width="4"/></svg>'),
    10, 10
  );
  assert(groups6.length >= 1, 'stroke-only line produces groups');

  // Display none
  const groups7 = backend.buildMeshes(
    parser.parse('<svg viewBox="0 0 100 100"><rect width="100" height="100" fill="red" display="none"/></svg>'),
    10, 10
  );
  assert(groups7.length === 0, 'display:none produces no groups');

  // Empty SVG
  const groups8 = backend.buildMeshes(
    parser.parse('<svg viewBox="0 0 100 100"></svg>'),
    10, 10
  );
  assert(groups8.length === 0, 'empty svg produces no groups');

  // fill="none" with stroke
  const groups9 = backend.buildMeshes(
    parser.parse('<svg viewBox="0 0 100 100"><circle cx="50" cy="50" r="30" fill="none" stroke="red" stroke-width="2"/></svg>'),
    10, 10
  );
  assert(groups9.length >= 1, 'fill none + stroke produces groups');
}

// ─── Error Boundary Tests ────────────────────────────

function testErrors(): void {
  print('[TestSuite] Error Boundaries');
  const parser = new SVGXMLParser();

  // Malformed XML
  let err1 = false;
  try { parser.parse('not xml at all'); } catch (e) { err1 = true; }
  assert(err1, 'malformed xml throws');

  // Unclosed tag
  let err2 = false;
  try { parser.parse('<svg><rect>'); } catch (e) { err2 = true; }
  assert(err2, 'unclosed tag throws');

  // Mismatched tags
  let err3 = false;
  try { parser.parse('<svg></div>'); } catch (e) { err3 = true; }
  assert(err3, 'mismatched tags throws');

  // Backend handles bad SVG gracefully
  const backend = new SpaceSVGMeshBackend();
  const emptyRect = backend.buildMeshes(
    parser.parse('<svg viewBox="0 0 100 100"><rect width="0" height="0" fill="red"/></svg>'),
    10, 10
  );
  assert(emptyRect.length === 0, 'zero-size rect produces no groups');
}

// ─── Run All Tests ───────────────────────────────────

@component
export default class SpaceSVGTestSuite extends BaseScriptComponent {
  onAwake() {
    print('═══════════════════════════════════════');
    print('[SpaceSVG Test Suite] Running...');
    print('═══════════════════════════════════════');

    passed = 0;
    failed = 0;
    errors = [];

    testXMLParser();
    testColorParser();
    testViewBox();
    testPathParser();
    testTessellator();
    testTriangulator();
    testStyleResolver();
    testIntegration();
    testErrors();

    print('═══════════════════════════════════════');
    print(`[SpaceSVG Test Suite] ${passed} passed, ${failed} failed`);
    if (errors.length > 0) {
      print(`  Failed: ${errors.join(', ')}`);
    }
    print('═══════════════════════════════════════');
  }
}
