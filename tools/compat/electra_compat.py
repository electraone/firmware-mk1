#!/usr/bin/env python3
"""Compatibility harness for the Electra One mk1 firmware.

Records what a firmware build does when given a fixed set of stimuli, and
compares two such recordings. The rule it enforces is that new versions may
add behaviour but must not change or remove what an older version did.

A recording holds three things:

  * the ordered behavioural trace read out of the unit over J-Link, covering
    incoming MIDI, calls into Lua, parameter map changes and outgoing MIDI,
  * everything the unit actually sent on its USB device port,
  * the identification of the firmware that produced it.

Typical use:

    ./electra_compat.py record --elf ../../build-trace/firmware.elf \\
        --scenario cc-sweep --out baseline/v3.5.4-cc-sweep.json

    ./electra_compat.py compare baseline/v3.5.4-cc-sweep.json \\
        candidate/v3.6.0-cc-sweep.json
"""

from __future__ import annotations

import argparse
import json
import platform
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

import electra_jlink as jlink_module
import electra_midi
import electra_trace
import scenarios

RECORDING_FORMAT = 1

# Categories that describe behaviour we promise not to break. Records outside
# this set are kept in the recording but not compared.
COMPARED_CATEGORIES = {"MI", "MO", "LC", "LR", "LX", "LE", "PM"}

TRACE_SYMBOLS = {"_SEGGER_RTT", "_ZN5TraceL7droppedE", "_ZN5TraceL8sequenceE"}


def _now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def _firmware_identity(trace: electra_trace.Trace) -> dict[str, str]:
    for record in trace.records:
        if record.category == "BOOT":
            return {
                "version": record.fields.get("fw", "unknown"),
                "application": record.fields.get("app", "unknown"),
            }
    return {"version": "unknown", "application": "unknown"}


def _git_revision() -> str:
    try:
        completed = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"],
            capture_output=True,
            text=True,
            cwd=Path(__file__).resolve().parent,
        )
        return completed.stdout.strip() or "unknown"
    except OSError:
        return "unknown"


def command_record(args: argparse.Namespace) -> int:
    elf = Path(args.elf).resolve()
    if not elf.exists():
        print(f"error: {elf} does not exist. Build with TRACE=1 first.")
        return 2

    scenario = scenarios.get(args.scenario)
    jlink = jlink_module.JLink(serial=args.serial)
    symbols = jlink_module.read_elf_symbols(elf, TRACE_SYMBOLS)
    control_block = symbols["_SEGGER_RTT"]

    print(f"scenario: {scenario.name}")
    print(f"  {scenario.description}")

    capture = None
    if not args.no_midi_capture:
        for port in electra_midi.ALL_PORTS:
            electra_midi.require_port(port)
        capture = electra_midi.start_capture(electra_midi.ALL_PORTS)

    # A reset gives every recording the same starting point, and empties the
    # trace buffer without having to walk it.
    print("resetting the unit ...")
    jlink.reset_and_run()

    # The reset disconnects the unit from USB. Stimuli sent before it has
    # enumerated again would be lost, which would look like changed behaviour.
    if not args.no_midi_capture:
        for _ in range(40):
            time.sleep(1)
            if electra_midi.PORT_CTRL in electra_midi.list_ports()[1]:
                break
        else:
            print("error: the unit did not reappear on USB after the reset")
            return 2

    time.sleep(args.boot_wait)

    for step in scenario.steps:
        print(f"  step: {step.label}")
        if step.send:
            electra_midi.send(step.port, step.send)
        time.sleep(step.settle)

    time.sleep(scenario.settle)

    print("reading the trace ...")
    payload, state = jlink_module.drain_trace(jlink, control_block)
    dropped_raw = jlink.read_memory(symbols["_ZN5TraceL7droppedE"], 4)
    dropped = int.from_bytes(dropped_raw, "little")

    midi_out: dict[str, list[str]] = {}
    if capture is not None:
        for port, lines in capture.stop().items():
            midi_out[port] = electra_midi.normalise(lines)

    trace = electra_trace.parse(payload.decode("ascii", "replace"))

    buffer_filled = state.write_offset >= state.size - 2
    problems = []
    if dropped:
        problems.append(f"{dropped} trace records were dropped")
    if trace.gaps:
        problems.append(f"{len(trace.gaps)} gaps in the trace sequence")
    if trace.malformed:
        problems.append(f"{len(trace.malformed)} unparsable trace lines")
    if buffer_filled:
        problems.append("the trace buffer filled up")
    unknown = trace.unknown_categories()
    if unknown:
        problems.append("unknown record categories: " + ", ".join(sorted(unknown)))

    recording = {
        "format": RECORDING_FORMAT,
        "recorded": _now(),
        "scenario": scenario.name,
        "scenario_description": scenario.description,
        "firmware": _firmware_identity(trace),
        "host": platform.platform(),
        "repository_revision": _git_revision(),
        "elf": str(elf),
        "trace": [record.raw for record in trace.records],
        "midi_out": midi_out,
        "integrity": {
            "records": len(trace.records),
            "dropped": dropped,
            "gaps": trace.gaps,
            "malformed": trace.malformed,
            "buffer_filled": buffer_filled,
            "problems": problems,
        },
    }

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(recording, indent=2) + "\n")

    print()
    print(f"firmware:  {recording['firmware']['version']} "
          f"({recording['firmware']['application']})")
    print(f"records:   {len(trace.records)}")
    for category, count in sorted(trace.categories().items()):
        meaning = electra_trace.KNOWN_CATEGORIES.get(category, "unknown")
        print(f"             {category:5s} {count:5d}  {meaning}")
    print("MIDI out captured on the wire:")
    for port in electra_midi.ALL_PORTS:
        print(f"             {len(midi_out.get(port, [])):5d}  {port}")
    print(f"written:   {out}")

    if problems:
        print()
        print("the recording is NOT trustworthy:")
        for problem in problems:
            print(f"  - {problem}")
        return 1
    return 0


