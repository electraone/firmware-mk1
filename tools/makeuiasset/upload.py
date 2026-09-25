#!/usr/bin/env python3
"""Sends the UI asset to a connected unit.

The asset is not a preset or a script, so it does not go through the preset
upload the electraone CLI offers. It goes through the toolkit file transfer
the firmware has for exactly this, which takes the file base64 encoded because
system exclusive cannot carry a byte with the top bit set.

    tools/makeuiasset/upload.py assets/ui-1.0.0.bmp

The unit writes it to the card under the name the running firmware expects, so
flash the firmware that knows about this asset before sending it.
"""

from __future__ import annotations

import argparse
import base64
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

# Electra, then upload, then the toolkit file.
MANUFACTURER = [0x00, 0x21, 0x45]
COMMAND_FILE_UPLOAD = 0x01
OBJECT_FILE_UI = 0x70

# The firmware skips two bytes between the object and the data. They carry
# nothing this tool needs to say, and it does not look at them.
UI_HEADER = [0x00, 0x00]

PORT = "Electra Controller Electra CTRL"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("asset", type=Path)
    parser.add_argument("--port", default=PORT)
    arguments = parser.parse_args()

    sendmidi = shutil.which("sendmidi")
    if not sendmidi:
        sys.exit("sendmidi was not found: brew install gbevin/tools/sendmidi")

    if not arguments.asset.exists():
        sys.exit(f"{arguments.asset} does not exist")

    encoded = base64.b64encode(arguments.asset.read_bytes())
    payload = bytes(
        MANUFACTURER + [COMMAND_FILE_UPLOAD, OBJECT_FILE_UI] + UI_HEADER
    ) + encoded

    if any(byte > 0x7F for byte in payload):
        sys.exit("the payload has a byte above 0x7F, which cannot be sent")

    print(
        f"{arguments.asset}: {arguments.asset.stat().st_size} bytes, "
        f"{len(payload)} encoded"
    )

    # Far too large for a command line, so it goes through a file.
    with tempfile.NamedTemporaryFile(suffix=".syx") as handle:
        handle.write(b"\xF0" + payload + b"\xF7")
        handle.flush()

        completed = subprocess.run(
            [sendmidi, "dev", arguments.port, "syf", handle.name],
            capture_output=True,
            text=True,
        )

    if completed.returncode != 0:
        sys.exit("sendmidi failed: " + (completed.stderr or completed.stdout))

    print("sent. The unit writes it to the card and reloads on the next boot.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
