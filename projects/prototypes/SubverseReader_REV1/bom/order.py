#!/usr/bin/env python3
"""Order planner for the SubverseReader BOMs: carts, landed cost, materials-in-hand date.

    python3 order.py                      # offline: snapshot prices, vendor ship-time table
    python3 order.py --live               # also query DigiKey / Mouser / Newark (keys in env)
    python3 order.py --live --mouser-cart # also create one Mouser cart per variant (writes to Mouser)
    python3 order.py --date 2026-11-09    # plan from another order date

Reads the SR-*.csv files that make_bom.py writes. Writes order/ORDER.md, order/order.json
(the design page reads it) and, per variant, order/SR-n.newark.txt.

Keys, all optional (nothing is sent anywhere without --live):
    DIGIKEY_CLIENT_ID, DIGIKEY_CLIENT_SECRET   developer.digikey.com, Product Information v4
    MOUSER_API_KEY                             mouser.com/api-hub, Search API key
    MOUSER_CART_KEY                            mouser.com/api-hub, Cart & Order API key
    ELEMENT14_API_KEY                          partner.element14.com, Product Search API

Click-to-order: DigiKey FastAdd links (forum.digikey.com/t/61356, Oct 2025) put every line
with a DigiKey part number into your DigiKey cart with no account or key. Mouser and Newark
have no link format, so they get their BOM-tool upload file (SR-n.mouser-import.csv) and a
paste list; with --mouser-cart the Mouser Cart API builds the cart instead.
"""
import argparse, csv, datetime as dt, glob, json, os, re, sys, urllib.parse, urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "order")
UNITS = 5  # make_bom.py BUILDS

# ------------------------------------------------------------------ assumptions (edit these)
# vendor: (shipping fee per order USD, handling days, transit days, ships from)
# Calendar days to your door in the US with the vendor's standard paid option. Fees are
# typical for a ~$30-150 order; free-shipping thresholds move, so treat them as estimates.
VENDORS = {
    "DigiKey":      (6.99, 1, 4, "Thief River Falls, MN"),
    "Mouser":       (7.99, 1, 4, "Mansfield, TX"),
    "Newark":       (8.95, 1, 4, "Gaffney, SC"),
    "Adafruit":     (9.00, 2, 4, "New York, NY"),
    "Crystalfontz": (9.00, 1, 4, "Spokane, WA (fee estimated)"),
    "Seeed":        (15.00, 3, 7, "Shenzhen (some US warehouse stock)"),
    "buyepaper":    (25.00, 3, 6, "Dalian (Good Display), DHL"),
    "LCSC":         (20.00, 2, 6, "Shenzhen, DHL"),
    "JLCPCB":       (20.00, 3, 6, "Shenzhen, DHL; bare boards, 3-day build"),
    "AliExpress":   (3.00, 3, 14, "China, AliExpress Standard"),
    "M5Stack":      (15.00, 3, 10, "Shenzhen"),
    "Amazon":       (0.00, 1, 2, "US, Prime; hobby shops (Adafruit, SparkFun) ~5 d; Alibaba for 100"),
    "Ponoko":       (12.00, 5, 4, "US laser cutting"),
    "sticker shop": (6.00, 4, 4, "US print shop"),
    "broker":       (25.00, 5, 7, "Win Source / Unikey, part sourced to order"),
    "own printer":  (0.00, 2, 0, "your bench"),
}
# Passes shipped somewhere other than the US: per-vendor (fee, handling days, transit days) to
# that country. Estimates for a small parcel; vendors not listed keep their US figures.
SHIP_TO = {"SR-1D": "JP"}
VENDORS_TO = {
    "JP": {
        "Seeed":       (12.00, 3, 5, "Shenzhen to Japan, courier (estimate)"),
        "buyepaper":   (20.00, 3, 4, "Dalian to Japan, DHL (estimate)"),
        "AliExpress":  (0.00, 3, 8, "AliExpress Choice / Standard to Japan (estimate)"),
        "own printer": (0.00, 2, 0, "your bench"),
        "sticker shop": (0.00, 4, 3, "local print shop or print it yourself"),
    },
}
# DigiKey, Mouser and Newark carry the same catalog parts: a builder places ONE distributor
# order, so they share one fee and one arrival date.
DISTRIBUTORS = ("DigiKey", "Mouser", "Newark")


