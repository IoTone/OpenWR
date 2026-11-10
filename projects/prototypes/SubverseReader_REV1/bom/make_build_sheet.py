#!/usr/bin/env python3
"""Build-cost workbook for the first SR-15 and SR-16 batches.

    python3 make_build_sheet.py            # writes build/SubverseReader-SR15-SR16-build.xlsx
    python3 make_build_sheet.py --force    # overwrite an existing workbook (loses entered actuals)

Reads the order planner's snapshot (order/order.json, from `python3 order.py`) for the parts,
quantities, estimate prices and per-vendor shipping. The workbook has one build sheet per
variant (estimate vs actual per line, then per-order shipping, tariff, tax) and a Profit sheet
(at-cost pricing, absorbed units). Every total is a formula, so it recalculates in Excel.
Needs openpyxl.
"""
import argparse, json, os, sys

from openpyxl import Workbook
from openpyxl.comments import Comment
from openpyxl.styles import Alignment, Border, Font, PatternFill, Side
from openpyxl.utils import get_column_letter as col
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.formatting.rule import FormulaRule

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "build", "SubverseReader-SR15-SR16-build.xlsx")
VARIANTS = ["SR-15", "SR-16"]
CHINA = {"Elecrow", "Seeed", "LCSC", "JLCPCB", "AliExpress", "M5Stack", "buyepaper"}
TARIFF_PLACEHOLDER = 0.30   # placeholder rate on China-shipped orders; replace from the invoice
# Part numbers the planner keeps only in its link table.
PN_OVERRIDE = {"DIV": "Yageo RC0603FR-07100KL", "PRF": "generic (see link)"}

F = "Arial"
BLUE, GREEN, BLACK, GREY = "0000FF", "008000", "000000", "666666"
INPUT_FILL = PatternFill("solid", fgColor="FFF2CC")      # pale yellow: fill these in
HEAD_FILL = PatternFill("solid", fgColor="1F4E5A")
SUB_FILL = PatternFill("solid", fgColor="DDEBF0")
TOT_FILL = PatternFill("solid", fgColor="EEEEEE")
THIN = Side(style="thin", color="BBBBBB")
BOX = Border(top=THIN, bottom=THIN, left=THIN, right=THIN)
USD = '$#,##0.00;($#,##0.00);"-"'
PCT = '0.0%;(0.0%);"-"'
INT = '#,##0;(#,##0);"-"'


def font(color=BLACK, bold=False, size=10, italic=False, underline=None):
    return Font(name=F, color=color, bold=bold, size=size, italic=italic, underline=underline)


def put(ws, ref, value, color=BLACK, bold=False, fmt=None, fill=None, size=10, wrap=False,
        italic=False, align=None, border=False, comment=None):
    c = ws[ref]
    c.value = value
    c.font = font(color, bold, size, italic)
    if fmt:
        c.number_format = fmt
    if fill:
        c.fill = fill
    if wrap or align:
        c.alignment = Alignment(wrap_text=wrap, vertical="top", horizontal=align)
    if border:
        c.border = BOX
    if comment:
        c.comment = Comment(comment, "SubverseReader")
    return c


def inp(ws, ref, value=None, fmt=None, comment=None):
    """A cell the builder fills in: blue text on yellow."""
    return put(ws, ref, value, BLUE, fmt=fmt, fill=INPUT_FILL, border=True, comment=comment)


def header(ws, row, labels, widths=None):
    for i, lab in enumerate(labels, 1):
        c = put(ws, f"{col(i)}{row}", lab, "FFFFFF", bold=True, fill=HEAD_FILL, wrap=True, border=True)
        c.alignment = Alignment(wrap_text=True, vertical="center")
    ws.row_dimensions[row].height = 30
    if widths:
        for i, w in enumerate(widths, 1):
            ws.column_dimensions[col(i)].width = w


def part_no(l):
    if l["ref"] in PN_OVERRIDE:
        return PN_OVERRIDE[l["ref"]]
    if l["mpn"]:
        return f'{l["mfr"]} {l["mpn"]}'.strip()
    src = l["src"]
    first = src.split(" ")[0] if src else ""
    if first in ("Seeed", "Elecrow") and len(src.split(" ")) == 2:
        return src
    if l["vendor"] == "own printer":
        return "printed in-house"
    return "generic (see link)"


