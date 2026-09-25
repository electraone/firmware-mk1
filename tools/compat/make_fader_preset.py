#!/usr/bin/env python3
"""Builds a preset that shows every shape a fader can take.

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

preset = {
    "version": 2,
    "name": "FADERS",
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
