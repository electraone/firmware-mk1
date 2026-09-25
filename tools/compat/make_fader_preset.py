#!/usr/bin/env python3
"""Builds a preset that shows every shape a control can take.

One control for each thing the fader draws differently - the variants, the two
polarities, the overlay treatments, and the sixteen faces - so that a change to
the look and feel can be looked at rather than reasoned about. The values are
static; this is for the eye, not for the MIDI.

    python3 tools/compat/make_fader_preset.py > /tmp/faders.epr
"""

import json
import sys

# The grid a page is laid out on, which is the mk2's: six columns of 146 by 56.
COLUMN_X = [20, 187, 354, 521, 688, 855]
ROW_Y = [28, 125, 222, 319, 416, 513]

COLOURS = ["FFFFFF", "F45C51", "F49500", "529DEC", "03A598", "C44795"]

controls = []
overlays = []
pages = []
next_id = 1


def control(page, slot, name, value, **extra):
    """One fader, at a slot counted left to right and top to bottom."""
    global next_id

    row, column = divmod(slot, 6)
    body = {
        "id": next_id,
        "type": "fader",
        "name": name,
        "color": extra.pop("color", "529DEC"),
        "bounds": [COLUMN_X[column], ROW_Y[row], 146, 56],
        "pageId": page,
        "controlSetId": (row % 3) + 1,
        "visible": True,
        "mode": extra.pop("mode", "unipolar"),
        "inputs": [{"potId": (slot % 12) + 1, "valueId": "value"}],
        "values": [
            {
                "id": "value",
                "min": extra.pop("min", 0),
                "max": extra.pop("max", 127),
                "defaultValue": value,
                "message": {
                    "deviceId": 1,
                    "type": "cc7",
                    "parameterNumber": next_id,
                    "min": extra.pop("msgMin", 0),
                    "max": extra.pop("msgMax", 127),
                },
            }
        ],
    }
    body.update(extra)
    controls.append(body)
    next_id += 1


def pad(page, slot, name, on, **extra):
    """One pad, at a slot counted left to right and top to bottom.

    `on` is the state it is drawn in: the default value is the MIDI value that
    state sends, so the pad comes up lit or dark without anything having to
    touch it.
    """
    global next_id

    row, column = divmod(slot, 6)
    body = {
        "id": next_id,
        "type": "pad",
        "name": name,
        "color": extra.pop("color", "529DEC"),
        "bounds": [COLUMN_X[column], ROW_Y[row], 146, 56],
        "pageId": page,
        "controlSetId": (row % 3) + 1,
        "visible": True,
        "mode": extra.pop("mode", "toggle"),
        "inputs": [{"potId": (slot % 12) + 1, "valueId": "value"}],
        "values": [
            {
                "id": "value",
                "defaultValue": 127 if on else 0,
                "message": {
                    "deviceId": 1,
                    "type": "cc7",
                    "parameterNumber": next_id,
                    "offValue": 0,
                    "onValue": 127,
                },
            }
        ],
    }
    overlay_id = extra.pop("overlayId", None)
    if overlay_id is not None:
        body["values"][0]["overlayId"] = overlay_id
    body.update(extra)
    controls.append(body)
    next_id += 1


def page(number, name):
    pages.append({"id": number, "name": name, "defaultControlSetId": 0})


# Page one: the variants, and what each does at the ends of its range.
page(1, "VARIANTS")
control(1, 0, "DEFAULT LOW", 12)
control(1, 1, "DEFAULT MID", 64)
control(1, 2, "DEFAULT FULL", 127)
control(1, 3, "THIN", 64, variant="thin")
control(1, 4, "DIAL", 64, variant="dial")
control(1, 5, "DIAL FULL", 127, variant="dial")
control(1, 6, "BIPOLAR BELOW", -40, mode="bipolar", min=-64, max=63)
control(1, 7, "BIPOLAR ZERO", 0, mode="bipolar", min=-64, max=63)
control(1, 8, "BIPOLAR ABOVE", 40, mode="bipolar", min=-64, max=63)
control(1, 9, "BIPOLAR DIAL", 0, mode="bipolar", min=-64, max=63,
        variant="dial")
control(1, 10, "THIN BIPOLAR", -30, mode="bipolar", min=-64, max=63,
        variant="thin")
control(1, 11, "NARROW RANGE", 3, min=0, max=7)

# Page two: the six colours a preset can ask for, at rest and at full.
page(2, "COLOURS")
for index, colour in enumerate(COLOURS):
    control(2, index, f"COLOUR {index + 1}", 90, color=colour)
