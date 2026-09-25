"""Parsing and normalising of the firmware behavioural trace.

A trace record is one line:

    <seq> <micros> <category> [key=value ...]

Comparison between firmware versions is about behaviour, not about speed, so
timestamps are dropped when a recording is normalised. Sequence numbers are
kept only to prove that nothing was lost.
"""

from __future__ import annotations

import re
from dataclasses import dataclass, field

# Categories the firmware emits. Kept here so an unknown one is reported
# rather than silently ignored, which would hide a new event type.
KNOWN_CATEGORIES = {
    "BOOT": "firmware identification",
    "MI": "MIDI message received",
    "MO": "MIDI message sent",
    "LC": "call into Lua",
    "LR": "return from Lua",
    "LX": "Lua handler not defined",
    "LE": "Lua raised an error",
    "PM": "parameter map change",
    "MARK": "harness marker",
    "LOG": "message the firmware logged",
}

# The firmware's own log text is free form and routinely contains '=', so it
# must not be split into key/value pairs. Only the leading level is a field.
FREE_TEXT_CATEGORIES = {"LOG", "MARK"}

# Fields that legitimately differ between two runs of the same firmware and
# must not be compared.
VOLATILE_FIELDS = {"micros"}

_RECORD = re.compile(r"^(\d+)\s+(\d+)\s+(\S+)(.*)$")


@dataclass
class Record:
    sequence: int
    micros: int
    category: str
    fields: dict[str, str] = field(default_factory=dict)
    text: str = ""
    raw: str = ""

    def signature(self) -> str:
        """The part of a record that must not change between versions."""
        parts = [self.category]
        for key in sorted(self.fields):
            if key in VOLATILE_FIELDS:
                continue
            parts.append(f"{key}={self.fields[key]}")
        if self.text:
            parts.append(self.text)
        return " ".join(parts)

    def describe(self) -> str:
        """A short human readable form used in comparison reports."""
        if self.category == "MI":
            return f"MIDI in  {self._midi()}"
        if self.category == "MO":
            return f"MIDI out {self._midi()}"
        if self.category == "LC":
            return f"Lua call {self._lua()}"
        if self.category == "LR":
            returned = self.fields.get("ret")
            suffix = f" returned {returned}" if returned is not None else ""
            return f"Lua done {self._lua()}{suffix}"
        if self.category == "LX":
            return f"Lua handler not defined: {self._lua()}"
        if self.category == "LE":
            return f"Lua error in {self._lua()}: {self.fields.get('err', '')}"
        if self.category == "PM":
            return (
                "parameter map change "
                f"device={self.fields.get('dev')} "
                f"type={self.fields.get('type')} "
                f"parameter={self.fields.get('no')} "
                f"value={self.fields.get('val')} "
                f"origin={self.fields.get('org')}"
            )
        if self.category == "BOOT":
            return (
                f"boot firmware={self.fields.get('fw')} "
                f"application={self.fields.get('app')}"
            )
        if self.category == "MARK":
            return f"marker {self.text}"
        if self.category == "LOG":
            return f"log[{self.fields.get('lvl', '?')}] {self.text}"
        return self.signature()

    def _midi(self) -> str:
        interface = self.fields.get("if", "?")
        if "cable" in self.fields:
            where = f"{interface} cable {self.fields['cable']}"
        else:
            where = f"{interface} port {self.fields.get('port', '?')}"
        if "d" in self.fields:
            return f"{where} bytes {self.fields['d']}"
        return (
            f"{where} channel {self.fields.get('ch')} "
            f"data {self.fields.get('d1')},{self.fields.get('d2')}"
        )

    def _lua(self) -> str:
        return f"{self.fields.get('kind', '?')}.{self.fields.get('fn', '?')}"


@dataclass
class Trace:
    records: list[Record] = field(default_factory=list)
    gaps: list[tuple[int, int]] = field(default_factory=list)
    malformed: list[str] = field(default_factory=list)

    @property
    def is_complete(self) -> bool:
        return not self.gaps and not self.malformed

    def categories(self) -> dict[str, int]:
        counts: dict[str, int] = {}
        for record in self.records:
            counts[record.category] = counts.get(record.category, 0) + 1
        return counts

    def unknown_categories(self) -> set[str]:
        return {r.category for r in self.records} - KNOWN_CATEGORIES.keys()


def parse(text: str) -> Trace:
    """Turns raw trace bytes into records, reporting anything lost."""
    trace = Trace()
    previous: int | None = None

    for line in text.splitlines():
        line = line.strip()
        if not line:
            continue

        match = _RECORD.match(line)
        if not match:
            trace.malformed.append(line)
            continue

        sequence = int(match.group(1))
        record = Record(
            sequence=sequence,
            micros=int(match.group(2)),
            category=match.group(3),
            raw=line,
        )

        tokens = match.group(4).split()

        if record.category in FREE_TEXT_CATEGORIES:
            # Only a leading key=value is a field; the rest is the message.
            if tokens and "=" in tokens[0]:
                key, _, value = tokens[0].partition("=")
                record.fields[key] = value
                tokens = tokens[1:]
            record.text = " ".join(tokens)
        else:
            for token in tokens:
                if "=" in token:
                    key, _, value = token.partition("=")
                    record.fields[key] = value
                else:
                    record.text = (record.text + " " + token).strip()

        if previous is not None and sequence != previous + 1:
            trace.gaps.append((previous, sequence))
        previous = sequence

        trace.records.append(record)

    return trace


def signatures(trace: Trace, categories: set[str] | None = None) -> list[str]:
    """The ordered behaviour of a run, with timing removed."""
    return [
        record.signature()
        for record in trace.records
        if categories is None or record.category in categories
    ]
