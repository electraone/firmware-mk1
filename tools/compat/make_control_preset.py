#!/usr/bin/env python3
"""Builds a preset that shows the control types the fader specimen has no room
for: the XY pad, the four-fader control, the text box, and the custom control.

A preset may only carry twelve pages, and make_fader_preset.py fills them. This
is the second volume rather than a second copy: it shares nothing with that
script on purpose, because the two are uploaded one after the other and a
change to one should not move the other's pages.

    python3 tools/compat/make_control_preset.py > /tmp/newcontrols.epr
"""

import json
import sys

# The grid a page is laid out on, which is the mk2's: six columns of 146 by 56.
COLUMN_X = [20, 187, 354, 521, 688, 855]
ROW_Y = [28, 125, 222, 319, 416, 513]

COLOURS = ["FFFFFF", "F45C51", "F49500", "529DEC", "03A598", "C44795"]

controls = []
pages = []
next_id = 1
next_parameter = 1


def page(number, name):
    pages.append({"id": number, "name": name, "defaultControlSetId": 0})


def control(page, slot, kind, name, value_ids, values, **extra):
    """One control, at a slot counted left to right and top to bottom."""
    global next_id, next_parameter

    row, column = divmod(slot, 6)
    span = extra.pop("span", 1)
    width = COLUMN_X[column + span - 1] + 146 - COLUMN_X[column]
    ranges = extra.pop("ranges", None) or [(0, 127)] * len(value_ids)

    body = {
        "id": next_id,
        "type": kind,
        "name": name,
        "color": extra.pop("color", "529DEC"),
        "bounds": [COLUMN_X[column], ROW_Y[row], width, 56],
        "pageId": page,
        "controlSetId": extra.pop("controlSetId", 1),
        "visible": True,
        "inputs": [
            {"potId": (index % 12) + 1, "valueId": valueId}
            for index, valueId in enumerate(value_ids)
        ],
        "values": [
            {
                "id": valueId,
                "min": ranges[index][0],
                "max": ranges[index][1],
                "defaultValue": values[index],
                "message": {
                    "deviceId": 1,
                    "type": "cc7",
                    "parameterNumber": next_parameter + index,
                    "min": 0,
                    "max": 127,
                },
            }
            for index, valueId in enumerate(value_ids)
        ],
    }
    body.update(extra)
    controls.append(body)
    next_id += 1
    # One CC per value, running from one: a parameter number worked out from
    # the control id runs past 127 on a page of four-value controls, and the
    # specimen is set from real CCs.
    next_parameter += len(value_ids)
    if next_parameter > 127:
        raise SystemExit("ran out of CC numbers for the specimen")


XY = ["x", "y"]
VF = ["f1", "f2", "f3", "f4"]
ONE = ["value"]

# Page one: the XY pad. The bottom left corner is (0, 0), so a pad with a low
# Y draws its horizontal line near the bottom.
page(1, "XY PAD")
control(1, 0, "xypad", "CENTRE", XY, [64, 64])
control(1, 1, "xypad", "LOW", XY, [10, 10])
control(1, 2, "xypad", "HIGH", XY, [120, 120])
control(1, 3, "xypad", "CORNER", XY, [0, 127])
control(1, 4, "xypad", "BIPOLAR", XY, [0, 0],
        ranges=[(-64, 63), (-64, 63)])
control(1, 5, "xypad", "NARROW", XY, [3, 5], ranges=[(0, 7), (0, 7)])
for index, colour in enumerate(COLOURS):
    control(1, index + 6, "xypad", f"COLOUR {index + 1}", XY,
            [30 + index * 15, 100 - index * 15], color=colour)
control(1, 12, "xypad", "WIDE", XY, [64, 96], span=2, color="F49500")

# Page two: the four vertical faders. The one the first input names is the one
# a knob turns, and it is outlined while a finger is on that knob.
page(2, "V FADER")
control(2, 0, "vfader", "RAMP", VF, [20, 50, 80, 110])
control(2, 1, "vfader", "ALL LOW", VF, [4, 4, 4, 4])
control(2, 2, "vfader", "ALL HIGH", VF, [127, 127, 127, 127])
control(2, 3, "vfader", "MIXED", VF, [127, 0, 96, 32])
control(2, 4, "vfader", "BIPOLAR", VF, [-60, -20, 20, 60],
        ranges=[(-64, 63)] * 4)
control(2, 5, "vfader", "ZEROES", VF, [0, 0, 0, 0])
for index, colour in enumerate(COLOURS):
    control(2, index + 6, "vfader", f"COLOUR {index + 1}", VF,
            [20, 60, 100, 127], color=colour)
control(2, 12, "vfader", "WIDE", VF, [20, 50, 80, 110], span=2,
        color="03A598")

# Page three: the text box, in its three shapes and its faces. The text is the
# value's label, set over SysEx or from a script; a box with none draws a dash.
page(3, "TEXT BOX")
control(3, 0, "textBox", "DEFAULT", ONE, [0])
control(3, 1, "textBox", "VALUE ONLY", ONE, [0], variant="valueOnly")
control(3, 2, "textBox", "BASELINE", ONE, [0], variant="baseline")
control(3, 3, "textBox", "LARGE", ONE, [0], font={"size": "large"})
control(3, 4, "textBox", "MONO", ONE, [0],
        font={"size": "medium", "spacing": "monospaced"})
control(3, 5, "textBox", "SMALL BOLD", ONE, [0],
        font={"size": "small", "weight": "bold"})
# A row of thin faders and lists beside a baseline box, to see the shared line.
control(3, 6, "textBox", "ON THE LINE", ONE, [0], variant="baseline",
        color="F49500")
control(3, 7, "fader", "THIN", ONE, [88], variant="thin")
control(3, 8, "textBox", "ON THE LINE", ONE, [0], variant="baseline",
        color="03A598")
control(3, 9, "fader", "THIN", ONE, [40], variant="thin")
control(3, 10, "textBox", "WIDE", ONE, [0], span=2, color="C44795")

# Page four: the custom control. Everything it draws comes from the preset's
# Lua script; a page of them with no script is the case that used to keep
# whatever pixels the last component left behind.
page(4, "CUSTOM")
for index, colour in enumerate(COLOURS):
    control(4, index, "custom", f"CUSTOM {index + 1}", ONE, [64 + index * 8],
            color=colour)
control(4, 6, "custom", "WIDE", ONE, [64], span=3, color="F49500")
control(4, 9, "custom", "TALL", ONE, [96], span=3)

preset = {
    "version": 2,
    "name": "CONTROLS 2",
    "projectId": "controls2",
    "pages": pages,
    "devices": [
        {"id": 1, "name": "Generic MIDI", "port": 1, "channel": 1},
    ],
    "overlays": [],
    "groups": [],
    "controls": controls,
}

json.dump(preset, sys.stdout, separators=(",", ":"))
