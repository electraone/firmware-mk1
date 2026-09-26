# Commands

A command is one thing the instrument can be told to do, named so that
something outside the firmware can ask for it. Five things can:

| Surface | How |
| --- | --- |
| A button on the panel | `hardware.buttons` in the configuration |
| An incoming MIDI message | `midiControl` in the configuration |
| A host | `08 13` over SysEx |
| A script | `commands.run()` |
| The screen | the FUNCTIONS window, and every window behind it |

They all end up in the same place. Written the usual way each would carry its
own list of names and its own switch, and the lists would drift - which is what
had already happened here: the configuration knew nine events, SysEx knew a
different handful of messages, and a script knew none of it.

---

## The list

One row per command in [`Commands.inc`](apps/controller/src/Api/Commands.inc),
and the enumerator, the name tables and the dispatch table are generated from
it, so they cannot disagree. Adding a command is a row there and a handler body
in `CommandManager.cpp`; nothing else.

| Name | What it does | Parameter |
| --- | --- | --- |
| `openPresetsMenu` | Open presets | — |
| `openPagesMenu` | Open pages | — |
| `openUsbHost` | Open USB host | — |
| `openSnapshots` | Open snapshots | — |
| `openPresetMenu` | Open preset functions | — |
| `closePopupWindow` | Close window | — |
| `openDetail` | Open detail | `control id`, or none for the knob in hand |
| `closeDetail` | Close detail | — |
| `lockDetail` | Lock detail | — |
| `switchPreset` | Switch to preset | `preset, 1 to 72` |
| `switchPresetNext` | Next preset | — |
| `switchPresetPrev` | Previous preset | — |
| `switchPresetBank` | Switch preset bank | `bank, 1 to 6` |
| `switchPage` | Switch to page | `page, 1 to 12` |
| `switchPageNext` | Next page | — |
| `switchPagePrev` | Previous page | — |
| `switchControlSet` | Set control set | `set, 1 to 3` |
| `switchControlSet1` | Control set 1 | — |
| `switchControlSet2` | Control set 2 | — |
| `switchControlSet3` | Control set 3 | — |
| `switchControlSetNext` | Next control set | — |
| `switchControlSetPrev` | Previous control set | — |
| `switchHandleNext` | Next handle | — |
| `switchHandlePrev` | Previous handle | — |
| `reloadPreset` | Restart preset | — |
| `requestPatch` | Request patch | — |
| `sendCurrent` | Send all values | — |
| `setAllDefaultValues` | Set all default values | — |
| `runUserFunction` | Run a preset function | `function, 1 to 12` |
| `switchSnapshotBank` | Switch snapshot bank | `bank, 1 to 12` |
| `loadSnapshot` | Load snapshot | `snapshot, 1 to 36` |
| `enableMidiLearn` | MIDI learn on | — |
| `disableMidiLearn` | MIDI learn off | — |
| `sleep` | Sleep | — |
| `reboot` | Reboot | — |

Every parameter counts from one, including the control set - which is an index
from zero inside the firmware, where it names a position rather than a set.

A command whose parameter is outside what it accepts does nothing and says so,
rather than being clamped into doing something nobody asked for.

The names are the mk2's, so a configuration written for one reads on the other.
What the mk2 has and this does not - captures, performances, the editor, the
morphing, button learn - is missing because the model is, not because the name
is spelled differently.

### There is no queue

The newer firmware puts commands through a queue because its callers are
separate threads. Every caller here is a task of one cooperative scheduler, so
a command runs on the thread that asked for it and is finished before the next
task gets a turn. That is what the MIDI path already did before any of this
existed; a queue would buy nothing and would need memory to hold.

---

## Trying it

`examples/commands.cfg` is a configuration that assigns a button and a handful
of CC numbers, and `examples/pot-scaling.lua` is a script that declares preset
functions. `examples/README.md` says how to put them on a unit.

---

## From a button

