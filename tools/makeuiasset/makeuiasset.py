#!/usr/bin/env python3
"""Builds the UI asset the firmware draws its text from.

The unit draws a character by copying it out of a strip held in the display
controller's memory: one BTE copy per glyph, the glyph already coloured and
already anti-aliased. That is cheap - a blit costs about 42 us of setting
registers plus 0.09 us a pixel - and it is why the text engine looks the way
it does. This tool builds that strip.

It also decides how the strip is laid out, and the layout is where the saving
is. The strip this replaces stored every glyph as a full cell in a band as
wide as the panel, which meant carrying two kinds of nothing:

  * the right hand half of every row, because a face's glyphs only reach
    about 55% of the way across - 45% of the strip was black padding,
  * the space above the ascenders and below the baseline of every cell,
    because a 13 px cell holds about 8 px of ink - another 53%.

So glyphs are packed here instead: each one is cut to the box its ink
actually occupies, and the boxes are shelved across the strip rather than laid
out one face to a row. 768 KB becomes 230 KB, in the file and in video memory
both, with no compression to undo at the other end.

Blitting the ink box rather than the cell is also faster, because a blit costs
by the pixel: about 10 us a glyph, which is most of what the packing saves
being given back.

Two families live in the strip, as they do on the mk2:

  system  what the firmware's own windows are written in. Fixed colours,
          because the system UI only ever draws on its own backgrounds.
  user    what a preset's controls are written in. One band per preset
          colour, because a control's text takes the control's colour and the
          colour has to be in the pixels for a plain copy to carry it.

    tools/makeuiasset/makeuiasset.py

Writes the asset next to itself and the tables into src/Graphics/Fonts.
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    sys.exit("Pillow is needed: pip3 install Pillow")

REPO = Path(__file__).resolve().parents[2]

# The strip is as wide as the panel, because the loader walks it a row at a
# time and the BTE reads it back with the same stride.
ATLAS_WIDTH = 1024
ATLAS_MAX_ROWS = 600

# The glyphs the firmware can draw: '!' through '~'. Space has no ink, so it
# is not in the strip; the renderer advances the pen for it instead.
FIRST_GLYPH = 0x21
LAST_GLYPH = 0x7E
NUM_GLYPHS = LAST_GLYPH - FIRST_GLYPH + 1

# The typefaces, the same ones the mk2 uses, so the two products read as one.
HELVETICA = "/System/Library/Fonts/Helvetica.ttc"
ARIAL_BOLD = "/System/Library/Fonts/Supplemental/Arial Bold.ttf"
OPEN_SANS = REPO.parent / "firmware-mk2/assets/fonts/OpenSans[wdth,wght].ttf"

# The colour a glyph is keyed against when it is blitted onto something else.
# Magenta rather than black, so that black text keys correctly too - keying
# black would erase the glyph rather than its background.
CHROMA_KEY = 0xF81F


class Variant:
    """One colour a face is stored in.

    `field` is what the glyph is drawn on, which matters because the
    anti-aliased edge blends towards it. A variant meant to land on anything
    is drawn on the chroma colour, so the blit keys the background away; one
    whose background is part of the look is drawn on that background instead,
    and then there is no chroma in its box for the same blit to key. So every
    variant is stored and drawn identically and only the colours differ.
    """

    def __init__(self, name, ink, field=CHROMA_KEY):
        self.name = name
        self.ink = ink
        self.field = field


# The six colours a preset can use, as the firmware holds them in RGB565.
# Anything a preset draws takes one of these, which is why there are exactly
# six bands of each user face.
PRESET_COLOURS = [
    Variant("White", 0xFFFF),
    Variant("Red", 0xF2EA),
    Variant("Orange", 0xF4A0),
    Variant("Blue", 0x54FD),
    Variant("Green", 0x0533),
    Variant("Purple", 0xC232),
]

# What the system's own windows draw in.
SYSTEM_VARIANTS = [
    Variant("White", 0xFFFF),
    Variant("BlackOnWhite", 0x0000, field=0xFFFF),
    Variant("WhiteOnDimmed", 0xFFFF, field=0x4208),
]


class Face:
    """One typeface at one size, and the cell it is drawn in.

    `cell` is the row height the firmware positions a line by, kept at what
    the strip this replaces used so that nothing in the UI moves when the
    typeface changes underneath it. `pixels` is what the rasteriser is asked
    for, chosen so the ink sits inside the cell with room for descenders.
    """

    def __init__(self, name, family, path, index, pixels, cell, variants):
        self.name = name
        self.family = family
        self.pixels = pixels
        self.cell = cell
        self.variants = variants
        self.font = ImageFont.truetype(str(path), pixels, index=index)
        self.advance = []
        self.box = []  # (left, top, width, height) of the ink, per glyph


def build_faces() -> list[Face]:
    """The faces that go in the strip.

    Adding one here is the whole change: the packing, the tables and the
    firmware's view of it all follow from this list, and what it costs is
    printed at the end.
    """
    return [
        Face("SystemSmall", "system", HELVETICA, 0, 10, 13, SYSTEM_VARIANTS),
        Face("SystemMedium", "system", ARIAL_BOLD, 0, 13, 17, SYSTEM_VARIANTS),
        Face("UserSmall", "user", OPEN_SANS, 0, 10, 13, PRESET_COLOURS),
        Face("UserMedium", "user", OPEN_SANS, 0, 13, 17, PRESET_COLOURS),
        Face("UserLarge", "user", OPEN_SANS, 0, 15, 19, PRESET_COLOURS),
    ]


def to_rgb(colour: int) -> tuple[int, int, int]:
    """RGB565 as the eight bit triple Pillow draws with.

    The low bits are replicated rather than zeroed so that a colour survives
    the trip back to 565 unchanged.
    """
    r = (colour >> 11) & 0x1F
    g = (colour >> 5) & 0x3F
    b = colour & 0x1F
    return ((r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2))


def pack565(r: int, g: int, b: int) -> int:
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def measure(face: Face) -> None:
    """Works out what each glyph advances by and where its ink sits.

    The ink box is taken from the glyph drawn white on black, so that it is
    the same box for every colour of the face - the shapes do not change with
    the colour, and the firmware's tables are smaller for sharing them.
    """
    for code in range(FIRST_GLYPH, LAST_GLYPH + 1):
        character = chr(code)

        # The advance is what the pen moves by, which is not the right edge
        # of the ink: a glyph has bearings either side of it, and taking the
        # ink extent instead draws the next character on top of this one.
        advance = max(1, int(round(face.font.getlength(character))))
        face.advance.append(min(advance, 255))

        # Rendered wide enough that nothing is clipped, whatever the glyph
        # does outside its advance. The box is measured, not assumed.
        coverage = Image.new("L", (advance + face.cell * 2, face.cell), 0)
        ImageDraw.Draw(coverage).text((0, 0), character, font=face.font,
                                      fill=255)
        bbox = coverage.getbbox()

        if bbox is None:
            face.box.append((0, 0, 1, 1))
        else:
            face.box.append(
                (bbox[0], bbox[1], bbox[2] - bbox[0], bbox[3] - bbox[1])
            )


def render(face: Face, variant: Variant, index: int) -> Image.Image:
    """One glyph of one face in one colour, cut to what will be stored.

    Drawn on the field it will be keyed or copied against, so that the
    anti-aliased edge blends towards that rather than towards whatever
    happened to be in the buffer when it lands.
    """
    left, top, width, height = face.box[index]
    advance = face.advance[index]

    cell = Image.new("RGB", (advance + face.cell * 2, face.cell),
                     to_rgb(variant.field))
    ImageDraw.Draw(cell).text(
        (0, 0),
        chr(FIRST_GLYPH + index),
        font=face.font,
        fill=to_rgb(variant.ink),
    )

    return cell.crop((left, top, left + width, top + height))


class Shelf:
    """A row of the strip that glyphs are laid along until it fills."""

    def __init__(self, top: int, height: int):
        self.top = top
        self.height = height
        self.x = 0


def pack(faces: list[Face]) -> tuple[Image.Image, int, dict]:
    """Lays every glyph of every variant into the strip.

    Shelved, and tallest first, which is what keeps the waste down: a shelf is
    as tall as the first glyph put on it, so mixing a 15 px glyph in among 8 px
    ones would pay 15 px for all of them.
    """
    pieces = []

    for face in faces:
        measure(face)
        for variant in face.variants:
            for index in range(NUM_GLYPHS):
                image = render(face, variant, index)
                pieces.append((image.height, image.width, face, variant,
                               index, image))

    # Tallest first. The index is not part of the comparison, so faces stay in
    # a stable order within a height and the output is reproducible.
    pieces.sort(key=lambda p: (-p[0], -p[1], p[2].name, p[3].name, p[4]))

    atlas = Image.new("RGB", (ATLAS_WIDTH, ATLAS_MAX_ROWS), to_rgb(CHROMA_KEY))
    placement = {}
    shelves = []
    used_rows = 0

    for height, width, face, variant, index, image in pieces:
        if width > ATLAS_WIDTH:
            raise SystemExit(f"{face.name} glyph {index} is {width} px wide")

        shelf = None
        for candidate in shelves:
            if (candidate.height >= height) and (
                (candidate.x + width) <= ATLAS_WIDTH
            ):
                shelf = candidate
                break

        if shelf is None:
            if (used_rows + height) > ATLAS_MAX_ROWS:
                raise SystemExit(
                    f"the faces need more than {ATLAS_MAX_ROWS} rows. "
                    "Drop a face or a variant in build_faces()."
                )
            shelf = Shelf(used_rows, height)
            shelves.append(shelf)
            used_rows += height

        atlas.paste(image, (shelf.x, shelf.top))
        placement[(face.name, variant.name, index)] = (shelf.x, shelf.top)
        shelf.x += width

    return atlas, used_rows, placement


def write_asset(atlas: Image.Image, rows: int, path: Path) -> None:
    """Writes the strip the way the firmware's loader reads it.

    The body is raw RGB565, little endian, top row first, with no padding of
    any kind - the loader streams it straight into video memory, so what is in
    the file is exactly what ends up there.
    """
    pixels = atlas.load()
    body = bytearray()

    for y in range(rows):
        for x in range(ATLAS_WIDTH):
            body += struct.pack("<H", pack565(*pixels[x, y]))

    header_size = 14 + 40
    header = struct.pack(
        "<2sIHHI", b"BM", header_size + len(body), 0, 0, header_size
    )
    info = struct.pack(
        "<IiiHHIIiiII", 40, ATLAS_WIDTH, -rows, 1, 16, 0, len(body),
        2835, 2835, 0, 0,
    )

    path.write_bytes(header + info + body)


def table(name: str, kind: str, values: list[int]) -> list[str]:
    out = [f"constexpr {kind} {name}[] = {{"]
    for index in range(0, len(values), 16):
        out.append("    " + ", ".join(str(v) for v in values[index:index + 16])
                   + ",")
    out.append("};")
    return out


def write_tables(faces, rows, placement, path: Path, asset: str) -> None:
    """The tables the firmware addresses the strip with.

    Generated rather than written by hand, because they are the one thing that
    has to agree with the pixels exactly. An offset out by a glyph draws text
    made of the wrong letters, and nothing in the firmware could notice.
    """
    out = [
        "/*",
        "* Electra One MIDI Controller Firmware",
        "* See COPYRIGHT file at the top of the source tree.",
        "*/",
        "",
        "/**",
        " * @file UiAsset.inc",
        " *",
        " * @brief Generated by tools/makeuiasset - do not edit.",
        " *",
        f" * Describes {asset}. Glyphs are packed rather than laid out in",
        " * bands, so a glyph is found by its own position and drawn at its",
        " * own size: X and Y are where its box sits in the strip, Width and",
        " * Height are how big the box is, and Left and Top are where the box",
        " * sits inside the character cell. Advance is how far the pen moves.",
        " */",
        "",
        "#pragma once",
        "",
        "#include <cstdint>",
        "",
        f"constexpr uint16_t uiAssetWidth = {ATLAS_WIDTH};",
        f"constexpr uint16_t uiAssetRows = {rows};",
        f"constexpr uint8_t uiAssetFirstGlyph = {FIRST_GLYPH};",
        f"constexpr uint8_t uiAssetLastGlyph = {LAST_GLYPH};",
        f"constexpr uint16_t uiAssetNumGlyphs = {NUM_GLYPHS};",
        f"constexpr uint16_t uiAssetChromaKey = 0x{CHROMA_KEY:04X};",
        "",
    ]

    for face in faces:
        out.append(f"/* {face.name}: {face.family}, {face.pixels} px in a "
                   f"{face.cell} px cell. */")
        out.append(f"constexpr uint8_t {face.name}Cell = {face.cell};")
        out += table(f"{face.name}Advance", "uint8_t", face.advance)
        out += table(f"{face.name}Width", "uint8_t",
                     [b[2] for b in face.box])
        out += table(f"{face.name}Height", "uint8_t",
                     [b[3] for b in face.box])
        out += table(f"{face.name}Left", "uint8_t", [b[0] for b in face.box])
        out += table(f"{face.name}Top", "uint8_t", [b[1] for b in face.box])

        for variant in face.variants:
            spots = [placement[(face.name, variant.name, i)]
                     for i in range(NUM_GLYPHS)]
            out.append(f"/* {face.name} in {variant.name} */")
            out += table(f"{face.name}{variant.name}X", "uint16_t",
                         [s[0] for s in spots])
            out += table(f"{face.name}{variant.name}Y", "uint16_t",
                         [s[1] for s in spots])
        out.append("")

    path.write_text("\n".join(out) + "\n")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--asset", default="ui-1.0.0.bmp")
    parser.add_argument("--out", type=Path,
                        default=Path(__file__).resolve().parent)
    parser.add_argument("--tables", type=Path,
                        default=REPO / "src/Graphics/Fonts/UiAsset.inc")
    parser.add_argument("--preview", type=Path, default=None,
                        help="also write a viewable PNG of the strip")
    arguments = parser.parse_args()

    faces = build_faces()
    atlas, rows, placement = pack(faces)

    asset_path = arguments.out / arguments.asset
    write_asset(atlas, rows, asset_path)
    write_tables(faces, rows, placement, arguments.tables, arguments.asset)

    if arguments.preview:
        atlas.crop((0, 0, ATLAS_WIDTH, rows)).save(arguments.preview)

    cells = sum(
        sum(face.advance) * face.cell * len(face.variants) for face in faces
    )
    print(f"{asset_path}: {asset_path.stat().st_size} bytes, "
          f"{ATLAS_WIDTH} x {rows}")
    for face in faces:
        ink = sum(b[2] * b[3] for b in face.box) * len(face.variants)
        print(f"  {face.name:14s} {face.family:6s} cell {face.cell:2d}  "
              f"{len(face.variants)} variants  ink {ink * 2 // 1024:4d} KB")
    print(f"  packed into {rows} of {ATLAS_MAX_ROWS} rows "
          f"({rows * ATLAS_WIDTH * 2 // 1024} KB), "
          f"against {cells * 2 // 1024} KB as full cells in full width bands")
    print(f"{arguments.tables}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