def group_of(vendor):
    return "Distributor" if vendor in DISTRIBUTORS else vendor


# where to find a line by hand: vendor -> search URL prefix (query appended, URL-encoded)
SEARCH = {
    "Amazon": "https://www.amazon.com/s?k=",
    "Adafruit": "https://www.adafruit.com/search?q=",
    "Crystalfontz": "https://www.crystalfontz.com/search.php?q=",
    "Seeed": "https://www.seeedstudio.com/catalogsearch/result/?q=",
    "buyepaper": "https://buyepaper.com/search?q=",
    "LCSC": "https://www.lcsc.com/search?q=",
    "AliExpress": "https://www.aliexpress.com/wholesale?SearchText=",
    "M5Stack": "https://shop.m5stack.com/search?q=",
    "broker": "https://octopart.com/search?q=",
    "Distributor": "https://octopart.com/search?q=",
}
# lines that cannot be bought in units of one (PCB fabs make 5)
MIN_QTY = {"PCB": 5}

# MPN or description fragment -> (vendor, days to door or None if no date, why)
LEAD = {
    "ESP32-C5-WROOM-1-N8R8": ("broker", 28, "no distributor stock of N8R8; Unikey broker or Espressif samples"),
    "RT9080-33GJ5":          ("broker", 14, "genuine Richtek at brokers; HT7833 at LCSC is the 8-day fix"),
    "ST25DV04K-JFR6D3":      ("Newark", 28, "JFR6D3 stock unconfirmed; IER6S3 (no LPD) is in stock at Newark"),
    "MDBT50Q-1MV2":          ("broker", 12, "TME / Win Source"),
    "K159":                  ("M5Stack", None, "sold out at M5Stack; LilyGo T5 4.7in is the fallback"),
    "TPS65185RGZR":          ("DigiKey", None, "out at TI, no authorised listings"),
    "ED060KC1":              ("AliExpress", None, "quote-only at distributors"),
}

# ------------------------------------------------------------------ the BOM


def vendor_of(r):
    """Where a line is bought in the offline plan."""
    for key, (v, _, _) in LEAD.items():
        if key == r["mpn"] or (not r["mpn"] and key in r["description"]):
            return v
    o = r["other_vendor"].lower()
    if "amazon" in o:
        return "Amazon"  # cells, ferrite, screws: US stock, two days
    for name in ("adafruit", "crystalfontz", "seeed", "buyepaper", "lcsc", "jlcpcb", "aliexpress", "m5stack",
                 "ponoko", "sticker shop", "own printer", "newark"):
        if name in o:
            return {"buyepaper": "buyepaper", "m5stack": "M5Stack", "own printer": "own printer",
                    "sticker shop": "sticker shop", "lcsc": "LCSC", "jlcpcb": "JLCPCB",
                    "aliexpress": "AliExpress", "newark": "Newark"}.get(name, name.title())
    if r["digikey_pn"]:
        return "DigiKey"
    if r["mouser_pn"]:
        return "Mouser"
    if r["mpn"]:
        return "DigiKey"  # catalog part with no PN recorded: found by MPN in the BOM tool
    return "Amazon"       # generic bench stock (perfboard, wire)


def load():
    boms = {}
    for f in sorted(glob.glob(os.path.join(HERE, "SR-*.csv")), key=lambda p: (len(p), p)):
        if "-import" in f:
            continue
        key = os.path.basename(f)[:-4]
        rows = list(csv.DictReader(open(f)))
        for r in rows:
            r["order_qty"] = int(r["order_qty"])
            r["unit_usd"] = float(r["unit_usd"])
            r["vendor"] = vendor_of(r)
            r["quote"] = None
        boms[key] = rows
    return boms

