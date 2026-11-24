#!/usr/bin/env python3
"""
svpk.py: the desktop pack builder for SubverseReader (pack format v0).

Turns zines into .svpk packs rendered for one pass's panel, so firmware can be
tested without the phone app (design doc section 12, step 2).

    svpk.py opds  https://spicemine.iotj.cc/opds            # list what a catalog has
    svpk.py build https://spicemine.iotj.cc/opds/zines -o /Volumes/SDCARD/packs
    svpk.py build zine.pdf -o packs/                        # a local PDF or CBZ
    svpk.py show  packs/zine-joblow.svpk --png out/         # check a pack, dump pages

Needs pypdfium2 and Pillow:  uv pip install pypdfium2 pillow

Pack format v0 (little-endian; the firmware reads it with a table lookup):

  header, 256 bytes
    0   char[4]  "SVPK"
    4   u16      version (0)
    6   u16      header size (256)
    8   u16      width, 10 u16 height    page size in pixels, as the reader holds the pass
    12  u8       bits per pixel (1)
    13  u8       orientation (0 portrait, 1 landscape)
    14  u16      flags (0)
    16  u32      page count
    20  u32      page table offset
    24  u32      reserved (TOC offset, 0)
    28  u8[32]   sha256 of the source file
    60  char[64]  title, UTF-8, NUL padded
    124 char[64]  author
    188 char[64]  source URL (truncated)
    252 u32      unix time the pack was built
  page table: page count x 12 bytes
    u32 offset, u32 length, u8 compression (0 raw, 1 PackBits), u8 hint (0 text, 1 image), u16 0
  pages: rows top to bottom, MSB first, 1 = black ink, (width+7)/8 bytes a row
"""

import argparse, hashlib, io, json, os, re, struct, sys, time, urllib.parse, urllib.request, zipfile

PROFILES = {
    # name: (width, height, bpp, orientation, description)
    "sr15": (300, 400, 1, 0, "SR-15 Dial Pass, CrowPanel 4.2in 400x300 held portrait"),
    "sr1": (300, 400, 1, 0, "SR-1 family, 4.2in 400x300 portrait"),
    "sr16": (240, 416, 1, 0, "SR-16 Setlist, 3.7in 240x416 portrait"),
}
UA = "svpk/0 (SubverseReader pack builder)"
MAGIC, VERSION, HDR = b"SVPK", 0, 256


# ---------------------------------------------------------------- fetching

def get(url):
    req = urllib.request.Request(url, headers={"User-Agent": UA, "Accept": "application/opds+json, */*"})
    with urllib.request.urlopen(req, timeout=60) as r:
        return r.read()


def opds_publications(url, seen=None):
    """Every publication under an OPDS 2.0 feed, following subsections and paging once each."""
    seen = seen if seen is not None else set()
    if url in seen:
        return []
    seen.add(url)
    feed = json.loads(get(url))
    pubs = list(feed.get("publications", []))
    for g in feed.get("groups", []):
        pubs += g.get("publications", [])
    for nav in feed.get("navigation", []) + [l for l in feed.get("links", []) if l.get("rel") == "next"]:
        if "opds+json" in nav.get("type", "") and nav.get("rel") in ("subsection", "next", None):
            pubs += opds_publications(urllib.parse.urljoin(url, nav["href"]), seen)
    out, ids = [], set()
    for p in pubs:
        pid = p["metadata"].get("identifier") or p["metadata"].get("title")
        if pid not in ids:
            ids.add(pid)
            out.append(p)
    return out


def best_acquisition(pub):
    """The reading copy (pages in order) beats the comic copy, which beats the print sheets."""
    acq = [l for l in pub.get("links", []) if "acquisition" in l.get("rel", "")]
    def rank(l):
        t, title = l.get("type", ""), l.get("title", "").lower()
        if t == "application/pdf" and "reading" in title: return 0
        if "comicbook" in t: return 1
        if t == "application/pdf" and "print" not in title: return 2
        if t == "application/pdf": return 3
        return 9
    acq.sort(key=rank)
    return acq[0] if acq and rank(acq[0]) < 9 else None


# ---------------------------------------------------------------- rendering

