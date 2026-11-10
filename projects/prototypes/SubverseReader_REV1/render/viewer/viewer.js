(function () {
"use strict";
// SubverseReader 3D review. Units: millimetres. Device front faces +Z, Y is up.
// Models are built from the same layout numbers as render/make_renders.py.

const ZINE = {
  basement: ["How to Run a Basement Show", "Sweep the floor before anyone sees it. Tape the setlist to the water heater. Somebody has to stand at the door with a coffee can and a smile, and it should be the person who knows every band's mother by name. Count the room twice: once for the landlord, once for the joy of it.\n\nThe PA is two speakers and a promise. Point them at the wall, not the neighbours. When the bass player blows a fuse, the show is not over; it is unplugged. Pass the hat, then pass the mop."],
  loadin: ["Notes From the Load-In", "Four in the afternoon and the venue smells like last night. We carry the cabs up the alley stairs because the freight lift has been broken since the first tour, and the stage manager writes our names on gaffer tape in silver pen.\n\nEvery pass on every lanyard is a small permission: this hallway, that door, the green room with the sad fruit plate. The zine table goes by the bar, where the light is good and the floor is sticky. We sell out of issue six before doors.\n\nSoundcheck is the only quiet hour. The drummer reads, the singer sleeps on a road case, and the merch kid counts shirts by size in a whisper that sounds like a prayer. Then the house lights drop, and nobody reads anything for two hours."],
  copier: ["Photocopier Love Letters", "The copy shop on Fifth stays open until two. We pay in quarters and make the toner do things it was never sold to do: grey skies out of black, a whole crowd out of a single blurred hand.\n\nFold, staple, trim. A zine is a letter written to whoever finds it."],
  drive: ["The Long Drive Home", "After the last show of the run we drive through the night with the windows down to stay awake. The van has one working speaker and a tape deck that eats every third cassette, so we take turns reading aloud instead: liner notes, gas station maps, the back of a cereal box, and finally the zine somebody handed us at the merch table in a city none of us can name anymore.\n\nIt is twelve pages of xeroxed photographs and handwriting about a skate park that was paved over. Nobody says much after it. The sky goes from black to the colour of a bruise to the colour of a peach, and the bassist, who has not slept, says that is the whole point of the thing: someone made it, someone carried it, someone else read it out loud on a highway at five in the morning.\n\nWe get home at nine. The first thing I do is staple my own."],
  nightbus: ["Reading on the Night Bus", "The 4 a.m. bus has three passengers and one working light, and the light is over the driver. So the pass glows a little, warm and low, enough for the page and not enough to wake the man asleep against the window.\n\nWe wrote this issue for the ride home: short pieces, one per stop, each finished before the doors open again."],
  quiet: ["No Signal", "There is no Wi-Fi in this thing. Not off; absent. It cannot be asked where it is, what you read, or for how long. It takes pages over a short Bluetooth hop or a USB cable, and then it keeps quiet.\n\nThat makes it welcome in places radios are not: the reading room, the ward, the flight, the classroom. It also makes the battery last a very long time."],
  pan: ["Flyer Wall, Zoomed", "This page is a scan of the wall outside the Corner Bar: forty flyers stapled over each other since 1994. On a small screen it is mush, so the pass keeps it at twice the panel's resolution and the thumbstick moves a window around it. Push and the view slides; the little map in the corner shows where you are.\n\nZines are made for paper, and paper does not reflow. Panning keeps the layout the maker chose instead of tearing it into columns."],
  tap: ["Tapped at the Merch Table", "There is a sticker on the corner of every poster on this tour, and another on the merch table, the record sleeves and the back of the bathroom door. Tap the back of your pass on one and the zine behind it is yours: a link to the full issue on Subverse, or, on the bigger tags, a short piece stored right there in the sticker.\n\nThis one came off the flyer by the bar. The full issue arrives the next time the pass finds Wi-Fi or your phone."],
  sched: ["Day 2 · Running Order", "16:00  Doors\n17:10  Copy Shop\n18:20  Night Bus\n19:40  Basement Saint\n21:00  Long Drive Home\n23:00  Curfew"],
  stage: ["Stage Plot / Set One", "1. Doors (instrumental, cue lights on the snare hit)\n2. Basement Saint\n3. Night Bus to Nowhere\n4. Copy Shop  (capo 2)\n5. Long Drive Home\n6. Encore: whatever the room wants\n\nNotes\n- Bass DI stage left, two wedges\n- House kills haze after song 3\n- Merch closes at 11:40\n- Load-out by the alley door"],
};
const MORE = "\n\nWe print a hundred copies and leave them in laundromats, record shops and the back booth of the diner where the night cook draws comics on the order tickets. Nobody tracks where they go. That is the deal: you make the thing, you let it walk away, and sometimes it comes back years later with somebody else's handwriting in the margins and a ticket stub for a show you also played.\n\nThis issue was laid out on a borrowed laptop and proofed on the bus. Mistakes are ours. Copy it, pass it on, read it on whatever you have.";
const POSTER_BODY = "Four bands, one basement, a photocopier that only prints in red and yellow. This issue is a poster first and a zine second: unfold it, pin it up, read the back on the bus. Covers, flyers and the pass itself are where colour earns its slow refresh; the stories inside stay black and white.";

const C5 = "Wi-Fi 6, 2.4 + 5 GHz · BLE 5", S3 = "Wi-Fi 4, 2.4 GHz only · BLE 5";
const V = [
  {key:"SR-1",name:"Laminate",band:"GUEST",W:86,T:12.5,aa:[63.6,84.8],res:[300,400],depth:"1",em:3.3,body:"#e2642a",bandBg:"#171717",bandFg:"#ffffff",strap:"#1b1b1b",controls:"three",ctrl:15,zine:"basement",lanyard:true,sd:false,
   p5:40.04,p100:31.94,dims:"86 × 138 × 12.5",g:95,panel:"4.2in 400×300, 1-bit (GDEY042T81)",pshort:"4.2in e-paper · 400×300 · 1-bit",chip:"XIAO ESP32-S3 Plus on the Seeed EE05 board",radio:"Wi-Fi 4, 2.4 GHz only; BLE 5",keys:"PREV · MENU · NEXT",sdT:"No (books live in 16 MB flash)",usb:"Charge, USB drive, console",back:"Plain, spec label",wear:"Lanyard",tag:"No custom PCB: the EE05 board carries the panel driver, keys and battery switch."},
  {key:"SR-15",name:"Dial Pass",band:"SOUNDCHECK",W:95,T:16,Hfix:140,aa:[63.6,84.8],res:[300,400],depth:"1",em:3.3,body:"#b23a8f",bandBg:"#171717",bandFg:"#ffffff",strap:"#1b1b1b",controls:"dial",ctrl:14,zine:"basement",lanyard:true,sd:true,
   p5:38.50,p100:26.80,dims:"95 × 140 × 16",g:150,panel:"4.2in 400×300, 1-bit (GDEY042T81)",pshort:"4.2in e-paper · 400×300 · 1-bit",chip:"Elecrow CrowPanel 4.2in (ESP32-S3, finished board)",radio:"Wi-Fi 4, 2.4 GHz only; BLE 5",keys:"Thumb dial (up · down · push) · MENU · EXIT",sdT:"Yes (on the board)",usb:"Charge, console (CH340), Web Serial loader",back:"Plain, spec label",wear:"Lanyard",tag:"A finished CrowPanel board in our shell; the thumb dial turns pages."},
  {key:"SR-16",name:"Setlist",band:"SETLIST",W:64,T:12,Hfix:120,aa:[47.04,81.54],res:[240,416],depth:"1",em:3.3,body:"#1f8f84",bandBg:"#171717",bandFg:"#ffffff",strap:"#1b1b1b",controls:"edge3",ctrl:0,zine:"basement",lanyard:true,sd:false,
   p5:37.09,p100:24.46,dims:"64 × 120 × 12",g:72,panel:"3.7in 240×416, 1-bit (Elecrow DIE04037S, UC8253)",pshort:"3.7in e-paper · 240×416 · 1-bit",chip:"XIAO ESP32-C5 + Seeed ePaper Driver Board",radio:C5,keys:"PREV · MENU · NEXT on the right edge",sdT:"No (books live in flash)",usb:"Charge, console, Web Serial loader",back:"Plain, spec label",wear:"Lanyard",tag:"The slim pass: a sharp 3.7in portrait panel, laminate-sized, off-the-shelf parts."},
  {key:"SR-1C",name:"Laminate C5",band:"GUEST",W:86,T:13,aa:[63.6,84.8],res:[300,400],depth:"1",em:3.3,body:"#2a6fe2",bandBg:"#171717",bandFg:"#ffffff",strap:"#1b1b1b",controls:"three",ctrl:15,zine:"basement",lanyard:true,sd:false,
   p5:43.45,p100:34.39,dims:"86 × 138 × 13",g:100,panel:"4.2in 400×300, 1-bit (GDEY042T81)",pshort:"4.2in e-paper · 400×300 · 1-bit",chip:"DFRobot FireBeetle 2 ESP32-C5 + Crystalfontz CFA-10084 adapter",radio:C5,keys:"PREV · MENU · NEXT",sdT:"No (books live in flash)",usb:"Charge, console, Web Serial loader",back:"Plain, spec label",wear:"Lanyard",tag:"SR-1 with dual-band Wi-Fi: FireBeetle 2 C5, hand-wired keys."},
  {key:"SR-1D",name:"Laminate Direct",band:"GUEST",W:86,T:12.5,aa:[63.6,84.8],res:[300,400],depth:"1",em:3.3,body:"#d8b21f",bandBg:"#171717",bandFg:"#ffffff",strap:"#1b1b1b",controls:"three",ctrl:15,zine:"basement",lanyard:true,sd:false,
   p5:41.79,p100:25.45,dims:"86 × 138 × 12.5",g:95,panel:"4.2in 400×300, 1-bit (GDEY042T81)",pshort:"4.2in e-paper · 400×300 · 1-bit",chip:"XIAO ESP32-C5 + Seeed ePaper Driver Board, all from China",radio:C5,keys:"PREV · MENU · NEXT",sdT:"No (books live in flash)",usb:"Charge, console, Web Serial loader",back:"Plain, spec label",wear:"Lanyard",tag:"The cheapest SR-1: one Seeed parcel and one AliExpress order, shipped to Japan."},
  {key:"SR-2",name:"All Access",band:"ALL ACCESS",W:109,T:15.5,aa:[88.2,118.8],res:[480,648],depth:"1",em:3.2,body:"#1e1f22",bandBg:"#f2b705",bandFg:"#111111",strap:"#f2b705",controls:"dpad",ctrl:19,zine:"loadin",lanyard:true,sd:true,pocket:"card",card:"#0f6f78",cardTxt:"ZINE CLUB",
   p5:54.99,p100:34.46,dims:"109 × 177 × 19.5 (15.5 without pocket)",g:150,panel:"5.83in 648×480, 1-bit (GDEY0583T81)",pshort:"5.83in e-paper · 648×480 · 1-bit",chip:"ESP32-S3 N16R8, custom PCB",radio:S3,keys:"5-way + PREV / NEXT",sdT:"Yes",usb:"Charge, USB drive, console",back:"Card pocket with window: ID-1 card + ticket",wear:"Lanyard",tag:"The product candidate. The panel is $24.20 of it."},
  {key:"SR-3",name:"Gray Pass",band:"PHOTO PIT",W:78,T:14.5,aa:[58.3,103.7],res:[540,960],depth:"16",em:3.0,body:"#12858c",bandBg:"#ecebe6",bandFg:"#111111",strap:"#3d3d40",controls:"side3",ctrl:7,zine:"copier",lanyard:true,sd:true,
   p5:36.55,p100:26.81,dims:"78 × 149 × 14.5",g:110,panel:"4.7in 960×540, 16-grey (ED047TC1)",pshort:"4.7in e-paper · 960×540 · 16 greys",chip:"M5Stack PaperDIY kit (ESP32-S3)",radio:S3,keys:"UP · OK · DOWN on the right edge + HOME",sdT:"Yes (on the kit)",usb:"Charge, USB drive, console",back:"Plain, spec label",wear:"Lanyard",tag:"Cheapest 16-grey page: M5Stack's $25 kit in our shell. The kit is sold out; on a LilyGo T5 4.7in S3 it is $51.54."},
  {key:"SR-4",name:"Headliner",band:"BACKSTAGE",W:109,T:13.5,aa:[90.6,122.4],res:[1072,1448],depth:"16",em:2.9,body:"#e3dbc9",bandBg:"#b3161b",bandFg:"#ffffff",strap:"#b3161b",controls:"rockers",ctrl:11,zine:"drive",lanyard:true,sd:true,pocket:"card",card:"#161616",cardTxt:"ADMIT ONE",
   p5:67.85,p100:41.30,dims:"109 × 172 × 17.5 (13.5 without pocket)",g:150,panel:"6in 1448×1072, 300 ppi, 16-grey (ED060KC1)",pshort:"6in e-paper · 1448×1072 · 300 ppi",chip:"ESP32-S3 + TPS65185, 4-layer PCB",radio:S3,keys:"Page rockers on both edges + BACK · MENU · SYNC",sdT:"Yes",usb:"Charge, USB drive, console",back:"Card pocket with window",wear:"Lanyard",tag:"Stretch: pick it only if a Kindle-class page is the point."},
  {key:"SR-5",name:"Crew Clip",band:"CREW",W:104,T:16,aa:[84.8,63.6],res:[400,300],depth:"1",em:3.3,body:"#5a6241",bandBg:"#ff6a10",bandFg:"#111111",strap:null,controls:"wheel",ctrl:14,zine:"stage",lanyard:false,sd:true,clip:true,
   p5:39.76,p100:23.14,dims:"104 × 109 × 21.5 with clip (16 body)",g:110,panel:"4.2in 400×300, landscape",pshort:"4.2in e-paper · 400×300 · landscape",chip:"ESP32-C5 custom PCB",radio:C5,keys:"Thumbwheel + BACK / OK",sdT:"Yes (it is the mass loader)",usb:"Charge, console, Web Serial loader",back:"Spring belt clip",wear:"Belt clip",tag:"SR-1 on its own board, worn on a belt. Cheapest at volume."},
  {key:"SR-6",name:"Night Shift",band:"AFTER HOURS",W:88,T:14.5,aa:[63.6,84.8],res:[300,400],depth:"1",em:3.3,body:"#1c2542",bandBg:"#f2a33a",bandFg:"#111111",strap:"#1c2542",controls:"four",ctrl:15,zine:"nightbus",lanyard:true,sd:true,lit:true,
   p5:48.61,p100:29.52,dims:"88 × 138 × 14.5",g:105,panel:"4.2in 400×300 with front light (GDEY042T81-FL02)",pshort:"4.2in e-paper + front light",chip:"ESP32-C5 custom PCB + LED driver",radio:C5,keys:"PREV · MENU · LIGHT · NEXT",sdT:"Yes",usb:"Charge, console, Web Serial loader",back:"Plain, spec label",wear:"Lanyard",tag:"E-paper can't be backlit; a light guide over the glass does the job."},
  {key:"SR-7",name:"Poster",band:"VIP",W:109,T:15.5,aa:[88.2,118.8],res:[480,648],depth:"4c",em:3.2,body:"#bf2a20",bandBg:"#161616",bandFg:"#f2c12e",strap:"#161616",controls:"dpad",ctrl:19,zine:"poster",lanyard:true,sd:true,pocket:"zine",card:"#e8e1cb",cardTxt:"MINI ZINE #7",
   p5:55.09,p100:34.51,dims:"109 × 177 × 19.5 (15.5 without sleeve)",g:150,panel:"5.83in 648×480, 4-colour B/W/R/Y (GDEY0583F41)",pshort:"5.83in e-paper · 4-colour B/W/R/Y",chip:"ESP32-S3, same board as SR-2",radio:S3,keys:"5-way + PREV / NEXT",sdT:"Yes",usb:"Charge, USB drive, console",back:"Sleeve for an A7 paper mini-zine",wear:"Lanyard",tag:"Colour for covers, flyers and the badge; about 12 s per refresh."},
  {key:"SR-8",name:"Quiet Pass",band:"OFFLINE",W:86,T:13,aa:[63.6,84.8],res:[300,400],depth:"1",em:3.3,body:"#3b2b57",bandBg:"#ecebe6",bandFg:"#111111",strap:"#3b2b57",controls:"three",ctrl:15,zine:"quiet",lanyard:true,sd:true,
   p5:46.84,p100:34.83,dims:"86 × 138 × 13",g:95,panel:"4.2in 400×300, 1-bit",pshort:"4.2in e-paper · 400×300 · 1-bit",chip:"XIAO nRF52840 Plus on the Seeed EE05 board",radio:"BLE 5 only, no Wi-Fi",keys:"PREV · MENU · NEXT",sdT:"Yes",usb:"Charge, USB drive, UF2 firmware updates",back:"Plain, spec label",wear:"Lanyard",tag:"No Wi-Fi radio at all. Lowest sleep current of the set."},
  {key:"SR-9",name:"Roadie",band:"ROADIE",W:88,T:14.5,aa:[63.6,84.8],res:[300,400],depth:"1",em:3.3,body:"#1f6b3a",bandBg:"#161616",bandFg:"#f0f0ea",strap:"#161616",controls:"stick",ctrl:26,zine:"pan",lanyard:true,sd:true,pan:true,
   p5:42.04,p100:25.36,dims:"88 × 149 × 14.5",g:105,panel:"4.2in 400×300, 1-bit",pshort:"4.2in e-paper · 400×300 · 1-bit",chip:"ESP32-C5 custom PCB",radio:C5,keys:"PSP thumbstick (Adafruit 444) + OK / BACK",sdT:"Yes",usb:"Charge, console, Web Serial loader",back:"Plain, spec label",wear:"Lanyard",tag:"The thumbstick pans a 2× zine page so the layout stays as printed."},
  {key:"SR-14",name:"Tap Pass",band:"TAP IN",W:80,T:12.5,Hfix:155,aa:[51.84,86.4],res:[480,800],depth:"1",em:3.0,body:"#3a6ea5",bandBg:"#111214",bandFg:"#7fe0ff",strap:"#3a6ea5",controls:"tap",ctrl:30,zine:"tap",lanyard:true,sd:true,
   head:["FROM TAG · MERCH TABLE","NEW"],edge:[[1,14,12,"UP"],[1,0,7,"OK"],[1,-14,12,"DOWN"]],
   p5:52.28,p100:31.92,dims:"80 × 155 × 12.5",g:95,panel:"3.97in 800×480, 1-bit, 235 ppi (GDEM0397T81P)",pshort:"3.97in e-paper · 800×480 · 235 ppi",chip:"Wi-Fi build: ESP32-C5 · tethered build: nRF52840 ($53.78 / $33.28)",radio:"Wi-Fi build: Wi-Fi 6 2.4 + 5 GHz + BLE · tethered: BLE only · both: NFC reader + tag",keys:"TAP on the front; UP · OK · DOWN on the right edge",sdT:"Yes",usb:"Wi-Fi build: charge, Web Serial · tethered: USB drive, UF2",back:"NFC tap target over the reader coil",wear:"Lanyard",tag:"Tap a tag on a poster or flyer: a link to the zine, or a short zine stored in the tag."},
  {key:"SR-10",name:"Credential",band:null,W:85,T:8.5,Hfix:55,top:3,aa:[66.9,29.1],res:[296,128],depth:"1",em:2.5,body:"#e8e6df",bandBg:"#2350c8",bandFg:"#ffffff",strap:"#2350c8",controls:"edge",ctrl:0,zine:"sched",lanyard:true,sd:true,
   edge:[[1,7,8,"UP"],[1,-1.5,6,"OK"],[1,-10,8,"DOWN"]],std:"85 × 55 mm card (CR80) · ≈ 3.5 × 2 in",
   p5:30.95,p100:16.79,dims:"85 × 55 × 8.5",g:40,panel:"2.9in 296×128, 1-bit (GDEY029T94)",pshort:"2.9in e-paper · 296×128 · 1-bit",chip:"ESP32-C5 custom PCB",radio:C5,keys:"UP · OK · DOWN on the right edge",sdT:"Yes",usb:"Charge, console, Web Serial loader",back:"Plain, spec label",wear:"Lanyard (slot punched like a card badge)",tag:"A credential and schedule screen, not a book reader: about eight lines of text."},
  {key:"SR-11",name:"Full Bleed",band:null,screenBand:"ALL ACCESS",W:76.2,T:11,Hfix:101.6,top:2.5,aa:[51.84,86.4],res:[480,800],depth:"1",em:3.0,body:"#151517",bandBg:"#ff4f9a",bandFg:"#111111",strap:"#ff4f9a",controls:"edge",ctrl:0,zine:"loadin",lanyard:true,bail:true,sd:true,
   edge:[[1,14,12,"UP"],[1,0,7,"OK"],[1,-14,12,"DOWN"],[-1,14,12,"BACK"]],std:"3 × 4 in (76.2 × 101.6 mm)",
   p5:41.88,p100:25.10,dims:"76.2 × 101.6 × 11 (+5 D-ring)",g:70,panel:"3.97in 800×480, 1-bit, 235 ppi (GDEM0397T81P)",pshort:"3.97in e-paper · 800×480 · 235 ppi",chip:"ESP32-C5 custom PCB",radio:C5,keys:"UP · OK · DOWN right edge, BACK left edge",sdT:"Yes",usb:"Charge, console, Web Serial loader",back:"D-ring for the lanyard, spec label",wear:"Lanyard (D-ring on the back)",tag:"All screen: the pass graphic is drawn on the page, not printed on a band."},
  {key:"SR-12",name:"Tour Laminate",band:"TOUR",W:80,T:12,Hfix:145,aa:[51.84,86.4],res:[480,800],depth:"1",em:3.0,body:"#c9962a",bandBg:"#161616",bandFg:"#f2c12e",strap:"#161616",controls:"dpad",ctrl:20,zine:"drive",lanyard:true,sd:true,std:"80 × 145 mm laminate",
   p5:43.54,p100:26.06,dims:"80 × 145 × 12",g:90,panel:"3.97in 800×480, 1-bit, 235 ppi (GDEM0397T81P)",pshort:"3.97in e-paper · 800×480 · 235 ppi",chip:"ESP32-C5 custom PCB",radio:C5,keys:"5-way + PREV / NEXT",sdT:"Yes",usb:"Charge, console, Web Serial loader",back:"Plain, spec label",wear:"Lanyard",tag:"The everyday reader at a sub-4 in width, with the sharpest SPI page."},
  {key:"SR-13",name:"Hybrid Credential",band:"ARTIST",W:94,T:14.5,Hfix:216,aa:[58.3,103.7],res:[540,960],depth:"16",em:3.0,body:"#2b2f36",bandBg:"#e6e5df",bandFg:"#111111",strap:"#c62f7a",controls:"three",ctrl:12,cred:58,zine:"copier",lanyard:true,sd:true,std:"3.69 × 8.49 in (94 × 216 mm)",
   p5:39.05,p100:27.96,dims:"94 × 216 × 14.5",g:165,panel:"4.7in 960×540, 16-grey (ED047TC1)",pshort:"4.7in e-paper · 960×540 · 16 greys",chip:"M5Stack PaperDIY kit (ESP32-S3)",radio:S3,keys:"PREV · MENU · NEXT",sdT:"Yes (on the kit)",usb:"Charge, USB drive, console",back:"Plain, spec label",wear:"Lanyard",tag:"Reader on top, the venue's printed ID-1 credential in a window below."},
];
V.sort((a, b) => parseInt(a.key.slice(3)) - parseInt(b.key.slice(3)));
const BY = Object.fromEntries(V.map(v => [v.key, v]));

// ------------------------------------------------------------------ layout
function layout(v) {
  const g = 2.8, slot_h = v.lanyard && !v.bail ? 4.5 : 0, top = v.top || 5.5;
  const band_h = v.band ? (v.W > 90 ? 10.5 : 9.5) : 0, scr_h = v.aa[1] + 5.5, cred_h = v.cred || 0;
  const used = top + (slot_h ? slot_h + g : 0) + (band_h ? band_h + g : 0) + scr_h + (v.ctrl ? g + v.ctrl : 0) + (cred_h ? g + cred_h : 0);
  const H = v.Hfix || used + 4.5;
  let y = H / 2 - top;
  const slot = y - slot_h / 2; if (slot_h) y -= slot_h + g;
  const band = y - band_h / 2; if (band_h) y -= band_h + g;
  const scr = y - scr_h / 2; y -= scr_h;
  if (v.ctrl) y -= g;
  const ctrl = y - v.ctrl / 2; y -= v.ctrl;
  if (cred_h) y -= g;
  const cred = y - cred_h / 2;
  return { H, slot, slot_h, band, band_h, scr, scr_h, ctrl, cred, cred_h };
}
const lum = hex => { const c = new THREE.Color(hex); return 0.299 * c.r + 0.587 * c.g + 0.114 * c.b; };

// ------------------------------------------------------------------ 2D page rendering (native panel pixels)
function lines(g, text, w) {
  const out = []; let cur = [];
  for (const wd of text.split(/\s+/).filter(Boolean)) {
    const t = cur.concat(wd).join(" ");
    if (g.measureText(t).width > w && cur.length) { out.push(cur); cur = [wd]; } else cur.push(wd);
  }
  if (cur.length) out.push(cur);
  return out;
}
function drawLines(g, ls, x, y, w, lh, bottom, justify) {
  for (let i = 0; i < ls.length; i++) {
    if (y > bottom) return { y, stop: true };
    const ws = ls[i], last = i === ls.length - 1;
    if (justify && !last && ws.length > 1) {
      const tot = ws.reduce((a, b) => a + g.measureText(b).width, 0);
      const gap = (w - tot) / (ws.length - 1); let xx = x;
      for (const wd of ws) { g.fillText(wd, xx, y); xx += g.measureText(wd).width + gap; }
    } else g.fillText(ws.join(" "), x, y);
    y += lh;
  }
  return { y, stop: false };
}
function paras(g, text, x, y, w, lh, bottom, justify) {
  for (const p of text.split("\n\n")) {
    const r = drawLines(g, lines(g, p, w), x, y, w, lh, bottom, justify);
    if (r.stop) return r.y;
    y = r.y + lh * 0.45;
  }
  return y;
}
function hash2(x, y) { const s = Math.sin(x * 127.1 + y * 311.7) * 43758.5453; return s - Math.floor(s); }
function vnoise(x, y) {
  const ix = Math.floor(x), iy = Math.floor(y), fx = x - ix, fy = y - iy;
  const sx = fx * fx * (3 - 2 * fx), sy = fy * fy * (3 - 2 * fy);
  const a = hash2(ix, iy), b = hash2(ix + 1, iy), c = hash2(ix, iy + 1), d = hash2(ix + 1, iy + 1);
  return a + (b - a) * sx + (c - a) * sy + (a - b - c + d) * sx * sy;
}
function fbm(x, y) { let v = 0, a = 0.5; for (let o = 0; o < 5; o++) { v += a * vnoise(x, y); x *= 2.03; y *= 2.03; a *= 0.5; } return v; }
function drawPhoto(g, x, y, w, h, seed) {
  x = Math.round(x); y = Math.round(y); w = Math.round(w); h = Math.round(h);
  const img = g.createImageData(w, h), p = img.data;
  for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) {
    const u = i / w, t = j / h;
    let val = 0.82 - 0.3 * t + 0.55 * (fbm(u * 3.2 + seed, t * 2.4) - 0.5);
    const ridge = 0.62 + 0.08 * Math.sin(u * 9 + seed) + 0.05 * (fbm(u * 6, seed) - 0.5);
    if (t > ridge) val = 0.12 + 0.25 * fbm(u * 14, t * 14 + seed);
    val = Math.max(0, Math.min(1, val)) * 255;
    const k = (j * w + i) * 4; p[k] = p[k + 1] = p[k + 2] = val; p[k + 3] = 255;
  }
  g.putImageData(img, x, y);
}
const BAYER = [0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5];
const PAL4 = [[221, 219, 211], [28, 28, 32], [158, 38, 30], [214, 166, 32]];
function quantize(g, v, photos, w, h) {
  const d = g.getImageData(0, 0, w, h), p = d.data;
  const paper = v.lit ? [238, 226, 200] : [221, 219, 211], ink = [28, 28, 32];
  const inPh = (x, y) => photos.some(r => x >= r[0] && x < r[0] + r[2] && y >= r[1] && y < r[1] + r[3]);
  for (let y = 0; y < h; y++) for (let x = 0; x < w; x++) {
    const i = (y * w + x) * 4, r = p[i], gg = p[i + 1], b = p[i + 2];
    if (v.depth === "4c") {
      let best = 0, bd = 1e9;
      for (let k = 0; k < 4; k++) { const c = PAL4[k], dd = (c[0] - r) ** 2 + (c[1] - gg) ** 2 + (c[2] - b) ** 2; if (dd < bd) { bd = dd; best = k; } }
      p[i] = PAL4[best][0]; p[i + 1] = PAL4[best][1]; p[i + 2] = PAL4[best][2];
    } else {
      const l = (0.299 * r + 0.587 * gg + 0.114 * b) / 255;
      let q;
      if (v.depth === "1") q = (photos.length && inPh(x, y)) ? (l > (BAYER[(y & 3) * 4 + (x & 3)] + 0.5) / 16 ? 1 : 0) : (l > 0.6 ? 1 : 0);
      else q = Math.round(l * 15) / 15;
      for (let c = 0; c < 3; c++) p[i + c] = ink[c] + (paper[c] - ink[c]) * q;
    }
    p[i + 3] = 255;
  }
  g.putImageData(d, 0, 0);
}
const F = {
  head: s => `600 ${s}px "Barlow Condensed", "Arial Narrow", sans-serif`,
  disp: s => `800 ${s}px "Barlow Condensed", Impact, sans-serif`,
  dispM: s => `700 ${s}px "Barlow Condensed", "Arial Narrow", sans-serif`,
  title: s => `600 ${s}px Literata, Georgia, serif`,
  body: s => `400 ${s}px Literata, Georgia, serif`,
  ital: s => `italic 400 ${s}px Literata, Georgia, serif`,
  mono: s => `500 ${s}px "IBM Plex Mono", Menlo, monospace`,
  monoB: s => `600 ${s}px "IBM Plex Mono", Menlo, monospace`,
};
function pageCanvas(v) {
  const [rw, rh] = v.res, s = rw / v.aa[0];
  const c = document.createElement("canvas"); c.width = rw; c.height = rh;
  const g = c.getContext("2d");
  g.fillStyle = "#fff"; g.fillRect(0, 0, rw, rh);
  const m = 3.2 * s, x0 = m, tw = rw - 2 * m, em = v.em * s, photos = [];
  g.fillStyle = "#000";
  let y = m + em * 0.8;
  g.font = F.head(em * 0.8);
  if (v.screenBand) { g.fillStyle = "#fff"; }
  g.fillText(v.screenBand ? "" : (v.head ? v.head[0] : "SUBVERSE — ISSUE 07"), x0, y);
  g.textAlign = "right";
  if (v.depth === "4c") g.fillStyle = "#c0261c";
  g.fillText(v.depth === "4c" ? "COVER" : (v.screenBand ? "" : (v.head ? v.head[1] : "14 / 52")), x0 + tw, y);
  g.textAlign = "left"; g.fillStyle = "#000";
  if (v.depth === "4c") {
    const bh = (rh - 2 * m) * 0.56, by = y + em * 0.8;
    g.fillStyle = "#c0261c"; g.fillRect(x0, by, tw, bh);
    g.fillStyle = "#e0ad1e"; g.beginPath(); g.arc(x0 + tw * 0.68, by + bh * 0.4, tw * 0.24, 0, 7); g.fill();
    g.fillStyle = "#000"; g.font = F.disp(tw * 0.25);
    g.fillText("LOUD", x0 + tw * 0.05, by + tw * 0.26); g.fillText("PAPER", x0 + tw * 0.05, by + tw * 0.5);
    const yb = by + bh + em * 0.6;
    g.fillStyle = "#e0ad1e"; g.fillRect(x0, yb, tw, em * 2.2);
    g.fillStyle = "#000"; g.font = F.dispM(em * 1.05); g.textAlign = "center";
    g.fillText("LIVE AT THE BASEMENT · FRI 11/14 · DOORS 8", x0 + tw / 2, yb + em * 1.5); g.textAlign = "left";
    g.font = F.body(em); drawLines(g, lines(g, POSTER_BODY, tw), x0, yb + em * 3.7, tw, em * 1.32, rh - m, true);
  } else {
    y += em * 0.45; g.fillRect(x0, y, tw, Math.max(1, 0.25 * s));
    const [title, body0] = ZINE[v.zine];
    const mono = v.zine === "stage" || v.zine === "sched";
    const body = mono ? body0 : body0 + MORE;
    if (v.screenBand) {
      g.fillStyle = "#000"; g.fillRect(0, 0, rw, em * 3.2 + m); g.fillStyle = "#fff"; g.font = F.disp(em * 1.9); g.textBaseline = "middle";
      g.fillText("SUBVERSE", x0, (em * 3.2 + m) / 2); g.textAlign = "right"; g.font = F.dispM(em * 1.15);
      g.fillText(v.screenBand, x0 + tw, (em * 3.2 + m) / 2 + em * 0.15); g.textAlign = "left"; g.textBaseline = "alphabetic"; g.fillStyle = "#000";
      y += em * 3.2 + m * 0.3;
    }
    const ts = rh < 200 ? em * 1.3 : em * 1.75;
    g.font = F.title(ts);
    const tl = lines(g, title, tw);
    y = drawLines(g, tl, x0, y + (rh < 200 ? em * 1.7 : em * (mono ? 2.0 : 2.3)), tw, ts * 1.1, rh, false).y + em * 0.15;
    if (v.depth === "16") {
      const ph = Math.min(rh * 0.3, tw * 0.62);
      drawPhoto(g, x0, y - em * 0.6, tw, ph, v.key.charCodeAt(3));
      photos.push([x0, y - em * 0.6, tw, ph]); y += ph + em * 0.7;
    }
    const bottom = rh - m - em * 1.8;
    g.fillStyle = "#000";
    if (mono) {
      g.font = F.mono(em * 0.92);
      for (const ln of body.split("\n")) { if (y > bottom) break; g.fillText(ln, x0, y); y += em * 1.38; }
    } else {
      g.font = F.body(em);
      paras(g, body, x0, y, tw, em * 1.36, bottom, true);
    }
    if (v.pan) {
      const iw = 15 * s, ih = 20 * s, ix = x0 + tw - iw, iy = rh - m - em * 1.6 - ih - 1.2 * s;
      g.fillStyle = "#fff"; g.fillRect(ix - 1.2 * s, iy - 1.2 * s, iw + 2.4 * s, ih + 2.4 * s);
      g.fillStyle = "#000"; g.lineWidth = Math.max(1, 0.35 * s); g.strokeStyle = "#000"; g.strokeRect(ix, iy, iw, ih);
      for (let k = 0; k < 7; k++) g.fillRect(ix + iw * 0.1, iy + 3 * s + k * 2.3 * s, iw * 0.8, Math.max(1, 0.35 * s));
      g.lineWidth = Math.max(2, 0.8 * s); g.strokeRect(ix + iw * 0.12, iy + ih * 0.1, iw * 0.5, ih * 0.5);
    }
    g.fillStyle = "#000";
    g.fillRect(x0, rh - m, tw, Math.max(1, 0.2 * s));
    g.fillRect(x0, rh - m - 0.45 * s, tw * 0.27, Math.max(2, 0.9 * s));
  }
  quantize(g, v, photos, rw, rh);
  return c;
}
function canvasTex(c, nearest) {
  const t = new THREE.CanvasTexture(c);
  t.encoding = THREE.sRGBEncoding;
  t.anisotropy = maxAniso;
  if (nearest) t.magFilter = THREE.NearestFilter;
  return t;
}
function mmCanvas(wmm, hmm, k = 10) {
  const c = document.createElement("canvas");
  c.width = Math.round(wmm * k); c.height = Math.round(hmm * k);
  return [c, c.getContext("2d"), k];
}

// ------------------------------------------------------------------ 3D helpers
let renderer, scene, camera, controls, maxAniso = 4;
const std = (color, rough = 0.5, metal = 0, extra = {}) => new THREE.MeshStandardMaterial(Object.assign({ color, roughness: rough, metalness: metal }, extra));
function rr(p, cx, cy, w, h, r) {
  r = Math.max(0.01, Math.min(r, w / 2 - 0.01, h / 2 - 0.01));
  const x = cx - w / 2, y = cy - h / 2;
  p.moveTo(x + r, y); p.lineTo(x + w - r, y); p.absarc(x + w - r, y + r, r, -Math.PI / 2, 0, false);
  p.lineTo(x + w, y + h - r); p.absarc(x + w - r, y + h - r, r, 0, Math.PI / 2, false);
  p.lineTo(x + r, y + h); p.absarc(x + r, y + h - r, r, Math.PI / 2, Math.PI, false);
  p.lineTo(x, y + r); p.absarc(x + r, y + r, r, Math.PI, Math.PI * 1.5, false);
  return p;
}
function ext(shape, z0, z1, bev, mat, seg = 24) {
  const depth = Math.max(0.01, z1 - z0 - 2 * bev);
  const g = new THREE.ExtrudeGeometry(shape, { depth, bevelEnabled: bev > 0, bevelThickness: bev, bevelSize: bev, bevelSegments: bev > 0 ? 5 : 0, curveSegments: seg });
  const m = new THREE.Mesh(g, mat); m.position.z = z0 + bev; return m;
}
function rbox(cx, cy, w, h, r, z0, z1, bev, mat) {
  return ext(rr(new THREE.Shape(), cx, cy, w - 2 * bev, h - 2 * bev, r - bev), z0, z1, bev, mat);
}
function cap(r, h, mat, concave) {
  // profile from the axis at the base, out, up the side, over the rounded rim, back to the axis
  const e = Math.min(0.9, r * 0.3), pts = [new THREE.Vector2(0.001, 0), new THREE.Vector2(r, 0)];
  for (let i = 0; i <= 6; i++) { const a = i / 6 * Math.PI / 2; pts.push(new THREE.Vector2(r - e + e * Math.cos(a), h - e + e * Math.sin(a))); }
  if (concave) { pts.push(new THREE.Vector2(r - 1.4, h)); pts.push(new THREE.Vector2(0.001, h - 0.6)); }
  else pts.push(new THREE.Vector2(0.001, h));
  const m = new THREE.Mesh(new THREE.LatheGeometry(pts, 48), mat);
  m.rotation.x = Math.PI / 2; return m;
}
function decal(w, h, r, mat) { return new THREE.Mesh(new THREE.ShapeGeometry(rr(new THREE.Shape(), 0, 0, w, h, r), 12), mat); }
function texPlane(w, h, tex, opts = {}) {
  const mat = new THREE.MeshStandardMaterial(Object.assign({ map: tex, roughness: 0.6, metalness: 0, transparent: !!opts.transparent }, opts.mat || {}));
  const m = new THREE.Mesh(new THREE.PlaneGeometry(w, h), mat);
  if (opts.back) m.rotation.y = Math.PI;
  return m;
}
function ribbon(pts, width, mat) {
  const pos = [], idx = [], back = new THREE.Vector3(0, 0, -1);
  pts.forEach((p, i) => {
    const t = pts[Math.min(i + 1, pts.length - 1)].clone().sub(pts[Math.max(i - 1, 0)]).normalize();
    const side = t.clone().cross(back).normalize().multiplyScalar(width / 2);
    const a = p.clone().sub(side), b = p.clone().add(side);
    pos.push(a.x, a.y, a.z, b.x, b.y, b.z);
    if (i) { const k = i * 2; idx.push(k - 2, k - 1, k + 1, k - 2, k + 1, k); }
  });
  const g = new THREE.BufferGeometry();
  g.setAttribute("position", new THREE.Float32BufferAttribute(pos, 3)); g.setIndex(idx); g.computeVertexNormals();
  return new THREE.Mesh(g, mat);
}
const qbez = (p0, p1, p2, n = 48) => Array.from({ length: n }, (_, i) => { const t = i / (n - 1); return p0.clone().multiplyScalar((1 - t) ** 2).add(p1.clone().multiplyScalar(2 * (1 - t) * t)).add(p2.clone().multiplyScalar(t * t)); });

// ------------------------------------------------------------------ the device
function build(v) {
  const L = layout(v), H = L.H, W = v.W, T = v.T, zs = T / 2 - 3, zMid = -1.5;
  const grp = new THREE.Group(), labels = [], refs = {};
  const lab = (t, p, n) => labels.push({ t, p: new THREE.Vector3(...p), n: new THREE.Vector3(...n).normalize() });
  const bodyM = std(v.body, 0.45), rubber = std("#141416", 0.75), metal = std("#d0d2d6", 0.22, 1), portM = std("#09090a", 0.55, 0, { side: THREE.DoubleSide });
  const R = 6, rearB = Math.min(2.4, T * 0.18), frontB = 1.0;
  const Rr = Math.max(7.5, T / 2 + 2.5), sag = Rr - Math.sqrt(Rr * Rr - (T / 2) ** 2), sh = sag + 1.6;
  // rear shell
  const rear = rr(new THREE.Shape(), 0, 0, W - 2 * rearB, H - 2 * rearB, R - rearB);
  if (v.lanyard && !v.bail) rear.holes.push(rr(new THREE.Path(), 0, L.slot, 18, sh, 1.6));
  grp.add(ext(rear, -T / 2, zs, rearB, bodyM, 32));
  // front bezel with window
  const bez = rr(new THREE.Shape(), 0, 0, W - 2 * frontB, H - 2 * frontB, R - frontB);
  bez.holes.push(rr(new THREE.Path(), 0, L.scr, v.aa[0] + 5.5, L.scr_h, 1.2));
  if (v.lanyard && !v.bail) bez.holes.push(rr(new THREE.Path(), 0, L.slot, 18, sh, 1.6));
  if (L.cred_h) bez.holes.push(rr(new THREE.Path(), 0, L.cred, 82, 50.5, 3));
  grp.add(ext(bez, zs, T / 2, frontB, bodyM, 32));
  if (!v.edge) lab("Bezel / back shell seam · 4× M2 screws", [W / 2, L.scr - L.scr_h * 0.3, zs], [1, 0, 0.15]);
  else lab("Bezel / back shell seam", [W / 2 - 10, -H / 2, zs], [0, -1, 0.2]);
  // panel and page
  const zp = T / 2 - 1.6;
  const pan = new THREE.Mesh(new THREE.PlaneGeometry(v.aa[0] + 5.5, L.scr_h), std(v.lit ? "#efe3c7" : "#d8d6ce", 0.9));
  pan.position.set(0, L.scr, zp); grp.add(pan);
  const ptex = canvasTex(pageCanvas(v), true);
  const smat = new THREE.MeshStandardMaterial({ map: ptex, roughness: 0.88, metalness: 0 });
  if (v.lit) { smat.emissive = new THREE.Color("#ffffff"); smat.emissiveMap = ptex; smat.emissiveIntensity = 0.55; pan.material.emissive = new THREE.Color("#ffe3b0"); pan.material.emissiveIntensity = 0.35; }
  refs.screen = smat; refs.panel = pan.material;
  const scr = new THREE.Mesh(new THREE.PlaneGeometry(v.aa[0], v.aa[1]), smat);
  scr.position.set(0, L.scr, zp + 0.05); grp.add(scr);
  lab(v.pshort, v.screenBand ? [-v.aa[0] / 2 + 4, L.scr - v.aa[1] / 2 + 6, zp] : [-v.aa[0] / 2 + 4, L.scr + v.aa[1] / 2 - 3, zp], [0, 0, 1]);
  if (v.lit) lab("Front light · LEDs under the top bezel", [v.aa[0] / 2 - 6, L.scr + L.scr_h / 2, T / 2], [0, 0.2, 1]);
  // laminate band
  const bw = W - 9;
  if (v.band) {
  const [bc, bg, bk] = mmCanvas(bw, L.band_h, 12);
  bg.fillStyle = v.bandBg; bg.fillRect(0, 0, bc.width, bc.height); bg.fillStyle = v.bandFg; bg.textBaseline = "middle";
  bg.font = F.disp(L.band_h * 0.72 * bk); bg.fillText("SUBVERSE", 3 * bk, bc.height / 2 + 0.4 * bk);
  bg.textAlign = "right"; bg.font = F.dispM(L.band_h * 0.42 * bk); bg.fillText(v.band, bc.width - 3 * bk, bc.height / 2 + 0.6 * bk);
  const band = texPlane(bw, L.band_h, canvasTex(bc), { mat: { roughness: 0.35 } });
  band.position.set(0, L.band, T / 2 + 0.05); grp.add(band);
  lab("Laminate band · swappable vinyl", [bw / 2 - 6, L.band + L.band_h / 2, T / 2], [0, 0, 1]);
  }
  if (v.screenBand) lab("Pass graphic drawn on the e-paper (no band)", [v.aa[0] / 2 - 8, L.scr + v.aa[1] / 2 - 4, zp], [0, 0, 1]);
  if (L.cred_h) {
    const [qc, qg, qk] = mmCanvas(85.6, 54, 10);
    qg.fillStyle = "#f4f3ee"; qg.fillRect(0, 0, qc.width, qc.height);
    qg.fillStyle = v.strap; qg.fillRect(0, 0, qc.width, 11 * qk);
    qg.fillStyle = "#fff"; qg.font = F.dispM(6 * qk); qg.fillText("VENUE CREDENTIAL", 4 * qk, 8 * qk);
    qg.fillStyle = "#b9b8b2"; qg.fillRect(4 * qk, 15 * qk, 22 * qk, 28 * qk);
    qg.fillStyle = "#111"; qg.font = F.disp(10 * qk); qg.fillText("ALL ACCESS", 30 * qk, 26 * qk);
    qg.font = F.mono(3.4 * qk); qg.fillText("NAME ··········", 30 * qk, 34 * qk); qg.fillText("ROLE  ARTIST", 30 * qk, 40 * qk);
    for (let i = 0; i < 46; i++) if (hash2(i, 3) > 0.35) qg.fillRect((30 + i * 1.1) * qk, 44 * qk, (hash2(i, 7) > 0.5 ? 0.7 : 0.35) * qk, 7 * qk);
    const cp = texPlane(85.6, 54, canvasTex(qc), { mat: { roughness: 0.5 } }); cp.position.set(0, L.cred, zs + 0.4); grp.add(cp);
    const pane = new THREE.Mesh(new THREE.PlaneGeometry(83, 51.5), new THREE.MeshStandardMaterial({ color: "#ffffff", roughness: 0.05, metalness: 0, transparent: true, opacity: 0.12 }));
    pane.position.set(0, L.cred, T / 2 - 0.5); grp.add(pane);
    const cs = decal(1.4, 56, 0.6, portM); cs.rotation.y = -Math.PI / 2; cs.position.set(-W / 2 - 0.08, L.cred, zs + 0.4); grp.add(cs);
    lab("ID-1 credential window · venue's printed badge", [30, L.cred - 20, T / 2], [0, 0, 1]);
    lab("Credential slides in here", [-W / 2, L.cred + 10, zs], [-1, 0, 0.15]);
  }
  // lanyard
  const strapGrp = new THREE.Group(); refs.lanyard = strapGrp;
  if (v.lanyard) {
    let cz = L.slot - sh / 2 + 0.7 + Rr, rz = 0;
    if (v.bail) {
      const dy = H / 2 - 4, dr = new THREE.Mesh(new THREE.TorusGeometry(5, 0.9, 12, 32, Math.PI), metal);
      dr.rotation.x = Math.PI / 2; dr.position.set(0, dy, -T / 2); grp.add(dr);
      rz = -T / 2 - 4; cz = dy + Rr - 0.6;
      lab("Fixed D-ring on the back · lanyard hook", [5, dy, -T / 2 - 5], [0, 0.3, -1]);
    }
    const ring = new THREE.Mesh(new THREE.TorusGeometry(Rr, 0.55, 14, 72), metal);
    ring.rotation.y = Math.PI / 2; ring.position.set(0, cz, rz); strapGrp.add(ring);
    const top = cz + Rr;
    const sw = new THREE.Mesh(new THREE.CylinderGeometry(2, 2, 10, 28), metal); sw.position.set(0, top + 4.5, 0); strapGrp.add(sw);
    const cr = rbox(0, top + 12, 17, 8, 1.2, -1.6, 1.6, 0.5, std("#222", 0.4, 0.6)); strapGrp.add(cr);
    const sm = std(v.strap, 0.85, 0, { side: THREE.DoubleSide });
    for (const sg of [-1, 1]) {
      const p0 = new THREE.Vector3(sg * 4.2, top + 12, 0);
      strapGrp.add(ribbon(qbez(p0, p0.clone().add(new THREE.Vector3(sg * 6, 70, -5)), p0.clone().add(new THREE.Vector3(sg * 45, 300, -50))), 15, sm));
    }
    grp.add(strapGrp);
    if (!v.bail) lab("Lanyard slot · 25 mm split ring", [9, L.slot, T / 2], [0, 0.3, 1]);
  }
  // ports
  const usb = decal(9.2, 3.3, 1.6, portM); usb.rotation.x = Math.PI / 2; usb.position.set(0, -H / 2 - 0.08, zMid); grp.add(usb);
  const tongue = decal(6.4, 0.8, 0.4, std("#3a3a3c", 0.5)); tongue.rotation.x = Math.PI / 2; tongue.position.set(0, -H / 2 - 0.14, zMid); grp.add(tongue);
  lab("USB-C · " + v.usb.toLowerCase(), [0, -H / 2, zMid], [0, -1, 0.1]);
  if (v.sd) {
    const sd = decal(2.0, 13, 0.8, portM); sd.rotation.y = -Math.PI / 2; const sdY = H < 80 ? -H / 2 + 12 : (v.edge && v.edge.some(e => e[0] < 0) ? -H / 2 + 14 : -H / 2 + 26);
    sd.position.set(-W / 2 - 0.08, sdY, zMid); grp.add(sd);
    lab("microSD slot (push-push)", [-W / 2, sdY, zMid], [-1, 0, 0.1]);
  }
  // controls
  const legend = [];
  const zf = T / 2 - 0.5;
  const key = (x, y, r, t, m = rubber, h = 1.7) => { const k = cap(r, h, m); k.position.set(x, y, zf); grp.add(k); if (t) legend.push([x, y - r - 2.4, t]); return k; };
  const sideKey = (sx, y, len) => { const k = rbox(sx * W / 2, y, 3, len, 1.2, zMid - 2.5, zMid + 2.5, 0.7, rubber); grp.add(k); };
  const c = v.controls;
  if (c === "three") {
    [[-22, "PREV"], [0, "MENU"], [22, "NEXT"]].forEach(([x, t]) => key(x, L.ctrl + 1.8, 4.2, t));
    lab("Keys · PREV · MENU · NEXT", [22, L.ctrl + 1.8, T / 2 + 1], [0, 0, 1]);
  } else if (c === "four") {
    [[-25.5, "PREV"], [-8.5, "MENU"], [8.5, "LIGHT"], [25.5, "NEXT"]].forEach(([x, t]) => key(x, L.ctrl + 1.8, 3.6, t));
    lab("LIGHT key · level, hold for off", [8.5, L.ctrl + 1.8, T / 2 + 1], [0, 0, 1]);
  } else if (c === "dpad") {
    const y = L.ctrl + 1.2;
    key(0, y, 7.2, null, rubber, 1.3);
    key(0, y, 2.6, null, std(v.bandBg === "#161616" ? "#444" : v.bandBg, 0.4), 2.2);
    for (let a = 0; a < 4; a++) {
      const tri = new THREE.Mesh(new THREE.CircleGeometry(0.9, 3), std("#7a7a7e", 0.5));
      const ang = a * Math.PI / 2; tri.rotation.z = ang; tri.position.set(5 * Math.cos(ang), y + 5 * Math.sin(ang), zf + 1.35); grp.add(tri);
    }
    const px = Math.min(33, W / 2 - 11), pr = W < 90 ? 4.5 : 5.5;
    key(-px, y, pr, "PREV"); key(px, y, pr, "NEXT");
    lab("5-way navigation switch", [0, y + 7, T / 2 + 1], [0, 0, 1]);
  } else if (c === "tap") {
    for (const [sx, dy, len] of v.edge) sideKey(sx, L.scr + dy, len);
    lab("Edge keys · UP · OK · DOWN", [W / 2 + 1.5, L.scr + 14, zMid], [1, 0, 0.1]);
    key(-24, L.ctrl + 3, 4.5, "TAP");
    lab("TAP key · reader field on for 10 s", [-24, L.ctrl + 3, T / 2 + 1.5], [0, 0, 1]);
    // NFC marks: printed on the front over the zone, and a tap target on the back over the coil
    const mark = (back) => {
      const [nc, ng, nk] = mmCanvas(40, 26, 14);
      ng.strokeStyle = back ? "#f2f1ec" : (lum(v.body) < 0.45 ? "#e8e7e2" : "#1a1a1a"); ng.fillStyle = ng.strokeStyle; ng.lineCap = "round";
      ng.lineWidth = 1.3 * nk;
      for (let i = 0; i < 3; i++) { ng.beginPath(); ng.arc(12 * nk, 13 * nk, (4 + i * 3.4) * nk, -0.9, 0.9); ng.stroke(); }
      ng.beginPath(); ng.arc(12 * nk, 13 * nk, 1.4 * nk, 0, 7); ng.fill();
      ng.font = F.dispM(4.2 * nk); ng.fillText(back ? "TAP" : "NFC", 24 * nk, 15 * nk);
      const pl = texPlane(40, 26, canvasTex(nc), { transparent: true, back, mat: { depthWrite: false } });
      return pl;
    };
    const fm = mark(false); fm.position.set(14, L.ctrl, T / 2 + 0.04); grp.add(fm);
    const disc = new THREE.Mesh(new THREE.CircleGeometry(19, 64), std("#16171a", 0.6, 0, { side: THREE.DoubleSide }));
    disc.rotation.y = Math.PI; disc.position.set(0, L.ctrl + 2, -T / 2 - 0.05); grp.add(disc);
    const bm = mark(true); bm.position.set(0, L.ctrl + 2, -T / 2 - 0.08); grp.add(bm);
    lab("NFC reader coil (CLRC663 plus, 13.56 MHz) · tap tags here", [-12, L.ctrl - 12, -T / 2], [0, 0, -1]);
    lab("Pass is a tag too: phones tap for the link / pairing", [10, L.ctrl + 22, -T / 2], [0, 0, -1]);
  } else if (c === "edge") {
    for (const [sx, dy, len, t] of v.edge) sideKey(sx, L.scr + dy, len);
    const r = v.edge.filter(e => e[0] > 0).map(e => e[3]).join(" · "), l = v.edge.filter(e => e[0] < 0).map(e => e[3]).join(" · ");
    lab("Edge keys · " + r, [W / 2 + 1.5, L.scr + v.edge[0][1], zMid], [1, 0, 0.1]);
    if (l) lab("Edge key · " + l, [-W / 2 - 1.5, L.scr + v.edge.find(e => e[0] < 0)[1], zMid], [-1, 0, 0.1]);
  } else if (c === "edge3") {
    [[L.scr + 22, 10], [L.scr + 8, 7], [L.scr - 6, 10]].forEach(([y, len]) => sideKey(1, y, len));
    lab("Side keys · PREV · MENU · NEXT", [W / 2 + 1.5, L.scr + 8, zMid], [1, 0, 0.1]);
  } else if (c === "side3") {
    [[L.scr + 24, 12], [L.scr + 9, 7], [L.scr - 6, 12]].forEach(([y, len]) => sideKey(1, y, len));
    key(0, L.ctrl, 2.2, "HOME", rubber, 1.2);
    lab("Side keys · UP · OK · DOWN", [W / 2 + 1.5, L.scr + 9, zMid], [1, 0, 0.1]);
  } else if (c === "rockers") {
    for (const sx of [-1, 1]) sideKey(sx, L.scr + 8, 32);
    [[-16, "BACK"], [0, "MENU"], [16, "SYNC"]].forEach(([x, t]) => key(x, L.ctrl + 1.2, 2.6, t, rubber, 1.3));
    lab("Page rocker (right)", [W / 2 + 1.5, L.scr + 8, zMid], [1, 0, 0.1]);
    lab("Page rocker (left)", [-W / 2 - 1.5, L.scr + 8, zMid], [-1, 0, 0.1]);
  } else if (c === "wheel") {
    const teeth = 36, sh2 = new THREE.Shape();
    for (let i = 0; i < teeth * 2; i++) { const a = Math.PI * 2 * i / (teeth * 2), r = i % 2 ? 8.5 * 0.93 : 8.5; i ? sh2.lineTo(r * Math.cos(a), r * Math.sin(a)) : sh2.moveTo(r, 0); }
    const wheel = ext(sh2, zMid - 2.75, zMid + 2.75, 0.3, rubber); wheel.position.x = W / 2 - 4.5; wheel.position.y = L.scr + 5; grp.add(wheel);
    const ws = decal(7.5, 22, 2, portM); ws.rotation.y = Math.PI / 2; ws.position.set(W / 2 + 0.08, L.scr + 5, zMid); grp.add(ws);
    key(-14, L.ctrl + 1.5, 4, "BACK"); key(14, L.ctrl + 1.5, 4, "OK");
    lab("Thumbwheel · roller encoder with push", [W / 2 + 4, L.scr + 5, zMid], [1, 0, 0.2]);
  } else if (c === "dial") {
    const teeth = 30, sh2 = new THREE.Shape();
    for (let i = 0; i < teeth * 2; i++) { const a = Math.PI * 2 * i / (teeth * 2), r = i % 2 ? 6.5 * 0.92 : 6.5; i ? sh2.lineTo(r * Math.cos(a), r * Math.sin(a)) : sh2.moveTo(r, 0); }
    const wheel = ext(sh2, zMid - 2.2, zMid + 2.2, 0.3, rubber); wheel.position.x = W / 2 - 3.2; wheel.position.y = L.scr + 10; grp.add(wheel);
    key(-14, L.ctrl + 1.5, 4, "MENU"); key(14, L.ctrl + 1.5, 4, "EXIT");
    lab("Thumb dial · up / down / push (CrowPanel)", [W / 2 + 4, L.scr + 10, zMid], [1, 0, 0.2]);
  } else if (c === "stick") {
    const sx = -20, sy = L.ctrl + 1.5;
    const well = cap(10.5, 0.9, std("#0b0b0c", 0.6)); well.position.set(sx, sy, zf); grp.add(well);
    const capM = cap(7.8, 2.6, rubber, true); capM.position.set(sx + 1.2, sy + 1, zf + 0.8); grp.add(capM);
    for (const rg of [5.6, 3.8]) { const t = new THREE.Mesh(new THREE.TorusGeometry(rg, 0.22, 8, 48), std("#2a2a2d", 0.9)); t.position.set(sx + 1.2, sy + 1, zf + 3.1); grp.add(t); }
    legend.push([sx, sy - 13, "PAN"]);
    key(16, L.ctrl - 2, 4.5, "BACK"); key(29, L.ctrl + 4.5, 4.5, "OK");
    lab("PSP thumbstick · Adafruit 444 · powered only when awake", [sx, sy + 9, T / 2 + 3], [0, 0, 1]);
  }
  if (legend.length) {
    const lw = W - 6, lh = v.ctrl + 10, ly = L.ctrl - 2;
    const [lc, lg, lk] = mmCanvas(lw, lh, 14);
    lg.fillStyle = lum(v.body) < 0.4 ? "#e8e7e2" : "#1a1a1a"; lg.textAlign = "center"; lg.textBaseline = "middle"; lg.font = F.dispM(2.1 * lk);
    for (const [x, y, t] of legend) lg.fillText(t, (x + lw / 2) * lk, (ly + lh / 2 - y) * lk);
    const lp = texPlane(lw, lh, canvasTex(lc), { transparent: true, mat: { depthWrite: false } });
    lp.position.set(0, ly, T / 2 + 0.03); grp.add(lp);
  }
  // back: spec label
  const labW = Math.min(62, W - 16), labY = v.clip ? H / 2 - 8.5 : (v.band ? L.band : (v.bail ? H / 2 - 16 : -H / 2 + 12));
  const [kc, kg, kk] = mmCanvas(labW, 13, 12);
  kg.fillStyle = "#ecebe5"; kg.fillRect(0, 0, kc.width, kc.height); kg.fillStyle = "#1a1a1a"; kg.font = F.monoB(2.3 * kk);
  [`SubverseReader ${v.key} · ${v.name}`, v.chip, "open hardware · CERN-OHL-S"].forEach((t, i) => kg.fillText(t, 2.5 * kk, (3.6 + i * 3.4) * kk));
  const kp = texPlane(labW, 13, canvasTex(kc), { back: true }); kp.position.set(0, labY, -T / 2 - 0.06); grp.add(kp);
  lab("Spec label", [-labW / 2 + 5, labY, -T / 2], [0, 0, -1]);
  if (v.pocket) pocket(v, L, grp, bodyM, lab);
  if (v.clip) {
    const cm = std("#141416", 0.35, 0.3), zt = H / 2 - 23;
    grp.add(rbox(0, zt, 28, 13, 2, -T / 2 - 5.5, -T / 2, 1, cm));
    grp.add(rbox(0, zt - 0.33 * H, 26, 0.7 * H, 2.5, -T / 2 - 7.4, -T / 2 - 5.4, 0.6, cm));
    grp.add(rbox(0, zt - 0.67 * H, 26, 5, 2, -T / 2 - 7.8, -T / 2 - 5.2, 0.8, cm));
    lab("Spring-steel belt clip", [0, zt - 0.33 * H, -T / 2 - 7.4], [0, 0, -1]);
  }
  // scale card
  const sc = new THREE.Group(); refs.scale = sc;
  const [cc, cg, ck] = mmCanvas(85.6, 54, 8);
  cg.fillStyle = "#f2f1ec"; cg.fillRect(0, 0, cc.width, cc.height); cg.strokeStyle = "#999"; cg.lineWidth = 3; cg.strokeRect(2, 2, cc.width - 4, cc.height - 4);
  cg.fillStyle = "#333"; cg.font = F.monoB(4 * ck); cg.fillText("ID-1 card", 6 * ck, 14 * ck); cg.font = F.mono(3.2 * ck); cg.fillText("85.6 × 54 mm, for scale", 6 * ck, 22 * ck);
  const card = texPlane(85.6, 54, canvasTex(cc), { mat: { side: THREE.DoubleSide } });
  sc.add(card); sc.position.set(W / 2 + 56, -H / 2 + 27, 0); sc.visible = false; grp.add(sc);
  return { grp, labels, refs, H, W, T };
}

function pocket(v, L, grp, bodyM, lab) {
  const T = v.T, H = L.H, zine = v.pocket === "zine";
  const [CW, CH] = zine ? [74, 105] : [85.6, 54];
  const PW = CW + 3, PH = CH + 2, ow = PW + 7, oh = PH + 5, zc = -H / 2 + 5.5 + oh / 2;
  const b = 0.6, w = ow - 2 * b, h = oh - 2 * b, r = 3, x0 = -w / 2, x1 = w / 2, y0 = zc - h / 2, y1 = zc + h / 2, nR = 11;
  const s = new THREE.Shape();
  s.moveTo(x0 + r, y0); s.lineTo(x1 - r, y0); s.absarc(x1 - r, y0 + r, r, -Math.PI / 2, 0, false);
  s.lineTo(x1, y1 - r); s.absarc(x1 - r, y1 - r, r, 0, Math.PI / 2, false);
  s.lineTo(nR, y1); s.absarc(0, y1, nR, 0, Math.PI, true);
  s.lineTo(x0 + r, y1); s.absarc(x0 + r, y1 - r, r, Math.PI / 2, Math.PI, false);
  s.lineTo(x0, y0 + r); s.absarc(x0 + r, y0 + r, r, Math.PI, Math.PI * 1.5, false);
  s.holes.push(rr(new THREE.Path(), 0, zc - 2.5, PW - 16, PH - 19, 2.5));
  grp.add(ext(s, -T / 2 - 3.8, -T / 2 - 1.6, b, bodyM, 32));
  grp.add(rbox(-ow / 2 + 1.75, zc, 3.5, oh, 1, -T / 2 - 1.7, -T / 2 + 0.1, 0, bodyM));
  grp.add(rbox(ow / 2 - 1.75, zc, 3.5, oh, 1, -T / 2 - 1.7, -T / 2 + 0.1, 0, bodyM));
  grp.add(rbox(0, zc - oh / 2 + 1.25, ow, 2.5, 1, -T / 2 - 1.7, -T / 2 + 0.1, 0, bodyM));
  const cy = zc - oh / 2 + 3.5 + CH / 2 + 11;
  const [cc, cg, k] = mmCanvas(CW, CH, 10);
  cg.fillStyle = v.card; cg.fillRect(0, 0, cc.width, cc.height);
  if (zine) {
    cg.fillStyle = "#b8281e"; cg.font = F.disp(11 * k); cg.fillText("SUBVERSE", 7 * k, 36 * k);
    cg.fillStyle = "#1a1a1a"; cg.font = F.dispM(6 * k); cg.fillText(v.cardTxt, 7 * k, 48 * k);
    cg.font = F.ital(3.4 * k); cg.fillText("fold, staple, pass it on", 7 * k, 96 * k);
  } else {
    cg.fillStyle = "#f3f2ec"; cg.font = F.disp(9 * k); cg.fillText("SUBVERSE", 8 * k, 32 * k);
    cg.fillStyle = v.bandBg; cg.font = F.dispM(5 * k); cg.fillText(v.cardTxt, 8 * k, 40 * k);
    cg.fillStyle = "#f3f2ec"; cg.font = F.monoB(3 * k); cg.fillText("MEMBER 0042 · EXP 12/27", 8 * k, 48 * k);
  }
  const card = texPlane(CW, CH, canvasTex(cc), { back: true, mat: { roughness: 0.35, side: THREE.DoubleSide } });
  card.position.set(0, cy, -T / 2 - 0.9); grp.add(card);
  if (!zine) {
    const tk = new THREE.Mesh(new THREE.PlaneGeometry(CW - 14, CH + 6), std("#e3dcc2", 0.85, 0, { side: THREE.DoubleSide }));
    tk.position.set(4, cy + 6, -T / 2 - 0.4); grp.add(tk);
  }
  lab(zine ? "Zine sleeve · A7 paper (74 × 105 mm)" : "Card pocket · ID-1 card + ticket, window", [ow / 2 - 6, zc - oh / 4, -T / 2 - 3.8], [0, 0, -1]);
  lab("Thumb notch", [0, zc + oh / 2 - 6, -T / 2 - 3.8], [0, 0.4, -1]);
}

// ------------------------------------------------------------------ app
const stage = document.getElementById("stage"), labelsEl = document.getElementById("labels"), specEl = document.getElementById("spec");
let cur = null, labelEls = [], tween = null;
const state = { labels: true, lanyard: true, light: true, scale: false, spin: false };
const reduce = matchMedia("(prefers-reduced-motion: reduce)").matches;

function dispose(o) {
  o.traverse(n => {
    if (n.geometry) n.geometry.dispose();
    if (n.material) [].concat(n.material).forEach(m => { ["map", "emissiveMap"].forEach(k => m[k] && m[k].dispose()); m.dispose(); });
  });
}
function viewDir(name) {
  return ({ front: [0, 0.04, 1], three: [0.62, 0.16, 0.77], back: [-0.35, 0.1, -0.93], side: [-1, 0.06, 0.12], bottom: [0.2, -0.88, 0.45] })[name];
}
function fitDist() { const h = Math.max(cur.H * 1.5, cur.W * 2.0 / camera.aspect); return h / (2 * Math.tan(THREE.MathUtils.degToRad(camera.fov / 2))); }
function goView(name, instant) {
  const d = new THREE.Vector3(...viewDir(name)).normalize().multiplyScalar(fitDist());
  if (instant || reduce) { camera.position.copy(d); controls.target.set(0, 0, 0); controls.update(); tween = null; }
  else tween = { from: camera.position.clone(), to: d, t0: performance.now(), dur: 650 };
  document.querySelectorAll("[data-view]").forEach(b => b.setAttribute("aria-pressed", String(b.dataset.view === name)));
}
function select(keyName) {
  const v = BY[keyName] || BY["SR-2"];
  if (cur) { scene.remove(cur.grp); dispose(cur.grp); }
  cur = build(v); cur.v = v; scene.add(cur.grp);
  labelsEl.innerHTML = ""; labelEls = cur.labels.map(l => { const e = document.createElement("div"); e.className = "lbl"; e.innerHTML = "<i></i><span></span>"; e.lastChild.textContent = l.t; labelsEl.appendChild(e); return e; });
  applyToggles(); goView("three", true); renderSpec(v);
  document.querySelectorAll(".picker button").forEach(b => b.setAttribute("aria-pressed", String(b.dataset.v === v.key)));
}
function applyToggles() {
  if (!cur) return;
  cur.refs.lanyard.visible = state.lanyard;
  cur.refs.scale.visible = state.scale;
  if (cur.v.lit) { cur.refs.screen.emissiveIntensity = state.light ? 0.55 : 0; cur.refs.panel.emissiveIntensity = state.light ? 0.35 : 0; }
  controls.autoRotate = state.spin;
  labelsEl.hidden = !state.labels;
  document.querySelectorAll("[data-toggle]").forEach(b => {
    const k = b.dataset.toggle; b.setAttribute("aria-pressed", String(state[k]));
    if (k === "light") b.hidden = !cur.v.lit;
    if (k === "lanyard") b.hidden = !cur.v.lanyard;
  });
}
const SUP = {"SR-1":"ok: all in stock (Seeed EE05 + XIAO S3 Plus)","SR-1C":"ok: FireBeetle C5 at DigiKey, CFA-10084 adapter at Crystalfontz (US)","SR-1D":"ok: Seeed + AliExpress, ships to Japan; check the cell can travel","SR-2":"watch: LDO via brokers (HT7833 alternative)","SR-3":"out: M5 PaperDIY sold out; LilyGo T5 4.7in S3 fallback ($51.54 at 5)","SR-4":"out: TPS65185 PMIC out at TI; 6in panel quote-only","SR-5":"watch: order C5 modules first","SR-6":"watch: order C5 modules first","SR-7":"watch: LDO via brokers (HT7833 alternative)","SR-8":"ok: all in stock","SR-9":"watch: C5 modules; Adafruit 444 has 35 in stock","SR-10":"watch: order C5 modules first","SR-11":"watch: order C5 modules first","SR-12":"watch: order C5 modules first","SR-13":"out: M5 PaperDIY sold out; LilyGo T5 fallback ($54.04 at 5)","SR-15":"ok: Elecrow in stock; ship-from not stated","SR-16":"ok: Seeed + Elecrow in stock; panel size to measure","SR-14":"watch: C5 modules; ST25DV 12-pin stock unconfirmed. Reader is now the CLRC663 plus (MFRC522 is EOL)"};
const BAT = {
"SR-1": "1000 mAh · 8 months at 300 pages/day · festival 9 months. Options: 500 mAh (10.5 mm deep) or 1500 mAh (13.5 mm)",
"SR-15": "1200 mAh · 7 months at 300 pages/day · festival 8 months (board sleep current unmeasured). Options: 1000 mAh (15 mm deep) or 1500 mAh (16 mm)",
"SR-16": "1200 mAh · 10 months at 300 pages/day · festival 11 months; a page holds about half a 4.2in page, so books take twice the turns. Options: 1000 mAh (11 mm deep) or 1500 mAh (12 mm)",
"SR-1D": "1000 mAh · 9 months at 300 pages/day · festival 9 months. Options: 500 mAh (10.5 mm deep) or 1500 mAh (13.5 mm)",
"SR-1C": "1000 mAh · 9 months at 300 pages/day · festival 10 months. Options: 500 mAh (11 mm deep) or 1500 mAh (14 mm)",
"SR-2": "1500 mAh · 11 months at 300 pages/day · festival 1 yr+. Options: 1000 mAh (18.5 mm deep) or 2500 mAh (23.5 mm)",
"SR-3": "1200 mAh · 4 months at 300 pages/day · festival 5 months. Options: 500 mAh (11.5 mm deep) or 2000 mAh (18.5 mm)",
"SR-4": "2000 mAh · 5 months at 300 pages/day · festival 7 months. Options: 1200 mAh (13.5 mm deep) or 2500 mAh (17.5 mm)",
"SR-5": "1200 mAh · 10 months at 300 pages/day · festival 11 months. Options: 1000 mAh (20.5 mm deep) or 2000 mAh (25.5 mm)",
"SR-6": "1500 mAh · 1 yr+ at 300 pages/day · festival 4 weeks. Options: 1000 mAh (13.5 mm deep) or 2500 mAh (18.5 mm)",
"SR-7": "1500 mAh · 3 months at 300 pages/day · festival 4 months. Options: 1000 mAh (18.5 mm deep) or 2500 mAh (23.5 mm)",
"SR-8": "1000 mAh · 1 yr+ at 300 pages/day · festival 1 yr+. Options: 500 mAh (11 mm deep) or 1500 mAh (14 mm)",
"SR-9": "1200 mAh · 10 months at 300 pages/day · festival 11 months. Options: 1000 mAh (13.5 mm deep) or 2000 mAh (18.5 mm)",
"SR-10": "350 mAh · 4 months at 300 pages/day · festival 4 months. Options: 250 mAh (7.5 mm deep) or 550 mAh (9.5 mm)",
"SR-11": "1000 mAh · 9 months at 300 pages/day · festival 10 months. Options: 500 mAh (9 mm deep) or 1200 mAh (12 mm)",
"SR-12": "1200 mAh · 10 months at 300 pages/day · festival 11 months. Options: 500 mAh (9 mm deep) or 1500 mAh (12 mm)",
"SR-13": "1500 mAh · 4 months at 300 pages/day · festival 6 months. Options: 1000 mAh (13.5 mm deep) or 2500 mAh (18.5 mm)",
"SR-14": "1200 mAh · Wi-Fi build 10 months at 300 pages/day · festival 8 months. Options: 500 mAh (9.5 mm deep) or 2000 mAh (16.5 mm); tethered build: 1 yr+ at 300 pages/day"
};
function renderSpec(v) {
  const ok = v.p5 <= 50 ? '<span class="chip ok">under $50</span>' : '<span class="chip warn">under $50 at volume</span>';
  const rows = [["Size", `${v.dims} mm · ~${v.g} g`], ...(v.std ? [["Pass size", v.std]] : []), ["Wear", v.wear], ["Panel", v.panel], ["Chip", v.chip], ["Radios", v.radio], ["Controls", v.keys], ["microSD", v.sdT], ["USB-C", v.usb], ["Back", v.back], ["Battery", (BAT[v.key] || "") + ". Auto-off; any key wakes"], ...(SUP[v.key] ? [["Supply", SUP[v.key]]] : [])];
  specEl.innerHTML = `<div class="spec-head"><span class="code" style="background:${v.body};color:${lum(v.body) < 0.45 ? "#fff" : "#111"}">${v.key}</span><h3>${v.name}</h3></div>
    <div class="price"><b>$${v.p5.toFixed(2)}</b><span>@5</span><span>$${v.p100.toFixed(2)} @100</span>${ok}</div>
    <dl>${rows.map(([a, b]) => `<dt>${a}</dt><dd>${b}</dd>`).join("")}</dl><p class="mono">${v.tag}</p>`;
}
function updateLabels() {
  if (!state.labels || !cur) return;
  const w = stage.clientWidth, h = stage.clientHeight, cam = camera.position;
  cur.labels.forEach((l, i) => {
    const e = labelEls[i], p = l.p, toCam = cam.clone().sub(p);
    const vis = l.n.dot(toCam) > 0.04 * toCam.length();
    const q = p.clone().project(camera);
    if (!vis || q.z > 1) { e.style.display = "none"; return; }
    const x = (q.x + 1) / 2 * w, y = (1 - q.y) / 2 * h;
    e.style.display = ""; e.classList.toggle("left", x > w * 0.6);
    e.style.transform = `translate(${x.toFixed(1)}px,${y.toFixed(1)}px)`;
  });
}
function resize() {
  const w = stage.clientWidth, h = stage.clientHeight;
  renderer.setSize(w, h, false); camera.aspect = w / h; camera.updateProjectionMatrix();
}
function loop(now) {
  requestAnimationFrame(loop);
  if (document.hidden) return;
  if (tween) {
    const t = Math.max(0, Math.min(1, (performance.now() - tween.t0) / tween.dur)), e = t < 0.5 ? 2 * t * t : 1 - (-2 * t + 2) ** 2 / 2;
    camera.position.lerpVectors(tween.from, tween.to, e); controls.target.lerp(new THREE.Vector3(), e);
    if (t >= 1) tween = null;
  }
  controls.update(); renderer.render(scene, camera); updateLabels();
}

async function start() {
  if (!window.THREE || !THREE.OrbitControls) { stage.querySelector(".fallback").hidden = false; return; }
  try {
    renderer = new THREE.WebGLRenderer({ canvas: document.getElementById("gl"), antialias: true, alpha: true });
  } catch (e) { stage.querySelector(".fallback").hidden = false; return; }
  if (THREE.ColorManagement) THREE.ColorManagement.legacyMode = false;
  renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
  renderer.outputEncoding = THREE.sRGBEncoding;
  renderer.toneMapping = THREE.NoToneMapping;
  maxAniso = renderer.capabilities.getMaxAnisotropy();
  scene = new THREE.Scene();
  if (THREE.RoomEnvironment) { const pm = new THREE.PMREMGenerator(renderer); scene.environment = pm.fromScene(new THREE.RoomEnvironment(), 0.04).texture; }
  renderer.physicallyCorrectLights = false;
  scene.add(new THREE.HemisphereLight(0xffffff, 0x50505a, 0.25));
  const keyL = new THREE.DirectionalLight(0xfff4e8, 0.7); keyL.position.set(-250, 320, 420); scene.add(keyL);
  const rim = new THREE.DirectionalLight(0xe8f0ff, 0.35); rim.position.set(260, 180, -360); scene.add(rim);
  camera = new THREE.PerspectiveCamera(30, 1, 2, 6000);
  controls = new THREE.OrbitControls(camera, renderer.domElement);
  Object.assign(controls, { enableDamping: true, dampingFactor: 0.08, minDistance: 45, maxDistance: 1500, autoRotateSpeed: 1.2 });
  controls.addEventListener("start", () => { tween = null; document.querySelectorAll("[data-view]").forEach(b => b.setAttribute("aria-pressed", "false")); });
  new ResizeObserver(resize).observe(stage); resize();
  try {
    await Promise.race([Promise.all(["600 20px 'Barlow Condensed'", "700 20px 'Barlow Condensed'", "800 20px 'Barlow Condensed'", "400 20px Literata", "600 20px Literata", "italic 400 20px Literata", "500 20px 'IBM Plex Mono'", "600 20px 'IBM Plex Mono'"].map(f => document.fonts.load(f))), new Promise(r => setTimeout(r, 3000))]);
  } catch (e) { /* fall back to system faces */ }
  // picker
  const picker = document.getElementById("picker");
  V.forEach(v => {
    const b = document.createElement("button"); b.type = "button"; b.dataset.v = v.key;
    b.innerHTML = `<i style="background:${v.body}"></i><b>${v.key}</b> ${v.name}`;
    b.addEventListener("click", () => { location.hash = v.key.toLowerCase(); });
    picker.appendChild(b);
  });
  document.querySelectorAll("[data-view]").forEach(b => b.addEventListener("click", () => goView(b.dataset.view)));
  document.querySelectorAll("[data-toggle]").forEach(b => b.addEventListener("click", () => { const k = b.dataset.toggle; state[k] = !state[k]; applyToggles(); }));
  document.querySelectorAll("tr[data-v]").forEach(tr => {
    tr.tabIndex = 0;
    const go = () => { location.hash = tr.dataset.v.toLowerCase(); document.getElementById("viewer").scrollIntoView({ behavior: reduce ? "auto" : "smooth" }); };
    tr.addEventListener("click", go); tr.addEventListener("keydown", e => { if (e.key === "Enter") go(); });
  });
  const fromHash = () => { const k = location.hash.slice(1).toUpperCase(); select(BY[k] ? k : (cur ? cur.v.key : "SR-2")); };
  window.addEventListener("hashchange", fromHash);
  fromHash();
  requestAnimationFrame(loop);
}
start();
})();
