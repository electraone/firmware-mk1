"""Host side MIDI for the compatibility harness.

Stimuli are sent to the unit and everything it sends back is captured, both
through the gbevin sendmidi and receivemidi command line tools. The capture is
what actually left the USB device port, so it is the authority on outgoing
MIDI; the firmware trace is the authority on ordering.
"""

from __future__ import annotations

import re
import shutil
import subprocess
import time
from dataclasses import dataclass

# The three USB MIDI cables the unit exposes.
PORT_ONE = "Electra Controller Electra Port 1"
PORT_TWO = "Electra Controller Electra Port 2"
PORT_CTRL = "Electra Controller Electra CTRL"

ALL_PORTS = [PORT_ONE, PORT_TWO, PORT_CTRL]


class MidiError(RuntimeError):
    pass


def _require(tool: str) -> str:
    found = shutil.which(tool)
    if not found:
        raise MidiError(
            f"{tool} was not found. Install the gbevin midi tools "
            "(brew install gbevin/tools/{tool})".format(tool=tool)
        )
    return found


def list_ports() -> tuple[list[str], list[str]]:
    outputs = subprocess.run(
        [_require("sendmidi"), "list"], capture_output=True, text=True
    ).stdout.splitlines()
    inputs = subprocess.run(
        [_require("receivemidi"), "list"], capture_output=True, text=True
    ).stdout.splitlines()
    return (
        [line.strip() for line in inputs if line.strip()],
        [line.strip() for line in outputs if line.strip()],
    )


def wait_for_port(name: str, timeout: float = 40.0) -> None:
    """Waits for a port to appear before giving up on it.

    The unit drops off USB whenever it is reset or reflashed and takes a
    moment to come back. Failing immediately would just mean the caller has to
    guess how long to sleep before trying again.
    """
    deadline = time.monotonic() + timeout

    while True:
        inputs, outputs = list_ports()
        if name in inputs and name in outputs:
            return
        if time.monotonic() >= deadline:
            raise MidiError(
                f"the MIDI port {name!r} did not appear within "
                f"{timeout:.0f}s. Is the unit connected over USB?\n"
                "Seen inputs: " + ", ".join(inputs)
            )
        time.sleep(1.0)


def require_port(name: str) -> None:
    wait_for_port(name)


@dataclass
class Capture:
    """Running receivemidi processes collecting what the unit sends.

    All three USB cables are watched at once. Outgoing traffic is split across
    them (the CTRL cable carries the Electra protocol and the logger), so
    watching only one would miss most of what the unit says.
    """

    processes: dict[str, subprocess.Popen]

    def stop(self) -> dict[str, list[str]]:
        captured: dict[str, list[str]] = {}

        for port, process in self.processes.items():
            process.terminate()
            try:
                stdout, _ = process.communicate(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill()
                stdout, _ = process.communicate()
            captured[port] = [
                line.strip() for line in stdout.splitlines() if line.strip()
            ]
        return captured


def start_capture(ports: list[str]) -> Capture:
    processes = {}
    for port in ports:
        processes[port] = subprocess.Popen(
            [_require("receivemidi"), "dev", port],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
        )
    # receivemidi needs a moment before it is actually listening.
    time.sleep(0.8)
    return Capture(processes)


def send(port: str, arguments: list[str]) -> None:
    """Sends one stimulus. Arguments are passed to sendmidi verbatim."""
    completed = subprocess.run(
        [_require("sendmidi"), "dev", port] + arguments,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        raise MidiError(
            "sendmidi failed: " + (completed.stderr or completed.stdout)
        )


_TIMESTAMP = re.compile(r"^\s*\d+:\d+:\d+\.\d+\s+")

def normalise(lines: list[str]) -> list[str]:
    """Strips timestamps so two captures can be compared.

    Nothing else is stripped. What the host heard is what the host heard, and
    where it heard less than the firmware sent, that is for the comparison to
    say rather than for this to hide - see the boot window note in
    electra_compat.
    """
    return [_TIMESTAMP.sub("", line).strip() for line in lines]
