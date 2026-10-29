#!/usr/bin/env python3
"""Builds a deliberately large preset, so a saving can be seen rather than
argued about.

The unit has a quarter of a megabyte of RAM and a preset is by far the largest
thing in it, so the honest way to show what moving data into the display's
memory is worth is to fill the unit up and read the free-RAM figure on either
side of the change. A small preset would hide a saving inside the noise of
whatever else happened to be allocated.

Two profiles:

  mixed   what a heavy but believable preset looks like. Faders, lists with
          long overlays, and envelopes, spread over all twelve pages.
  max     every control an envelope, which is the declared ceiling of values
          a preset can hold: 432 controls, four values each.
  sysex   what a synth editor looks like: every control a fader whose value
          goes out as a SysEx message with a template of its own, the way
          the Access Virus preset does it. 432 controls, 432 values, 432
          templates. It is the SysEx templates this one is there to weigh.

Both fill twelve pages with thirty-six controls each by default, which is the
declared maximum of 432. The difference between them is how many ControlValues
that comes to, which is what the value label saving scales with.

A preset that leaves the unit with less than 62000 bytes free is refused by
the firmware as too large, so --pages is there to bring one back under that
ceiling when it will not load.

    python3 tools/compat/make_stress_preset.py --profile mixed --pages 8
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

PAGES = 12
CONTROLS_PER_PAGE = 36
COLUMNS = 6
ROWS = 6

# The mk1 screen, as the stock presets lay it out.
SLOT_WIDTH = 146
SLOT_HEIGHT = 56
FIRST_X = 20
FIRST_Y = 28
STEP_X = 167
STEP_Y = 88

COLOURS = ["FFFFFF", "F45C51", "529DEC", "03A598", "C44795", "F49500"]

# Long enough to be realistic and to make the per-instance cost visible. The
# firmware truncates a group label at 40 characters and a control name at 15.
GROUP_LABEL = "GROUP WITH A DELIBERATELY LONG LABEL 1234"

ENVELOPE_VALUE_IDS = ["attack", "decay", "sustain", "release"]


def bounds(slot: int) -> list[int]:
    column = slot % COLUMNS
    row = slot // COLUMNS
    return [
        FIRST_X + column * STEP_X,
        FIRST_Y + row * STEP_Y,
        SLOT_WIDTH,
        SLOT_HEIGHT,
    ]


def overlay(overlay_id: int, items: int) -> dict:
    """A list of items, each with a label of a length that occurs in real
    presets. These already live in the display's memory, so they are here to
    keep the preset representative rather than to be saved again."""
    return {
        "id": overlay_id,
        "items": [
            {
                "value": index,
                "label": f"OVERLAY {overlay_id} ITEM {index:03d} LABEL",
            }
            for index in range(items)
        ],
    }


def value(
    value_id: str,
    device_id: int,
    parameter_number: int,
    formatter: str | None,
    overlay_id: int | None,
) -> dict:
    entry = {
        "id": value_id,
        "min": 0,
        "max": 127,
        "defaultValue": 0,
        "message": {
            "deviceId": device_id,
            "type": "cc7",
            "parameterNumber": parameter_number,
            "min": 0,
            "max": 127,
        },
    }
    if formatter:
        entry["formatter"] = formatter
    if overlay_id is not None:
        entry["overlayId"] = overlay_id
        entry["max"] = 63
        entry["message"]["max"] = 63
    return entry


def sysex_value(control_id: int, device_id: int, parameter_number: int) -> dict:
    """A value sent as SysEx, with a template of the shape the Access Virus
    preset uses: a fixed header, the parameter's page and number as bytes of
    the template itself, the value, and the end byte. Every control gets a
    template that differs from every other one in those two bytes, so there
    is nothing for the firmware to share between them - which is what a real
    editor looks like, and what the template storage has to carry."""
    return {
        "id": "value",
        "min": 0,
        "max": 127,
        "defaultValue": 0,
        "message": {
            "deviceId": device_id,
            "type": "sysex",
            "parameterNumber": parameter_number,
            "min": 0,
            "max": 127,
            "data": [
                "00",
                "20",
                "33",
                "01",
                "10",
                f"{0x70 + (control_id >> 7):02X}",
                f"{control_id & 0x7F:02X}",
                {"type": "value"},
            ],
        },
    }


def control(
    control_id: int,
    page_id: int,
    slot: int,
    kind: str,
    device_id: int,
    parameter_base: int,
    overlay_id: int | None,
    sysex: bool = False,
) -> dict:
    entry = {
        "id": control_id,
        "type": kind,
        "name": f"CONTROL {control_id:03d}"[:15],
        "color": COLOURS[control_id % len(COLOURS)],
        "bounds": bounds(slot),
        "pageId": page_id,
        "controlSetId": (slot // 12) + 1,
        "visible": True,
        "mode": "unipolar",
        "inputs": [{"potId": (slot % 12) + 1, "valueId": "value"}],
        "values": [],
    }

    if kind == "adsr":
        entry["inputs"] = [
            {"potId": (slot % 12) + 1, "valueId": ENVELOPE_VALUE_IDS[0]}
        ]
        for index, value_id in enumerate(ENVELOPE_VALUE_IDS):
            entry["values"].append(
                value(
                    value_id,
                    device_id,
                    (parameter_base + index) % 120 + 1,
                    # One formatter per envelope, not four: a formatter runs a
                    # Lua call on every value change, and the point here is to
                    # prove the label path works, not to measure Lua.
                    "formatValue" if index == 0 else None,
                    None,
                )
            )
    elif kind == "list":
        entry["values"].append(
            value("value", device_id, parameter_base % 120 + 1, None, overlay_id)
        )
    elif sysex:
        # The parameter number is what the parameter map files the value
        # under; distinct per control, so every value has an entry of its own.
        entry["values"].append(sysex_value(control_id, device_id, control_id))
    else:
        entry["values"].append(
            value(
                "value",
                device_id,
                parameter_base % 120 + 1,
                "formatValue",
                None,
            )
        )

    return entry


def make_preset(profile: str, overlay_items: int, pages: int) -> dict:
    devices = [
        {
            "id": device_id,
            "name": f"DEVICE NUMBER {device_id:02d}",
            "port": ((device_id - 1) % 2) + 1,
            "channel": ((device_id - 1) % 16) + 1,
            "rate": 20,
        }
        for device_id in range(1, 5)
    ]

    page_list = [
        {"id": page_id, "name": f"PAGE {page_id:02d} OF TWELVE"[:20],
         "defaultControlSetId": 1}
        for page_id in range(1, pages + 1)
    ]

    overlays = [overlay(overlay_id, overlay_items) for overlay_id in (1, 2, 3)]

    groups = []
    controls = []
    group_id = 1
    control_id = 1

    for page_id in range(1, pages + 1):
        for row in range(ROWS):
            groups.append(
                {
                    "id": group_id,
                    "pageId": page_id,
                    "bounds": [FIRST_X, FIRST_Y + row * STEP_Y - 22, 970, 20],
                    # "name", which is what the firmware reads; a "label" key
                    # is ignored, and the groups came out blank.
                    "name": f"{GROUP_LABEL[:34]} P{page_id:02d}R{row}",
                    "color": COLOURS[row % len(COLOURS)],
                }
            )
            group_id += 1

        for slot in range(CONTROLS_PER_PAGE):
            if profile == "max":
                kind = "adsr"
                overlay_id = None
            elif profile == "sysex":
                kind = "fader"
                overlay_id = None
            else:
                # Twelve of each per page, so every path the painters take is
                # represented at scale.
                which = slot // 12
                kind = ["fader", "list", "adsr"][which]
                overlay_id = (slot % 3) + 1 if kind == "list" else None

            controls.append(
                control(
                    control_id,
                    page_id,
                    slot,
                    kind,
                    ((control_id - 1) % 4) + 1,
                    control_id * 4,
                    overlay_id,
                    sysex=(profile == "sysex"),
                )
            )
            control_id += 1

    return {
        "version": 2,
        "name": f"STRESS {profile.upper()}"[:20],
        "projectId": f"stress-{profile}",
        "pages": page_list,
        "devices": devices,
        "overlays": overlays,
        "groups": groups,
        "controls": controls,
    }


LUA = """-- Formatter for the stress preset.
--
-- Every fader and every envelope attack runs this on each value change, so it
-- is what writes the value labels the measurement is about.

