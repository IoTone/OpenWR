# PCB fabrication, logistics and compliance for 1-5 Pod-0 sets, US and Japan (as of 2026-11-10)

Reading key for every fact: **[READ]** = I read it on the page itself on 2026-11-10; **[SNIPPET]** = it came from a search-result summary of that page and was not opened; **[CALC]** = my arithmetic on READ figures; **unconfirmed** = no usable source found. Board areas used throughout: pod 20 x 45 mm = 1.395 sq in; puck 30 x 40 mm = 1.860 sq in [CALC].

## 1. US-only PCB fab, stencils, assembly (scenario a: US buyer, US sources)

### Takeaway
OSH Park is the only US fab whose price I could confirm: 6 pods + 6 pucks (enough for 4-5 sets) cost $46.50 standard or $93.00 Super Swift, shipped free, in roughly 1-2 weeks. Prices for every other US fab, for OSH Stencils and for US turnkey assembly are unconfirmed (quote tools only), so the US assembled-board cost is an open item.

### Cited Findings
- OSH Park 2-layer Prototype: $5/sq in per set of 3, 9-12 calendar days; 2-layer Super Swift: $10/sq in per set of 3, 4-5 business days. [READ] — [OSH Park services](https://docs.oshpark.com/services/)
- OSH Park 4-layer Prototype (FR408, ENIG): $10/sq in per set of 3, 9-14 calendar days; 4-layer Super Swift: $20/sq in per set of 3, 5-6 business days. [READ] — [OSH Park services](https://docs.oshpark.com/services/)
- OSH Park boards are "shipped for free to anywhere in the world"; no paid shipping tiers are listed on that page. [READ] — [OSH Park services](https://docs.oshpark.com/services/)
- OSH Park Medium Run (4-layer $2/sq in) needs a 100 sq in minimum, so it does not fit this build. [READ] — [OSH Park services](https://docs.oshpark.com/services/)
- CircuitHub advertises assembled prototypes "from 3 days" built in Massachusetts; its quick-turn "Grid Proto" is USA only, quantity 1-10, SMT only, 2 or 4 layers. [SNIPPET] — [CircuitHub prototyping](https://www.circuithub.com/capabilities/prototyping-capabilities)
- MacroFab lists low-volume lead time "as little as 10 days after order received or 3 days after parts received". [SNIPPET] — [MacroFab lead times](https://www.macrofab.com/pcb-assembly/lead-times/)
- Screaming Circuits is described in a 2026 third-party roundup as 1-5 day quick-turn and accepting single-piece orders; no price given. [SNIPPET, secondary] — [onstarry roundup](https://onstarry.com/best-pcb-assembly-services-for-prototypes-in-2026/)
- The only side-by-side MacroFab/CircuitHub prices found are from a circa-2018 forum thread (27-part board: MacroFab $251 for 10, CircuitHub $315 for 10). Stale; not usable for 2026. [SNIPPET] — [EmbDev thread](https://embdev.net/topic/469516)

### Inferences
- OSH Park cost per order [CALC from READ rates]. Sets of 3 only, so 1-3 working sets need 3 of each board, 4-5 sets need 6 of each:

| Working sets wanted | Boards ordered | Pod 4-layer std | Puck 2-layer std | Total std | Total Super Swift |
|---|---|---|---|---|---|
| 1, 2 or 3 | 3 + 3 | $13.95 | $9.30 | $23.25 | $46.50 |
| 4 or 5 | 6 + 6 | $27.90 | $18.60 | $46.50 | $93.00 |

- 3 boards for 3 sets leaves no spare for a hand-assembly failure; ordering 6 + 6 even for 3 sets costs only $23.25 more.
- OSH Park calendar: standard 9-14 calendar days for the 4-layer pod plus domestic mail; Super Swift 5-6 business days plus mail. Ordered by about Nov 13, standard boards should arrive before the end of November, leaving two weeks for assembly before mid-December. The Thanksgiving days (Nov 26-27) fall inside that window; OSH Park's holiday handling is unconfirmed.
- Hand assembly of LGA-14 (BMI270), VQFN-16 (BQ24074) and TDFN-8 (MAX17048) with a stencil and hot plate: I found no source that I read on this in this session, so no cited verdict. It is common hobbyist practice, but that statement is my general knowledge, not a sourced finding. The 30-pin 0.5 mm FPC connector and the ESP32 module's bottom ground pad are the other bridging/rework risks.

### Gaps
- OSH Stencils price per square inch, minimum and lead time: unconfirmed. The site's home page returned no pricing text, and search found only undated third-party remarks.
- Advanced Circuits, Sunstone, Bay Area Circuits, DigiKey DKRed, MacroFab (bare), PCB Unlimited: prices and lead times for these boards unconfirmed (all behind quote tools; not reached).
- US turnkey assembly cost for 5 sets (MacroFab, Screaming Circuits, CircuitHub, Sierra): unconfirmed. Needs live quotes with the real BOM.
- Whether OSH Park has a minimum order charge for boards this small: unconfirmed.

## 2. China fab and JLCPCB PCBA (scenario b: Japan buyer; fallback note for US)

### Takeaway
JLCPCB's assembly fee schedule is confirmed and every checked IC on the Pod-0 BOM is in its library as an Extended part, so turnkey PCBA is feasible, but bare-board prices for these two exact boards and current shipping fees to Japan were not confirmed. The ESP32-C3-MINI-1-N4 is flagged NRND on LCSC.

### Cited Findings
- JLCPCB PCBA fees (page last updated Oct 22, 2026) [READ] — [JLCPCB assembly price](https://jlcpcb.com/help/article/pcb-assembly-price):
  - Economic: setup $8.18, stencil $1.53, extended-part feeder fee $3.07 per part type, SMT $0.0016/joint.
  - Standard: setup $25.56 single-side / $51.12 double-side, stencil $8.21 / $16.42, feeder fee $1.53 per part type (basic and extended), handling fee of at least $14.93 in some cases, fixtures from $16.42 (2 fixtures, 1-29 pcs).
  - Hand-soldering labor $3.58 per order; X-ray $1.64 per piece for 1-10 pcs.
- Economic PCBA assembles one side only; double-sided needs Standard. Economic is 2-50 pcs, Standard 2-80,000 pcs (so minimum assembled quantity is 2). [SNIPPET of JLCPCB pages] — [JLCPCB prototype assembly](https://jlcpcb.com/solutions/pcb-prototype-assembly)
- Parts bought for PCBA go into the buyer's JLCPCB parts library and cannot be shipped separately. [SNIPPET] — [JLCPCB part page](https://jlcpcb.com/partdetail/TexasInstruments-BQ24074RGTR/C54313)
- Library status of BOM parts:

| MPN | LCSC / JLC no. | Class | Stock / price | How known |
|---|---|---|---|---|
| ESP32-C3-MINI-1-N4 | C2838502 | unconfirmed (class not seen) | 15,111 in stock, $3.8274 at qty 1 | [SNIPPET] [LCSC](https://www.lcsc.com/product-detail/C2838502.html); page itself [READ] shows "Not Recommended for New Design", recommends ESP32-C3-MINI-1-H4X |
| BMI270 | C2836813 | Extended | stock conflicting: 8,306 / 2,608 / out of stock on different cached LCSC pages | class [READ] [JLCPCB](https://jlcpcb.com/partdetail/C2836813); stock [SNIPPET] |
| VEML7700-TT | C1850416 | Extended | unconfirmed | [SNIPPET] [JLCPCB](https://jlcpcb.com/partdetail/VishayIntertech-VEML7700TT/C1850416) |
| AP2112K-3.3TRG1 | C51118 | Extended | 7,765 in stock, from $0.0925 (volume tier) | [SNIPPET] [JLCPCB](https://jlcpcb.com/partdetail/AP2112K-3.3TRG1/C51118), [LCSC](https://www.lcsc.com/product-detail/C51118.html) |
| USBLC6-2SC6 | C7519 | Extended | about 28,668, $0.1793 | [SNIPPET] [JLCPCB](https://jlcpcb.com/partdetail/STMicroelectronics-USBLC62SC6/C7519) |
| BQ24074RGTR | C54313 | Extended | unconfirmed | [SNIPPET] [JLCPCB](https://jlcpcb.com/partdetail/TexasInstruments-BQ24074RGTR/C54313) |
| MAX17048G+T10 | C2682616 | Extended | unconfirmed | [SNIPPET] [JLCPCB](https://jlcpcb.com/partdetail/MaximIntegrated-MAX17048GT10/C2682616) |
| SM10B-SRSS-TB(LF)(SN) | C5306083 | Extended | unconfirmed | [SNIPPET] [JLCPCB](https://jlcpcb.com/partdetail/JST-SM10B_SRSS_TB_LF_SN/C5306083) |
| S2B-PH-SM4-TB(LF)(SN) | C295747 | Extended | unconfirmed | [SNIPPET] [JLCPCB](https://jlcpcb.com/partdetail/Jst-S2B_PH_SM4_TB_LF_SN/C295747) |
| TYPE-C-31-M-12 | unconfirmed | unconfirmed | unconfirmed | no result returned |
| FH12-30S-0.5SH(55) | unconfirmed | unconfirmed | unconfirmed | no result returned |

- JLCPCB to Japan, hobbyist reports (dated about 2024-2025, not current price lists): OCS Express about $2.01 and 6-8 days; DHL Express about $14.80 and 1-3 days; OCS NEP from about $0.97 for parcels up to 0.6 kg and 10 x 10 cm; typical order-to-door about 8 days (1-3 build + 5-6 ship). [SNIPPET, secondary] — [lang-ship survey](https://lang-ship.com/blog/work/pcb-color/), [burariweb 2025 guide](https://burariweb.info/electronic-work/jlcpcb-ordering-guide.html), [note.com/mold55](https://note.com/mold55/n/ne5e25f82dd43)
- JLCPCB's shipping help page gives no day counts or fees in text (they are in an image); it says duties and taxes are the buyer's responsibility and not included in shipping. [READ] — [JLCPCB shipping help](https://jlcpcb.com/help/article/shipping-methods-and-delivery-time)
- Third-party 2026 price indications for 4-layer, 5 pcs (100 x 100 mm, not our size): JLCPCB about $32.90; PCBWay's 4-layer page lists $48.00 from 5 pcs. Sources disagree widely ($2 promo to $32.90 for JLCPCB). [SNIPPET, secondary] — [Informic comparison](https://electroniccomponent.com/jlcpcb-vs-pcbway-vs-nextpcb-pcba-comparison-2026/), [PCBWay 4-layer page](https://www.pcbway.com/pcb_prototype/4_Layer_pcb/100x100mm.html)

### Inferences
- JLCPCB assembly fees only, excluding boards, parts, shipping and tax [CALC from READ fee table; part-type counts are my assumption from the brief]:
  - Pod, single-sided, Economic, about 7 extended part types: 8.18 + 1.53 + 7 x 3.07 = $31.20 plus joints (well under $1).
  - Puck, single-sided, Economic, about 5 extended part types: 8.18 + 1.53 + 5 x 3.07 = $25.06 plus joints.
  - If the pod needs parts on both sides: Standard double-side 51.12 + 16.42 + 7 x 1.53 = $78.25 plus fixtures (from $16.42) and possible $14.93 handling.
  - These fees are per order, not per board, so 2 sets and 5 sets cost nearly the same in fees; the difference is parts.
- Because assembly minimum is 2 and bare boards are normally sold in 5s, the natural JLCPCB orders are "5 boards, 2 assembled" or "5 boards, 5 assembled".
- The N4 module's NRND flag is worth a design note: the H4X variant is Espressif's suggested replacement, and its certifications must be checked separately (see section 7).

### Gaps
- Exact JLCPCB / PCBWay / Elecrow / Seeed Fusion price for 5 pcs of a 20 x 45 mm 4-layer and a 30 x 40 mm 2-layer board, and build days: unconfirmed (quote calculators not reachable by fetch).
- Whether JLCPCB Economic PCBA accepts 4-layer boards of this size and thickness: unconfirmed.
- Current (Nov 2026) shipping fees and days to Japan for OCS, DHL, SF, Yun Express, Japan Post: unconfirmed; only older blog figures found.
- Typical all-in total for 5 assembled sets: unconfirmed; depends on parts prices not established here.
- PCBWay, Elecrow, Seeed assembly terms: not researched.

## 3. Japan domestic fab

### Takeaway
Japanese domestic fabs have confirmed-ish lead times of about 5-9 business days but no confirmed prices for these boards; P-ban raised prices about 15% on Sept 1, 2026.

### Cited Findings
- P-ban.com: manufacturing prices rise about 15% for quotes saved on or after Sept 1, 2026; standard lead time changed from 4 to 5 days on May 26, 2026. [SNIPPET] — [P-ban news](https://www.p-ban.com/information/news_bk.html)
- P-ban.com lead-time table: 4-layer Normal 6 business days, Quick 4, Super Quick 3, Ultra Quick 2. [SNIPPET; conflicts slightly with the "5 days" notice above, which may refer to 2-layer] — [P-ban shipping](https://www.p-ban.com/information/shipping.html)
- Unicraft: 4-layer standard lead time 5-9 business days, shortening available. [SNIPPET] — [Unicraft PCB](https://unicraft-jp.com/pcb/)

### Inferences
- Domestic lead time is not shorter than JLCPCB + OCS (about 8 days door to door in blog reports), so the domestic route buys freedom from customs and holiday risk rather than speed.

### Gaps
- Prices at P-ban, Unicraft, PCBGOGO Japan, Elecrow Japan, Switch Science PCB for 5 pcs: unconfirmed (p-ban.com/price/ returned 404; others not reached).

## 4. US import costs for China-made PCBs, November 2026

### Takeaway
De minimis is suspended, so every China parcel is dutiable. On bare 2- and 4-layer rigid FR-4 boards the statutory additional duty is reported as 12.5% while a Section 301 exclusion holds (one source says it was extended on Nov 9, 2026 to Jan 9, 2027), and 37.5% otherwise, including for assembled boards. JLCPCB pre-collects a flat 35% on PCB and SMT orders under mandatory DDP for individuals.

### Cited Findings
- HTS 8534.00.00 (printed circuits) is on the Section 301 China list under heading 9903.88.03; the USITC reference list (last updated July 28, 2026) also notes a separate set of Section 301 duties tied to forced-labor investigations, in U.S. note 52 to subchapter III of chapter 99. [READ, PDF text] — [USITC China Tariffs list](https://hts.usitc.gov/reststop/file?release=currentRelease&filename=China+Tariffs)
- USTR notice dated July 23, 2026 imposes 12.5% tariffs on China-origin products, except goods in its annexes. [SNIPPET] — [USTR FRN](https://ustr.gov/sites/default/files/files/Press/Releases/2026/FLIP%20301%20Investigation%20Final%20Action%20FRN%207-23-26%20FINAL.pdf)
- AtlasPCB (fabricator blog, Aug 21, 2026): base layer 12.5% (Section 301 forced-labor tariff that replaced the Section 122 surcharge on July 24, 2026) plus 25% Section 301 on HTS 8534.00 = 37.5%; qualifying 2- and 4-layer rigid glass-reinforced boards are excluded from the 25% under 9903.88.69, so 12.5% total. The exclusion does not cover boards over 4 layers, flex, rigid-flex, or assemblies with mounted components. It was due to expire Nov 9, 2026 unless extended. [READ, secondary] — [AtlasPCB](https://www.atlaspcb.com/news/news-pcb-tariff-section-301-exclusion-expiry-november-2026/)
- MCL (PCB supplier blog, "last updated November 10th, 2026"): on Nov 9, 2026 the Section 301 exclusions were extended through Jan 9, 2027; rates unchanged at 12.5% for 2/4-layer FR-4 and 37.5% for other PCB types. [READ, secondary, single source for the extension] — [MCL tariff update](https://www.mclpcb.com/blog/important-tariff-update/)
- JLCPCB US tariff FAQ (last updated Oct 29, 2026) [READ] — [JLCPCB tariff FAQ](https://jlcpcb.com/help/article/us-tariff-policy-faq):
  - Fentanyl-related and reciprocal tariffs went to 0 effective Feb 24, 2026; a 10% provisional tariff ran Feb 24 to Jul 24, 2026; a 12.5% tariff took effect Jul 24, 2026 (JLCPCB labels it "Section 122" and says it applies 10%); Section 301 is 25%; regular duty 0-7.5%.
  - Orders under $800: individuals must ship DDP; companies may use CPT (needs EIN).
  - For orders pre-collected in the 35%-90% range JLCPCB neither refunds nor bills the difference from actual duty.
  - No brokerage or handling fee amount is stated.
- JLCPCB DDP pre-collection rates from March 17, 2026: PCB 35%, SMT 35%, stencil 90%. [SNIPPET of JLCPCB notice; the rate table is an image] — [JLCPCB tariff notice](https://jlcpcb.com/news/logistics-update-us-china-tariff)
- De minimis ($800) has been suspended for China/Hong Kong since May 2, 2025 and for all countries since Aug 29, 2025. [SNIPPET, secondary] — [Avalara](https://www.avalara.com/us/en/blog/de-minimis-exemption-changes-coming.html), [importcostcalculator](https://www.importcostcalculator.com/de-minimis-800-rule.html)
- IEEPA tariffs were struck down by the Supreme Court and China's 20% IEEPA rate removed Feb 20, 2026; the 10% Section 122 surcharge expired July 24, 2026. [SNIPPET, secondary guides; consistent with JLCPCB's dates] — [gingercontrol guide](https://gingercontrol.com/blog/importing-from-china-tariff-guide), [tariffstool](https://www.tariffstool.com/tariffs-from-china)
- A statutory repeal of de minimis is said to take effect July 1, 2027. [SNIPPET, secondhand, unconfirmed against a primary source] — [smartasset](https://smartasset.com/taxes/de-minimis-trump)
- Anecdote: a forum user reported about $40 of brokerage folded into JLCPCB shipping. [SNIPPET, anecdotal] — [EEVblog thread](https://www.eevblog.com/forum/manufacture/jlc-tarriffs-etc-to-us/)

### Inferences
- Worked example, $60 China PCB order to a US individual [CALC]:
  - Via JLCPCB DDP: 35% x $60 = $21.00 pre-collected at checkout, no later true-up. Landed = $60 + $21 + shipping (shipping unconfirmed).
  - Statutory duty if the 2/4-layer exclusion applies: 12.5% x $60 = $7.50. If it does not (assembled boards, or exclusion lapsed): 37.5% x $60 = $22.50. Regular duty on 8534 and any carrier entry/MPF fees are unconfirmed and would add to this.
  - So JLCPCB's flat 35% over-collects by about $13.50 on bare 2/4-layer boards relative to the reported statutory rate, and is about right for assembled boards.
- Naming conflict: JLCPCB calls the 12.5% layer "Section 122"; USTR, USITC and AtlasPCB describe it as a Section 301 (forced-labor) action. The USITC reference to note 52 supports the Section 301 reading. The rate (12.5%) and date (July 24, 2026) agree across sources.
- This section is only a fallback for the US scenario, since scenario (a) is US sources only.

### Gaps
- The USTR notice extending 9903.88.69 to Jan 9, 2027: not read; rests on one supplier blog.
- Whether HTS 8534 or PCBA classifications sit in the 12.5% action's exempt annexes: unconfirmed.
- PCBWay's US tariff handling and DDP fees: not researched.
- Carrier fees on small China parcels (entry fee, MPF, disbursement fee): unconfirmed.

## 5. Japan import costs

### Takeaway
Goods with a customs value of 10,000 yen or less enter Japan free of duty and consumption tax, and that rule is unchanged for 2026; the announced reform takes effect April 1, 2028. The 60% valuation that stretches the threshold to about 16,666 yen applies only to personal-use imports.

### Cited Findings
- Goods with taxable value of 10,000 yen or less are generally exempt from customs duty and consumption tax (alcohol, tobacco and some listed items excepted). Personal-use imports are valued at 60% of the overseas price; other goods at price plus shipping and insurance. Simplified tariff "other goods" rate is 5% for consignments of 200,000 yen or less. The page lists no scheduled change. [READ; page text partly garbled] — [Japan Customs simplified rates](https://www.customs.go.jp/tsukan/kanizeiritsu.htm)
- Reform: consumption tax will be charged on cross-border mail-order goods of 10,000 yen or less (seller/platform liable), applying from April 1, 2028 (Reiwa 10). [SNIPPET of MOF documents] — [MOF FY2026 tax reform brochure](https://www.mof.go.jp/tax_policy/publication/brochure/zeisei2026_pdf/zeisei26_03.pdf), [MOF explanation](https://www.mof.go.jp/tax_policy/tax_reform/outline/fy2026/explanation/PDF/p0823-0860.pdf)
- The 60% personal-use valuation is to be abolished, also effective April 1, 2028; the article's claim that the law is already amended may be premature (it cites a bill). [READ, secondary, dated Apr 7, 2026] — [THE CKB](https://www.theckb.com/archive/how-much-is-the-import-duty-for-taobao-purchases/)
- DHL Japan advance-payment fee (page effective Jan 1, 2026): for a recipient without a DHL account, 2,200 yen per waybill or 2% of duties and taxes, whichever is higher; for account-holding importers 2,530 yen or 2%. [SNIPPET of DHL page; whether still current in Nov 2026 unconfirmed] — [DHL Japan customs services](https://mydhl.express.dhl/jp/ja/ship/customs-services.html)
- DHL Express announced a rate revision from Jan 1, 2027. [SNIPPET] — [ecnomikata](https://ecnomikata.com/ecnews/51785/)
- DigiKey Japan: free shipping at 6,000 yen or more, otherwise 2,000 yen; usually delivered within 3-4 days; DDP (DigiKey pays duties) or CPT. [SNIPPET of digikey.jp pages] — [DigiKey Japan](https://www.digikey.jp/ja/maker/profiles/3caaacd00406450db1fcb40633312a8e)
- Mouser Japan: yen prices are DDP with consumption tax added at order; free shipping on most orders over 6,000 yen. [SNIPPET] — [Mouser Japan](https://www.mouser.jp/en/manufacturer/fujitsu/)

### Inferences
- At 158.29 yen/USD, 10,000 yen is about $63 of customs value; under the personal-use 60% rule, about $105 of purchase price [CALC]. A JLCPCB bare-board order for both boards should fall under it; a 5-set PCBA order with parts may not.
- Above the threshold, expect 10% consumption tax on the customs value. If it arrives by DHL with tax advanced, the 2,200 yen fee can exceed the tax itself on a small order, which argues for OCS or postal routes, or for keeping each consignment under the threshold. (Splitting one order into several parcels does not help: customs totals them. [READ, same customs page])

### Gaps
- Japan's duty rate on bare PCBs (HS 8534) and on assembled boards: unconfirmed in this session.
- FedEx Japan advance-payment fee: unconfirmed.
- How OCS handles tax collection on JLCPCB parcels to Japan: unconfirmed.
- Whether the 60% rule can be claimed by an unincorporated team buying prototypes: unconfirmed; it is stated for personal-use goods only.

## 6. Lithium battery logistics and Japan PSE

### Takeaway
Whether a 500 mAh 3.7 V pouch cell falls under PSE depends on its exact dimensions: the threshold is 400 Wh/L per cell, which for 1.85 Wh means any cell smaller than about 4.6 cm3 is in scope. Loose cells ship as UN3480 with air restrictions; the simplest path is to buy cells domestically in each country.

### Cited Findings
- Electrical Appliance and Material Safety Act covers lithium-ion batteries only where a single cell's volumetric energy density is 400 Wh/L or more; automotive, moped, medical and industrial-equipment uses are excluded, as are cells fixed in equipment by soldering or similar so they cannot easily be removed. [SNIPPET of METI documents] — [METI PSE overview](https://www.meti.go.jp/policy/consumer/seian/denan/file/PSE_gaiyo.pdf), [METI ordinance text](https://www.meti.go.jp/policy/consumer/seian/denan/topics/mlb/lawtext_of_lib.pdf)
- The system is self-declaration: the manufacturer or importer files a business notification, ensures technical conformity and applies the PSE mark at sale; it is not government certification. [SNIPPET] — [METI PSE overview](https://www.meti.go.jp/policy/consumer/seian/denan/file/PSE_gaiyo.pdf), [METI no-action letter reply](https://www.meti.go.jp/policy/no_action_letter/downloadfiles/120824_nigataseiki_kaito.pdf)
- Energy density = rated capacity x rated voltage / volume. The responsible party is the Japan-side importer. [SNIPPET, secondary] — [pse-mark.jp](https://pse-mark.jp/lithiumionbattery-pse/), [METI mobile battery FAQ](https://www.meti.go.jp/policy/consumer/seian/denan/mlb_faq.html) (FAQ page returned 403 when fetched)
- Standalone lithium-ion batteries (UN3480) are barred from passenger aircraft and must be at or below 30% state of charge by air; from Jan 1, 2026 the 30% limit also applies to batteries packed with equipment. [SNIPPET, secondary] — [triplicatedocs](https://triplicatedocs.com/trade-notes/lithium-battery-shipping/), [shipmercury](https://www.shipmercury.com/blog/shipping-lithium-ion-batteries-un3480-un3481)
- USPS lists UN3480 lithium-ion (including polymer) as restricted mail; standalone batteries go by surface only. [SNIPPET] — [USPS Publication 52](https://pe.usps.com/cpim/ftp/pubs/pub52/pub52.pdf)

### Inferences
- [CALC] 3.7 V x 0.5 Ah = 1.85 Wh. 1.85 Wh / 400 Wh/L = 4.625 cm3. A 502535 cell (5 x 25 x 35 mm = 4.375 cm3) works out to about 423 Wh/L, above the threshold; a 503035 cell (5.25 cm3) to about 352 Wh/L, below it. So common 500 mAh cells straddle the line and the datasheet dimensions decide. Lithium-ion cells in scope carry the round PSE mark (non-specified product) per the METI summary material; I did not read the mark category on a primary page.
- The simplest compliant route for Japan is a cell already sold in Japan with a PSE mark by a domestic importer, which also avoids UN3480 air freight.

### Gaps
- Whether importing a handful of cells for the team's own prototype use (not for sale) triggers the importer notification: unconfirmed.
- Specific policies of JLCPCB, LCSC and AliExpress on shipping loose LiPo cells to Japan or the US: unconfirmed.
- Japan domestic carrier rules for loose cells: unconfirmed.

## 7. Radio compliance for prototypes

### Takeaway
Japan's 180-day experimental notification does cover 2.4 GHz Wi-Fi and Bluetooth, but it is for equipment that already meets an equivalent foreign standard; the cleaner basis for Pod-0 is the module's own Japanese certification, whose number I could not confirm. In the US, pre-authorization operation for development is permitted under 47 CFR 2.805.

### Cited Findings
- Japan 180-day rule: radio equipment used for experiments that is not conformity-marked may be used for up to 180 days from the date of notification to the Minister, provided it conforms to technical standards equivalent to those in the Radio Act (Radio Act art. 4-2 paras 2 and 3). In effect since 2019. [READ, MIC PDF] — [MIC document](https://www.soumu.go.jp/main_content/000973381.pdf)
- Eligible equipment includes 2.4 GHz wireless LAN and Bluetooth (2.4 GHz); 5 GHz and 6 GHz WLAN with conditions. [READ, same PDF] — [MIC document](https://www.soumu.go.jp/main_content/000973381.pdf)
- Individuals and corporations can both file, with no fee; online filing since May 27, 2020. [SNIPPET] — [Keitai Watch](https://k-tai.watch.impress.co.jp/docs/news/1255144.html), [denpa-shinsei.jp](https://denpa-shinsei.jp/giteki-mitokutoku-tokurei/)
- Commentary says the scheme targets equipment with foreign approvals (FCC ID, CE) and that self-built equipment is outside it. [SNIPPET, secondary/private sites; not confirmed in MIC text] — [Internet Watch](https://internet.watch.impress.co.jp/docs/news/1219638.html), [kinopy.info](https://kinopy.info/diy/?p=11757)
- MIC's March 19, 2026 material frames the 180-day rule as a further exception and says long experiments should use an experimental station licence or proper certification. [SNIPPET] — [Cabinet Office WG paper](https://www8.cao.go.jp/kisei-kaikaku/kisei/meeting/wg/2501_04startup/260319/startup12_01.pdf)
- Espressif Japanese certificates found for sibling modules: ESP32-C3-WROOM-02 is 201-220555; ESP32-C3-MINI-1U is listed by an aggregator as 201-230074. No number was found for ESP32-C3-MINI-1. [SNIPPET] — [Espressif WROOM-02 certificate](https://documentation.espressif.com/ESP32-C3-WROOM-02%20MIC%20Certification.pdf), [device.report](https://device.report/espressif/esp32-c3-mini-1u)
- US: 47 CFR 2.805 permits operating a device before authorization for evaluating performance during development, design or pre-production; 2.803 bars marketing before authorization and limits offers of pre-production devices to business/industrial/scientific users. [SNIPPET of eCFR text] — [eCFR 2.803](https://www.ecfr.gov/current/title-47/chapter-I/subchapter-A/part-2/subpart-I/section-2.803), [eCFR subpart I](https://www.ecfr.gov/current/title-47/chapter-I/subchapter-A/part-2/subpart-I)

### Inferences
- If the ESP32-C3-MINI-1 carries a Japanese construction-type certification and is used unmodified with its on-module antenna, the notification route is likely unnecessary; that depends on confirming the certificate, which is a gap.

### Gaps
- MIC certification number of ESP32-C3-MINI-1 (and of the H4X variant): unconfirmed. Check Espressif's certificates page and the MIC search at tele.soumu.go.jp/giteki.
- A primary MIC statement on whether a host device built around a certified module may be operated without further certification: not read (MIC page returned 403).
- FCC modular approval rule (47 CFR 15.212) and the module's FCC ID: not confirmed in this session.

## 8. Exchange rates

### Takeaway
Use 158.29 yen and 6.676 yuan per US dollar (Nov 9, 2026).

### Cited Findings
- USD/JPY 158.286093 and USD/CNY 6.675871, table dated Nov 09, 2026 17:00 UTC. [READ] — [x-rates historical](https://www.x-rates.com/historical/?from=USD&amount=1&date=2026-11-09)
- USD/JPY 158.71 for Nov 10, 2026. [READ] — [Trading Economics](https://tradingeconomics.com/japan/currency)

### Inferences
- The two sources agree to within 0.3%.

### Gaps
- Federal Reserve H.10 for November 2026 was not available in search; latest H.10 data seen was early August 2026.

## 9. Door-to-door times and holiday cut-offs

### Takeaway
Only DigiKey to Japan (3-4 days) and blog-reported JLCPCB to Japan (about 8 days from order) have any support; China's official Spring Festival holiday is Feb 5-13, 2027, well after the mid-December target.

### Cited Findings
- China's 2027 Spring Festival public holiday: Feb 5 (Fri) to Feb 13 (Sat), make-up workdays Jan 31 and Feb 14. [SNIPPET of official notice republished Nov 9, 2026] — [Beijing government](https://english.beijing.gov.cn/latest/news/202611/t20261109_4937887.html)
- JLCPCB's 2026 pattern: factory production break Feb 16-19, 2026; no 2027 notice published yet. [SNIPPET] — [JLCPCB Spring Festival 2026](https://jlcpcb.com/news/service-availability-during-the-annual-spring-festival)
- DigiKey to Japan: usually 3-4 days. [SNIPPET] — [DigiKey Japan](https://www.digikey.jp/ja/maker/profiles/3caaacd00406450db1fcb40633312a8e)
- JLCPCB to Japan: about 6-8 days by OCS after 1-3 build days in blog reports from 2024-2025. [SNIPPET, secondary] — [burariweb guide](https://burariweb.info/electronic-work/jlcpcb-ordering-guide.html)
- JLCPCB says tariff bills from carriers arrive 2-8 weeks after US delivery, and Air Mail can sit 5-7 working days in customs without tracking updates. [READ] — [JLCPCB tariff FAQ](https://jlcpcb.com/help/article/us-tariff-policy-faq), [JLCPCB shipping help](https://jlcpcb.com/help/article/shipping-methods-and-delivery-time)

### Inferences
- For a mid-December finish, the Japanese year-end break (Dec 29 - Jan 3) and China's Spring Festival do not bind; the real constraints are ordering boards in the week of Nov 10-16 and US Thanksgiving (Nov 26-27) slowing US carriers and fabs.

### Gaps
- Unconfirmed, not researched to a citable level: DigiKey US to US, Mouser to Japan days, Adafruit to US, AliExpress standard to US and Japan, LCSC to US and Japan, JLCPCB to US days in November 2026.
- Vendor-specific Thanksgiving and year-end cut-off notices for 2026: none found.
- Shipping terms for SparkFun, Edmund Optics, Thorlabs, Amazon, Seeed, Switch Science, Akizuki, Marutsu: not researched; entered as unconfirmed in the JSON.