# ------------------------------------------------------------------ live quotes


def http(url, data=None, headers=None, form=False):
    body = None
    h = {"User-Agent": "subversereader-order/1", "Accept": "application/json"}
    h.update(headers or {})
    if data is not None:
        if form:
            body = urllib.parse.urlencode(data).encode()
            h["Content-Type"] = "application/x-www-form-urlencoded"
        else:
            body = json.dumps(data).encode()
            h["Content-Type"] = "application/json"
    with urllib.request.urlopen(urllib.request.Request(url, body, h), timeout=30) as resp:
        return json.load(resp)


def price_at(breaks, qty):
    """breaks: [(min_qty, unit_usd)]; price for qty, or the first break if qty is below it."""
    breaks = sorted(breaks)
    p = breaks[0][1] if breaks else None
    for q, u in breaks:
        if qty >= q:
            p = u
    return p


def days_from(text):
    m = re.search(r"(\d+)\s*(day|week)", str(text or ""), re.I)
    if not m:
        return None
    return int(m.group(1)) * (7 if m.group(2).lower().startswith("week") else 1)


class DigiKey:
    name = "DigiKey"

    def __init__(self):
        self.id, self.secret = os.environ.get("DIGIKEY_CLIENT_ID"), os.environ.get("DIGIKEY_CLIENT_SECRET")
        self.token = None

    def ok(self):
        return bool(self.id and self.secret)

    def quote(self, r):
        if not self.token:
            self.token = http("https://api.digikey.com/v1/oauth2/token",
                              {"client_id": self.id, "client_secret": self.secret,
                               "grant_type": "client_credentials"}, form=True)["access_token"]
        pn = r["digikey_pn"] or r["mpn"]
        d = http(f"https://api.digikey.com/products/v4/search/{urllib.parse.quote(pn, safe='')}/productdetails",
                 headers={"X-DIGIKEY-Client-Id": self.id, "Authorization": f"Bearer {self.token}",
                          "X-DIGIKEY-Locale-Site": "US", "X-DIGIKEY-Locale-Currency": "USD"})["Product"]
        best = None
        for var in d.get("ProductVariations", []):
            if var.get("MinimumOrderQuantity", 1) > r["order_qty"]:
                continue  # full reels
            p = price_at([(b["BreakQuantity"], b["UnitPrice"]) for b in var.get("StandardPricing", [])], r["order_qty"])
            if p is not None and (best is None or p < best[1]):
                best = (var["DigiKeyProductNumber"], p, var.get("QuantityAvailableforPackageType", 0))
        if not best:
            return None
        lead = d.get("ManufacturerLeadWeeks")
        return dict(dist="DigiKey", pn=best[0], unit=best[1], stock=int(best[2] or d.get("QuantityAvailable", 0)),
                    lead_days=int(lead) * 7 if str(lead or "").isdigit() else None, url=d.get("ProductUrl"))


class Mouser:
    name = "Mouser"

    def __init__(self):
        self.key = os.environ.get("MOUSER_API_KEY")

    def ok(self):
        return bool(self.key)

    def quote(self, r):
        pn = r["mouser_pn"] or r["mpn"]
        d = http(f"https://api.mouser.com/api/v1/search/partnumber?apiKey={self.key}",
                 {"SearchByPartRequest": {"mouserPartNumber": pn, "partSearchOptions": "Exact"}})
        parts = (d.get("SearchResults") or {}).get("Parts") or []
        parts = [p for p in parts if p.get("MouserPartNumber") not in (None, "", "N/A")]
        if not parts:
            return None
        p = parts[0]
        br = [(int(b["Quantity"]), float(re.sub(r"[^\d.]", "", b["Price"]))) for b in p.get("PriceBreaks", [])]
        stock = int(re.sub(r"\D", "", str(p.get("AvailabilityInStock") or "0")) or 0)
        return dict(dist="Mouser", pn=p["MouserPartNumber"], unit=price_at(br, r["order_qty"]), stock=stock,
                    lead_days=days_from(p.get("LeadTime")), url=p.get("ProductDetailUrl"))


