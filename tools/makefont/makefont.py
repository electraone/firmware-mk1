#!/usr/bin/env python3
"""Builds the faces the firmware draws its text with.

The unit draws a character by copying it out of an atlas held in the display
controller's memory: one BTE copy per glyph, the glyph already coloured and
already anti-aliased. A blit costs about 42 us of setting registers plus
0.09 us a pixel, which is why the text engine looks the way it does - the mk2's
layered approach, three blits a glyph, was measured here at three times the
cost and rejected.

Colour has to be in the pixels because the chip cannot put it there on the way
past: its raster operations are boolean, so they can mask a glyph but not
modulate one, and its one blending register takes a single value for a whole
blit rather than a value per pixel. So a face is stored once per colour it can
be drawn in.

Once per colour in video memory, that is - not in flash. A glyph's shape does
not change with its colour, so what ships is the shape alone: one byte a pixel
saying how much ink covers it. The coloured bands are composited from that on
the unit when it starts. Six preset colours therefore cost one copy of each
letter rather than six.

The faces are the mk2's, so the two products read as one:

  system  what the firmware's own windows are written in. Arial Bold at two
          sizes, in the three fixed colour schemes the system UI uses.
  user    what a preset's controls are written in. Four sizes, two weights and
          two spacings - sixteen faces - each in the six colours a preset can
          choose from.

Monospaced is a face of its own rather than a wider pen. Advancing a
proportional face by its widest glyph is what a W or an @ costs, nearly twice a
digit, and a value readout drawn that way reads as loose single characters. The
monospaced faces are Menlo, whose cell lands within a pixel of what the
proportional digits already advance by.

    tools/makefont/makefont.py

Writes src/Graphics/Fonts/Fonts.h and Fonts.inc.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    sys.exit("Pillow is needed: pip3 install Pillow")

REPO = Path(__file__).resolve().parents[2]

# The atlas is as wide as the panel, because the blit reads its source with the
# canvas stride and the canvas is the screen's width.
ATLAS_WIDTH = 1024

# The glyphs the firmware can draw: space through '~'. Space has no ink, so
# nothing of it is stored; only its advance is.
FIRST_GLYPH = 32
LAST_GLYPH = 126
NUM_GLYPHS = LAST_GLYPH - FIRST_GLYPH + 1

# The typefaces, and the same reasoning the mk2 records for each of them.
HELVETICA = "/System/Library/Fonts/Helvetica.ttc"
ARIAL_BOLD = "/System/Library/Fonts/Supplemental/Arial Bold.ttf"
MENLO = "/System/Library/Fonts/Menlo.ttc"
OPEN_SANS = REPO.parent / "firmware-mk2/assets/fonts/OpenSans[wdth,wght].ttf"

# The cell a size is drawn in, and the row its baseline sits on. Sharing a
# baseline is what lets a name and a monospaced value stand on one line without
# stepping by a pixel when one of them changes face.
SIZES = [
    ("Small", 12, 9),
    ("Medium", 17, 13),
    ("Large", 23, 18),
    ("Huge", 47, 36),
]

# The two spacings and the two weights, and what each one is cut from.
#
# Medium and Large are what a preset's control names and values are read in, so
# they are a label face rather than a grotesque: Open Sans at SemiCondensed
# width, the regular weight pushed to 550 because light text on a dark panel
# reads thinner than it is drawn. The mk2 carries the measurements.
OPEN_SANS_REGULAR = {"Weight": 550, "Width": 87.5}
OPEN_SANS_BOLD = {"Weight": 700, "Width": 87.5}

PROPORTIONAL = {
    ("Regular", "Small"): (HELVETICA, 0, None),
    ("Regular", "Medium"): (OPEN_SANS, 0, OPEN_SANS_REGULAR),
    ("Regular", "Large"): (OPEN_SANS, 0, OPEN_SANS_REGULAR),
    ("Regular", "Huge"): (HELVETICA, 0, None),
    ("Bold", "Small"): (ARIAL_BOLD, 0, None),
    ("Bold", "Medium"): (OPEN_SANS, 0, OPEN_SANS_BOLD),
    ("Bold", "Large"): (OPEN_SANS, 0, OPEN_SANS_BOLD),
    ("Bold", "Huge"): (ARIAL_BOLD, 0, None),
}

# Menlo has a real bold, its stems land on pixel boundaries, and its ancestry
# is Bitstream Vera Mono - the one candidate on the machine whose design is
# free to embed.
MONOSPACED = {
    "Regular": (MENLO, 0, None),
    "Bold": (MENLO, 1, None),
}

# The two sizes the system's own windows draw with. Arial Bold in its own
# right, because the status bar, the menus and the forms were laid out around
# it and the user Medium is a label face now.
SYSTEM = {
    "Small": (ARIAL_BOLD, 0, None),
    "Medium": (ARIAL_BOLD, 0, None),
}

SPACINGS = ["Proportional", "Monospaced"]
WEIGHTS = ["Regular", "Bold"]

# The mask atlas is addressed eight bits to the byte and 1024 bytes to the
# row, which is what the blitter's colour expansion reads: a byte is eight
# pixels, so a row is 8192 of them.
MASK_WIDTH = 1024

# Where the line between ink and nothing falls when a glyph is cut down to one
# bit. Half covered.
MASK_THRESHOLD = 128

# A colour that appears in no band, for the bands that carry their own
# background and so have nothing to key away.
NO_KEY = 0xF81F


class Variant:
    """One colour scheme a face is stored in.

    `field` is what the glyph is drawn on, and it matters because the
    anti-aliased edge blends towards it: a fringe mixed against the wrong
    colour is the wrong colour. `key` is what the blit drops on the way to the
    screen.

    A variant meant to land on anything is drawn on black and keys black away,
    which is right wherever the background is black - everywhere the UI draws
    text, bar the two schemes below that carry their own background and so key
    nothing.
    """

    def __init__(self, name, ink, field=0x0000, key=0x0000):
        self.name = name
        self.ink = ink
        self.field = field
        self.key = key


# The six colours a preset can use, as the firmware holds them in RGB565.
PRESET_COLOURS = [
    Variant("White", 0xFFFF),
    Variant("Red", 0xF2EA),
    Variant("Orange", 0xF4A0),
    Variant("Blue", 0x54FD),
    Variant("Green", 0x0533),
    Variant("Purple", 0xC232),
]

# What the system's own windows draw in. The last two carry their own
# background, so there is nothing in their boxes to key.
SYSTEM_VARIANTS = [
    Variant("White", 0xFFFF),
    Variant("BlackOnWhite", 0x0000, field=0xFFFF, key=NO_KEY),
    Variant("WhiteOnDimmed", 0xFFFF, field=0x4208, key=NO_KEY),
]


class Face:
    """One typeface at one size: its metrics, its shapes and its colours."""

    def __init__(self, name, source, path, index, axes, cell, baseline,
                 variants):
        self.name = name
        self.source = source
        self.cell = cell
        self.baselineRow = baseline
        self.variants = variants
        self.path = str(path)
        self.index = index
        self.axes = axes
        self.pixels = 0
        self.baseline = 0
        self.row = 0
        self.rows = 0
        self.advance = []
        self.box = []  # (left, top, width, height) of the ink, per glyph
        self.inkTop = 0
        self.inkHeight = 0


def open_font(path, index, pixels, axes=None):
    """The typeface at a pixel size, with its variation axes set by name."""
    font = ImageFont.truetype(str(path), pixels, index=index)

    if axes:
        # By name and not by position: the axis order is the font's, and
        # assuming it silently gives the wrong weight at the right width.
        font.set_variation_by_axes([
            axes[axis["name"].decode()]
            if axis["name"].decode() in axes else axis["default"]
            for axis in font.get_variation_axes()
        ])

    return font


def ink_extent(font):
    """Rows the face actually marks, measured from the top of the line box."""
    top, bottom = None, None

    for code in range(FIRST_GLYPH, LAST_GLYPH + 1):
        try:
            box = font.getbbox(chr(code))
        except OSError:
            # Some sizes are too small for the rasteriser to scale a glyph.
            return (0, 0)
        if not box or box[3] <= box[1]:
            continue
        top = box[1] if top is None else min(top, box[1])
        bottom = box[3] if bottom is None else max(bottom, box[3])

    return (top or 0), (bottom or 0)


def pick_pixel_size(path, index, cell, axes=None):
    """Largest rasteriser size whose ink fits the cell.

    Sizing by the face's declared ascent and descent throws away usable pixels:
    those figures carry the face's own leading, so a face can want 19 rows by
    that measure while marking only 16. Measuring what the glyphs actually
    cover is the difference between a 12 row cap height and a 10 row one.
    """
    best = 1

    for pixels in range(6, cell * 3):
        font = open_font(path, index, pixels, axes)
        top, bottom = ink_extent(font)
        if bottom > top and (bottom - top) <= cell:
            best = pixels

    return best


def measure(face: Face) -> None:
    """Works out the face's size, its baseline, and every glyph's ink box."""
    face.pixels = pick_pixel_size(face.path, face.index, face.cell, face.axes)
    font = open_font(face.path, face.index, face.pixels, face.axes)
    ascent, _descent = font.getmetrics()

    # Sit the topmost ink on row 0, so the cell holds glyphs rather than the
    # face's idea of a line box.
    top, bottom = ink_extent(font)
    face.baseline = ascent - top

    # Then down onto the family's baseline, as far as the rows left under the
    # deepest descender allow.
    if face.baseline < face.baselineRow:
        face.baseline += min(face.baselineRow - face.baseline,
                             face.cell - (bottom - top))

    face.font = font

    for code in range(FIRST_GLYPH, LAST_GLYPH + 1):
        character = chr(code)

        # What the pen moves by, which is not the right edge of the ink: a
        # glyph has bearings either side of it, and taking the ink extent
        # instead draws the next character on top of this one.
        advance = max(1, int(round(font.getlength(character))))
        face.advance.append(min(advance, 255))

        box = render(face, len(face.advance) - 1).getbbox()

        if box is None:
            face.box.append((0, 0, 0, 0))
        else:
            face.box.append(
                (box[0], box[1], box[2] - box[0], box[3] - box[1])
            )

    # Where the ink of a digit lands inside the cell. A line box holds
    # ascender and descender room a number does not use, so centring the box
    # does not centre the number.
    digits = [face.box[ord(d) - FIRST_GLYPH] for d in "0123456789"]
    face.inkTop = min(b[1] for b in digits)
    face.inkHeight = max(b[1] + b[3] for b in digits) - face.inkTop


def render(face: Face, index: int) -> Image.Image:
    """One glyph as coverage, in its cell, with no colour in it at all.

    Drawn at x = 0 and on the face's baseline, so that the ink box measured
    here and the pixels composited on the unit are the same pixels.
    """
    ascent, _descent = face.font.getmetrics()
    cell = Image.new(
        "L", (face.advance[index] + face.cell * 2, face.cell), 0
    )
    ImageDraw.Draw(cell).text(
        (0, face.baseline - ascent),
        chr(FIRST_GLYPH + index),
        font=face.font,
        fill=255,
    )
    return cell


def coverage(face: Face, index: int) -> bytes:
    """The glyph's ink box, two pixels to a byte.

    Four bits, not eight. Sixteen levels of coverage is past the point where
    another one changes a pixel on this panel - the mk2 quantises the same
    glyphs into three - and the half that is thrown away is a third of the
    firmware image. Each row starts on a byte so the compositor can index one
    without unpacking the rows above it.
    """
    left, top, width, height = face.box[index]

    if width == 0 or height == 0:
        return b""

    ink = render(face, index).crop((left, top, left + width, top + height))
    pixels = ink.load()
    packed = bytearray()

    for y in range(height):
        for x in range(0, width, 2):
            high = pixels[x, y] >> 4
            low = pixels[x + 1, y] >> 4 if (x + 1) < width else 0
            packed.append((high << 4) | low)

    return bytes(packed)


def build_faces() -> tuple[list[Face], list[Face]]:
    """Every face that goes in the atlas: the system's two, the user's sixteen.

    Adding one here is the whole change - the packing, the tables and the
    firmware's view of it all follow from this list, and what it costs is
    printed at the end.
    """
    system = []
    for size, cell, baseline in SIZES:
        if size not in SYSTEM:
            continue
        path, index, axes = SYSTEM[size]
        system.append(Face(f"System{size}", "system", path, index, axes,
                           cell, baseline, SYSTEM_VARIANTS))

    user = []
    for spacing in SPACINGS:
        for weight in WEIGHTS:
            for size, cell, baseline in SIZES:
                if spacing == "Proportional":
                    path, index, axes = PROPORTIONAL[(weight, size)]
                else:
                    path, index, axes = MONOSPACED[weight]
                user.append(Face(f"User{spacing}{weight}{size}", "user",
                                 path, index, axes, cell, baseline,
                                 PRESET_COLOURS))

    return system, user


def mask(face: Face, index: int) -> bytes:
    """The glyph's ink box as one bit a pixel, rows padded to whole bytes.

    The blitter's colour expansion reads its source a byte at a time and
    paints the set bits in a colour it is given per blit, leaving the clear
    ones alone - so a mask can be drawn in any colour on any background, and
    there is no fringe to have been mixed against the wrong one. That is the
    whole reason the masks exist: the coloured bands cannot leave a background
    alone, and text on a fader's bar is text on a background the bands were
    not built for.

    The most significant bit is the leftmost pixel, which is the order the
    chip expands them in.
    """
    left, top, width, height = face.box[index]

    if width == 0 or height == 0:
        return b""

    ink = render(face, index).crop((left, top, left + width, top + height))
    pixels = ink.load()
    packed = bytearray()

    for y in range(height):
        for byte in range((width + 7) // 8):
            value = 0
            for bit in range(8):
                x = byte * 8 + bit
                if x < width and pixels[x, y] >= MASK_THRESHOLD:
                    value |= 0x80 >> bit
            packed.append(value)

    return bytes(packed)


class Shelf:
    """A row of the atlas that glyphs are laid along until it fills."""

    def __init__(self, top: int, height: int):
        self.top = top
        self.height = height
        self.x = 0


def pack(face: Face, firstRow: int) -> tuple[int, dict]:
    """Decides where every glyph of every band of one face sits in the atlas.

    Only positions are worked out here; no pixels are placed, because the
    pixels are built on the unit. Shelved and tallest first, which is what
    keeps the waste down: a shelf is as tall as the first glyph put on it, so
    mixing a 30 px glyph in among 8 px ones would pay 30 px for all of them.

    One face at a time, and so one contiguous run of rows per face. Packing
    everything together would waste a few rows fewer and cost the unit a great
    deal more: the compositor builds the atlas a row at a time, and it can only
    skip a face it knows cannot reach that row.
    """
    pieces = []

    for variant in face.variants:
        for index in range(NUM_GLYPHS):
            _left, _top, width, height = face.box[index]
            if width == 0 or height == 0:
                continue
            pieces.append((height, width, variant, index))

    pieces.sort(key=lambda p: (-p[0], -p[1], p[2].name, p[3]))

    placement = {}
    shelves = []
    rows = 0

    for height, width, variant, index in pieces:
        shelf = None
        for candidate in shelves:
            if (candidate.height >= height) and (
                (candidate.x + width) <= ATLAS_WIDTH
            ):
                shelf = candidate
                break

        if shelf is None:
            shelf = Shelf(firstRow + rows, height)
            shelves.append(shelf)
            rows += height

        placement[(face.name, variant.name, index)] = (shelf.x, shelf.top)
        shelf.x += width

    return rows, placement


def pack_masks(faces: list[Face]) -> tuple[int, dict]:
    """Where every glyph's mask sits, in byte columns rather than pixels.

    A mask is read a byte at a time, so a glyph can start only on a byte -
    which costs up to seven pixels of padding a glyph and buys the whole of
    the addressing.

    All the faces are packed together here, unlike the coloured bands: the
    masks go into video memory as they are rather than being composited, so
    there is no per row work for a face to be skipped in, and a row left half
    empty is a row uploaded for nothing.
    """
    pieces = []

    for face in faces:
        for index in range(NUM_GLYPHS):
            _left, _top, width, height = face.box[index]
            if width == 0 or height == 0:
                continue
            pieces.append((height, (width + 7) // 8, face, index))

    pieces.sort(key=lambda p: (-p[0], -p[1], p[2].name, p[3]))

    placement = {}
    shelves = []
    rows = 0

    for height, bytes_wide, face, index in pieces:
        shelf = None
        for candidate in shelves:
            if (candidate.height >= height) and (
                (candidate.x + bytes_wide) <= MASK_WIDTH
            ):
                shelf = candidate
                break

        if shelf is None:
            shelf = Shelf(rows, height)
            shelves.append(shelf)
            rows += height

        placement[(face.name, index)] = (shelf.x, shelf.top)
        shelf.x += bytes_wide

    return rows, placement


def table(name: str, kind: str, values: list[int], perLine: int = 16):
    out = [f"const {kind} {name}[] = {{"]
    for index in range(0, len(values), perLine):
        out.append("    "
                   + ", ".join(str(v) for v in values[index:index + perLine])
                   + ",")
    out.append("};")
    return out


def emit_face(face: Face, placement: dict, masks: dict) -> list[str]:
    """One face's shapes, metrics and bands."""
    out = [
        f"/* {face.name}: {Path(face.path).name}, {face.pixels} px on row "
        f"{face.baseline} of a {face.cell} px cell. */",
    ]

    blob = bytearray()
    offsets = []
    for index in range(NUM_GLYPHS):
        offsets.append(len(blob))
        blob += coverage(face, index)

    out.append(f"const uint8_t {face.name}Coverage[] = {{")
    for index in range(0, len(blob), 24):
        out.append("    " + ", ".join(str(v) for v in blob[index:index + 24])
                   + ",")
    out.append("};")

    if len(blob) > 0xFFFF:
        raise SystemExit(f"{face.name}: the coverage no longer fits a 16 bit "
                         "offset - widen FontFace::coverageAt")

    out += table(f"{face.name}CoverageAt", "uint16_t", offsets, 8)

    out.append(f"const FontGlyph {face.name}Glyph[] = {{")
    for index in range(NUM_GLYPHS):
        left, top, width, height = face.box[index]
        out.append(f"    {{ {face.advance[index]}, {width}, {height}, "
                   f"{left}, {top} }},")
    out.append("};")

    for variant in face.variants:
        spots = [placement.get((face.name, variant.name, index), (0, 0))
                 for index in range(NUM_GLYPHS)]
        out += table(f"{face.name}{variant.name}X", "uint16_t",
                     [s[0] for s in spots])
        out += table(f"{face.name}{variant.name}Y", "uint16_t",
                     [s[1] for s in spots])

    if face.source == "user":
        blob = bytearray()
        offsets = []
        for index in range(NUM_GLYPHS):
            offsets.append(len(blob))
            blob += mask(face, index)

        out.append(f"const uint8_t {face.name}Mask[] = {{")
        for index in range(0, len(blob), 24):
            out.append("    "
                       + ", ".join(str(v) for v in blob[index:index + 24])
                       + ",")
        out.append("};")

        if len(blob) > 0xFFFF:
            raise SystemExit(f"{face.name}: the masks no longer fit a 16 bit "
                             "offset - widen FontFace::maskAt")

        out += table(f"{face.name}MaskAt", "uint16_t", offsets, 8)
        out += table(f"{face.name}MaskX", "uint16_t",
                     [masks.get((face.name, i), (0, 0))[0]
                      for i in range(NUM_GLYPHS)])
        out += table(f"{face.name}MaskY", "uint16_t",
                     [masks.get((face.name, i), (0, 0))[1]
                      for i in range(NUM_GLYPHS)])

    out.append(f"const FontBand {face.name}Band[] = {{")
    for variant in face.variants:
        out.append(f"    {{ {face.name}{variant.name}X, "
                   f"{face.name}{variant.name}Y, 0x{variant.ink:04X}, "
                   f"0x{variant.field:04X}, 0x{variant.key:04X} }},"
                   f" /* {variant.name} */")
    out.append("};")
    out.append("")

    return out