def build_sheet(wb, plan, units):
    v = plan["variant"]
    ws = wb.create_sheet(f"{v} Build")
    lines = plan["lines"]
    fee_of = {g: d["fee"] for g, d in plan["by_vendor"].items()}
    # line vendor -> planner group (DigiKey lines share the "Distributor" fee)
    vendors = []
    for l in lines:
        if l["vendor"] not in vendors:
            vendors.append(l["vendor"])

    put(ws, "A1", f"{v} build cost: estimate vs actual", bold=True, size=14)
    put(ws, "A2", f"Estimates from the order planner snapshot of {plan_date}. Yellow cells are yours "
                  "to fill in; everything else is a formula.", GREY, italic=True)

    # --- batch settings + summary (rows 4-12)
    put(ws, "A4", "Batch", bold=True)
    put(ws, "A5", "Passes planned")
    inp(ws, "C5", units, INT, "Units the order quantities are sized for (planner default 5).")
    put(ws, "A6", "Passes completed")
    inp(ws, "C6", units, INT, "Passes that came out working. Cost per pass divides by this.")
    put(ws, "A7", "Spare parts kept ($)")
    inp(ws, "C7", 0, USD, "Value of leftover parts you keep for the next batch; subtracted "
                          "from the batch cost (optional).")

    put(ws, "E4", "Summary", bold=True)
    put(ws, "F4", "Estimate", bold=True, align="right")
    put(ws, "G4", "Actual", bold=True, align="right")
    put(ws, "H4", "Variance", bold=True, align="right")

    first = 16
    last = first + len(lines) - 1
    trow = last + 1                       # parts total row
    ohead = trow + 3                      # orders header
    ofirst, olast = ohead + 1, ohead + len(vendors)
    otot = olast + 1

    summary = [
        ("Parts", f"=J{trow}", f"=M{trow}"),
        ("Shipping in", f"=E{otot}", f"=F{otot}"),
        ("Tariff / duty", f"=H{otot}", f"=I{otot}"),
        ("Brokerage + sales tax", "=0", f"=J{otot}+K{otot}"),
        ("Less spare parts kept", "=0", "=-C7"),
    ]
    r = 5
    for lab, est, act in summary:
        put(ws, f"E{r}", lab)
        put(ws, f"F{r}", est, fmt=USD)
        put(ws, f"G{r}", act, fmt=USD)
        put(ws, f"H{r}", f"=G{r}-F{r}", fmt=USD)
        r += 1
    put(ws, f"E{r}", "Batch total", bold=True)
    put(ws, f"F{r}", f"=SUM(F5:F{r-1})", bold=True, fmt=USD, fill=TOT_FILL)
    put(ws, f"G{r}", f"=SUM(G5:G{r-1})", bold=True, fmt=USD, fill=TOT_FILL)
    put(ws, f"H{r}", f"=G{r}-F{r}", bold=True, fmt=USD, fill=TOT_FILL)
    tot_r = r
    r += 1
    put(ws, f"E{r}", "Cost per pass", bold=True)
    put(ws, f"F{r}", f"=IF($C$5>0,F{tot_r}/$C$5,0)", bold=True, fmt=USD)
    put(ws, f"G{r}", f"=IF($C$6>0,G{tot_r}/$C$6,0)", bold=True, fmt=USD,
        comment="Actual batch total / passes completed.")
    put(ws, f"H{r}", f"=G{r}-F{r}", bold=True, fmt=USD)
    per_r = r
    r += 1
    put(ws, f"E{r}", "Cost basis for Profit")
    put(ws, f"F{r}", f'=IF(M{trow}>0,"actual","estimate (no actuals yet)")', italic=True)
    put(ws, f"G{r}", f"=IF(M{trow}>0,G{per_r},F{per_r})", bold=True, fmt=USD,
        comment="What the Profit sheet uses: the actual cost per pass once any actual "
                "price is entered, the estimate until then.")
    basis_r = r

    # --- parts table
    put(ws, f"A{first-2}", "Parts", bold=True, size=12)
    header(ws, first - 1, ["Ref", "Description", "Vendor", "Part #", "DigiKey #", "Buy link",
                           "Qty / pass", "Qty needed", "Est unit $", "Est ext $",
                           "Actual qty bought", "Actual unit $", "Actual ext $", "Variance $",
                           "Order # / date", "Status", "Note"],
           [7, 46, 13, 24, 13, 12, 9, 9, 10, 11, 10, 10, 11, 10, 16, 12, 44])
    for i, l in enumerate(lines):
        r = first + i
        put(ws, f"A{r}", l["ref"], bold=True, border=True)
        put(ws, f"B{r}", l["desc"], wrap=True, border=True)
        put(ws, f"C{r}", l["vendor"], border=True)
        put(ws, f"D{r}", part_no(l), wrap=True, border=True)
        put(ws, f"E{r}", l["dk"] or None, border=True)
        if l.get("link"):
            c = put(ws, f"F{r}", "open", border=True)
            c.hyperlink = l["link"]
            c.font = font("0563C1", underline="single")
            if l.get("alt"):
                c.comment = Comment(f"Or: {l['alt'][0]}\n{l['alt'][1]}", "SubverseReader")
        else:
            put(ws, f"F{r}", None, border=True)
        put(ws, f"G{r}", l["per"], BLUE, fmt=INT, border=True)
        put(ws, f"H{r}", f"=G{r}*$C$5", fmt=INT, border=True)
        put(ws, f"I{r}", l["unit"], BLUE, fmt=USD, border=True)
        put(ws, f"J{r}", f"=H{r}*I{r}", fmt=USD, border=True)
        inp(ws, f"K{r}", None, INT)
        inp(ws, f"L{r}", None, USD)
        put(ws, f"M{r}", f"=IF(K{r}=\"\",0,K{r}*L{r})", fmt=USD, border=True)
        put(ws, f"N{r}", f"=IF(K{r}=\"\",0,M{r}-J{r})", fmt=USD, border=True)
        inp(ws, f"O{r}")
        inp(ws, f"P{r}", "Not ordered")
        put(ws, f"Q{r}", l["note"] or None, GREY, wrap=True, border=True)
    put(ws, f"A{trow}", "Total", bold=True, fill=TOT_FILL)
    for c_ in "BCDEFGHIKLOPQ":
        ws[f"{c_}{trow}"].fill = TOT_FILL
    for c_ in "JMN":
        put(ws, f"{c_}{trow}", f"=SUM({c_}{first}:{c_}{last})", bold=True, fmt=USD, fill=TOT_FILL)

    dv = DataValidation(type="list", formula1='"Not ordered,Ordered,Received,Short,Returned,In stock"',
                        allow_blank=True)
    ws.add_data_validation(dv)
    dv.add(f"P{first}:P{last}")
    ws.conditional_formatting.add(
        f"N{first}:N{last}", FormulaRule(formula=[f"N{first}>0.005"], font=Font(name=F, color="C00000")))
    ws.conditional_formatting.add(
        f"N{first}:N{last}", FormulaRule(formula=[f"N{first}<-0.005"], font=Font(name=F, color=GREEN)))

    # --- orders table (one row per vendor order)
    put(ws, f"A{ohead-1}", "Orders: shipping, tariff, tax", bold=True, size=12)
    labels = ["Vendor", "Lines", "Est parts $", "Actual parts $", "Est shipping $",
              "Actual shipping $", "Tariff rate (est)", "Est tariff $", "Actual duty $",
              "Brokerage / customs fee $", "Sales tax $", "Order total (actual) $", "Order #",
              "Date ordered", "Date received", "Ships from", "Note"]
    for i, lab in enumerate(labels, 1):
        c = put(ws, f"{col(i)}{ohead}", lab, "FFFFFF", bold=True, fill=HEAD_FILL, wrap=True, border=True)
        c.alignment = Alignment(wrap_text=True, vertical="center")
    ws.row_dimensions[ohead].height = 30
    rng_v = f"$C${first}:$C${last}"
    for i, vk in enumerate(vendors):
        r = ofirst + i
        grp = next(l["group"] for l in lines if l["vendor"] == vk)
        fee = fee_of.get(grp, 0.0)
        put(ws, f"A{r}", vk, bold=True, border=True)
        put(ws, f"B{r}", f'=COUNTIF({rng_v},A{r})', fmt=INT, border=True)
        put(ws, f"C{r}", f"=SUMIFS($J${first}:$J${last},{rng_v},A{r})", fmt=USD, border=True)
        put(ws, f"D{r}", f"=SUMIFS($M${first}:$M${last},{rng_v},A{r})", fmt=USD, border=True)
        put(ws, f"E{r}", fee, BLUE, fmt=USD, border=True,
            comment="Planner estimate for one standard order to the US (order.py VENDORS).")
        inp(ws, f"F{r}", None, USD)
        if vk in CHINA:
            inp(ws, f"G{r}", TARIFF_PLACEHOLDER, PCT,
                "PLACEHOLDER rate for a China-shipped parcel. US duty on Chinese goods changed "
                "several times in 2025 and the de minimis exemption is gone, so the courier "
                "(DHL/FedEx/UPS) bills duty plus a brokerage fee. Replace with the real figure.")
        else:
            inp(ws, f"G{r}", 0, PCT, "Ships from inside the US: no import duty.")
        put(ws, f"H{r}", f"=C{r}*G{r}", fmt=USD, border=True)
        inp(ws, f"I{r}", None, USD)
        inp(ws, f"J{r}", None, USD)
        inp(ws, f"K{r}", None, USD)
        put(ws, f"L{r}", f"=D{r}+SUM(F{r},I{r}:K{r})", fmt=USD, border=True)
        inp(ws, f"M{r}")
        inp(ws, f"N{r}", None, "yyyy-mm-dd")
        inp(ws, f"O{r}", None, "yyyy-mm-dd")
        ships = VENDOR_FROM.get(vk, "")
        put(ws, f"P{r}", ships, GREY, wrap=True, border=True)
        note = ""
        if vk == "Elecrow":
            note = "SR-15 and SR-16 both buy from Elecrow: one order covers both; split its shipping and duty between the sheets."
        elif vk == "own printer":
            note = "Filament at estimate; no shipping."
        put(ws, f"Q{r}", note or None, GREY, wrap=True, border=True)
    put(ws, f"A{otot}", "Total", bold=True, fill=TOT_FILL)
    for c_ in "BCDEFHIJKL":
        put(ws, f"{c_}{otot}", f"=SUM({c_}{ofirst}:{c_}{olast})", bold=True,
            fmt=INT if c_ == "B" else USD, fill=TOT_FILL)
    for c_ in "GMNOPQ":
        ws[f"{c_}{otot}"].fill = TOT_FILL
    put(ws, f"A{otot+1}", f'=IF(ABS(D{otot}-M{trow})>0.005,"Check: order parts total does not match the parts table (a vendor name was changed?)","")',
        "C00000", italic=True)

    ws.freeze_panes = f"C{first}"
    ws.sheet_view.zoomScale = 110
    return dict(sheet=ws.title, per_est=f"F{per_r}", per_act=f"G{per_r}", basis=f"G{basis_r}",
                basis_lab=f"F{basis_r}", done="C6", planned="C5", total_act=f"G{tot_r}",
                total_est=f"F{tot_r}", first=first, last=last)