class Newark:
    name = "Newark"

    def __init__(self):
        self.key = os.environ.get("ELEMENT14_API_KEY")

    def ok(self):
        return bool(self.key)

    def quote(self, r):
        q = urllib.parse.urlencode({
            "term": f"manuPartNum:{r['mpn']}", "storeInfo.id": "www.newark.com",
            "resultsSettings.offset": 0, "resultsSettings.numberOfResults": 5,
            "resultsSettings.responseGroup": "large", "callInfo.responseDataFormat": "JSON",
            "callInfo.apiKey": self.key})
        d = http(f"https://api.element14.com/catalog/products?{q}")
        prods = (d.get("manufacturerPartNumberSearchReturn") or {}).get("products") or []
        if not prods:
            return None
        p = prods[0]
        st = p.get("stock") or {}
        br = [(int(x["from"]), float(x["cost"])) for x in p.get("prices", [])]
        lead = st.get("leastLeadTime")
        return dict(dist="Newark", pn=p.get("sku"), unit=price_at(br, r["order_qty"]),
                    stock=int(st.get("level") or 0), lead_days=int(lead) if str(lead or "").isdigit() else None,
                    url=f"https://www.newark.com/{p.get('sku')}")


def live(boms, date):
    dists = [c() for c in (DigiKey, Mouser, Newark)]
    dists = [d for d in dists if d.ok()]
    if not dists:
        sys.exit("--live: no DIGIKEY_CLIENT_ID/SECRET, MOUSER_API_KEY or ELEMENT14_API_KEY in the environment")
    cache_f = os.path.join(OUT, f"quotes-{date}.json")
    cache = json.load(open(cache_f)) if os.path.exists(cache_f) else {}
    for rows in boms.values():
        for r in rows:
            if not (r["mpn"] or r["digikey_pn"] or r["mouser_pn"]) or r["vendor"] in ("Seeed", "buyepaper", "Adafruit", "Crystalfontz", "M5Stack"):
                continue
            quotes = []
            for d in dists:
                k = f"{d.name}|{r['mpn'] or r['digikey_pn']}|{r['order_qty']}"
                if k not in cache:
                    try:
                        cache[k] = d.quote(r)
                    except Exception as e:  # a miss at one distributor is not fatal
                        cache[k] = {"error": str(e)[:200]}
                    print(f"  {d.name:8} {r['mpn'] or r['digikey_pn']}: {cache[k]}", file=sys.stderr)
                q = cache[k]
                if q and "error" not in q and q.get("unit") is not None:
                    quotes.append(q)
            full = [q for q in quotes if q["stock"] >= r["order_qty"]]
            if full:
                r["quote"] = min(full, key=lambda q: q["unit"])
            elif quotes:
                r["quote"] = min(quotes, key=lambda q: q["lead_days"] or 999)
            if r["quote"]:
                q = r["quote"]
                r["vendor"], r["unit_usd"] = q["dist"], q["unit"]
                if q["dist"] == "DigiKey":
                    r["digikey_pn"] = q["pn"]
                elif q["dist"] == "Mouser":
                    r["mouser_pn"] = q["pn"]
    json.dump(cache, open(cache_f, "w"), indent=1)


def mouser_cart(key, rows):
    """Mouser Cart API: one new cart per variant. Returns the CartKey."""
    items = [{"MouserPartNumber": r["mouser_pn"], "Quantity": r["order_qty"], "CustomerPartNumber": r["ref"]}
             for r in rows if r["vendor"] == "Mouser" and r["mouser_pn"]]
    if not items:
        return None
    d = http("https://api.mouser.com/api/v1.0/cart/items/insert?" + urllib.parse.urlencode(
        {"apiKey": os.environ["MOUSER_CART_KEY"], "countryCode": "US", "currencyCode": "USD"}),
        {"CartKey": "", "CartItems": items})
    return d.get("CartKey")