def _load(path: Path) -> dict:
    data = json.loads(path.read_text())
    if data.get("format") != RECORDING_FORMAT:
        raise SystemExit(
            f"{path} was written in format {data.get('format')}, "
            f"this tool reads format {RECORDING_FORMAT}"
        )
    return data


def _diff(baseline: list[str], candidate: list[str]) -> list[tuple[str, str]]:
    """Longest common subsequence diff, reported as removals and additions."""
    import difflib

    changes: list[tuple[str, str]] = []
    matcher = difflib.SequenceMatcher(a=baseline, b=candidate, autojunk=False)

    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        if tag == "equal":
            continue
        for line in baseline[i1:i2]:
            changes.append(("removed", line))
        for line in candidate[j1:j2]:
            changes.append(("added", line))
    return changes


def command_compare(args: argparse.Namespace) -> int:
    baseline = _load(Path(args.baseline))
    candidate = _load(Path(args.candidate))

    if baseline["scenario"] != candidate["scenario"]:
        print(
            f"error: these recordings are of different scenarios "
            f"({baseline['scenario']} and {candidate['scenario']})"
        )
        return 2

    for name, recording in (("baseline", baseline), ("candidate", candidate)):
        problems = recording.get("integrity", {}).get("problems", [])
        if problems:
            print(f"warning: the {name} recording is not trustworthy:")
            for problem in problems:
                print(f"  - {problem}")

    print(f"scenario:  {baseline['scenario']}")
    print(f"baseline:  {baseline['firmware']['version']}  {args.baseline}")
    print(f"candidate: {candidate['firmware']['version']}  {args.candidate}")
    print()

    base_trace = electra_trace.parse("\n".join(baseline["trace"]))
    cand_trace = electra_trace.parse("\n".join(candidate["trace"]))

    base_signatures = electra_trace.signatures(base_trace, COMPARED_CATEGORIES)
    cand_signatures = electra_trace.signatures(cand_trace, COMPARED_CATEGORIES)

    behaviour_changes = _diff(base_signatures, cand_signatures)
    midi_changes = []
    for port in sorted(
        set(baseline["midi_out"]) | set(candidate["midi_out"])
    ):
        for kind, line in _diff(
            baseline["midi_out"].get(port, []),
            candidate["midi_out"].get(port, []),
        ):
            midi_changes.append((kind, f"[{port}] {line}"))

    removed_behaviour = [c for c in behaviour_changes if c[0] == "removed"]
    added_behaviour = [c for c in behaviour_changes if c[0] == "added"]
    removed_midi = [c for c in midi_changes if c[0] == "removed"]
    added_midi = [c for c in midi_changes if c[0] == "added"]

    if not behaviour_changes and not midi_changes:
        print("identical. The candidate behaves exactly like the baseline.")
        return 0

    if removed_behaviour or removed_midi:
        print("BROKEN COMPATIBILITY")
        print("The candidate no longer does things the baseline did.")
    else:
        print("COMPATIBLE, WITH ADDITIONS")
        print("Everything the baseline did still happens, in the same order.")
    print()

    def _report(title: str, changes: list[tuple[str, str]], limit: int) -> None:
        if not changes:
            return
        print(f"{title} ({len(changes)}):")
        for kind, line in changes[:limit]:
            marker = "-" if kind == "removed" else "+"
            print(f"  {marker} {line}")
        if len(changes) > limit:
            print(f"  ... {len(changes) - limit} more")
        print()

    _report("behaviour the baseline had and the candidate lost",
            removed_behaviour, args.limit)
    _report("behaviour the candidate adds", added_behaviour, args.limit)
    _report("MIDI the baseline sent and the candidate does not",
            removed_midi, args.limit)
    _report("MIDI the candidate adds", added_midi, args.limit)

    return 1 if (removed_behaviour or removed_midi) else 0


