#!/usr/bin/env python3
"""Measures what a preset costs in RAM, on a given firmware image.

Uploads a preset, restarts the unit so the preset is loaded off the card the
way it is on a normal boot, and reads back the memory snapshot the firmware
leaves in RAM once the preset and its Lua script are in place. Run it against
two images with the same preset and the difference is what a change is worth.

    python3 tools/compat/measure_memory.py \\
        --hex ../../build/firmware.hex \\
        --elf ../../build/firmware.elf \\
        --preset stress-mixed --slot 2

The preset stays on the card, so a second image can be measured against the
same data with --skip-upload, which is both faster and a fairer comparison.

The figures are read straight out of target RAM rather than out of the trace,
so this works on the production build. That matters: a traced build spends
tens of kilobytes on its buffer, which is exactly the resource being measured,
and the firmware refuses a preset that would leave too little free.
"""

from __future__ import annotations

import argparse
import struct
import sys
import time
from pathlib import Path

import electra_jlink as jlink_module
import electra_midi
import electra_preset

SNAPSHOT_SYMBOL = "memorySnapshot"

# char stage[16], then free, heap, vram used, vram refused, update count.
SNAPSHOT_FORMAT = "<16siiIII"
SNAPSHOT_SIZE = struct.calcsize(SNAPSHOT_FORMAT)


class Snapshot:
    def __init__(self, raw: bytes):
        (
            stage,
            self.free_ram,
            self.heap_used,
            self.vram_used,
            self.vram_rejected,
            self.updates,
        ) = struct.unpack(SNAPSHOT_FORMAT, raw)
        self.stage = stage.split(b"\0")[0].decode("ascii", errors="replace")

    def __str__(self) -> str:
        return (
            f"{self.stage}: free={self.free_ram} heap={self.heap_used} "
            f"vram={self.vram_used} refused={self.vram_rejected} "
            f"updates={self.updates}"
        )


def read_snapshot(jlink, address: int) -> Snapshot:
    return Snapshot(jlink.read_memory(address, SNAPSHOT_SIZE))


def wait_for_stage(jlink, address: int, stage: str, timeout: float):
    """Polls target RAM until the firmware reports reaching a stage."""
    deadline = time.monotonic() + timeout

    while time.monotonic() < deadline:
        time.sleep(5.0)
        try:
            snapshot = read_snapshot(jlink, address)
        except jlink_module.JLinkError:
            continue
        if snapshot.stage == stage:
            return snapshot
    return None


REFUSED = (
    "The firmware rejects a preset that would leave less than 62000 bytes "
    "free, so a preset that is too big for the image under test never "
    "reports as loaded."
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--elf", required=True)
    parser.add_argument("--hex", default=None, help="flash this image first")
    parser.add_argument("--preset", default="stress-mixed")
    parser.add_argument("--slot", type=int, default=2)
    parser.add_argument("--serial", default=None)
    parser.add_argument(
        "--skip-upload",
        action="store_true",
        help="the preset is already in the slot from an earlier run",
    )
    parser.add_argument("--timeout", type=float, default=600.0)
    parser.add_argument(
        "--directory", default=str(Path(__file__).parent / "presets")
    )
    arguments = parser.parse_args()

    elf = Path(arguments.elf).resolve()
    symbols = jlink_module.read_elf_symbols(elf, {SNAPSHOT_SYMBOL})
    address = symbols[SNAPSHOT_SYMBOL]
    jlink = jlink_module.JLink(serial=arguments.serial)

    if arguments.hex:
        print(f"flashing {arguments.hex}")
        jlink.flash(Path(arguments.hex).resolve())

    print("waiting for the unit to come back on USB")
    electra_midi.wait_for_port(electra_midi.PORT_CTRL, timeout=90)
    time.sleep(3.0)

    if not arguments.skip_upload:
        preset, lua = electra_preset.load(
            Path(arguments.directory), arguments.preset
        )
        print(
            f"uploading {arguments.preset} into slot {arguments.slot}: "
            f"{len(preset)} bytes of preset, {len(lua)} of Lua"
        )

        electra_preset.select_slot(arguments.slot)
        time.sleep(3.0)
        electra_preset.upload_preset(preset)

        # The unit has to receive it, write it to the card and parse it. How
        # long that takes depends entirely on the size of the preset.
        loaded = wait_for_stage(
            jlink, address, "preset-loaded", arguments.timeout
        )

        if loaded is None:
            print("the uploaded preset never reported as loaded. " + REFUSED)
            return 1

        if lua:
            electra_preset.upload_lua(lua)
            time.sleep(15.0)

    # Measured on a plain boot, with the preset read off the card, so nothing
    # left over from the upload is counted.
    print("restarting the unit to measure a clean load")
    jlink.reset_and_run()
    electra_midi.wait_for_port(electra_midi.PORT_CTRL, timeout=90)

    loaded = wait_for_stage(jlink, address, "preset-loaded", arguments.timeout)

    if loaded is None:
        print("the preset never reported as loaded after the restart. ")
        print(REFUSED)
        print(f"last snapshot: {read_snapshot(jlink, address)}")
        return 1

    print()
    print(f"preset      {arguments.preset}, slot {arguments.slot}")
    print(f"free RAM    {loaded.free_ram} bytes")
    print(f"heap used   {loaded.heap_used} bytes")
    print(
        f"display RAM {loaded.vram_used} bytes used, "
        f"{loaded.vram_rejected} records refused"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
