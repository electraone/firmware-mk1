#!/usr/bin/env python3
"""Builds a preset for feeling how a knob is scaled.

Three control sets of the same twelve ranges - sixteen values up to sixteen
thousand - so that one sweep of a knob can be compared across resolutions
without changing page. The first set takes the values as they are, the second
asks for a tenth of the usual rate and the third for ten times it, which is what
"sensitivity" in a value means.

A second page carries the things that count rather than sweep: lists of four and
of forty, a pad, and a relative control.

    python3 tools/compat/make_pot_preset.py > /tmp/pots.epr
"""

import json
import sys

COLUMN_X = [20, 187, 354, 521, 688, 855]
ROW_Y = [28, 116, 204, 292, 380, 468]

# Twelve ranges, one per knob, from something a list-length short to 14 bits.
RANGES = [16, 32, 64, 100, 127, 256, 512, 1000, 2048, 4096, 8192, 16383]

# One set to a sensitivity, and a colour each so a glance says which is which.
SETS = [
    (None, "529DEC", ""),
    (0.1, "03A598", " F"),
    (10.0, "F45C51", " C"),
]

controls = []
next_id = 1


def fader(page, slot, name, span, sensitivity, colour, parameter):
    global next_id

    row, column = divmod(slot, 6)
    fourteen = span > 127
    value = {
        "id": "value",
        "min": 0,
        "max": span,
        "defaultValue": span // 2,
        "message": {
            "deviceId": 1,
            "type": "cc14" if fourteen else "cc7",
            "parameterNumber": parameter,
            "min": 0,
            "max": 16383 if fourteen else 127,
        },
    }

    if sensitivity is not None:
        value["sensitivity"] = sensitivity

    controls.append(
        {
            "id": next_id,
            "type": "fader",
            "name": name,
            "color": colour,
            "bounds": [COLUMN_X[column], ROW_Y[row], 146, 56],
            "pageId": page,
            "controlSetId": (row // 2) + 1,
            "visible": True,
            "mode": "unipolar",
            "inputs": [{"potId": (slot % 12) + 1, "valueId": "value"}],
            "values": [value],
        }
    )
    next_id += 1


for setIndex, (sensitivity, colour, suffix) in enumerate(SETS):
    for knob, span in enumerate(RANGES):
        fader(
            1,
            (setIndex * 12) + knob,
            "%d%s" % (span, suffix),
            span,
            sensitivity,
            colour,
            (setIndex * 12) + knob + 1,
        )


def discrete(page, slot, kind, name, colour, parameter, span, overlayId=None,
             sensitivity=None, mode="unipolar"):
    global next_id

    row, column = divmod(slot, 6)
    value = {
        "id": "value",
        "min": 0,
        "max": span,
        "defaultValue": 0,
        "message": {
            "deviceId": 1,
            "type": "cc7",
            "parameterNumber": parameter,
            "min": 0,
            "max": 127,
        },
    }

    if overlayId is not None:
        value["overlayId"] = overlayId
    if sensitivity is not None:
        value["sensitivity"] = sensitivity

    controls.append(
        {
            "id": next_id,
            "type": kind,
            "name": name,
            "color": colour,
            "bounds": [COLUMN_X[column], ROW_Y[row], 146, 56],
            "pageId": page,
            "controlSetId": (row // 2) + 1,
            "visible": True,
            "mode": mode,
            "inputs": [{"potId": (slot % 12) + 1, "valueId": "value"}],
            "values": [value],
        }
    )
    next_id += 1


overlays = [
    {
        "id": 1,
        "items": [{"value": i, "label": "ITEM %d" % (i + 1)} for i in range(4)],
    },
    {
        "id": 2,
        "items": [{"value": i, "label": "STEP %02d" % (i + 1)} for i in range(40)],
    },
]

discrete(2, 0, "list", "LIST 4", "529DEC", 40, 3, overlayId=1)
discrete(2, 1, "list", "LIST 40", "529DEC", 41, 39, overlayId=2)
discrete(2, 2, "list", "LIST 40 F", "03A598", 42, 39, overlayId=2, sensitivity=0.25)
discrete(2, 3, "pad", "PAD", "F49500", 43, 1, mode="toggle")
discrete(2, 4, "fader", "16 AGAIN", "C44795", 44, 16)
discrete(2, 5, "fader", "16383 AGAIN", "C44795", 45, 16383)

preset = {
    "version": 2,
    "name": "POT SCALING",
    "projectId": "pot-scaling",
    "pages": [
        {"id": 1, "name": "RANGES", "defaultControlSetId": 1},
        {"id": 2, "name": "DETENTS", "defaultControlSetId": 1},
    ],
    "devices": [{"id": 1, "name": "HARNESS", "port": 1, "channel": 1, "rate": 20}],
    "overlays": overlays,
    "groups": [],
    "controls": controls,
}

json.dump(preset, sys.stdout, separators=(",", ":"))
sys.stdout.write("\n")
