#!/usr/bin/env python3
"""Measures the same presets on two firmware images and prints the difference.

This is the answer to "what did moving that data into the display's memory
actually save". It flashes an image, switches the unit through the preset
slots to be measured, and reads the firmware's memory snapshot straight out of
target RAM over the probe. Then it does the same for the other image and puts
the two side by side.

    python3 tools/compat/compare_memory.py \\
        --before /path/to/old/build --after build \\
        --slot 2:stress-mixed --slot 3:stress-max

The presets have to be on the card already, in the slots named here. Upload
them first with the electraone CLI:

    electraone --port "Electra CTRL" preset switch -b 0 -s 2
    electraone --port "Electra CTRL" preset upload presets/stress-mixed.epr
    electraone --port "Electra CTRL" lua upload presets/stress-mixed.lua

The figures come out of target RAM rather than the trace, so this works on the
production build. That matters: a traced build spends tens of kilobytes on its
buffer, which is the resource being measured, and the firmware refuses a
preset that would leave less than 62000 bytes free.
"""

from __future__ import annotations

import argparse
import sys
import time
from pathlib import Path

import electra_jlink as jlink_module
import electra_midi
import electra_preset
from measure_memory import SNAPSHOT_SYMBOL, read_snapshot


def measure(jlink, address: int, slot: int, timeout: float = 180.0):
    """Switches to a slot and waits for the firmware to report it loaded.

    The snapshot carries an update counter, so a stale reading from the
    previous slot cannot be mistaken for this one.
    """
    before = read_snapshot(jlink, address)
    electra_preset.select_slot(slot)

    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        time.sleep(5.0)
        try:
            snapshot = read_snapshot(jlink, address)
        except jlink_module.JLinkError:
            continue
        if snapshot.updates != before.updates and (
            snapshot.stage == "preset-loaded"
        ):
            return snapshot
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--before", required=True, help="build directory")
    parser.add_argument("--after", required=True, help="build directory")
    parser.add_argument(
        "--slot",
        action="append",
        required=True,
        metavar="SLOT:NAME",
        help="a slot to measure and what to call it, repeatable",
    )
    parser.add_argument("--serial", default=None)
    arguments = parser.parse_args()

    slots = []
    for entry in arguments.slot:
        number, _, name = entry.partition(":")
        slots.append((int(number), name or f"slot {number}"))

    jlink = jlink_module.JLink(serial=arguments.serial)
    results: dict[str, dict[str, object]] = {}

    for image in ("before", "after"):
        directory = Path(getattr(arguments, image)).resolve()
        print(f"\n=== {image}: {directory}")

        jlink.flash(directory / "firmware.hex")
        electra_midi.wait_for_port(electra_midi.PORT_CTRL, timeout=90)
        time.sleep(5.0)

        address = jlink_module.read_elf_symbols(
            directory / "firmware.elf", {SNAPSHOT_SYMBOL}
        )[SNAPSHOT_SYMBOL]

        for number, name in slots:
            snapshot = measure(jlink, address, number)
            if snapshot is None:
                print(f"  {name}: never reported as loaded")
                continue
            print(f"  {name}: {snapshot}")
            results.setdefault(name, {})[image] = snapshot

    print()
    print(f"{'preset':<16}{'free before':>13}{'free after':>12}{'saved':>9}")
    for _number, name in slots:
        pair = results.get(name, {})
        if "before" in pair and "after" in pair:
            before = pair["before"].free_ram
            after = pair["after"].free_ram
            print(f"{name:<16}{before:>13}{after:>12}{after - before:>9}")
        else:
            print(f"{name:<16}{'incomplete':>13}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
