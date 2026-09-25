# Firmware compatibility harness

This harness answers one question: **does a new firmware build still do
everything v3.5.4 did, in the same order?** New behaviour is allowed. Changed
or missing behaviour is not.

It works by recording what the unit does when given a fixed set of stimuli,
and diffing two such recordings.

## What is recorded

Two independent views of the same run:

| View | Source | What it proves |
|---|---|---|
| Behavioural trace | firmware, over SEGGER RTT via J-Link | the order in which things happened |
| MIDI on the wire | host, from all three USB MIDI cables | what actually left the unit |

The trace covers, in the order the firmware processed them:

- `MI` a MIDI message entering the application
- `MO` a MIDI message queued, dispatched to an interface, or put on the USB wire
- `LC` / `LR` a call into Lua and its return, including what a formatter returned
- `LX` a Lua handler a preset did not define
- `LE` a Lua error
- `PM` a parameter map change notification, with device, type, parameter, value and origin

The trace leaves over RTT rather than over MIDI on purpose. Sending it over
MIDI would pollute the very thing being measured.

## Requirements

- A J-Link attached to the unit. Pass `--serial` when more than one probe is
  connected, or the wrong target may be addressed.
- The `sendmidi` and `receivemidi` command line tools:
  `brew install gbevin/tools/sendmidi gbevin/tools/receivemidi`
- A firmware image built with tracing enabled, and its ELF. The ELF is how the
  harness locates the trace buffer, so it must match the image on the unit.

## Building a traced image

Object files do not depend on the build flags, so a traced build needs its own
build directory. Building it into `build/` would silently mix traced and
untraced objects.

```bash
make APPPATH=apps/controller TRACE=1 BUILDDIR=build-trace hex
```

Add `SD_FORCE_1BIT_BUS=1` on a unit whose SD card data lines are damaged.

Tracing costs about 3 KB of flash and 12 KB of RAM, all of it inside the traced
build. A normal build is byte for byte unaffected, because the trace
translation unit compiles to nothing and the RTT sources are dropped from the
file list.

## Recording a baseline

```bash
cd tools/compat
./electra_compat.py record \
    --elf ../../build-trace/firmware.elf \
    --scenario cc-sweep \
    --serial 601006607 \
    --out baseline/v3.5.4-cc-sweep.json
```

The unit is reset first so every recording starts from the same state.

A recording is only usable if the harness reports no problems. It refuses to
call a run trustworthy when trace records were dropped, when the sequence
numbers have gaps, or when the trace buffer filled up. Raise
`TRACE_BUFFER_SIZE` in the Makefile if a long scenario overruns the buffer.

## Comparing

```bash
./electra_compat.py compare \
    baseline/v3.5.4-cc-sweep.json \
    candidate/v3.6.0-cc-sweep.json
```

The exit status is 0 when the candidate is compatible, whether or not it added
behaviour, and 1 when it lost or changed behaviour the baseline had. That makes
it usable directly from a build script.

Timestamps are stripped before comparing, so a slower or faster build is not
reported as a difference. Everything else about a record is compared.

## Other commands

```bash
./electra_compat.py scenarios          # list the stimulus scenarios
./electra_compat.py show <recording>   # print a recording in readable form
./electra_compat.py ports              # list the MIDI ports on this machine
```

`show` takes `--only` with a comma separated list of categories, for example
`--only PM,LC,LR` to look at just the parameter map and Lua activity.

## Adding scenarios

Scenarios live in `scenarios.py`. Add new ones freely. Do not change or remove
an existing one: every baseline recorded with it becomes meaningless, which
defeats the point of the harness.

## The test preset

What a scenario can observe depends entirely on the preset loaded on the unit.
A preset that maps nothing to the stimulus produces MIDI records and nothing
else. `presets/compat.epr` and `presets/compat.lua` exist so that a recording
exercises every path the harness watches: a 7 bit and a 14 bit control change,
an NRPN, a note, pitch bend and a program change, with Lua formatters, Lua
control functions, `parameterMap.onChange`, the preset lifecycle handlers and
the incoming MIDI handlers all defined.

The baselines in `baseline/` were recorded with it loaded. Upload it, into the
slot the unit currently has selected, with:

```python
import electra_preset, json
from pathlib import Path
electra_preset.upload_lua(Path("presets/compat.lua").read_text())
electra_preset.upload_preset(
    json.dumps(json.loads(Path("presets/compat.epr").read_text()),
               separators=(",", ":")))
```

Upload the Lua script first. Uploading the preset makes the unit reload the
slot, and the script has to be on the card by then to be picked up.

## Caveats

- Record the baseline and the candidate with the same preset loaded, or the
  comparison is meaningless.
- The harness reads the trace buffer directly out of target RAM rather than
  through the SEGGER RTT host tools, which do not reliably locate the control
  block on this part.
- A reset disconnects the unit from USB. The harness waits for it to enumerate
  again before sending anything, which is why a recording takes a few seconds
  longer than the scenario itself.
- Close other MIDI software before recording. Anything that queries the unit,
  an editor or a DAW scanning ports, makes it answer, and that answer lands in
  the recording as behaviour the firmware did not initiate. It shows up as an
  addition rather than a break, so it does not fail a comparison, but it is
  noise.