def profit_sheet(wb, refs, plans):
    ws = wb.create_sheet("Profit")
    put(ws, "A1", "Profit: first batches at cost", bold=True, size=14)
    put(ws, "A2", "These passes go to engineering or user feedback: each one is either kept and "
                  "absorbed, given away and absorbed, or sold at cost. Markup is 0% by default.",
        GREY, italic=True)
    ws.column_dimensions["A"].width = 46
    for c_ in "BCD":
        ws.column_dimensions[c_].width = 15
    ws.column_dimensions["E"].width = 60
    header(ws, 4, ["", "SR-15", "SR-16", "Both", "How it is worked out"])
    for c_ in "ABCDE":
        ws[f"{c_}4"].alignment = Alignment(horizontal="left" if c_ in "AE" else "right", vertical="center")

    r15, r16 = refs
    S15, S16 = r15["sheet"], r16["sheet"]

    # key, label, SR-15, SR-16, sum in Both?, how, kind, fmt. Templates: {X} = this column,
    # {key} = that row's number, resolved once every row is numbered.
    spec = [
        ("sec", "From the build sheets"),
        ("done", "Passes completed", f"='{S15}'!{r15['done']}", f"='{S16}'!{r16['done']}", True,
         "build sheet C6", "link", INT),
        ("basis", "Cost basis", f"='{S15}'!{r15['basis_lab']}", f"='{S16}'!{r16['basis_lab']}", False,
         "actual once any actual price is entered on the build sheet", "link", "@"),
        ("cost", "Cost per pass", f"='{S15}'!{r15['basis']}", f"='{S16}'!{r16['basis']}", False,
         "batch total / passes completed", "link", USD),
        ("spent", "Batch total spent",
         f"=IF(LEFT({{X}}{{basis}},6)=\"actual\",'{S15}'!{r15['total_act']},'{S15}'!{r15['total_est']})",
         f"=IF(LEFT({{X}}{{basis}},6)=\"actual\",'{S16}'!{r16['total_act']},'{S16}'!{r16['total_est']})",
         True, "actual batch total (estimate until actuals exist)", "link", USD),
        ("sec", "Where the passes go (enter counts)"),
        ("eng", "Kept for engineering (absorbed)", 2, 2, True, "stay on the bench; cost absorbed", "in", INT),
        ("gift", "Given to user testers (absorbed)", 1, 1, True,
         "shipped free; cost + outbound shipping absorbed", "in", INT),
        ("sold", "Sold to user testers at cost", 2, 2, True, "tester pays the price per pass below", "in", INT),
        ("left", "Unallocated (still on hand)", "={X}{done}-SUM({X}{eng}:{X}{sold})", None, True,
         "completed minus allocated; negative means over-allocated", "calc", INT),
        ("sec", "Pricing (enter rates)"),
        ("ship", "Outbound shipping per pass ($)", 6.00, 6.00, False,
         "PLACEHOLDER: small padded mailer, US ground; replace with your label cost", "in", USD),
        ("fee", "Payment fee (% of sale)", 0.0, 0.0, False,
         "card / PayPal fee, e.g. 0.029; 0 for cash or invoice", "in", PCT),
        ("mark", "Markup on cost", 0.0, 0.0, False, "0% = at cost; e.g. 0.25 for 25%", "in", PCT),
        ("price", "Price per pass sold",
         "=IF({X}{fee}<1,({X}{cost}+{X}{ship})*(1+{X}{mark})/(1-{X}{fee}),0)", None, False,
         "(cost + shipping) x (1 + markup) / (1 - fee): at 0% markup a sale nets zero", "calc", USD),
        ("sec", "Result"),
        ("rev", "Revenue", "={X}{sold}*{X}{price}", None, True, "passes sold x price", "calc", USD),
        ("fees", "Payment fees", "=-{X}{rev}*{X}{fee}", None, True, "", "calc", USD),
        ("cogs", "Cost of passes sold (incl. shipping)", "=-{X}{sold}*({X}{cost}+{X}{ship})", None, True,
         "", "calc", USD),
        ("profit", "Profit on sales", "={X}{rev}+{X}{fees}+{X}{cogs}", None, True,
         "0 when sold at cost; markup shows up here", "total", USD),
        ("abs_eng", "Absorbed: engineering", "=-{X}{eng}*{X}{cost}", None, True, "", "calc", USD),
        ("abs_gift", "Absorbed: given to testers", "=-{X}{gift}*({X}{cost}+{X}{ship})", None, True,
         "", "calc", USD),
        ("absorbed", "Total absorbed (cost of the program)", "={X}{abs_eng}+{X}{abs_gift}", None, True,
         "what the batch costs you after at-cost sales", "total", USD),
        ("inv", "Held in inventory (unallocated, at cost)", "={X}{left}*{X}{cost}", None, True,
         "", "calc", USD),
        ("cash", "Net cash (negative = out of pocket)", "=-{X}{spent}-({X}{gift}+{X}{sold})*{X}{ship}+{X}{rev}+{X}{fees}", None,
         True, "batch spend + outbound shipping, less what testers paid; = absorbed + inventory + profit",
         "total", USD),
        ("sec", "Later: what a retail price would earn (reference)"),
        ("p100", "Est parts cost per pass at 100 units (BOM)",
         f"=SUMPRODUCT('{S15}'!G{r15['first']}:G{r15['last']},'{S15}'!R{r15['first']}:R{r15['last']})",
         f"=SUMPRODUCT('{S16}'!G{r16['first']}:G{r16['last']},'{S16}'!R{r16['first']}:R{r16['last']})",
         False, "BOM price at 100 x qty per pass (hidden column R on the build sheets); "
                "no shipping, duty or assembly", "link", USD),
        ("retail", "Target retail price", 69.00, 59.00, False, "PLACEHOLDER: your call", "in", USD),
        ("gm", "Gross margin at retail, 100 units", "=IF({X}{retail}>0,({X}{retail}-{X}{p100})/{X}{retail},0)",
         None, False, "(price - parts at 100) / price; before assembly, shipping, fees", "calc", PCT),
    ]
    num, r = {}, 5
    for item in spec:
        if item[0] != "sec":
            num[item[0]] = r
        r += 1

    r = 5
    for item in spec:
        if item[0] == "sec":
            put(ws, f"A{r}", item[1], bold=True, fill=SUB_FILL)
            for c_ in "BCDE":
                ws[f"{c_}{r}"].fill = SUB_FILL
            r += 1
            continue
        key, label, a, b, summed, how, kind, fmt = item
        if b is None:
            b = a
        bold = kind == "total"
        put(ws, f"A{r}", label, bold=bold)
        for c_, val in (("B", a), ("C", b)):
            if isinstance(val, str):
                val = val.replace("{X}", c_).format(**num) if "{" in val else val
            if kind == "in":
                inp(ws, f"{c_}{r}", val, fmt)
            else:
                put(ws, f"{c_}{r}", val, GREEN if kind == "link" else BLACK, bold=bold, fmt=fmt,
                    fill=TOT_FILL if bold else None)
        if summed:
            put(ws, f"D{r}", f"=B{r}+C{r}", bold=bold, fmt=fmt, fill=TOT_FILL if bold else None)
        put(ws, f"E{r}", how, GREY, italic=True, wrap=True)
        r += 1
    rows = num
    ws.freeze_panes = "B5"
    return rows


