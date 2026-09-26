# The SysEx control API

Every Electra SysEx message this firmware answers, and every documented one it
does not. Written against the published implementation and checked against the
`electraone` command-line client, which is a complete host of the protocol -
where the documentation and a working client disagree, the client wins.

All messages are `F0 00 21 45 <command> <object> [ … ] F7`. A host may put a
transaction id in front of the command - `00 <lsb> <msb>` straight after the
manufacturer id - and it is echoed on the ACK or NACK.

Every request is answered. A request this firmware does not implement is
refused with a NACK rather than ignored: a host cannot tell a message still in
flight from one that will never be answered, so silence stops its queue.

---

## Answered

### Information

| Message | What it does |
| --- | --- |
| `02 7F` | Electra info: firmware version, serial, hardware revision |
| `02 7C` | App info: the running application and the loaded preset |
| `02 7E` | Runtime info: free memory |
| `14 7E` | Reset the memory markers the free-memory figure is measured from |
| `02 10` | The devices plugged into the USB host sockets |

### Presets

| Message | What it does |
| --- | --- |
| `01 01`, `01 00` | Upload a preset into the selected slot |
| `02 01 [bank] [slot]` | Fetch a preset. Without the two bytes, the loaded one |
| `05 01 [bank] [slot]` | Remove a preset and its script |
| `05 08 [bank] [slot]` | Clear a slot, which is the same thing here |
| `02 04` | The preset list |
| `02 08 [bank] [slot]` | One slot: name, project id, whether it has a script |
| `09 08 [bank] [slot]` | Switch to a slot |
| `14 08 [bank] [slot]` | Select a slot without loading it |
| `08 08 [[bank] [slot]]` | Load a slot again. Without the bytes, the current one |
| `04 08` | Load a preloaded preset |

### Lua

| Message | What it does |
| --- | --- |
| `01 0C` | Upload a script into the selected slot |
| `02 0C [bank] [slot]` | Fetch a script. Without the two bytes, the loaded one |
| `05 0C [bank] [slot]` | Remove a script |
| `08 0D` | Run a line of Lua. `08 0C` is accepted too |

Uploading a preset deletes the slot's script, so a host writing both has to
send the preset first.

### Configuration

| Message | What it does |
| --- | --- |
| `01 02` | Upload the configuration |
| `02 02` | Fetch the configuration |
| `05 02` | Remove it, leaving the firmware on its defaults |

### Snapshots

| Message | What it does |
| --- | --- |
| `02 05` | The snapshot list of a project |
| `02 03` | One snapshot |
| `01 03` | Upload a snapshot |
| `04 06` | Update a snapshot's name and colour |
| `05 06` | Remove a snapshot |
| `06 06` | Swap two snapshots |
| `14 09` | Select the snapshot slot to save into |
| `7E 04` | Switch the snapshot bank |

### Live control

| Message | What it does |
| --- | --- |
| `14 07 [id lsb] [id msb]` | Update a control: name, colour, visibility, value |
| `14 0E [id lsb] [id msb] [value]` | Override a value's text |
| `09 0A [page]` | Switch page |
| `09 0B [set]` | Switch control set |
| `14 77` | The text in the bottom bar, up to forty characters |
| `03 01`, `03 00` | MIDI learn on and off |

### Events and logging

| Message | What it does |
| --- | --- |
| `14 79 [flags]` | Which events to report |
| `14 7B [port]` | Which port to report them on |
| `7F 7D [on] [level]` | The logger, and its level |
| `14 7D [port]` | Which port the logger writes to |

### System

| Message | What it does |
| --- | --- |
| `7F 7A [0\|1]` | Stop and resume display repaints |
| `7F 76` | Take a screenshot |
| `7F 75` | Save the application state. Already saved here, and said so |
| `7F 78` | Reboot |
| `7F 7F` | Reboot into update mode |

### Files

The file transfer API's reading and managing half. A file is named by a
descriptor rather than by a path, and only the `tmp` location and the
`screenshot` type are served: a descriptor naming anything else is refused
rather than guessed at, because a guess here is a path.

| Message | What it does |
| --- | --- |
| `02 34` | List a location |
| `05 34` | Remove every file in a location |
| `02 35` | Fetch a file, base64 encoded |
| `05 35` | Remove one file |
| `07 36` | Make a location's directory. Already there is success |
| `05 36` | Remove it. Refused unless it is empty |

### Events the controller sends

`7E 01` ACK · `7E 00` NACK · `7E 02` preset switched · `7E 03` snapshot changed
· `7E 04` snapshot bank switched · `7E 05` preset list changed · `7E 06` page
switched · `7E 07` control set switched · `7E 08` preset bank switched · `7E 09`
USB host changed · `7E 0A` pot touched · `03` MIDI learn report · `7F 00` log
line.

---

## Not answered

Each of these is refused with a NACK. They are listed with what implementing
them would actually take, which in every case is more than a binding.

### Staged upload — `01 2D`, `01 2E`, `01 2F`, `04 2D`, `7E 2D`

The cache transaction: open, register each file with its size, send it in
chunks, commit with an MD5 of each. Presets, scripts and configuration all have
their own upload messages here and those work, so this is needed only for what
this model does not have anyway - firmware images, preloaded modules, asset
packs.

Two things are missing rather than unwritten. There is no MD5 anywhere in this
firmware, and a commit that skipped the digest would defeat the point of the
transaction. And the encoding of a chunk's payload is not documented; guessing
it would produce a firmware that accepts uploads and silently corrupts them,
which is worse than one that refuses them.

### Captures — `02 30`, `02 31`, `04 06`, `05 06`, `06 06`, `14 33`, `01 30`

Recorded MIDI, with its own bank and slot storage. Out of scope.

### The Lua debugger — `14 40`, `02 40`

Attach, breakpoints, stepping, stack and variable inspection, all as
sub-commands of one object. A debugger, not a message.

### Device overrides, performance and persisted data — `0F`, `11`, `12`

Three files a newer preset slot carries and this one does not: per-preset device
overrides, a performance layout, and a Lua data file. Each needs the model to
gain a file, not the API to gain a call.

### Snapshot banks — `02 38`, `04 37`, `05 37`, `05 38`

Naming and clearing whole snapshot banks. The snapshot database here records no
bank name, so this needs a field in it and a migration for the databases already
on cards.

### Pinning a slot — `04 08` with a `pinned` document

A pinned preset keeps running after the controller switches away from it. There
is one preset in memory on this model.

### The parameter map dump — `02 41`

A debugging aid rather than something an editor needs. Cheap to add if it is
ever wanted: the map already knows how to write itself to a file, and a file is
already sendable.

### Other locations for the file API

`slots`, `modules`, `presets`, `updates` and `assets` are refused. The
directories behind them either do not exist on this model or are not something
to expose to a path built from a wire message.
