# SubverseReader 3D review

`index.html` is the team review page (published as the SubverseReader REV1 artifact). It builds all fourteen
variants in three.js (0.147.0 from jsDelivr) from the same layout numbers as `../make_renders.py`, in
millimetres, and draws each screen at the panel's native pixels and colour depth (1-bit, 16 grey, 4-colour).

`viewer.js` is the same script as the inline one in `index.html`, kept separate for diffs. Edit
`viewer.js`, then paste it into the `<script>` block at the end of `index.html`.

To open locally, serve the folder (`python3 -m http.server`) and open `index.html`; it needs network for
three.js and Google Fonts. Deep links: `index.html#sr-1` … `#sr-9`.

Labels mark the USB-C port, microSD slot, lanyard slot, bezel seam, keys/stick/wheel, pockets, clip
and front light. Toggles: labels, lanyard, front light (SR-6), ID-1 card for scale, spin.