def source_pages(data, kind, w, h):
    """Yield greyscale PIL images of each page, fitted inside w x h."""
    from PIL import Image
    if kind == "pdf":
        import pypdfium2 as pdfium
        doc = pdfium.PdfDocument(data)
        for i in range(len(doc)):
            pw, ph = doc[i].get_size()
            scale = min(w / pw, h / ph) * 2          # render at 2x, then downsample: smoother text
            img = doc[i].render(scale=scale, grayscale=True).to_pil().convert("L")
            yield fit(img, w, h)
    elif kind == "cbz":
        z = zipfile.ZipFile(io.BytesIO(data))
        for name in sorted(n for n in z.namelist() if re.search(r"\.(png|jpe?g|webp|gif)$", n, re.I)):
            yield fit(Image.open(z.open(name)).convert("L"), w, h)
    else:
        raise SystemExit(f"svpk: can't read {kind}")


def fit(img, w, h):
    from PIL import Image
    img = trim_margins(img)
    s = min(w / img.width, h / img.height)
    img = img.resize((max(1, round(img.width * s)), max(1, round(img.height * s))), Image.LANCZOS)
    page = Image.new("L", (w, h), 255)
    page.paste(img, ((w - img.width) // 2, (h - img.height) // 2))
    return page


def trim_margins(img, pad=6):
    """Crop plain white margins so the zine fills the glass."""
    from PIL import ImageOps
    box = ImageOps.invert(img).point(lambda v: 255 if v > 24 else 0).getbbox()
    if not box:
        return img
    l, t, r, b = box
    return img.crop((max(0, l - pad), max(0, t - pad), min(img.width, r + pad), min(img.height, b + pad)))


def to_1bit(img):
    """Text and line art get a threshold; pages with lots of mid-grey (photos) get Floyd-Steinberg."""
    from PIL import Image
    hist = img.histogram()
    mid = sum(hist[48:208]) / (img.width * img.height)
    if mid > 0.12:
        return img.convert("1", dither=Image.FLOYDSTEINBERG), 1
    return img.point(lambda v: 255 if v > 215 else 0).convert("1", dither=Image.NONE), 0


def pack_bits(img):
    """PIL '1' mode is 1 = white; the pack stores 1 = black ink."""
    raw = img.tobytes()
    return bytes(b ^ 0xFF for b in raw)


def packbits(data):
    out, i, n = bytearray(), 0, len(data)
    while i < n:
        j = i
        while j + 1 < n and data[j + 1] == data[i] and j - i < 127:
            j += 1
        if j > i:                                       # a run of i..j
            out += bytes([257 - (j - i + 1), data[i]])
            i = j + 1
            continue
        j = i
        while j < n and j - i < 128 and not (j + 2 < n and data[j] == data[j + 1] == data[j + 2]):
            j += 1
        out += bytes([j - i - 1]) + data[i:j]
        i = j
    return bytes(out)


def unpackbits(data, size):
    out, i = bytearray(), 0
    while len(out) < size and i < len(data):
        c = data[i]; i += 1
        if c < 128:
            out += data[i:i + c + 1]; i += c + 1
        elif c > 128:
            out += bytes([data[i]]) * (257 - c); i += 1
    return bytes(out[:size])


# ---------------------------------------------------------------- pack I/O

def fixed(s, n):
    b = (s or "").encode("utf-8")[: n - 1]
    return b + b"\0" * (n - len(b))


def write_pack(path, profile, pages, title, author, source_url, sha):
    w, h, bpp, orient, _ = PROFILES[profile]
    blobs = []
    for img, hint in pages:
        raw = pack_bits(img)
        comp = packbits(raw)
        blobs.append((comp, 1, hint) if len(comp) < len(raw) else (raw, 0, hint))
    table_off = HDR
    off = table_off + 12 * len(blobs)
    table = b""
    for data, comp, hint in blobs:
        table += struct.pack("<IIBBH", off, len(data), comp, hint, 0)
        off += len(data)
    hdr = (MAGIC + struct.pack("<HHHHBBHIII", VERSION, HDR, w, h, bpp, orient, 0, len(blobs), table_off, 0)
           + sha + fixed(title, 64) + fixed(author, 64) + fixed(source_url, 64) + struct.pack("<I", int(time.time())))
    assert len(hdr) == HDR, len(hdr)
    with open(path, "wb") as f:
        f.write(hdr + table + b"".join(d for d, _, _ in blobs))
    return off


def read_pack(path):
    data = open(path, "rb").read()
    if data[:4] != MAGIC:
        raise SystemExit(f"{path}: not a .svpk")
    ver, hdr, w, h, bpp, orient, flags, n, toff, _ = struct.unpack_from("<HHHHBBHIII", data, 4)
    s = lambda o: data[o:o + 64].split(b"\0")[0].decode("utf-8", "replace")
    pages = [struct.unpack_from("<IIBBH", data, toff + 12 * i) for i in range(n)]
    return dict(version=ver, w=w, h=h, bpp=bpp, orient=orient, n=n, title=s(60), author=s(124),
                source=s(188), built=struct.unpack_from("<I", data, 252)[0], pages=pages, data=data)


# ---------------------------------------------------------------- commands

def slug(s):
    return re.sub(r"[^a-z0-9]+", "-", s.lower()).strip("-")[:40] or "pack"


def build_one(data, kind, profile, out_dir, title, author, url, name=None):
    w, h = PROFILES[profile][:2]
    pages = [to_1bit(p) for p in source_pages(data, kind, w, h)]
    os.makedirs(out_dir, exist_ok=True)
    path = os.path.join(out_dir, (name or slug(title)) + ".svpk")
    size = write_pack(path, profile, pages, title, author, url, hashlib.sha256(data).digest())
    print(f"  {os.path.basename(path)}: {len(pages)} pages, {size / 1024:.1f} KB  ({title})")
    return path


def cmd_opds(a):
    for p in opds_publications(a.url):
        m, acq = p["metadata"], best_acquisition(p)
        print(f"- {m.get('title')}  [{m.get('numberOfPages', '?')} pp]  "
              f"{acq['type'] + ' ' + acq['href'] if acq else '(nothing this builder can read)'}")


def cmd_build(a):
    src = a.source
    if src.startswith("http") and not re.search(r"\.(pdf|cbz)(\?|$)", src, re.I):
        pubs = opds_publications(src)
        print(f"{len(pubs)} publications in {src}")
        for p in pubs:
            m, acq = p["metadata"], best_acquisition(p)
            if not acq:
                print(f"  skip {m.get('title')}: no PDF or CBZ"); continue
            data = get(urllib.parse.urljoin(src, acq["href"]))
            kind = "cbz" if "comicbook" in acq["type"] else "pdf"
            author = ", ".join(x.get("name", "") for x in m.get("author", []))
            ident = m.get("identifier", "")
            build_one(data, kind, a.profile, a.out, m.get("title", "untitled"), author, ident,
                      slug(os.path.splitext(os.path.basename(urllib.parse.urlparse(ident).path))[0] or m.get("title", "")))
        return
    data = get(src) if src.startswith("http") else open(src, "rb").read()
    kind = "cbz" if src.lower().endswith(".cbz") else "pdf"
    title = a.title or os.path.splitext(os.path.basename(urllib.parse.urlparse(src).path))[0]
    build_one(data, kind, a.profile, a.out, title, a.author or "", src if src.startswith("http") else "")


def cmd_show(a):
    p = read_pack(a.pack)
    print(f"{a.pack}: v{p['version']} {p['w']}x{p['h']} {p['bpp']}-bit, {p['n']} pages, "
          f"'{p['title']}' by {p['author'] or '?'}, built {time.strftime('%Y-%m-%d %H:%M', time.localtime(p['built']))}")
    stride = (p["w"] + 7) // 8
    for i, (off, ln, comp, hint, _) in enumerate(p["pages"]):
        blob = p["data"][off:off + ln]
        raw = unpackbits(blob, stride * p["h"]) if comp == 1 else blob
        print(f"  page {i + 1}: {ln} bytes, {'packbits' if comp else 'raw'}, {'image' if hint else 'text'}")
        if a.png:
            from PIL import Image
            os.makedirs(a.png, exist_ok=True)
            Image.frombytes("1", (p["w"], p["h"]), bytes(b ^ 0xFF for b in raw)).save(
                os.path.join(a.png, f"page{i + 1:03d}.png"))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    o = sub.add_parser("opds", help="list a catalog's publications"); o.add_argument("url"); o.set_defaults(fn=cmd_opds)
    b = sub.add_parser("build", help="build packs from an OPDS feed, a URL or a file")
    b.add_argument("source"); b.add_argument("-o", "--out", default="packs")
    b.add_argument("-p", "--profile", default="sr15", choices=sorted(PROFILES))
    b.add_argument("--title"); b.add_argument("--author"); b.set_defaults(fn=cmd_build)
    s = sub.add_parser("show", help="describe a pack"); s.add_argument("pack"); s.add_argument("--png")
    s.set_defaults(fn=cmd_show)
    a = ap.parse_args()
    a.fn(a)


if __name__ == "__main__":
    main()
