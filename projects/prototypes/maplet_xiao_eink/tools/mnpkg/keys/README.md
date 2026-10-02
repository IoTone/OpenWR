# OTA release keys (DEV)

`dev_release_ed25519.pem` (key_id `f0846c794440d0ea`) and `dev_release_p256.pem`
are the MagNET bench keys, copied from ProjectMagNET so goggl1 packages verify
against the same key store the mesh nodes use. They are for the bench only:
`mnpkg.py build` refuses them unless the variant starts with `dev`, and the
firmware prints a warning at boot while it carries them. Replace both before
anything leaves the bench (see OTA-PACKAGE.md §6 in ProjectMagNET).
