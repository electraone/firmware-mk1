"""J-Link access used by the compatibility harness.

The trace lives in a SEGGER RTT up-buffer in target RAM. Rather than relying
on the SEGGER RTT host tools, which need to locate the control block by
searching memory and do not find it reliably on this part, the buffer is read
straight out of target RAM through J-Link Commander. That is deterministic and
does not perturb the timing of the firmware under test.
"""

from __future__ import annotations

import re
import shutil
import struct
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path

DEVICE = "MK66FX1M0XXX18"
INTERFACE = "SWD"
SPEED = 4000

# Offsets inside the SEGGER RTT control block: a 16 byte identifier, the two
# buffer counts, then the up-buffer descriptors.
_ID_LENGTH = 16
_UP_BUFFER_ARRAY_OFFSET = _ID_LENGTH + 8
_UP_BUFFER_FORMAT = "<IIIIII"  # sName, pBuffer, size, wrOff, rdOff, flags
_UP_BUFFER_SIZE = 24

# The firmware puts the trace on channel 1, because SEGGER only honours a
# caller supplied buffer for a non-zero channel.
TRACE_CHANNEL = 1
_UP_BUFFER_OFFSET = _UP_BUFFER_ARRAY_OFFSET + TRACE_CHANNEL * _UP_BUFFER_SIZE


class JLinkError(RuntimeError):
    pass


@dataclass
class UpBuffer:
    address: int
    size: int
    write_offset: int
    read_offset: int


def find_jlink_exe() -> str:
    found = shutil.which("JLinkExe")
    if found:
        return found
    fallback = Path("/Applications/SEGGER/JLink/JLinkExe")
    if fallback.exists():
        return str(fallback)
    raise JLinkError("JLinkExe was not found on this machine")


class JLink:
    """Runs one J-Link Commander script per call.

    Each call is a separate process. That is slower than holding a session
    open, but it keeps the probe free between steps and avoids a stuck
    session leaving the target halted.
    """

    def __init__(self, serial: str | None = None, speed: int = SPEED):
        self.exe = find_jlink_exe()
        self.serial = serial
        self.speed = speed

    def run(self, commands: list[str], timeout: int = 120) -> str:
        script = "\n".join(commands + ["qui", ""])
        with tempfile.NamedTemporaryFile(
            "w", suffix=".jlink", delete=False
        ) as handle:
            handle.write(script)
            script_path = handle.name

        argv = [self.exe, "-nogui", "1"]
        if self.serial:
            argv += ["-SelectEmuBySN", self.serial]
        argv += [
            "-device",
            DEVICE,
            "-if",
            INTERFACE,
            "-speed",
            str(self.speed),
            "-autoconnect",
            "1",
            "-CommandFile",
            script_path,
        ]

        try:
            completed = subprocess.run(
                argv, capture_output=True, text=True, timeout=timeout
            )
        except subprocess.TimeoutExpired as error:
            raise JLinkError(f"J-Link timed out after {timeout}s") from error
        finally:
            Path(script_path).unlink(missing_ok=True)

        output = completed.stdout + completed.stderr
        if "Cannot connect to target" in output or "too low" in output:
            raise JLinkError(
                "could not reach the target. Is the unit powered and the "
                "SWD cable seated?\n" + output[-800:]
            )
        return output

    def read_memory(self, address: int, length: int) -> bytes:
        """Reads a block of target memory."""
        with tempfile.TemporaryDirectory() as directory:
            dump = Path(directory) / "dump.bin"
            output = self.run(
                [
                    "connect",
                    f"savebin {dump} 0x{address:08X} 0x{length:X}",
                    "g",
                ]
            )
            if not dump.exists():
                raise JLinkError(
                    f"reading 0x{address:08X} failed\n" + output[-800:]
                )
            return dump.read_bytes()

    def write_word(self, address: int, value: int) -> None:
        self.run(["connect", f"w4 0x{address:08X}, 0x{value:08X}", "g"])

    def reset_and_run(self) -> None:
        self.run(["connect", "r", "g"])

    def flash(self, hex_path: Path) -> str:
        output = self.run(
            ["connect", f"loadfile {hex_path}", "r", "g"], timeout=300
        )
        if "O.K." not in output:
            raise JLinkError("flashing failed\n" + output[-1500:])
        return output


def read_elf_symbols(elf_path: Path, names: set[str]) -> dict[str, int]:
    """Looks up symbol addresses in the firmware ELF."""
    objdump = shutil.which("arm-none-eabi-objdump") or (
        "/usr/local/arm/bin/arm-none-eabi-objdump"
    )
    completed = subprocess.run(
        [objdump, "-t", str(elf_path)], capture_output=True, text=True
    )
    if completed.returncode != 0:
        raise JLinkError(f"could not read symbols from {elf_path}")

    found: dict[str, int] = {}
    for line in completed.stdout.splitlines():
        match = re.match(r"^([0-9a-fA-F]{8})\s.*\s(\S+)$", line)
        if match and match.group(2) in names:
            found[match.group(2)] = int(match.group(1), 16)

    missing = names - found.keys()
    if missing:
        raise JLinkError(
            "the firmware ELF is missing "
            + ", ".join(sorted(missing))
            + ". Was it built with TRACE=1?"
        )
    return found


def read_up_buffer(jlink: JLink, control_block_address: int) -> UpBuffer:
    """Reads the RTT control block and returns the trace up-buffer state."""
    raw = jlink.read_memory(
        control_block_address, _UP_BUFFER_OFFSET + _UP_BUFFER_SIZE
    )

    if not raw.startswith(b"SEGGER RTT"):
        raise JLinkError(
            "the RTT control block is not initialised. The firmware either "
            "was not built with TRACE=1 or has not booted yet."
        )

    fields = struct.unpack_from(_UP_BUFFER_FORMAT, raw, _UP_BUFFER_OFFSET)
    _name, buffer_address, size, write_offset, read_offset, _flags = fields
    return UpBuffer(buffer_address, size, write_offset, read_offset)


def drain_trace(
    jlink: JLink, control_block_address: int, advance: bool = True
) -> tuple[bytes, UpBuffer]:
    """Reads everything the target has written since the last drain.

    Unless `advance` is cleared, the read offset is moved up afterwards so the
    target may reuse the space and a later drain returns only new records.
    """
    state = read_up_buffer(jlink, control_block_address)

    if state.write_offset == state.read_offset:
        return b"", state

    raw = jlink.read_memory(state.address, state.size)

    if state.write_offset > state.read_offset:
        payload = raw[state.read_offset : state.write_offset]
    else:
        payload = raw[state.read_offset :] + raw[: state.write_offset]

    if advance:
        # The read offset is the fifth word of the up-buffer descriptor.
        read_offset_address = (
            control_block_address + _UP_BUFFER_OFFSET + 4 * 4
        )
        jlink.write_word(read_offset_address, state.write_offset)

    return payload, state