# ------------------------------------------------------------------ plan


def fastadd(rows):
    """DigiKey FastAdd GET links, split to stay under the ~1700-character limit."""
    base = "https://www.digikey.com/classic/ordering/fastadd.aspx?"
    links, params, n = [], [], 0
    for r in rows:
        if r["vendor"] != "DigiKey" or not r["digikey_pn"]:
            continue
        n += 1
        p = [(f"part{n}", r["digikey_pn"]), (f"qty{n}", r["order_qty"]), (f"cref{n}", r["ref"])]
        if params and len(base + urllib.parse.urlencode(params + p)) > 1650:
            links.append(base + urllib.parse.urlencode(params))
            n, params = 1, []
            p = [(f"part{n}", r["digikey_pn"]), (f"qty{n}", r["order_qty"]), (f"cref{n}", r["ref"])]
        params += p
    if params:
        links.append(base + urllib.parse.urlencode(params))
    return links


def plan(key, rows, date):
    by = {}
    for r in rows:
        ext = r["unit_usd"] * r["order_qty"]
        lead = None
        hit = next((v for k, v in LEAD.items() if k == r["mpn"] or (not r["mpn"] and k in r["description"])), None)
        if r["quote"] and r["quote"]["stock"] < r["order_qty"]:
            lead = (r["quote"]["lead_days"] or 56, "short at every distributor; factory lead")
        elif r["quote"]:
            lead = None
        elif hit:
            lead = (hit[1], hit[2])
        g = group_of(r["vendor"])
        vk = "DigiKey" if g == "Distributor" else r["vendor"]
        fee, hand, transit, _ = VENDORS_TO.get(SHIP_TO.get(key), {}).get(vk) or VENDORS[vk]
        days = hand + transit if lead is None else lead[0]
        why = "in stock" if lead is None else lead[1]
        if days is not None and r["vendor"] == "JLCPCB" and "PCB" in r["ref"]:
            why = "bare boards; add ~5 days for assembly"
        r["days"], r["why"] = days, why
        r["group"] = g
        v = by.setdefault(g, dict(lines=0, parts=0.0, fee=fee, days=0))
        v["lines"] += 1
        v["parts"] += ext
        v["days"] = None if (days is None or v["days"] is None) else max(v["days"], days)
    parts = sum(v["parts"] for v in by.values())
    ship = sum(v["fee"] for v in by.values())
    blocked = [r for r in rows if r["days"] is None]
    dated = [r for r in rows if r["days"] is not None]
    gate = max(dated, key=lambda r: r["days"]) if dated else None
    eta = None if blocked else (date + dt.timedelta(days=gate["days"]))
    dk = [r for r in rows if r["vendor"] == "DigiKey"]
    return dict(
        variant=key, parts=round(parts, 2), shipping=round(ship, 2), landed=round(parts + ship, 2),
        per_unit=round((parts + ship) / UNITS, 2), vendors=len(by),
        eta=eta.isoformat() if eta else None, eta_days=None if blocked else gate["days"],
        gate=None if not gate else f"{gate['ref']} {gate['mpn'] or gate['description'][:40]} ({gate['vendor']}: {gate['why']})",
        blocked=[f"{r['ref']} {r['mpn'] or r['description'][:40]}: {r['why']}" for r in blocked],
        by_vendor={k: dict(v, parts=round(v["parts"], 2)) for k, v in sorted(by.items(), key=lambda kv: -kv[1]["parts"])},
        digikey_links=fastadd(rows),
        pcb=any(r["ref"] == "PCB" for r in rows),
        lines=[dict(ref=r["ref"], part=r["mpn"] or r["description"], desc=r["description"], mfr=r["manufacturer"],
                    mpn=r["mpn"], dk=r["digikey_pn"], mo=r["mouser_pn"], vendor=r["vendor"], group=r["group"],
                    per=int(r["qty_per_unit"]), min=MIN_QTY.get(r["ref"], 0), unit=r["unit_usd"],
                    days=r["days"], why=r["why"], src=r["other_vendor"], note=r["note"],
                    link=(SEARCH.get(r["group"]) or SEARCH.get(r["vendor"]) or "") and
                    (SEARCH.get(r["group"]) or SEARCH.get(r["vendor"])) + urllib.parse.quote(
                        r["mpn"] or r["description"].split(",")[0][:60]))
               for r in rows],
        digikey_missing=[f"{r['ref']} {r['mpn']}" for r in dk if not r["digikey_pn"]],
        mouser_lines=sum(1 for r in rows if r["vendor"] == "Mouser"),
        newark_lines=sum(1 for r in rows if r["vendor"] == "Newark"),
    )


