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

## Reading: the mk2's decoder and the mk2's scan

The knobs are dual-gang continuous-rotation potentiometers whose two gangs sit a
quarter turn apart, so read against each other they trace a diamond: with `a` and
`b` measured from each gang's mid-point, `|a| + |b|` is constant and one
revolution walks once round the perimeter. That makes the pair an absolute angle
sensor. `Pot::process()` recovers the angle as a phase, 8192 counts to the turn -
what a twelve bit converter can resolve of it - and reports the wrapped
difference between consecutive readings.

`Pot` is the mk2's decoder, line for line apart from where the conversions come
from, and `readPots()` is the mk2's scan. The two models read a knob the same
way, so what was measured on one holds for the other.

**Nothing is rounded away.** The phase is tracked, and the fraction of a step
left over after each report is carried into the next, so the only travel ever
lost is the backlash, once, on a reversal.

**Every knob is read, all the time.** The multiplexer rests on one knob for a
whole 1 ms tick (`POT_SCAN_TICK`) before it is read: switching it dumps charge
onto the selected wiper, and a reading taken straight after carries a little of
the knob read before. A background rota visits all twelve in turn; a knob that
moved in the last 250 ms (`POT_ACTIVE_HOLD_MS`) also joins a fast rota that takes
every other tick, so a knob in a hand is read 500 times a second. Nothing here
asks the touch sensor which knob to read, so a knob turned by a hand the sensor
missed still works, and there is no stale reference to throw away when a finger
arrives - the reference never went stale.

The six buttons sit on the same multiplexer, at addresses 0 to 5, so the scan
reads the button at the parked address on its way past - background rota only,
so their cadence does not change with whichever knob is being turned. Each button
is visited every 12 to 24 ms, and debounces against the clock (6 ms, the mk2's
figure) rather than relying on being read seldom. `readButtons` only runs while
the controller sleeps, when the pot scan does not.

**A reading is eight conversions of each gang, trimmed.** Both gangs are
converted at once, one on each of the K66's two converters, at twelve bits with
four averaged in hardware, as on the mk2. The two highest and two lowest of the
eight are thrown away and the middle four averaged, which keeps a single bad
conversion - the mk2 sees one 20 to 40 counts off every few thousand - out of the
reading. Then the movement acted on is the median of the last three readings'
distances from the reference, so a lone outlier is outvoted and costs a turn
nothing but one scan slot.

**The backlash is eight counts**, a third of a degree, for a knob under a finger
with both gangs clear of the converter's rails. It widens to twenty-four for a
knob with a gang within twelve counts of a rail, where a shift in the readings
cannot cancel between the gangs, and for a knob the touch sensor does not see a
finger on. A quick turn clears either on its first reading; a slow untouched turn
gives up its first degree of travel. With knob touch turned off at boot only the
rail rule applies. `Pot::gateFor()` decides it per reading.

**The calibration is learned.** Each gang's extremes are tracked from what it
reads, seeded at 2048 ± 1800 and widened as soon as a reading goes past them.
They are pulled back in by a count every 512 readings of confirmed movement, so a
one-off wide reading heals, and never on a knob nobody is turning.

**What it costs.** One reading is 80 µs, measured with the `potRead` profile
probe, so the scan is about 8 % of the processor whether or not anything is
moving - it used to be one percent while a knob was in a hand and nothing
otherwise. That bought the knob in a hand two and a half times the readings, and
a panel whose knobs work without the touch sensor. The converter's default
sampling speed made it 144 µs; the wiper sits on a large capacitor, so the
in-spec high-speed setting reads it no worse - the noise of a knob at rest was
1 to 5 counts peak to peak either way.

**Measured on the bench**, 2026-10-29: in a two minute idle soak none of the
eleven untouched knobs changed its tracked phase once.

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

A proportional control with **a very short range** reads as an encoder too. A
range of two spanned in one sweep is 300 degrees of knob for one step, and every
partial turn is lost when the finger leaves, so such a control was dead to its
knob. No value is worth more travel than a detent: below 300 / 45, about seven
values, the detent rule takes over, and the two rules agree where they meet.

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
`Pot` carries the mk2's learned calibration and the two readings the median
needs, so it is 56 bytes - 672 for the panel, once.

## The constants

All in `src/settings.h`:

| Name | Value | What it is |
| --- | --- | --- |
| `POT_COUNTS_PER_REVOLUTION` | 8192 | phase counts to a whole turn |
| `POT_SWEEP_DEGREES` | 300 | travel a proportional control spans in |
| `POT_ENCODER_DETENT_DEGREES` | 45 | travel one step of a discrete control is worth |
| `POT_SCAN_TICK` | 1000 µs | how long the multiplexer rests on a knob before reading it |
| `POT_ACTIVE_HOLD_MS` | 250 ms | how long a knob that moved stays in the fast rota |

The backlash, the rail margin, the oversampling and the calibration seeds are in
`src/Hardware/Pot.h`, because they are properties of the converter rather than of
the feel.

## Not done here

**A fine-adjustment mode.** The mk2 has a hardware command that multiplies every
knob by a tenth for as long as it is held, which is how a 14-bit parameter is
reached there without the preset asking for anything. This model has no spare
button binding for it, and `"sensitivity"` covers the case a preset knows about
in advance. The factor it would apply is the only part missing: everything it
would multiply already runs through one place.

**Banking the movement.** The mk2's scan runs in its own thread and banks
movement for the application thread to collect, holding it back while the DIN
queue is nearly full. Here the scan and the delivery are the same cooperative
loop, so there is nothing to hand across, and because the phase is absolute a
reading that comes late still carries all the travel since the last one.