def readme(wb, plans):
    ws = wb.active
    ws.title = "Read Me"
    ws.column_dimensions["A"].width = 26
    ws.column_dimensions["B"].width = 90
    put(ws, "A1", "SubverseReader build-cost workbook: SR-15 Dial Pass and SR-16 Setlist", bold=True, size=14)
    put(ws, "A2", f"Generated by bom/make_build_sheet.py from the planner snapshot of {plan_date}.", GREY, italic=True)
    info = [
        ("How to use", ""),
        ("1. Order", "Each build sheet lists every line with a buy link, part number and the planner's "
                     "estimate. Order from the links; one row per vendor in the Orders table."),
        ("2. Enter actuals", "Per line: Actual qty bought and Actual unit $ (what the invoice says, "
                             "per piece). Per order: Actual shipping, Actual duty, Brokerage fee, Sales tax."),
        ("3. Count passes", "Set Passes completed (C6) to the number that came out working."),
        ("4. Allocate", "On Profit, split the completed passes between engineering, testers you give "
                        "one to, and testers who buy at cost."),
        ("", ""),
        ("Cell colours", ""),
        ("Yellow, blue text", "Yours to fill in or change."),
        ("Blue text, no fill", "Planner estimates (qty per pass, est unit price, est shipping); edit if they are wrong."),
        ("Green text", "Pulled from another sheet."),
        ("Black text", "Formula; leave it."),
        ("", ""),
        ("Example line", "KEY  Omron B3F-1000  Qty needed 15  Est unit $0.35  ->  Actual qty bought 20 (a 20-pack "
                         "was cheaper), Actual unit $0.29, Order # DK-81234567 2026-11-11, Status Received."),
        ("", ""),
        ("Assumptions", ""),
        ("Shipping", "One standard paid order per vendor to a US address (order.py VENDORS). Free-shipping "
                     "thresholds move."),
        ("Tariff", f"China-shipped orders (Elecrow, Seeed) carry a {TARIFF_PLACEHOLDER:.0%} PLACEHOLDER rate. "
                   "The US ended the de minimis exemption, so couriers bill duty plus a brokerage fee on "
                   "delivery; enter the invoice figures in Actual duty and Brokerage."),
        ("Shared Elecrow order", "SR-15's board and SR-16's panel can ship in one Elecrow order. Enter that "
                                 "order's shipping and duty once, split between the two sheets as you like."),
        ("Cost basis", "Profit uses the estimate until any actual price is entered on that build sheet, then "
                       "the actual. Partly filled sheets understate the cost: finish a sheet before trusting it."),
        ("At cost", "Price per pass = (cost + outbound shipping) x (1 + markup) / (1 - payment fee). "
                    "With 0% markup a sale recovers its cost exactly and profit on sales is zero."),
        ("Outbound shipping, retail price", "Placeholders ($6.00; $69 / $59). Replace them."),
        ("Regenerating", "Running make_build_sheet.py again refuses to overwrite this file without --force, "
                         "which would lose every actual you entered."),
    ]
    r = 4
    for a, b in info:
        if a and not b:
            put(ws, f"A{r}", a, bold=True, fill=SUB_FILL)
            ws[f"B{r}"].fill = SUB_FILL
        else:
            put(ws, f"A{r}", a, bold=bool(a))
            put(ws, f"B{r}", b, wrap=True)
        r += 1
    ws["A11"].fill = INPUT_FILL
    ws["A11"].font = font(BLUE, True)
    ws["A12"].font = font(BLUE, True)
    ws["A13"].font = font(GREEN, True)


