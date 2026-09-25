"""Stimulus scenarios for the compatibility harness.

A scenario is a named list of steps replayed against the unit. The point is
that the same steps run against any firmware version, so the recordings can be
compared. Adding a scenario is cheap; removing or changing one invalidates
every baseline recorded with it, so change them by adding instead.
"""

from __future__ import annotations

from dataclasses import dataclass, field

from electra_midi import PORT_CTRL, PORT_ONE


@dataclass
class Step:
    """One stimulus. `send` is passed to sendmidi verbatim."""

    label: str
    send: list[str] | None = None
    port: str = PORT_ONE
    settle: float = 0.5


@dataclass
class Scenario:
    name: str
    description: str
    steps: list[Step] = field(default_factory=list)
    # Seconds to wait after the last step before reading the trace.
    settle: float = 1.5


def _control_change_sweep() -> list[Step]:
    steps = []
    for value in (0, 1, 63, 64, 126, 127):
        steps.append(
            Step(
                label=f"control change 7 = {value}",
                send=["ch", "1", "cc", "7", str(value)],
            )
        )
    return steps


SCENARIOS: dict[str, Scenario] = {
    "boot": Scenario(
        name="boot",
        description=(
            "Reset the unit and record everything it does while starting up "
            "and loading the stored preset. No stimulus is sent."
        ),
        steps=[],
        settle=6.0,
    ),
    "cc-sweep": Scenario(
        name="cc-sweep",
        description=(
            "Send control change 7 across its range on channel 1 and record "
            "how each value is processed and what is sent back."
        ),
        steps=_control_change_sweep(),
    ),
    "cc-channels": Scenario(
        name="cc-channels",
        description=(
            "Send the same control change on several channels to check "
            "channel handling and parameter map routing."
        ),
        steps=[
            Step(
                label=f"channel {channel} control change 10 = 100",
                send=["ch", str(channel), "cc", "10", "100"],
            )
            for channel in (1, 2, 16)
        ],
    ),
    "notes": Scenario(
        name="notes",
        description=(
            "Play a note on and note off pair, and a pitch bend, to cover the "
            "message types that bypass the outgoing queue."
        ),
        steps=[
            Step(label="note on", send=["ch", "1", "on", "60", "100"]),
            Step(label="note off", send=["ch", "1", "off", "60", "0"]),
            Step(label="pitch bend centre", send=["ch", "1", "pb", "8192"]),
            Step(label="pitch bend low", send=["ch", "1", "pb", "0"]),
        ],
    ),
    "program-change": Scenario(
        name="program-change",
        description="Send program changes, which select presets or patches.",
        steps=[
            Step(label="program change 0", send=["ch", "1", "pc", "0"]),
            Step(label="program change 5", send=["ch", "1", "pc", "5"]),
        ],
    ),
    "nrpn": Scenario(
        name="nrpn",
        description=(
            "Send an NRPN as its four constituent control changes and check "
            "the firmware reassembles it the same way."
        ),
        steps=[
            Step(label="NRPN MSB", send=["ch", "1", "cc", "99", "1"]),
            Step(label="NRPN LSB", send=["ch", "1", "cc", "98", "20"]),
            Step(label="data entry MSB", send=["ch", "1", "cc", "6", "64"]),
            Step(label="data entry LSB", send=["ch", "1", "cc", "38", "0"]),
        ],
    ),
    "ctrl-port": Scenario(
        name="ctrl-port",
        description=(
            "Exercise the CTRL port, which carries the Electra system "
            "exclusive protocol and the logger."
        ),
        steps=[
            Step(
                label="request firmware information",
                send=["syx", "hex", "00", "21", "45", "02", "7F"],
                port=PORT_CTRL,
            ),
        ],
    ),
}


def get(name: str) -> Scenario:
    if name not in SCENARIOS:
        raise KeyError(
            f"unknown scenario {name!r}. Available: "
            + ", ".join(sorted(SCENARIOS))
        )
    return SCENARIOS[name]