def batch_of(plans):
    ok = [p for p in plans if p["eta"]]
    fees = {}
    for p in ok:
        for v, b in p["by_vendor"].items():
            fees[v] = b["fee"]
    parts = sum(p["parts"] for p in ok)
    sep = sum(p["shipping"] for p in ok)
    return dict(variants=len(ok), parts=round(parts, 2), shipping=round(sum(fees.values()), 2),
                shipping_separate=round(sep, 2), eta=max(p["eta"] for p in ok) if ok else None,
                ship_share=round(100 * sep / (parts + sep)) if ok else 0)


def write(plans, boms, date, mode, carts):
    os.makedirs(OUT, exist_ok=True)
    for key, rows in boms.items():
        with open(os.path.join(OUT, f"{key}.newark.txt"), "w") as f:
            f.write("# Newark Quick Paste / BOM upload: manufacturer part number, quantity, reference\n")
            for r in rows:
                if r["mpn"] and r["vendor"] in ("DigiKey", "Mouser", "Newark"):
                    f.write(f"{r['mpn']},{r['order_qty']},{r['ref']}\n")
    batch = batch_of(plans)
    json.dump(dict(date=date.isoformat(), mode=mode, units=UNITS, vendors=VENDORS, plans=plans, batch=batch,
                   mouser_carts=carts),
              open(os.path.join(OUT, "order.json"), "w"), indent=1)
    md = ["# SubverseReader REV1: order plan", "",
          f"Generated by `order.py` ({mode}) for an order placed **{date.isoformat()}**, {UNITS} units per "
          "variant. *Landed* = parts + one standard shipping fee per vendor (no tax or import duty; "
          "China-shipped orders may add tariffs). *Materials in hand* = the slowest line's handling "
          "+ transit, or its lead time; PCB design and assembly time are not included.", "",
          "Offline numbers use the BOM snapshot prices (2026-11-06/07) and the vendor table at the "
          "top of `order.py`. Run `python3 order.py --live` with distributor keys for today's "
          "stock, price breaks and lead times.", "",
          "| Variant | Parts | Shipping | Landed (5) | Per unit | Vendors | Materials in hand | Gating line | DigiKey cart |",
          "|---|---|---|---|---|---|---|---|---|"]
    for p in plans:
        eta = f"{p['eta']} ({p['eta_days']} d)" if p["eta"] else "**blocked**"
        gate = p["gate"] if p["eta"] else "; ".join(p["blocked"])
        links = " ".join(f"[{'cart' if i == 0 else f'part {i + 1}'}]({u})" for i, u in enumerate(p["digikey_links"]))
        md.append(f"| {p['variant']} | ${p['parts']:,.2f} | ${p['shipping']:,.2f} | ${p['landed']:,.2f} | "
                  f"${p['per_unit']:,.2f} | {p['vendors']} | {eta} | {gate} | {links or '—'} |")
    md += ["", f"**Shipping is the hidden cost at 5 units:** {batch['ship_share']}% of the landed total across "
           f"all variants. Ordering every buildable variant together pays each vendor's fee once: parts "
           f"${batch['parts']:,.2f} + shipping ${batch['shipping']:,.2f} (instead of ${batch['shipping_separate']:,.2f} "
           f"ordered one variant at a time) for {batch['variants']} variants, materials in hand {batch['eta']}.",
           "", "## How to order a variant", "",
           "1. **DigiKey:** click the *cart* link above (DigiKey FastAdd). It adds every DigiKey line "
           "with its quantity and reference designator to your cart; no account needed until checkout. "
           "Lines listed under *DigiKey by MPN* have no DigiKey number yet: add them in DigiKey's BOM "
           "Manager from `../SR-n.digikey-import.csv`, or run `--live` to look the numbers up.",
           "2. **Mouser:** upload `../SR-n.mouser-import.csv` at mouser.com/bom, or run "
           "`--live --mouser-cart` with a Cart API key to build the cart directly.",
           "3. **Newark:** paste `SR-n.newark.txt` into Newark's Quick Paste / BOM upload.",
           "4. **Direct:** the remaining vendors (Good Display panels, Seeed, Adafruit, AliExpress "
           "cells, LCSC passives, JLCPCB boards) are listed per variant below.", "",
           "## Vendor assumptions", "", "| Vendor | Fee | Handling + transit | Ships from |", "|---|---|---|---|"]
    for k, (fee, h, t, frm) in VENDORS.items():
        md.append(f"| {k} | ${fee:.2f} | {h} + {t} d | {frm} |")
    for p in plans:
        rows = boms[p["variant"]]
        md += ["", f"## {p['variant']}", ""]
        if p["blocked"]:
            md.append("**Blocked:** " + "; ".join(p["blocked"]))
            md.append("")
        md += ["| Vendor | Lines | Parts | Fee | Days |", "|---|---|---|---|---|"]
        for v, b in p["by_vendor"].items():
            md.append(f"| {v} | {b['lines']} | ${b['parts']:,.2f} | ${b['fee']:.2f} | {b['days'] if b['days'] is not None else 'blocked'} |")
        if p["digikey_missing"]:
            md += ["", "DigiKey by MPN (not in the cart link): " + ", ".join(p["digikey_missing"])]
        direct = [r for r in rows if r["vendor"] not in ("DigiKey", "Mouser", "Newark", "own printer")]
        if direct:
            md += ["", "Direct: " + "; ".join(f"{r['ref']} ×{r['order_qty']} {r['mpn'] or r['description'][:38]} "
                                              f"({r['vendor']}, ${r['unit_usd']:.2f})" for r in direct)]
    open(os.path.join(OUT, "ORDER.md"), "w").write("\n".join(md) + "\n")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--live", action="store_true", help="query distributor APIs (needs keys)")
    ap.add_argument("--mouser-cart", action="store_true", help="create Mouser carts (needs MOUSER_CART_KEY)")
    ap.add_argument("--date", default=dt.date.today().isoformat(), help="order date, YYYY-MM-DD")
    a = ap.parse_args()
    date = dt.date.fromisoformat(a.date)
    os.makedirs(OUT, exist_ok=True)
    boms = load()
    if a.live:
        live(boms, date)
    carts = {}
    if a.mouser_cart:
        if not os.environ.get("MOUSER_CART_KEY"):
            sys.exit("--mouser-cart needs MOUSER_CART_KEY")
        for k, rows in boms.items():
            carts[k] = mouser_cart(k, rows)
            print(f"{k}: Mouser CartKey {carts[k]}")
    plans = [plan(k, rows, date) for k, rows in boms.items()]
    write(plans, boms, date, "live" if a.live else "offline", carts)
    for p in plans:
        print(f"{p['variant']:6} parts ${p['parts']:7.2f} + ship ${p['shipping']:6.2f} = ${p['landed']:7.2f} "
              f"(${p['per_unit']:.2f}/unit)  {p['eta'] or 'BLOCKED':10}  {p['gate'] if p['eta'] else p['blocked'][0]}")


if __name__ == "__main__":
    main()
