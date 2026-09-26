# How a knob moves a value

What one turn of a knob is worth, and where each piece of that decision lives.
This replaces the acceleration tables, which is the thing the mk2 changed and
this now matches.

---

## The shape of it

Three stages, and each one only knows its own job:

| Stage | Where | What it answers |
| --- | --- | --- |
| Reading | `src/Hardware/Pot.cpp` | how far the knob turned, in phase counts |
| Scaling | `src/Events/ChangeAccumulator.h` | what that travel is worth to this control |
| Declaring | `ControlValue::getSensitivity()` | how fast this particular value wants to move |

## Reading: phase counts, not thresholds

The knobs are dual-gang continuous-rotation potentiometers whose two gangs sit a
quarter turn apart, so read against each other they trace a diamond: with `a` and
`b` measured from each gang's mid-point, `|a| + |b|` is constant and one
revolution walks once round the perimeter. That makes the pair an absolute angle
sensor. `Pot::process()` recovers the angle as a phase, 4096 counts to the turn -
what a ten bit converter can resolve of it - and reports the wrapped difference
between consecutive readings.

Two things about it are worth knowing.

**Nothing is rounded away.** What this replaced measured the larger of the two
gangs' deltas, divided it by sixteen and dropped the remainder, which on the
bench cost between half and three quarters of every turn - a full revolution of
the knob arrived as the equivalent of about 1950 of its 4096 counts. The phase is
tracked instead, so the only travel that is ever lost is the backlash, once, on a
reversal.

**The backlash is eight counts**, three quarters of a degree. A stationary knob's
recovered phase wanders by two counts peak to peak, measured, so eight leaves
four times that in hand. It has to: a proportional control no longer scales small
movements down, so a band the noise could cross even occasionally would show as a
parameter creeping on its own. Against that it costs a fifth of one percent of a
control's range on each reversal.

A knob is only read while a finger is on it, so one can be turned while nothing
is watching - by a hand the capacitive sensor missed, or in a case. Touching a
knob therefore calls `Pot::resync()`, which throws the stale reference away
rather than reporting the whole gap as one movement.

Readings are 5 ms apart (`POT_SCAN_TICK`), and only touched knobs are read, so
that is the cadence the knob in a hand gets: two hundred a second, where it used
to be sixty-six. One reading costs 60 µs, measured, so a knob in a hand is a
little over one percent of the processor and an untouched panel is none of it.
This matters more than it used to - travel is no longer multiplied by how fast
the knob is moving, so reading too seldom lands a quick sweep as a handful of
jumps instead of a slide.

## Scaling: three hundred degrees, whatever the range

`ChangeAccumulator` turns phase counts into values, and it needs to know only
what kind of control it is looking at.

A **proportional** control - a fader, a knob, an envelope segment - spans its
whole range in `POT_SWEEP_DEGREES`, which is 300: about as far as a wrist reaches
in one go, so minimum to maximum is a single gesture with no regripping. A
control holding 16 values and one holding 16384 take the same travel end to end.
The step between two messages is the range divided by the sweep, and the fraction
left over is carried into the next reading, so a short control still moves one
value at a time under a slow turn rather than rounding the turn away.

A **discrete** control - a list, a pad - reads like an encoder: one step per
`POT_ENCODER_DETENT_DEGREES`, which is 45, so eight steps to a turn whether the
list holds four entries or forty. A control says it is one of these by calling
`enableEncoderMode(true)`; so does a listener that never declared a range at all.

Nothing is multiplied by speed. The tables that used to do that reached 512 times
at the top, so the same gesture could move a parameter by a hundredth of its
range or by half of it depending on how quickly a finger happened to be moving.
That is what made them feel harsh, and what made the far end of a 14-bit
parameter reachable only by flicking.

## Declaring: `"sensitivity"` on a value

A preset value may ask for a factor on the usual rate:

```json
{ "id": "value", "min": 0, "max": 16383, "sensitivity": 0.1,
  "message": { "deviceId": 1, "type": "cc14", "parameterNumber": 1 } }
```

`0.1` is a tenth of the usual rate, so ten sweeps of the knob end to end, which
is what makes every value of a 14-bit parameter reachable. `10` is ten times it.
The range is `0.001` to `100`; anything outside it is clamped and anything that is
not a positive number is ignored. A value that declares none moves at the usual
rate, which is what every value did before.

On a discrete control the factor stretches or shrinks the detent instead: `0.25`
on a list means a quarter turn per entry.

It is per value, not per control, and it is read on every turn - so an envelope
whose decay asks for a tenth and whose attack does not changes rate as the focus
moves between segments, and a script that writes a new factor is obeyed by the
next turn. `value:setSensitivity(factor)` and `value:getSensitivity()` are the
Lua side of the same thing.

### What it costs in memory

A byte per value, and the byte is one the bitfields in `ControlValue` already
wasted on alignment - `sizeof(ControlValue)` is 56 with it and was 56 without, so
a preset with four hundred values pays nothing at all for this.

A byte cannot hold a float, so what is stored is a mantissa of five bits and a
power of ten of three: `mantissa × 10^(exponent-3)`, with zero meaning "declares
none". Every figure a preset is likely to write lands exactly - 0.001, 0.002,
0.005, 0.01, 0.05, 0.1, 0.25, 0.3, 0.5, 1, 2, 5, 10, 100 - and one that needs
three significant figures is rounded, at worst by a tenth (0.333 becomes 0.3),
which is well below what a hand can feel. Nothing on this model writes a preset
back out as JSON - a fetched preset is the stored file - so the rounding has no
round trip to show up in.

The rest of the change costs nothing either: `ChangeAccumulator` replaced the
timestamp and step counter `PotListener` used to carry, and `sizeof(PotListener)`
is 16 as it was, which matters because every Component on the screen is one.
`Pot` lost the two previous converter readings and gained a tracked phase, so it
went from 16 bytes to 12.

## The constants

All in `src/settings.h`:

| Name | Value | What it is |
| --- | --- | --- |
| `POT_COUNTS_PER_REVOLUTION` | 4096 | phase counts to a whole turn |
| `POT_SWEEP_DEGREES` | 300 | travel a proportional control spans in |
| `POT_ENCODER_DETENT_DEGREES` | 45 | travel one step of a discrete control is worth |
| `POT_SCAN_TICK` | 5000 µs | how often the knob under a finger is read |
| `POT_SCAN_TICK_ALL` | 2500 µs | the same, for the boot that sweeps all twelve |

`Pot::hysteresis`, the backlash, is in `src/Hardware/Pot.h` because it is a
property of the converter rather than of the feel.

## Not done here

**A fine-adjustment mode.** The mk2 has a hardware command that multiplies every
knob by a tenth for as long as it is held, which is how a 14-bit parameter is
reached there without the preset asking for anything. This model has no spare
button binding for it, and `"sensitivity"` covers the case a preset knows about
in advance. The factor it would apply is the only part missing: everything it
would multiply already runs through one place.
