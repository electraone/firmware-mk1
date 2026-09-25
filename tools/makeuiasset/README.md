# The UI asset

`makeuiasset.py` builds the strip the firmware draws its text from, and the
tables that address it. Both are generated together, because they are the one
pair that has to agree exactly: an offset out by a glyph draws text made of
the wrong letters, and nothing in the firmware could notice.

```bash
python3 tools/makeuiasset/makeuiasset.py
python3 tools/makeuiasset/upload.py assets/ui-1.0.0.bmp
```

Writes `assets/ui-1.0.0.bmp` and `src/Graphics/Fonts/UiAsset.inc`. Flash the
firmware before sending the asset: the unit writes it under the name the
running firmware asks for, and the tables have to be the ones built from the
same run. A reboot loads it.

The two go together and neither is any use without the other, which is why
they are generated in one pass. A table out of step with the pixels draws text
made of the wrong letters and nothing in the firmware can tell.

## What is in it

Two families, as on the mk2:

| family | typeface | drawn by |
|---|---|---|
| system | Helvetica, Arial Bold | the firmware's own windows |
| user | Open Sans | a preset's controls |

The system faces carry three variants, the user faces one per preset colour -
six - because a plain BTE copy carries whatever colour is in the pixels, so a
colour a face can be drawn in is a band of its own.

## Why it is packed

The strip this replaces stored every glyph as a full cell in a band as wide as
the panel, which carried two kinds of nothing: the right hand half of every
row, since a face only reaches about 55% of the way across, and the space
above the ascenders and below the baseline of every cell, since a 13 px cell
holds about 8 px of ink.

Cutting each glyph to its ink box and shelving the boxes across the strip
takes **768 KB to 286 KB**, in the file and in video memory both, with no
compression to undo at the other end. Blitting the box rather than the cell is
also about 10 us a glyph faster, because a blit costs by the pixel.

Deflating the file would take it under 100 KB on top of that, at the cost of an
inflate in the firmware and a loader that no longer streams rows straight
through. It is not done, because 286 KB is already small enough to not be worth
the decompressor.

## What the tables say

Per face, shared by all of its colours: `Advance` is how far the pen moves,
`Width` and `Height` size the blit, `Left` and `Top` place the ink box back
inside the character cell. Per face and colour: `X` and `Y` locate the box in
the strip.

`Advance` is the pen advance and not the right edge of the ink - a glyph has
bearings either side of it, and the two are not the same number. Taking the
ink extent instead draws every character partly on top of the one before, and
is worth knowing about because it looks like a packing fault rather than a
measurement one.

## Adding a face

Add it to `build_faces()`. The packing, the tables and what it costs are all
worked out from that list, and the tool refuses rather than overflowing if the
faces no longer fit in the strip's 600 rows.
