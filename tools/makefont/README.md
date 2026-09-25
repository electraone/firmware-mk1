# The faces

`makefont.py` builds every face the firmware can draw and the tables that
address them. Both come out of one run, because they are the one pair that has
to agree exactly: an offset out by a glyph draws text made of the wrong
letters, and nothing in the firmware could notice.

```bash
python3 tools/makefont/makefont.py
python3 tools/makefont/makefont.py --preview /tmp/specimen.png
```

Writes `src/Graphics/Fonts/Fonts.h` and `Fonts.inc`. There is no asset file and
nothing to upload: flashing the firmware is the whole of it.

`--preview` writes a specimen page and the atlas it is cut from, composited
and blitted on the host the same way the unit will do it, out of the same
packed coverage that is about to ship. It is the cheapest way to find a face
that is packed wrong, and it has found one.

## What is in it

| family | faces | typeface | drawn by |
|---|---|---|---|
| system | 2 | Arial Bold | the firmware's own windows |
| user | 16 | Open Sans, Helvetica, Arial Bold, Menlo | a preset's controls |

The user faces are four sizes, two weights and two spacings, which is the
mk2's matrix and the mk2's typefaces - Open Sans for the label sizes, Menlo
for the monospaced ones, the grotesques for the rest. Monospaced is a face of
its own rather than a wider pen; `MONOSPACED` in the script says why.

The system faces carry three colour schemes and the user faces six, because a
plain BTE copy carries whatever colour is in the pixels: a colour a face can
be drawn in is a band of its own. Six is the palette a preset can choose from,
and text asked for in anything else is drawn in the nearest of them.

## What it costs

Nothing like what the bands would cost if they were carried. A glyph's shape
does not change with its colour, so what ships is the shape alone - four bits
a pixel of coverage, 184 KB - and the 4.1 MB of coloured bands is composited
into the display's memory when the unit starts, at about a second and a half
of SPI.

Four bits and not eight: sixteen levels of coverage is past the point where
another one changes a pixel on this panel, and the half thrown away is a third
of the firmware image.

The atlas is packed one face at a time, so each face owns a contiguous run of
rows. Packing all of them together wastes about 190 rows fewer and costs the
unit far more: the compositor builds the atlas a row at a time and can only
skip a face it knows cannot reach that row.