def command_scenarios(args: argparse.Namespace) -> int:
    for name in sorted(scenarios.SCENARIOS):
        scenario = scenarios.SCENARIOS[name]
        print(f"{name}")
        print(f"  {scenario.description}")
        print(f"  {len(scenario.steps)} steps")
        print()
    return 0


def command_show(args: argparse.Namespace) -> int:
    recording = _load(Path(args.recording))
    trace = electra_trace.parse("\n".join(recording["trace"]))

    print(f"scenario:  {recording['scenario']}")
    print(f"firmware:  {recording['firmware']['version']}")
    print(f"recorded:  {recording['recorded']}")
    print()

    for record in trace.records:
        if args.only and record.category not in args.only.split(","):
            continue
        print(f"{record.sequence:6d}  {record.describe()}")

    print()
    print("MIDI captured on the wire:")
    for port, lines in sorted(recording["midi_out"].items()):
        print(f"  {port}: {len(lines)} messages")
        for line in lines[: args.limit]:
            print(f"    {line}")
        if len(lines) > args.limit:
            print(f"    ... {len(lines) - args.limit} more")
    return 0


def command_ports(args: argparse.Namespace) -> int:
    inputs, outputs = electra_midi.list_ports()
    print("MIDI inputs:")
    for name in inputs:
        print(f"  {name}")
    print("MIDI outputs:")
    for name in outputs:
        print(f"  {name}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    record = subparsers.add_parser(
        "record", help="run a scenario against the unit and save a recording"
    )
    record.add_argument(
        "--elf",
        required=True,
        help="firmware ELF built with TRACE=1, used to locate the trace buffer",
    )
    record.add_argument("--scenario", required=True, help="scenario name")
    record.add_argument("--out", required=True, help="recording file to write")
    record.add_argument(
        "--serial", default=None, help="J-Link serial number to use"
    )
    record.add_argument(
        "--boot-wait",
        type=float,
        default=6.0,
        help="seconds to wait after reset before sending stimuli",
    )
    record.add_argument(
        "--no-midi-capture",
        action="store_true",
        help="record only the firmware trace, without host MIDI capture",
    )
    record.set_defaults(handler=command_record)

    compare = subparsers.add_parser(
        "compare", help="compare a candidate recording against a baseline"
    )
    compare.add_argument("baseline")
    compare.add_argument("candidate")
    compare.add_argument("--limit", type=int, default=40)
    compare.set_defaults(handler=command_compare)

    show = subparsers.add_parser("show", help="print a recording in full")
    show.add_argument("recording")
    show.add_argument(
        "--only", default=None, help="comma separated categories to show"
    )
    show.add_argument("--limit", type=int, default=40)
    show.set_defaults(handler=command_show)

    listing = subparsers.add_parser("scenarios", help="list the scenarios")
    listing.set_defaults(handler=command_scenarios)

    ports = subparsers.add_parser("ports", help="list MIDI ports")
    ports.set_defaults(handler=command_ports)

    args = parser.parse_args()
    try:
        return args.handler(args)
    except (jlink_module.JLinkError, electra_midi.MidiError) as error:
        print(f"error: {error}")
        return 2


if __name__ == "__main__":
    sys.exit(main())