```json
"hardware": {
  "buttons": {
    "rightTop": { "primary": "openPresetMenu", "knobTouch": "openDetail" },
    "rightMiddle": { "primary": "switchPage", "parameter": 2 }
  }
}
```

The six are `leftTop`, `leftMiddle`, `leftBottom`, `rightTop`, `rightMiddle`
and `rightBottom`. `primary` is what the button does on its own; `knobTouch` is
what it does while a finger is on a knob, which is already how the panel works -
the three buttons that pick a control set open the detail of whatever is being
turned instead.

`parameter` is the button's, not the command's, because a button that runs two
commands wanting two different numbers is not a thing anyone has asked for.

A button the file does not name keeps what it has always done, and so does one
whose command is not a name this instrument has. A configuration with no
`hardware` section leaves the panel exactly as it was.

## From an incoming MIDI message

The `midiControl` section, which already existed and now reaches every command
rather than nine of them:

```json
"midiControl": [
  { "midiMessage": "cc7", "parameterNumber": 110,
    "command": { "type": "openPresetMenu" } },
  { "midiMessage": "cc7", "parameterNumber": 112,
    "command": { "type": "switchPage", "parameter": 2 } },
  { "midiMessage": "cc7", "parameterNumber": 117, "event": "switchPageNext" }
]
```

Both spellings are read: the older `"event"` with `"eventParameter"`, and
`"command"` with `"parameter"`. Inside `"command"`, `pageId` and `controlSetId`
are still accepted as the parameter, which is what they always meant. All nine
of the names the old list could carry are command names, so a configuration
written before this still reads unchanged.

Which port and channel these arrive on is `router.midiControlPort` and
`router.midiControlChannel`, as before - the control port and any channel by
default. Whatever window is open is closed first, as it always was: a message
from off the instrument means the hands are somewhere else.

## From a host

`F0 00 21 45 08 13 <payload> F7`, where the payload is either the name on its
own:

```
F0 00 21 45 08 13 6F 70 65 6E 50 72 65 73 65 74 4D 65 6E 75 F7   "openPresetMenu"
```

or a JSON object when the command takes a number:

```json
{"command":"switchPage","parameter":3}
```

ACK when it ran, NACK when the name is not one the instrument has or the number
is outside what the command takes.

## From a script

```lua
commands.run("openPresetMenu")
commands.run("switchPage", 3)

if commands.exists("triggerCapture") then ... end

for i, name in ipairs(commands.getNames()) do print(name) end
```

`run` returns whether the command ran, and raises only when the name is not one
the instrument has - a mistake in the script rather than a thing that happened.

---

## The FUNCTIONS window

`openPresetMenu` opens it. Twelve buttons, one under each knob, holding what
the preset itself offers, and a row above them of the four things that apply to
any preset:

| Button | Command |
| --- | --- |
| SEND VALUES | `sendCurrent` |
| REQUEST | `requestPatch` |
| DEFAULTS | `setAllDefaultValues` |
| RESTART | `reloadPreset` |

The twelve come from the script, which declares them as `preset.userFunctions`:

```lua
preset.userFunctions = {
    pot1 = { name = "DEFAULTS", call = function()
                 commands.run("setAllDefaultValues")
             end },
    pot2 = { name = "PAGE 1", call = function()
                 commands.run("switchPage", 1)
             end, close = true }
}
```

`name` goes on the button, `call` is what running it does, and `close` asks for
the window to be put away afterwards - which is what a function that switches
page wants. An entry the preset does not declare leaves its button blank and
its knob deaf, so turning it lights nothing.

Nothing is stored for them in the firmware: the table belongs to the script and
both the labels and the call read it where it lives, so a preset with no script
and a preset that declares no functions cost the same. The names are read when
the window opens rather than on every repaint - a script is free to rewrite the
table, and a paint that ran Lua would be running it from the repaint task.

`runUserFunction` is the same thing as a command, so a hardware button or an
incoming MIDI message can fire one of the preset's own functions without the
window being open at all.