def face_initialiser(face: Face) -> str:
    if face.source == "user":
        masks = (f"{face.name}Mask, {face.name}MaskAt, {face.name}MaskX, "
                 f"{face.name}MaskY")
    else:
        masks = "nullptr, nullptr, nullptr, nullptr"

    return (f"    {{ {face.cell}, {face.baseline}, {face.inkTop}, "
            f"{face.inkHeight}, {face.row}, {face.rows}, {face.name}Glyph, "
            f"{face.name}Coverage, {face.name}CoverageAt, {masks}, "
            f"{face.name}Band, {len(face.variants)} }},")


HEADER_DOC = """/*
* Electra One MIDI Controller Firmware
* See COPYRIGHT file at the top of the source tree.
*/

/**
 * @file Fonts.h
 *
 * @brief Generated by tools/makefont - do not edit.
 *
 * What ships is the shape of every glyph and nothing else: four bits a
 * pixel saying how much ink covers it, two pixels to a byte, each row of
 * a glyph starting on a byte of its own. The firmware composites the coloured bands
 * from that into the display's memory when it starts, which is how six preset
 * colours cost one copy of each letter rather than six.
 *
 * Per glyph: advance moves the pen, width and height size the blit, left and
 * top put the ink back where the character cell wants it. Per band: x and y
 * are where the composited pixels ended up in the atlas, and ink, field and
 * key are what they were composited in.
 */

#pragma once

#include <cstdint>
"""