for index, colour in enumerate(COLOURS):
    control(2, index + 6, f"FULL {index + 1}", 127, color=colour)

# Page three: the four sizes, in both weights.
page(3, "SIZES")
for index, size in enumerate(["small", "medium", "large", "huge"]):
    control(3, index, size.upper(), 88, font={"size": size})
for index, size in enumerate(["small", "medium", "large", "huge"]):
    control(3, index + 6, size.upper() + " BOLD", 88,
            font={"size": size, "weight": "bold"})

# Page four: the same four, monospaced - where the digits stop shuffling.
page(4, "MONOSPACED")
for index, size in enumerate(["small", "medium", "large", "huge"]):
    control(4, index, size.upper(), 88,
            font={"size": size, "spacing": "monospaced"})
for index, size in enumerate(["small", "medium", "large", "huge"]):
    control(4, index + 6, size.upper() + " BOLD", 88,
            font={"size": size, "weight": "bold", "spacing": "monospaced"})

# Page five: overlays. A fader with one is drawn thin and prints the word;
# an entry may name a colour of its own.
page(5, "OVERLAYS")
overlays.append({
    "id": 1,
    "items": [
        {"value": 0, "label": "OFF"},
        {"value": 1, "label": "LOW"},
        {"value": 2, "label": "MID"},
        {"value": 3, "label": "HIGH"},
    ],
})
overlays.append({
    "id": 2,
    "items": [
        {"value": 0, "label": "SAFE", "color": "03A598"},
        {"value": 1, "label": "WARN", "color": "F49500"},
        {"value": 2, "label": "ALERT", "color": "F45C51"},
        {"value": 3, "label": "OVER", "color": "C44795"},
    ],
})

for index, value in enumerate([0, 1, 2, 3]):
    control(5, index, f"WORD {value}", value, min=0, max=3)
    controls[-1]["values"][0]["overlayId"] = 1
for index, value in enumerate([0, 1, 2, 3]):
    control(5, index + 6, f"TINTED {value}", value, min=0, max=3)
    controls[-1]["values"][0]["overlayId"] = 2

# Page six: lists. Few entries draw dots, many draw the bar that stands in
# for them; a long label is cut to the control with a marker.
page(6, "LISTS")

overlays.append({
    "id": 3,
    "items": [{"value": v, "label": f"ITEM {v}"} for v in range(5)],
})
overlays.append({
    "id": 4,
    "items": [{"value": v, "label": f"STEP {v:02d}"} for v in range(24)],
})
overlays.append({
    "id": 5,
    "items": [
        {"value": 0, "label": "SHORT"},
        {"value": 1, "label": "A RATHER LONGER LABEL"},
        {"value": 2, "label": "WWWWWWWWWWWWWWWW"},
    ],
})
# Fifty-five: three pages of the detail's grid with the last one part full, so
# both strips have something to turn and the end of the list is not a tidy
# multiple of a page.
overlays.append({
    "id": 7,
    "items": [{"value": v, "label": f"NOTE {v:02d}"} for v in range(55)],
})
overlays.append({
    "id": 6,
    "items": [
        {"value": 0, "label": "GREEN", "color": "03A598"},
        {"value": 1, "label": "AMBER", "color": "F49500"},
        {"value": 2, "label": "RED", "color": "F45C51"},
    ],
})

LIST_PAGES = [
    (6, "list", [
        ("DOTS", 3, 2, {}),
        ("DOTS FULL", 3, 4, {}),
        ("BAR", 4, 11, {}),
        ("BAR END", 4, 23, {}),
        ("LONG LABEL", 5, 1, {}),
        ("TOO LONG", 5, 2, {}),
        ("TINTED", 6, 1, {}),
        ("TINTED RED", 6, 2, {}),
        ("HUGE FACE", 3, 2, {"font": {"size": "large"}}),
        ("BOLD", 3, 3, {"font": {"weight": "bold"}}),
        ("SMALL", 3, 1, {"font": {"size": "small"}}),
        ("MONO", 4, 7, {"font": {"spacing": "monospaced"}}),
        ("MANY", 7, 5, {}),
        ("MANY LATE", 7, 50, {}),
    ]),
    (7, "list-button", [
        ("VALUE ONLY", 3, 2, {"variant": "valueOnly"}),
        ("V FULL", 3, 4, {"variant": "valueOnly"}),
        ("V BAR", 4, 11, {"variant": "valueOnly"}),
        ("V LONG", 5, 1, {"variant": "valueOnly"}),
        ("V TINTED", 6, 0, {"variant": "valueOnly"}),
        ("V SMALL", 3, 1, {"variant": "valueOnly", "font": {"size": "small"}}),
    ]),
]