function formatValue (valueObject, value)
   return (string.format("%d%%", math.floor(value * 100 / 127)))
end
"""


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--profile", choices=["mixed", "max", "sysex"], default="mixed"
    )
    parser.add_argument(
        "--pages",
        type=int,
        default=PAGES,
        help="pages to fill, each with 36 controls. Fewer if the firmware "
        "under test refuses the preset as too large",
    )
    parser.add_argument(
        "--overlay-items",
        type=int,
        default=64,
        help="items in each of the three overlays",
    )
    parser.add_argument(
        "--directory",
        type=Path,
        default=Path(__file__).parent / "presets",
        help="where to write the preset and its Lua script",
    )
    arguments = parser.parse_args()

    preset = make_preset(
        arguments.profile, arguments.overlay_items, arguments.pages
    )
    name = f"stress-{arguments.profile}"

    preset_path = arguments.directory / f"{name}.epr"
    lua_path = arguments.directory / f"{name}.lua"

    # Compact, because the whole thing travels to the unit over MIDI and
    # then gets parsed on a 180 MHz part. Whitespace costs time at both ends
    # and changes nothing about what ends up in RAM.
    preset_path.write_text(json.dumps(preset, separators=(",", ":")))
    lua_path.write_text(LUA)

    values = sum(len(entry["values"]) for entry in preset["controls"])
    print(f"{preset_path}: {preset_path.stat().st_size} bytes")
    print(
        f"  {len(preset['controls'])} controls, {values} values, "
        f"{len(preset['groups'])} groups, "
        f"{sum(len(o['items']) for o in preset['overlays'])} list items"
    )
    print(f"  value labels moved out of RAM: {values * 12} bytes")


if __name__ == "__main__":
    main()
