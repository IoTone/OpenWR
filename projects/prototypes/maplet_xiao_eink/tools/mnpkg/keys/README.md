# OTA release keys (DEV)

`dev_release_ed25519.pem` (key_id `f0846c794440d0ea`) and `dev_release_p256.pem`
are the MagNET bench keys, copied from ProjectMagNET so goggl1 packages verify
against the same key store the mesh nodes use. They are for the bench only:
`mnpkg.py build` refuses them unless the variant starts with `dev`, and the
firmware prints a warning at boot while it carries them. Replace both before
anything leaves the bench (see OTA-PACKAGE.md §6 in ProjectMagNET).

## Release keys (2026-10-02)

The real release keys live outside the repo in `~/.hanasu/release-keys/`
(private halves mode 0600). Firmware carries only the public halves, generated
into `src/mnpkg_keys.h` with:

    mnpkg.py c-keys --release ~/.hanasu/release-keys/ota_release.pub.pem \
                              ~/.hanasu/release-keys/ota_release_next.pub.pem

| key | id | role |
|---|---|---|
| ota_release | 1500c14420e0910a | signs every release package |
| ota_release_next | bd0d667bf74d1d52 | trusted now; signs after ota_release is retired |

Release firmware does not trust the DEV key above. A bench build made with
`-DMNPKG_TRUST_DEV_KEY` adds it back as key2. Sign a release package with:

    mnpkg.py build --key ~/.hanasu/release-keys/ota_release.pem ...
