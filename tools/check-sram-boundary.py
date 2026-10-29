#!/usr/bin/env python3
"""Fail the build when a static object straddles 0x20000000.

The K66 reaches SRAM_L below that address over the code bus and SRAM_U at and
above it over the system bus. A single access spanning the two - a memcpy
burst, a struct copy - raises a bus error that escalates to a hard fault. The
linker lays .data and .bss straight across the boundary, so which object
straddles it depends only on how much static data precedes it, and any change
anywhere can move a large buffer onto it. Run on every link.

    check-sram-boundary.py <objdump> <elf>
"""

import subprocess
import sys

BOUNDARY = 0x20000000


def main() -> int:
    objdump, elf = sys.argv[1], sys.argv[2]
    table = subprocess.run(
        [objdump, "-t", elf], check=True, capture_output=True, text=True
    ).stdout

    crossing = []

    for line in table.splitlines():
        fields = line.split()

        # address flags... section size name
        for index, field in enumerate(fields):
            if field.startswith((".bss", ".data", ".noinit", ".dmabuffers",
                                 ".usbbuffers")):
                break
        else:
            continue

        try:
            address = int(fields[0], 16)
            size = int(fields[index + 1], 16)
        except (ValueError, IndexError):
            continue

        if address < BOUNDARY < address + size:
            crossing.append(f"  {fields[-1]} at 0x{address:08x}, {size} bytes")

    if crossing:
        print("error: static objects straddle the SRAM bank boundary "
              "0x20000000:", file=sys.stderr)
        print("\n".join(crossing), file=sys.stderr)
        print("Place them in .bss.lowram, or align them to their own size.",
              file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