def write_header(system, user, systemRows, userRows, maskRows,
                 path: Path) -> None:
    out = [HEADER_DOC, """
/** One character of one face. */
struct FontGlyph {
    uint8_t advance;
    uint8_t width;
    uint8_t height;
    uint8_t left;
    uint8_t top;
};

/**
 * @brief One face in one colour scheme.
 *
 * `key` is the colour the blit drops. A band drawn on a background of its own
 * has nothing to drop, and says so with a colour that is not in it.
 */
struct FontBand {
    const uint16_t *x;
    const uint16_t *y;
    uint16_t ink;
    uint16_t field;
    uint16_t key;
};

/** One face: what it is made of, and every colour it is stored in. */
struct FontFace {
    uint8_t cell; /**< the line box a caller positions by */
    uint8_t baseline; /**< the row inside it the glyphs stand on */
    uint8_t inkTop; /**< where the ink of a digit starts */
    uint8_t inkHeight; /**< and how tall it is */
    uint16_t row; /**< the first row of the atlas the face occupies */
    uint16_t rows; /**< and how many of them, all of them its own */
    const FontGlyph *glyph;
    const uint8_t *coverage;
    const uint16_t *coverageAt;
    const uint8_t *mask; /**< one bit a pixel, rows padded to whole bytes */
    const uint16_t *maskAt;
    const uint16_t *maskX; /**< a byte column, not a pixel column */
    const uint16_t *maskY;
    const FontBand *band;
    uint8_t numBands;
};
"""]

    out.append(f"constexpr uint16_t fontAtlasWidth = {ATLAS_WIDTH};")
    out.append(f"constexpr uint8_t fontFirstGlyph = {FIRST_GLYPH};")
    out.append(f"constexpr uint8_t fontLastGlyph = {LAST_GLYPH};")
    out.append(f"constexpr uint16_t fontNumGlyphs = {NUM_GLYPHS};")
    out.append("")
    out.append("/* The atlas in rows. The two families sit in ranges of their")
    out.append(" * own, so each renderer composites only what it draws")
    out.append(" * from. */")
    out.append("constexpr uint16_t fontSystemRow = 0;")
    out.append(f"constexpr uint16_t fontSystemRows = {systemRows};")
    out.append(f"constexpr uint16_t fontUserRow = {systemRows};")
    out.append(f"constexpr uint16_t fontUserRows = {userRows};")
    out.append(f"constexpr uint16_t fontAtlasRows = {systemRows + userRows};")
    out.append("")
    out.append("/* The mask atlas, which is a different shape: a byte is")
    out.append(" * eight pixels, so a row is 1024 bytes and 8192 of them. */")
    out.append(f"constexpr uint16_t fontMaskWidth = {MASK_WIDTH};")
    out.append(f"constexpr uint16_t fontMaskRows = {maskRows};")
    out.append("")
    out.append("/* Indexed by TextSize; the system draws in two of them. */")
    out.append(f"extern const FontFace fontSystem[{len(system)}];")
    out.append("")
    out.append("/* Indexed by [spacing][weight][size], which is the order the")
    out.append(" * three enumerations are declared in. */")
    out.append("extern const FontFace fontUser"
               f"[{len(SPACINGS)}][{len(WEIGHTS)}][{len(SIZES)}];")
    out.append("")

    path.write_text("\n".join(out) + "\n")


