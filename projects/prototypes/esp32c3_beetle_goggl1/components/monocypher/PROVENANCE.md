# Provenance

Monocypher 4.0.2: `src/monocypher.c`, `src/monocypher.h`, and the optional
`src/optional/monocypher-ed25519.c/.h` (standard Ed25519 with SHA-512, which
is what .mnpkg signatures use; the core `crypto_eddsa_*` is the BLAKE2b variant).
Upstream: https://github.com/LoupVaillant/Monocypher — dual-licensed
CC0-1.0 / BSD-2-Clause. Used for Ed25519 signature checks on `.mnpkg`
OTA packages (`crypto_ed25519_check`).
