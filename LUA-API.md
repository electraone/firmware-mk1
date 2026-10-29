# The Lua extension API

What a preset's Lua script can call on this controller, and what the firmware
calls back. Written against the mk2's published API: where a name differs from
the mk2's, both are accepted, and where something is missing it is listed under
[Not implemented](#not-implemented) with the reason.

The API is in two halves. `src/Lua/` holds what belongs to the firmware itself
and is compiled into every app - MIDI, the timer, transport, the window, the
controller, SysexBlock. `apps/controller/src/Lua/` holds what belongs to the
MIDI Controller app and exists only when that app is built - the preset model,
the parameter map, the graphics library.

---

## Modules

### `midi` — `src/Lua/luaMidi.cpp`

Every send function takes either `(port, ...)` or `(interface, port, ...)`.
Left out, the interface defaults to all of them. The optional trailing flags -
`lsbFirst`, `resetRpn` - take `true` / `false` or `0` / `1`.

| Function | Notes |
| --- | --- |
| `sendMessage([interface,] port, midiMessage)` | `midiMessage` is a table with `type` and the fields that type needs |
| `sendControlChange([interface,] port, channel, controllerNumber, value)` | |
| `sendNoteOn([interface,] port, channel, noteNumber, velocity)` | |
| `sendNoteOff([interface,] port, channel, noteNumber, velocity)` | |
| `sendProgramChange([interface,] port, channel, programNumber)` | |
| `sendPitchBend([interface,] port, channel, value)` | `value` is −8191..8192 |
| `sendAfterTouchPoly([interface,] port, channel, noteNumber, pressure)` | |
| `sendAfterTouchChannel([interface,] port, channel, pressure)` | |
| `sendClock([interface,] port)` | |
| `sendStart([interface,] port)` | |
| `sendStop([interface,] port)` | |
| `sendContinue([interface,] port)` | |
| `sendActiveSensing([interface,] port)` | |
| `sendSystemReset([interface,] port)` | |
| `sendTuneRequest([interface,] port)` | |
| `sendSongSelect([interface,] port, songNumber)` | |
| `sendSongPosition([interface,] port, position)` | |
| `sendSysex([interface,] port, data)` | `data` is an array of 7-bit bytes without `F0`/`F7`, or a `SysexBlock` |
| `sendNrpn([interface,] port, channel, parameterNumber, value [, lsbFirst [, resetRpn]])` | `resetRpn` defaults to true |
| `sendRpn([interface,] port, channel, parameterNumber, value)` | |
| `sendControlChange14Bit([interface,] port, channel, controllerNumber, value [, lsbFirst])` | |
| `sendControlChange14(...)` | the mk2's name for the same function |
| `flush()` | pushes the output queue out now |

Callbacks. Each is called only if the script defines it; the firmware looks for
them once, when the preset loads. `midiInput` is a table of `interface` and
`port`.

`midi.onMessage(midiInput, midiMessage)`,
`midi.onNoteOn(midiInput, channel, noteNumber, velocity)`,
`midi.onNoteOff(midiInput, channel, noteNumber, velocity)`,
`midi.onControlChange(midiInput, channel, controllerNumber, value)`,
`midi.onProgramChange(midiInput, channel, programNumber)`,
`midi.onAfterTouchPoly(midiInput, channel, noteNumber, pressure)`,
`midi.onAfterTouchChannel(midiInput, channel, pressure)`,
`midi.onPitchBend(midiInput, channel, value)`,
`midi.onSongSelect(midiInput, songNumber)`,
`midi.onSongPosition(midiInput, position)`,
`midi.onClock(midiInput)`, `midi.onStart(midiInput)`, `midi.onStop(midiInput)`,
`midi.onContinue(midiInput)`, `midi.onActiveSensing(midiInput)`,
`midi.onSystemReset(midiInput)`, `midi.onTuneRequest(midiInput)`,
`midi.onSysex(midiInput, sysexBlock)`.

### `transport` — `src/Lua/luaTransport.cpp`

`enable()`, `disable()`, `isEnabled()`.

Callbacks: `transport.onClock(midiInput)`, `onStart`, `onStop`, `onContinue`,
`onSongSelect(midiInput, songNumber)`, `onSongPosition(midiInput, position)`.

The transport handlers are a cheaper path to the clock than `midi.onClock`:
they are wired only while `transport.enable()` is in force.

### `timer` — `src/Lua/luaTimer.cpp`

`enable()`, `disable()`, `isEnabled()`, `setPeriod(millis)` (10..60000),
`getPeriod()`, `setBpm(bpm)` (1..6000), `getBpm()`.

Callback: `timer.onTick()`.

### `controller` — `src/Lua/luaController.cpp`

| Function | Answers |
| --- | --- |
| `uptime()` | milliseconds since reset |
| `getModel()` | `"mk1"` |
| `getNumModel()` | `MODEL_MK1` |
| `getFirmwareVersion()` | e.g. `"v3.5.4"` |
| `getFirmwareNumVersion()` | e.g. `300500400` |
| `require(model, minimumVersion)` | raises if this controller does not meet it |
| `isRequired(model, minimumVersion)` | the same as a boolean |

`model` is a name (`"mk1"`) or a number (`MODEL_MK1`); `minimumVersion` is a
string (`"3.5.4"`, `"v3.5"`, `"3"`) or the numeric sequence. Either argument may
be omitted.

### `window` — `src/Lua/luaWindow.cpp`

`repaint()`, `clear()`, `stop()`, `resume()`,
`addAndMakeVisible(component)`, `findChildById(id)`.

`stop()` and `resume()` bracket a run of drawing so the display is not
repainted halfway through it.

### `graphics` — `src/Lua/luaGraphics.cpp`

Only from a control's paint callback; called from anywhere else it raises,
because outside a paint the shapes would land wherever the last component left
the drawing window. The measuring and colour functions are the exception - they
touch no framebuffer and may be called at load.

| Function | |
| --- | --- |
| `setColor(colour)` | 24-bit `0xRRGGBB` |
| `setBackgroundColor(colour)` | what anti-aliased text is blended onto |
| `drawPixel(x, y)` | |
| `drawLine(x1, y1, x2, y2)` | |
| `drawRect(x, y, width, height)` / `fillRect(...)` | |
| `drawRoundRect(x, y, width, height, radius)` / `fillRoundRect(...)` | |
| `drawTriangle(x1, y1, x2, y2, x3, y3)` / `fillTriangle(...)` | |
| `drawCircle(x, y, radius)` / `fillCircle(...)` | |
| `drawEllipse(x, y, radiusX, radiusY)` / `fillEllipse(...)` | |
| `fillCurve(x, y, radius, segment)` | one quadrant |
| `drawArc(x, y, radius, fromDegrees, toDegrees [, thickness])` | degrees clockwise from twelve o'clock |
| `print(x, y, text, width, align [, fontface, size, spacing])` | |
| `getTextWidth(text [, fontface, size, spacing])` | |
| `getTextHeight([fontface, size])` | |
| `rgb(red, green, blue)` | a colour from three channels |
| `dim(colour, factor)` | the panel's own darkening |
| `blend(from, to, position)` | `position` 0..1 |

The same functions as the mk2's, with the same arguments.

### `parameterMap` — `apps/controller/src/Lua/luaParameterMap.cpp`

| Function | |
| --- | --- |
| `get(deviceId, type, parameterNumber)` | the stored MIDI value |
| `set(deviceId, type, parameterNumber, midiValue)` | stores, sends, and calls back |
| `updateValue(deviceId, type, parameterNumber, midiValue)` | stores and repaints only - no MIDI, no callback |
| `apply(deviceId, type, parameterNumber, midiValueFragment)` | ORs a fragment into the value |
| `modulate(deviceId, type, parameterNumber, modulationValue, depth)` | sends a modulated value without storing it |
| `getValues(deviceId, type, parameterNumber)` | the `ControlValue`s bound to that parameter |
| `map(deviceId, type, parameterNumber)` | a closure: called with a number it sets, called with none it gets |
| `resetAll()` | every value to zero |
| `resetDevice(deviceId)` | one device's values to zero |
| `keep()` / `recall()` / `forget()` | the snapshot the map keeps across a preset load |
| `print()` | the whole map, to the log |
| `send(...)` | deprecated; validates its arguments and does nothing |

Callback: `parameterMap.onChange(controlValues, origin, midiValue)`. `origin` is
`INTERNAL`, `MIDI` or `LUA`.

### `preset` — `apps/controller/src/Lua/luaPreset.cpp`

No functions; five callbacks, the mk2's, called as the mk2 calls them for a
preset it does not keep in memory. This model only ever holds one preset, so
switching to another destroys it - leaving and exiting come together - and
every entry to a preset is a fresh load.

| When | Called, in order |
| --- | --- |
| A preset is loaded: switched to, reloaded, uploaded into the current slot, at power-on | the script's main chunk, `preset.onLoad()`, the initial values, `preset.onReady()`, `preset.onEnter()` |
| It is left: another preset is loaded, it is reloaded or removed, or its files are replaced | `preset.onLeave()`, `preset.onExit()` |
| Only its Lua script is replaced, by an upload | the old script's `preset.onExit()`, then the new script's main chunk, `preset.onLoad()`, `preset.onReady()`, `preset.onEnter()` |

`onReady` and `onEnter` come on the first repaint pass after the load, once the
parameter map has been brought up to date. A preset left before that pass never
had `onEnter`, and gets no `onLeave` either - only `onExit`. Replacing the
script calls `onExit` without `onLeave`, as the mk2 does: the preset stays on
the screen. Sleep, power-off and a reboot call nothing, on either model.

`preset.userFunctions` is a table the script writes rather than a function it
calls: what the preset offers under FUNCTIONS, one entry per knob.

```lua
preset.userFunctions = {
    pot1 = { name = "DEFAULTS",
             call = function() commands.run("setAllDefaultValues") end },
    pot2 = { name = "PAGE 1",
             call = function() commands.run("switchPage", 1) end,
             close = true }
}
```

`name` goes on the button, `call` is what running it does, and `close` asks for
the window to be put away afterwards. The same entries are reachable as the
`runUserFunction` command, so a hardware button or an incoming MIDI message can
fire one without the window being open - see COMMANDS.md.

### `patch` — `apps/controller/src/Lua/luaPatch.cpp`

`requestAll()`.

Callbacks: `patch.onRequest(device)`,
`patch.onResponse(device, responseId, sysexBlock)`.

### `pages` — `apps/controller/src/Lua/luaPage.cpp`

`get(pageId)`, `getAll()`, `getActive()`, `display(pageId [, controlSetId])`,
`setActiveControlSet(controlSetId)`, `getActiveControlSet()`.

Callback: `pages.onChange(newPageId, oldPageId)`.

### `controls` — `apps/controller/src/Lua/luaControl.cpp`

`get(controlId)`, `print()`.

### `groups` — `apps/controller/src/Lua/luaGroup.cpp`

`get(groupId)`.

### `devices` — `apps/controller/src/Lua/luaDevice.cpp`

`get(deviceId)`, `getByPortChannel(port, channel)`,
`create(deviceId, name, port, channel)`.

### `overlays` — `apps/controller/src/Lua/luaOverlay.cpp`

`get(overlayId)`,
`create(overlayId, {{value=, label=, bitmap=}, ...})`.

### `events` — `apps/controller/src/Lua/luaEvents.cpp`

`subscribe(eventFlags)` - an OR of `PAGES`, `CONTROL_SETS`, `POTS`, `TOUCH`,
`BUTTONS`, `WINDOWS`, `USB_HOST_PORT`, or `NONE`. `setPort(port)` chooses the
port the events are reported on.

Callbacks: `events.onPageChange(newPageId, oldPageId)`,
`events.onPotTouchChange(potId, controlId, touched)` (also accepted under the
older name `events.onPotTouch`),
`events.onUsbHostChange(port, eventType)`.

### `commands` — `apps/controller/src/Lua/luaCommands.cpp`

| Function | What it does |
| --- | --- |
| `run(name [, parameter])` | asks the instrument for one of the things it can do |
| `exists(name)` | whether a name is one it has |
| `getNames()` | every name, as an array |

`run` is the same road a button on the panel, a MIDI control assignment and a
host over SysEx take, so a script cannot ask for anything they cannot. It
returns whether the command ran, and raises only when the name is not one the
instrument has. Every name is in COMMANDS.md.

```lua
commands.run("openPresetMenu")
commands.run("switchPage", 3)
```

### `info` — `apps/controller/src/Lua/luaInfo.cpp`

`setText(text)`, `getText()`. Twenty characters, in the bottom bar.

### `schedule` — `apps/controller/src/Lua/luaSchedule.cpp`

Functions run later, or when a combination of notes is held. The mk2's module,
with its names, arguments and answers. `timer.onTick` is unchanged; this is for
what a preset used to multiplex into it by hand.

| Function | |
| --- | --- |
| `after(ms, fn, ...)` | runs `fn(...)` once, `ms` from now; answers a handle |
| `every(ms, fn, ...)` | runs `fn(...)` every `ms` (at least 1), the first `ms` from now; answers a handle |
| `cancel(handle)` | `true` when it was still waiting; works for `whenNotes` handles too |
| `now()` | `millis()` |
| `stats()` | `pending`, `ran`, `errors`, `worstOverrun` (ms), `capacity`, `whenNotes` |
| `whenNotes(notes, fn [, options])` | runs `fn(device, held, match)` when the notes are held; answers a handle |

Thirty-two functions may wait at once, and thirty-two `whenNotes` triggers. A
repeating function is re-armed from when it ran, not from when it was due, so
one that fell behind does not fire a burst to catch up. One that raises is
dropped and counted in `errors`. Everything waiting is dropped when the preset
is loaded again or another is.

They run from a task in the main loop every millisecond, so a due function can
wait a few milliseconds behind a repaint or a SysEx message. Nothing stops one
that never returns - as nothing stops a `timer.onTick` that never returns.

`notes` is a note number or a list of them. A trigger fires on the change that
makes a device's held notes match, and not again until a change has made them
stop matching: a chord held down fires once. `match` has `notes` (the held notes
that made it), `root`, `transpose`, `velocity` (their average) and `spread`
(ms between the first and last press). Options:

| Option | |
| --- | --- |
| `device` | a `Device` or a device id; every device of the preset when left out |
| `exact` | nothing else may be held |
| `anyOctave` | notes by name: C is C in every octave |
| `anyKey` | the same shape on any root |
| `minVelocity`, `maxVelocity` | every matching note within 0..127 |
| `within` | the notes pressed within this many ms of each other, 0..60000 |
| `once` | the trigger is removed when it fires |
| `onRelease` | `fn(device, held)` when the notes stop matching |

A note belongs to the device the preset hands that port and channel to. The held
notes are followed from power-on, preset or none, so a trigger set while a chord
is already down sees it. A Note Off, a Note On of velocity 0, All Sound Off,
All Notes Off, the mode messages and a System Reset release notes; the sustain
pedal does not.

### `helpers` — `apps/controller/src/Lua/luaHelpers.cpp`

`slotToBounds(slot)`, `boundsToSlot({x, y, width, height})`, `delay(millis)`
(1..5000).

The arithmetic between a knob and what it drives, the same as the mk2's. All
pure, so they work at load time too.

| Function | Answers |
| --- | --- |
| `clamp(value, low, high)` | `value` held to the range; `low` and `high` may come either way round |
| `map(value, inLow, inHigh, outLow, outHigh)` | `value` moved from one range to the other, held to the output range. An empty input range raises an error |
| `scale(value, inLow, inHigh, outLow, outHigh [, curve])` | `map()` with the position raised to `curve` first: above 1 the low end gets more of the travel. `curve` defaults to 1 and must be above 0 |
| `midiToSigned(midiValue [, bitWidth [, signMode]])` | a MIDI value read as a signed number: `midiToSigned(127)` is `-1` |
| `signedToMidi(value [, bitWidth [, signMode]])` | a signed number as MIDI, clamped to what the width holds: `signedToMidi(-64)` is `64` |

`bitWidth` is 1 to 14 and defaults to 7; `signMode` is `"twosComplement"` (the
default) or `"signBit"`. A MIDI value wider than `bitWidth` raises an error
rather than being masked.

---

## Objects

### `Control` — from `controls.get()`

`getId()`, `getName()` / `setName(name)`, `getColor()` / `setColor(colour)`,
`getVariant()` / `setVariant(variant)`, `isVisible()` / `setVisible(bool)`,
`getBounds()` / `setBounds({x, y, width, height})`,
`setSlot(slot [, pageId])`, `setPot(controlSet, potId)`,
`getValueIds()`, `getValue([valueId])`, `getValues()`, `repaint()`, `print()`,
`setPaintCallback(fn)`, `setTouchCallback(fn)`, `setPotCallback(fn)`,
`setPotTouchCallback(fn)`, `setSwitchCallback(fn)`.

A control is also a table: fields a script sets on it survive to the next
callback, so a Custom control can keep its own state on itself.

The five callbacks are each handed the control as their first argument.

* paint: `fn(control)` - draw with `graphics`
* touch: `fn(control, event)` - `type`, `id`, `x`, `y`, `touchDownX`,
  `touchDownY`
* pot: `fn(control, event)` - `type`, `id`, `valueId`, `delta`
* pot touch: `fn(control, event)` - the same table, with `delta` zero
* switch: `fn(control, potId, pressed)`

All five exist on the mk2 as well.

Variant numbers are the mk2's, gaps and all: `0` default, `2` thin, `3` value
only, `4` dial, `5` checkbox, `7` baseline.

### `ControlValue` — from `control:getValue()`

`getId()`, `getControl()`, `getMessage()`,
`getDefault()` / `setDefault(value)`, `getMin()` / `setMin(value)`,
`getMax()` / `setMax(value)`,
`setRange(min, max, default, updateMessage)`,
`getOverlayId()` / `setOverlayId(overlayId)`,
`getSensitivity()` / `setSensitivity(factor)`,
`overrideValue(text)`, `cancelOverride()`,
`toMidi(displayValue)`, `fromMidi(midiValue)`, `print()`.

`toMidi()` is the MIDI value a display value is sent as, and `fromMidi()` the
display value a MIDI value is shown as - the same translation the control uses,
sign mode included. A list's display value is an item index and its MIDI value
that item's value; a pad's display value is `0` or `1` and its MIDI value the
message's off or on value. Arguments outside the range are held to it: on a
`100`..`8000` fader sent as a 7-bit CC, `toMidi(99999)` is `127`.

`setSensitivity(factor)` is how far the knob turns to move this value, as a
factor on the usual rate: `0.1` is a tenth of it, `10` ten times. The usual rate
is a whole range per three hundred degrees of travel, whatever the range. It is
read on every turn, so the next one already moves at the new rate, and `0.001` to
`100` is the range a factor may take. A preset carries the same thing as
`"sensitivity"` on a value.

The device keeps a factor in one byte, so what comes back out of
`getSensitivity()` is the nearest of two significant figures times a power of
ten - every round figure exactly, and `0.333` as `0.3`.

### `Message` — from `value:getMessage()`

`getType()` / `setType(type)`, `getDeviceId()` / `setDeviceId(deviceId)`,
`getParameterNumber()` / `setParameterNumber(n)`,
`getValue()` / `setValue(midiValue)`,
`getMin()` / `setMin(v)`, `getMax()` / `setMax(v)`, `setRange(min, max)`,
`getOnValue()` / `setOnValue(v)`, `getOffValue()` / `setOffValue(v)`,
`getBitWidth()`, `isLsbFirst()`, `getSignMode()`, `print()`.

`getSignMode()` answers the preset's own word for it: `"noSign"`,
`"twosComplement"`, `"signBit"`, `"signBit2"` or `"binOffset"`.

### `Page` — from `pages.get()`

`getId()`, `getName()` / `setName(name)`, `isHidden()` / `setHidden(bool)`,
`print()`.

### `Group` — from `groups.get()`

`getId()`, `getLabel()` / `setLabel(label)`, `getColor()` / `setColor(colour)`,
`isVisible()` / `setVisible(bool)`,
`getBounds()` / `setBounds({x, y, width, height})`,
`setSlot(slot, width [, height])`, `setHorizontalSpan(width)`,
`setVerticalSpan(height)`, `setVariant(variant)`, `print()`.

### `Device` — from `devices.get()`

`getId()`, `getName()` / `setName(name)`, `getPort()` / `setPort(port)`,
`getChannel()` / `setChannel(channel)`, `getRate()` / `setRate(millis)`,
`print()`.

### `Overlay` — from `overlays.get()`

`print()`.

### `SysexBlock` — from `SysexBlock()`, `midi.onSysex()`, `patch.onResponse()`

`getLength()`, `getManufacturerSysexId()`, `read()`, `write(byte)`,
`seek(position)`, `peek(position)`, `poke(position, byte)`, `close()`,
`print()`. Positions count from one.

A block a script makes with `SysexBlock()` is freed when Lua collects it; one
handed over by a callback belongs to the firmware and is not.

### `Component` — from `window.findChildById()`

Opaque; hand it to `window.addAndMakeVisible()`.

---

## Preset functions

Three kinds of plain global function, named from the preset rather than
registered by the script.

* a **formatter**, named by a value's `formatter` field:
  `fn(valueObject, value)` returning the text to show
* a **function**, named by a value's `function` field: `fn(valueObject, value)`,
  called whenever the value changes
* a **SysEx template function**, named inside a message's byte pattern:
  `fn(device, value)` returning one byte

A formatter that raises, returns nothing, or returns something that is not text
leaves the value showing what it showed before, and says so in the log.

---

## Global constants

| Group | Names |
| --- | --- |
| Interfaces | `MIDI_IO` 0, `USB_DEV` 1, `USB_HOST` 2 |
| Ports | `PORT_1` 0, `PORT_2` 1, `PORT_CTRL` 2 |
| Origins | `INTERNAL` 0, `MIDI` 1, `LUA` 2 |
| Models | `MODEL_MK1` 1, `MODEL_MK2` 2, `MODEL_MINI` 3 |
| Buttons | `BUTTON_1` .. `BUTTON_6` |
| Pots | `POT_1` .. `POT_12` |
| Touch points | `TOUCH_POINT_1` .. `TOUCH_POINT_5` |
| Message types | `NOTE_OFF` 128, `NOTE_ON` 144, `POLY_PRESSURE` 160, `CONTROL_CHANGE` 176, `PROGRAM_CHANGE` 192, `CHANNEL_PRESSURE` 208, `PITCH_BEND` 224, `SYSEX` 240, `TIME_CODE_QUARTER_FRAME` 241, `SONG_POSITION` 242, `SONG_SELECT` 243, `TUNE_REQUEST` 246, `CLOCK` 248, `START` 250, `CONTINUE` 251, `STOP` 252, `ACTIVE_SENSING` 254, `RESET` 255 |
| Bounds indices | `X` 1, `Y` 2, `WIDTH` 3, `HEIGHT` 4 |
| Event flags | `NONE` 0, `PAGES` 1, `CONTROL_SETS` 2, `USB_HOST_PORT` 4, `POTS` 8, `TOUCH` 16, `BUTTONS` 32, `WINDOWS` 64 |
| Colours | `WHITE`, `RED`, `ORANGE`, `BLUE`, `GREEN`, `PURPLE` |
| Parameter types | `PT_VIRTUAL` 0, `PT_CC7` 1, `PT_CC14` 2, `PT_NRPN` 3, `PT_RPN` 4, `PT_NOTE` 5, `PT_PROGRAM` 6, `PT_SYSEX` 7, `PT_START` 8, `PT_STOP` 9, `PT_TUNE` 10, `PT_ATPOLY` 11, `PT_ATCHANNEL` 12, `PT_PITCHBEND` 13, `PT_SPP` 14, `PT_RELCC` 15, `PT_NONE` 16 |
| Control sets | `CONTROL_SET_1` .. `CONTROL_SET_3` |
| Group variants | `VT_DEFAULT` 0, `VT_HIGHLIGHTED` 1 |
| Text alignment | `ALIGN_LEFT` 0, `ALIGN_CENTER` 1, `ALIGN_RIGHT` 2 |
| Text face | `FONT_REGULAR` 0, `FONT_BOLD` 1 |
| Text size | `SIZE_SMALL` 0, `SIZE_MEDIUM` 1, `SIZE_LARGE` 2, `SIZE_HUGE` 3 |
| Text spacing | `SPACING_PROPORTIONAL` 0, `SPACING_MONOSPACED` 1 |

The Lua standard library is open, and `print()` goes to the firmware log.

---

## Not implemented

Deliberately, because each needs functionality this model does not have rather
than a binding:

* **Data pipes** (`pipe.acquire`, `pipe.send`, `pipe.release`) and the capture
  API. Out of scope for this firmware.
