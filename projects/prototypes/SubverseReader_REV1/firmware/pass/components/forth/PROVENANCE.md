# Provenance

Vendored copy of `components/forth/` from ESPIDFORTH.

- Upstream: https://github.com/IoTone/ESPIDFORTH
- Revision: 8651fcc (v0.5.0), copied 2026-09-30
- License: Apache-2.0 (see upstream LICENSE)

Copied verbatim, per the "Option 1: Copy the Component" integration path in
the upstream README. To update, re-copy the six files from upstream and bump
the revision above. Nothing in this directory is modified locally.

## Local change

`idf_component.yml` lists `esp32c5` as a target here (SR-16 on a XIAO ESP32-C5); upstream stops at the C6.
