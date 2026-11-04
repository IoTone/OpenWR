# Provenance

Vendored subset of Alpha Fairy, the reference for the ESPIDFORTH port in
`../../src/`.

- Upstream: https://github.com/frank26080115/alpha-fairy
- Revision: fd1f2989f952a77d01cdb23b788058c39f091c89 (main), copied 2026-11-03
- License: MIT, Copyright (c) Frank Zhao (see `LICENSE`)

Kept: `LICENSE`, the top-level docs, `doc/*.md`, `platformio.ini`,
`arduino_workspace/AlphaFairy/` (sources only), and the libraries
`SonyHttpCamera`, `PtpIpCamera`, `AlphaFairy_NetMgr`, `DebuggingSerial`.

Left out (about 135 MB): images and screen captures, `ext_docs/` PDFs, and the
third-party Arduino libraries it bundles (M5StickC-Plus, ESPAsyncWebServer,
AsyncTCP, Lepton, Wire, M5DisplayExt, FairyKeyboard, AlphaFairyImu).

Nothing here is modified or built. Ported code lives in `../../src/` and
names the file it came from.