def write_data(system, user, placement, masks, path: Path) -> None:
    out = [
        "/*",
        "* Electra One MIDI Controller Firmware",
        "* See COPYRIGHT file at the top of the source tree.",
        "*/",
        "",
        "/**",
        " * @file Fonts.inc",
        " *",
        " * @brief Generated by tools/makefont - do not edit.",
        " *",
        " * Included by exactly one translation unit, so that a quarter of a",
        " * megabyte of coverage is in the image once.",
        " */",
        "",
        '#include "Fonts/Fonts.h"',
        "",
    ]

    for face in system + user:
        out += emit_face(face, placement, masks)

    out.append("const FontFace fontSystem[] = {")
    for face in system:
        out.append(face_initialiser(face))
    out.append("};")
    out.append("")

    out.append("const FontFace fontUser"
               f"[{len(SPACINGS)}][{len(WEIGHTS)}][{len(SIZES)}] = {{")
    at = 0
    for spacing in SPACINGS:
        out.append(f"    {{ /* {spacing} */")
        for weight in WEIGHTS:
            out.append(f"        {{ /* {weight} */")
            for _size, _cell, _baseline in SIZES:
                out.append("            " + face_initialiser(user[at]).strip())
                at += 1
            out.append("        },")
        out.append("    },")
    out.append("};")

    path.write_text("\n".join(out) + "\n")