def add_hidden_100(ws_refs, plans, wb):
    """Column R: BOM price at 100 units, used by Profit's reference rows."""
    for ref, plan in zip(ws_refs, plans):
        ws = wb[ref["sheet"]]
        put(ws, f"R{ref['first']-1}", "Est unit $ at 100", "FFFFFF", bold=True, fill=HEAD_FILL, wrap=True)
        for i, l in enumerate(plan["lines"]):
            put(ws, f"R{ref['first']+i}", u100[(plan["variant"], l["ref"])], BLUE, fmt=USD)
        ws.column_dimensions["R"].hidden = True


def load_u100():
    import csv
    out = {}
    for v in VARIANTS:
        with open(os.path.join(HERE, f"{v}.csv")) as f:
            for row in csv.DictReader(f):
                out[(v, row["ref"])] = float(row["unit_usd_100"] or 0)
    return out


def vendor_from():
    sys.path.insert(0, HERE)
    import order
    return {k: v[3] for k, v in order.VENDORS.items()}


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--force", action="store_true", help="overwrite an existing workbook")
    ap.add_argument("--out", default=OUT)
    a = ap.parse_args()
    if os.path.exists(a.out) and not a.force:
        sys.exit(f"{a.out} exists and may hold your actuals; pass --force to overwrite it")
    snap = json.load(open(os.path.join(HERE, "order", "order.json")))
    plan_date = snap["date"]
    units = snap["units"]
    plans = [next(p for p in snap["plans"] if p["variant"] == v) for v in VARIANTS]
    VENDOR_FROM = vendor_from()
    u100 = load_u100()
    wb = Workbook()
    readme(wb, plans)
    refs = [build_sheet(wb, p, units) for p in plans]
    add_hidden_100(refs, plans, wb)
    profit_sheet(wb, refs, plans)
    wb.calculation.fullCalcOnLoad = True
    os.makedirs(os.path.dirname(a.out), exist_ok=True)
    wb.save(a.out)
    print(a.out)
