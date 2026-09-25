"""Uploading presets and Lua scripts to the unit over the Electra SysEx API.

A preset and its Lua script are separate uploads. Both land in the slot the
unit currently has selected, so switch to the slot you want to overwrite
before uploading.
"""

from __future__ import annotations

import subprocess
from pathlib import Path

from electra_midi import PORT_CTRL, MidiError, _require

# Electra manufacturer identifier, then the command and object bytes.
MANUFACTURER = [0x00, 0x21, 0x45]
COMMAND_FILE_UPLOAD = 0x01
OBJECT_PRESET = 0x01
OBJECT_LUA = 0x0C


def _send_sysex(payload: bytes, port: str = PORT_CTRL) -> None:
    """Sends one system exclusive message, without the F0/F7 wrapper.

    sendmidi adds the framing itself, so `payload` is the content only.
    """
    if any(byte > 0x7F for byte in payload):
        raise MidiError(
            "the payload contains a byte above 0x7F, which cannot travel "
            "inside a system exclusive message"
        )

    arguments = [_require("sendmidi"), "dev", port, "syx", "hex"]
    arguments += [f"{byte:02X}" for byte in payload]

    completed = subprocess.run(arguments, capture_output=True, text=True)
    if completed.returncode != 0:
        raise MidiError(
            "sendmidi failed: " + (completed.stderr or completed.stdout)
        )


def upload_preset(preset_json: str, port: str = PORT_CTRL) -> int:
    """Uploads a preset into the currently selected slot.

    Returns the number of payload bytes sent.
    """
    body = preset_json.encode("ascii")
    payload = bytes(MANUFACTURER + [COMMAND_FILE_UPLOAD, OBJECT_PRESET]) + body
    _send_sysex(payload, port)
    return len(body)


def upload_lua(lua_source: str, port: str = PORT_CTRL) -> int:
    """Uploads a Lua script for the currently selected slot."""
    body = lua_source.encode("ascii")
    payload = bytes(MANUFACTURER + [COMMAND_FILE_UPLOAD, OBJECT_LUA]) + body
    _send_sysex(payload, port)
    return len(body)


def load(directory: Path, name: str) -> tuple[str, str]:
    """Reads a preset and its Lua script from a directory."""
    preset = (directory / f"{name}.epr").read_text()
    lua_path = directory / f"{name}.lua"
    lua = lua_path.read_text() if lua_path.exists() else ""
    return preset, lua