page(7, "VALUE ONLY")

for pageId, kind, entries in LIST_PAGES:
    for slot, (name, overlayId, value, extra) in enumerate(entries):
        control(pageId, slot, name, value, min=0,
                max=max(v["value"] for o in overlays if o["id"] == overlayId
                        for v in o["items"]),
                **extra)
        controls[-1]["type"] = "list"
        controls[-1]["values"][0]["overlayId"] = overlayId

# Page eight: the default pad. Both states, both modes, and the six colours -
# a pad that is on carries a strip of its colour down the left whether it is
# momentary or not.
page(8, "PADS")
pad(8, 0, "OFF", False)
pad(8, 1, "ON", True)
# Only at rest: this firmware writes a momentary pad's off value when the
# component goes away, so one is always dark by the time a page has been
# switched to. The lit shape is the same either way - paintPad() draws the
# strip from the state alone, as the mk2 does.
pad(8, 2, "MOMENTARY", False, mode="momentary")
pad(8, 3, "A LONGER NAME", False)
pad(8, 4, "LIT LONGER", True)
for index, colour in enumerate(COLOURS):
    pad(8, index + 6, f"COLOUR {index + 1}", False, color=colour)
for index, colour in enumerate(COLOURS):
    pad(8, index + 12, f"LIT {index + 1}", True, color=colour)

# Page nine: the faces a pad's word is printed in. A pad prints its name
# across the middle here, so the face is the control's own.
page(9, "PAD FACES")
for index, size in enumerate(["small", "medium", "large", "huge"]):
    pad(9, index, size.upper(), False, font={"size": size})
for index, size in enumerate(["small", "medium", "large", "huge"]):
    pad(9, index + 6, size.upper(), True,
        font={"size": size, "weight": "bold"})

# Page ten: the valueOnly pad, which shows the state as a word from its
# overlay - keyed by the MIDI value the state sends, not by an index.
overlays.append({
    "id": 8,
    "items": [
        {"value": 0, "label": "OFF"},
        {"value": 127, "label": "ON"},
    ],
})
overlays.append({
    "id": 9,
    "items": [
        {"value": 0, "label": "MONO"},
        {"value": 127, "label": "POLY"},
    ],
})

page(10, "PAD VALUE")
pad(10, 0, "STATE", False, variant="valueOnly", overlayId=8)
pad(10, 1, "STATE", True, variant="valueOnly", overlayId=8)
pad(10, 2, "VOICES", False, variant="valueOnly", overlayId=9)
pad(10, 3, "VOICES", True, variant="valueOnly", overlayId=9)
# No overlay: the name is what is left to print, as the default pad does.
pad(10, 4, "NO OVERLAY", False, variant="valueOnly")
pad(10, 5, "NO OVERLAY", True, variant="valueOnly")
for index, colour in enumerate(COLOURS):
    pad(10, index + 6, "STATE", index % 2 == 1, variant="valueOnly",
        overlayId=8, color=colour)
pad(10, 12, "LARGE", False, variant="valueOnly", overlayId=8,
    font={"size": "large"})
pad(10, 13, "LARGE", True, variant="valueOnly", overlayId=8,
    font={"size": "large"})
pad(10, 14, "SMALL", True, variant="valueOnly", overlayId=8,
    font={"size": "small"})

# Page eleven: the checkbox, which stands on the row a fader's track ends on
# and keeps its name underneath. A checkbox with no name has the whole cell.
page(11, "CHECKBOX")
pad(11, 0, "OFF", False, variant="checkbox")
pad(11, 1, "ON", True, variant="checkbox")
pad(11, 2, "MOMENTARY", False, variant="checkbox", mode="momentary")
pad(11, 3, "", True, variant="checkbox")
pad(11, 4, "", False, variant="checkbox")
pad(11, 5, "A LONGER NAME", True, variant="checkbox")
for index, colour in enumerate(COLOURS):
    pad(11, index + 6, f"COLOUR {index + 1}", index % 2 == 1,
        variant="checkbox", color=colour)

preset = {
    "version": 2,
    "name": "CONTROLS",
    "projectId": "faders",
    "pages": pages,
    "devices": [
        {"id": 1, "name": "Generic MIDI", "port": 1, "channel": 1},
    ],
    "overlays": overlays,
    "groups": [],
    "controls": controls,
}

json.dump(preset, sys.stdout, separators=(",", ":"))