def to_rgb(colour):
    """RGB565 as the eight bit triple Pillow draws with.

    The low bits are replicated rather than zeroed, so that a colour survives
    the trip back to 565 unchanged.
    """
    red = (colour >> 11) & 0x1F
    green = (colour >> 5) & 0x3F
    blue = colour & 0x1F

    return ((red << 3) | (red >> 2),
            (green << 2) | (green >> 4),
            (blue << 3) | (blue >> 2))


def blend(ink, field, covered):
    """What the unit will mix, mixed the same way - in RGB565, per channel."""
    out = 0

    for shift, mask in ((11, 0x1F), (5, 0x3F), (0, 0x1F)):
        one = (ink >> shift) & mask
        other = (field >> shift) & mask
        out |= (other + ((one - other) * covered) // 255) << shift

    return out


def composite(faces, placement, rows):
    """The atlas as the unit will build it, out of the packed coverage.

    Not out of the rasteriser's own pixels: this reads back what was written
    to the file, four bits a pixel, and mixes the bands from that. A glyph
    packed wrong or an offset out by one shows up here rather than on the
    panel.
    """
    atlas = Image.new("RGB", (ATLAS_WIDTH, rows), (0, 0, 0))
    pixels = atlas.load()

    for face in faces:
        blob = bytearray()
        offsets = []
        for index in range(NUM_GLYPHS):
            offsets.append(len(blob))
            blob += coverage(face, index)

        for variant in face.variants:
            ramp = [to_rgb(blend(variant.ink, variant.field, level * 17))
                    for level in range(16)]

            for index in range(NUM_GLYPHS):
                _left, _top, width, height = face.box[index]
                if width == 0 or height == 0:
                    continue

                x, y = placement[(face.name, variant.name, index)]
                stride = (width + 1) // 2

                for row in range(height):
                    at = offsets[index] + row * stride
                    for column in range(width):
                        packed = blob[at + column // 2]
                        level = (packed >> 4) if (column % 2) == 0 \
                            else (packed & 0x0F)
                        pixels[x + column, y + row] = ramp[level]

    return atlas


def blit(page, atlas, face, variant, placement, x, y, text):
    """One string, copied glyph by glyph out of the atlas, as the unit does."""
    pen = 0

    # A scheme with a field of its own has it inside the glyph boxes and
    # nowhere else, so the renderer lays it behind the whole string first.
    if variant.field:
        width = sum(face.advance[ord(c) - FIRST_GLYPH] for c in text)
        ImageDraw.Draw(page).rectangle(
            [x - 2, y - 1, x + width, y + face.cell],
            fill=to_rgb(variant.field),
        )

    for character in text:
        index = ord(character) - FIRST_GLYPH
        if index < 0 or index >= NUM_GLYPHS:
            continue

        _left, _top, width, height = face.box[index]

        if width and height:
            at = placement[(face.name, variant.name, index)]
            box = atlas.crop((at[0], at[1], at[0] + width, at[1] + height))
            page.paste(box, (x + pen + face.box[index][0],
                             y + face.box[index][1]))

        pen += face.advance[index]

    return pen


def build_masks(faces, masks, rows):
    """The mask atlas as the unit will hold it, out of the packed bytes.

    Read back from what was written to the file rather than from the
    rasteriser, so that a mask packed wrong, an offset out by one or a bit
    order the wrong way round shows up here.
    """
    atlas = [bytearray(MASK_WIDTH) for _ in range(rows)]

    for face in faces:
        blob = bytearray()
        offsets = []
        for index in range(NUM_GLYPHS):
            offsets.append(len(blob))
            blob += mask(face, index)

        for index in range(NUM_GLYPHS):
            _left, _top, width, height = face.box[index]
            if width == 0 or height == 0:
                continue

            x, y = masks[(face.name, index)]
            stride = (width + 7) // 8

            for row in range(height):
                at = offsets[index] + row * stride
                atlas[y + row][x:x + stride] = blob[at:at + stride]

    return atlas


def expand(page, atlas, face, masks, x, y, text, colour):
    """One string, expanded out of the mask atlas, as the unit does it."""
    pen = 0
    pixels = page.load()

    for character in text:
        index = ord(character) - FIRST_GLYPH
        if index < 0 or index >= NUM_GLYPHS:
            continue

        left, top, width, height = face.box[index]

        if width and height:
            column, row = masks[(face.name, index)]
            for line in range(height):
                for bit in range(width):
                    byte = atlas[row + line][column + bit // 8]
                    if byte & (0x80 >> (bit % 8)):
                        pixels[x + pen + left + bit, y + top + line] = \
                            to_rgb(colour)

        pen += face.advance[index]

    return pen


def write_preview(system, user, placement, rows, masks, maskRows, path):
    """A specimen page, drawn the way the firmware will draw it."""
    atlas = composite(system + user, placement, rows)

    page = Image.new("RGB", (1024, 600), (0, 0, 0))
    sample = "Ag0123"
    y = 4

    for face in system:
        for index, variant in enumerate(face.variants):
            blit(page, atlas, face, variant, placement,
                 4 + index * 260, y, f"{face.name} {variant.name}")
        y += face.cell + 2

    y += 6

    for face in user:
        for index, variant in enumerate(face.variants):
            blit(page, atlas, face, variant, placement,
                 index * (1024 // 6) + 4, y, sample)
        y += face.cell + 1

    # And the masks, on the backgrounds they exist for.
    maskAtlas = build_masks(user, masks, maskRows)
    y += 8

    for index, face in enumerate(user):
        if face.cell != 17:
            continue
        for column, bar in enumerate([v.ink for v in PRESET_COLOURS]):
            box = (column * 170 + 4, y, column * 170 + 166, y + 22)
            ImageDraw.Draw(page).rectangle(box, fill=to_rgb(bar))
            expand(page, maskAtlas, face, masks, column * 170 + 8, y + 3,
                   "-12.5 dB", 0xFFFF)
        y += 26

    page.save(path)
    atlas.save(str(path).replace(".png", "-atlas.png"))

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path,
                        default=REPO / "src/Graphics/Fonts")
    parser.add_argument("--preview", type=Path,
                        help="write a specimen page, and the atlas it is cut "
                             "from, as PNGs")
    arguments = parser.parse_args()

    system, user = build_faces()

    for face in system + user:
        if not Path(face.path).exists():
            sys.exit(f"makefont: typeface not found: {face.path}")
        measure(face)

    placement = {}
    rows = 0

    for face in system + user:
        face.row = rows
        face.rows, spots = pack(face, rows)
        placement.update(spots)
        rows += face.rows


    maskRows, masks = pack_masks(user)

    systemRows = sum(face.rows for face in system)
    userRows = sum(face.rows for face in user)

    write_header(system, user, systemRows, userRows, maskRows,
                 arguments.out / "Fonts.h")
    write_data(system, user, placement, masks,
               arguments.out / "Fonts.inc")

    if arguments.preview:
        write_preview(system, user, placement, rows, masks, maskRows,
                      arguments.preview)

    flash = 0
    vram = 0

    for face in system + user:
        pixels = sum(b[2] * b[3] for b in face.box)
        flash += sum(((b[2] + 1) // 2) * b[3] for b in face.box)
        vram += pixels * len(face.variants) * 2
        print(f"  {face.name:26s} {face.pixels:2d} px in {face.cell:2d}  "
              f"{len(face.variants)} bands  {pixels // 2:6d} B")

    print(f"  coverage in flash      {flash:8d} B  ({flash / 1024:.0f} KB)")
    print(f"  bands in video memory  {vram:8d} B  ({vram / 1024:.0f} KB)")
    print(f"  atlas                  {rows} rows, "
          f"{rows * ATLAS_WIDTH * 2 / 1024:.0f} KB of the region")
    maskBytes = maskRows * MASK_WIDTH
    maskFlash = sum(len(mask(f, i)) for f in user
                    for i in range(NUM_GLYPHS))
    print(f"  masks in flash         {maskFlash:8d} B")
    print(f"  masks in video memory  {maskBytes:8d} B  "
          f"({maskBytes / 1024:.0f} KB) in {maskRows} rows")
    print(f"  compositing at boot    "
          f"{(rows * ATLAS_WIDTH * 2 + maskBytes) / 3305 :.0f} ms")
    return 0


if __name__ == "__main__":
    sys.exit(main())
