"""Uploading presets and Lua scripts to the unit over the Electra SysEx API.

A preset and its Lua script are separate uploads. Both land in the slot the
unit currently has selected, so switch to the slot you want to overwrite
before uploading.

Where the electraone CLI is available it does the work, because it streams a
large preset far faster than handing the whole thing to sendmidi does, and it
waits for the unit to acknowledge instead of guessing. Without it the upload
falls back to sendmidi, which still works but takes minutes on a preset of a
few hundred kilobytes rather than seconds.

These are the direct uploads, not the File Transfer API, which the mk1
firmware does not implement.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path

from electra_midi import PORT_CTRL, MidiError, _require

# Electra manufacturer identifier, then the command and object bytes.
MANUFACTURER = [0x00, 0x21, 0x45]
COMMAND_FILE_UPLOAD = 0x01
COMMAND_SWITCH = 0x09
OBJECT_PRESET = 0x01
OBJECT_PRESET_SLOT = 0x08
OBJECT_LUA = 0x0C

# Beyond this the message goes through a file. The limit is the operating
# system's on the length of a command line, not the unit's.
_MAX_INLINE_BYTES = 2048

# Where to look for the electraone CLI: an explicit path first, then the
# usual one, then wherever it was built.
_CLI_CANDIDATES = [
    os.environ.get("ELECTRAONE_CLI"),
    "electraone",
    str(Path.home() / "Repos/electraone-cli/build/electraone"),
]

# The unit takes a moment to answer a large upload, and it has to write the
# file to the card before it does.
_CLI_TIMEOUT_MS = 60000


def find_cli() -> str | None:
    """The electraone CLI, if this machine has one."""
    for candidate in _CLI_CANDIDATES:
        if not candidate:
            continue
        found = shutil.which(candidate)
        if found:
            return found
        if Path(candidate).is_file():
            return candidate
    return None


def _run_cli(arguments: list[str]) -> None:
    cli = find_cli()
    assert cli is not None

    completed = subprocess.run(
        [cli, "--port", _CLI_PORT, "--timeout", str(_CLI_TIMEOUT_MS)]
        + arguments,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        raise MidiError(
            "the electraone CLI failed: "
            + (completed.stderr or completed.stdout).strip()
        )


# Matched against the port names. The unit shows three cables and the
# Electra protocol lives on the control one.
_CLI_PORT = "Electra CTRL"


def _send_sysex(payload: bytes, port: str = PORT_CTRL) -> None:
    """Sends one system exclusive message, without the F0/F7 wrapper.

    sendmidi adds the framing itself, so `payload` is the content only.
    """
    if any(byte > 0x7F for byte in payload):
        raise MidiError(
            "the payload contains a byte above 0x7F, which cannot travel "
            "inside a system exclusive message"
        )

    sendmidi = _require("sendmidi")

    if len(payload) <= _MAX_INLINE_BYTES:
        arguments = [sendmidi, "dev", port, "syx", "hex"]
        arguments += [f"{byte:02X}" for byte in payload]
        completed = subprocess.run(arguments, capture_output=True, text=True)
    else:
        # A preset is far too large to pass on a command line, so it goes
        # through a file instead.
        with tempfile.NamedTemporaryFile(suffix=".syx") as handle:
            handle.write(b"\xF0" + payload + b"\xF7")
            handle.flush()
            completed = subprocess.run(
                [sendmidi, "dev", port, "syf", handle.name],
                capture_output=True,
                text=True,
            )

    if completed.returncode != 0:
        raise MidiError(
            "sendmidi failed: " + (completed.stderr or completed.stdout)
        )


def select_slot(slot: int, port: str = PORT_CTRL) -> None:
    """Switches the unit to a preset slot, which is where uploads land.

    Slots run 0 to 71 and are addressed as a bank of twelve plus a position
    inside it, the way the unit's own preset grid is arranged.
    """
    if not 0 <= slot <= 71:
        raise MidiError(f"slot {slot} is outside the 0 to 71 the unit has")

    if find_cli():
        _run_cli(
            ["preset", "switch", "-b", str(slot // 12), "-s", str(slot % 12)]
        )
        return

    payload = bytes(
        MANUFACTURER
        + [COMMAND_SWITCH, OBJECT_PRESET_SLOT, slot // 12, slot % 12]
    )
    _send_sysex(payload, port)


def upload_preset(preset_json: str, port: str = PORT_CTRL) -> int:
    """Uploads a preset into the currently selected slot.

    Returns the number of payload bytes sent.
    """
    body = preset_json.encode("ascii")

    if find_cli():
        _upload_through_cli(["preset", "upload"], body, ".epr")
        return len(body)

    payload = bytes(MANUFACTURER + [COMMAND_FILE_UPLOAD, OBJECT_PRESET]) + body
    _send_sysex(payload, port)
    return len(body)


def upload_lua(lua_source: str, port: str = PORT_CTRL) -> int:
    """Uploads a Lua script for the currently selected slot."""
    body = lua_source.encode("ascii")

    if find_cli():
        _upload_through_cli(["lua", "upload"], body, ".lua")
        return len(body)

    payload = bytes(MANUFACTURER + [COMMAND_FILE_UPLOAD, OBJECT_LUA]) + body
    _send_sysex(payload, port)
    return len(body)


def _upload_through_cli(
    command: list[str], body: bytes, suffix: str
) -> None:
    """The CLI takes a file, and the caller has the content in hand."""
    with tempfile.NamedTemporaryFile(suffix=suffix) as handle:
        handle.write(body)
        handle.flush()
        _run_cli(command + [handle.name])


def load(directory: Path, name: str) -> tuple[str, str]:
    """Reads a preset and its Lua script from a directory."""
    preset = (directory / f"{name}.epr").read_text()
    lua_path = directory / f"{name}.lua"
    lua = lua_path.read_text() if lua_path.exists() else ""
    return preset, lua
