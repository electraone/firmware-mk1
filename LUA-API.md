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

`drawArc`, the three colour functions and the two measuring functions are this
model's own; the rest is the mk2's set.

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

No functions; three callbacks. `preset.onLoad()` when the script has been read,
`preset.onReady()` when the parameter map has been brought up to date, and
`preset.onExit()` when the preset is being unloaded.

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

### `info` — `apps/controller/src/Lua/luaInfo.cpp`

`setText(text)`, `getText()`. Twenty characters, in the bottom bar.

### `helpers` — `apps/controller/src/Lua/luaHelpers.cpp`

`slotToBounds(slot)`, `boundsToSlot({x, y, width, height})`, `delay(millis)`
(1..5000).

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

`setPotTouchCallback` and `setSwitchCallback` are this model's own; the other
three are the mk2's.

Variant numbers are the mk2's, gaps and all: `0` default, `2` thin, `3` value
only, `4` dial, `5` checkbox, `7` baseline.

### `ControlValue` — from `control:getValue()`

`getId()`, `getControl()`, `getMessage()`,
`getDefault()` / `setDefault(value)`, `getMin()` / `setMin(value)`,
`getMax()` / `setMax(value)`,
`setRange(min, max, default, updateMessage)`,
`getOverlayId()` / `setOverlayId(overlayId)`,
`getSensitivity()` / `setSensitivity(factor)`,
`overrideValue(text)`, `cancelOverride()`, `print()`.

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
`print()`.

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
* **`preset.onEnter` / `preset.onLeave`**. On the mk2 a preset can be entered
  and left without being unloaded. Here loading a preset is entering it, so
  `preset.onLoad` and `preset.onExit` already mark both edges; adding the pair
  would mean a separate preset-residency lifecycle.
* **`preset.userFunctions`**. Needs an entry in the Preset Menu to trigger the
  functions from, which is UI work rather than a binding.
