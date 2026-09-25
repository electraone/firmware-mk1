# Findings from instrumenting v3.5.4

Everything here was found while building the compatibility harness against the
real unit. Each item says how it was confirmed. Nothing here has been fixed;
they are reported so the fixes can be scheduled deliberately, and so the
harness can be used to prove a fix does not change anything else.

Ordered by how much damage each can do.

---

## 1. Any large object in `.bss` can hard fault the firmware

**Confirmed by reproducing the crash and fixing it by alignment alone.**

This part reaches SRAM_L below `0x20000000` over the code bus and SRAM_U at
and above it over the system bus. A single memory access may not span the
boundary between them; one that does raises a bus fault. The linker script
lays `.data` and `.bss` straight across the boundary, so whether a given
object straddles it is decided by nothing more than how much static data
happens to precede it.

The trace buffer landed across the boundary. Every large SysEx upload then
hard faulted the firmware inside a `memcpy`, reproducibly, with an imprecise
bus error escalated to a forced hard fault. Aligning the buffer to its own
size, which makes straddling impossible, fixed it completely with no other
change.

No object in the current production build crosses the boundary, so this is
latent rather than live. It will not stay latent: adding or enlarging any
static buffer reshuffles every address after it. A buffer that works today
faults tomorrow because something unrelated grew, and the symptom is a hard
fault in `memcpy` far from the change that caused it.

Worth fixing before the memory work starts, not after. The cheapest fix is a
linker script that keeps `.bss` objects from spanning the boundary; the next
cheapest is aligning every large buffer to its own size, as
[Trace.cpp](../../src/System/Trace.cpp) now does.

## 2. `micros()` re-enables interrupts its caller had masked

**Confirmed by reading [pins_electra.c:582](../../bsp/pins_electra.c:582).**

`micros()` brackets its work in `__disable_irq()` and `__enable_irq()`, and the
enable is unconditional. Calling it from inside a critical section, or from an
interrupt handler that masked interrupts, silently lifts that protection for
everything that follows.

`delay()` calls `micros()`, so the hazard travels with it. The SD card driver
calls both. Nothing is visibly broken today, but the failure mode is
corruption at a distance, which is the kind that costs a week to find.

Saving and restoring the mask instead of forcing it on would make it safe
everywhere.

## 3. The logger throws away almost everything it is given

**Confirmed by reading [Logger.cpp:80](../../src/System/Logger.cpp:80) and by
tracing: 62 messages were logged during one boot and none reached the wire.**

The gate is

```c
if ((enabled && (level < minimumLevel)) || (level == LOG_LUA))
```

`minimumLevel` starts at `LOG_ERROR`, which is 0, so `level < minimumLevel` is
`level < 0` and never true for an unsigned level. The comparison is also the
wrong way round for a minimum. The effect:

- every `LOG_ERROR`, `LOG_WARNING`, `LOG_INFO` and `LOG_TRACE` message is
  discarded, whatever the configuration,
- `LOG_LUA` messages are sent regardless of `enabled`, so disabling the logger
  does not disable them,
- `setLevel()` clamps to 3, so it can never admit `LOG_LUA` through the first
  clause either.

The intended condition is almost certainly `enabled && (level <= minimumLevel)`.
Be aware that fixing it turns on a large amount of currently silent output,
some of it in hot paths.

There is also a `volatile bool loggerEnabled` global that `write()` never
consults, and the saved logger status is not applied at boot: the line that
would do it is commented out at
[ControllerApp.cpp:86](../../apps/controller/src/ControllerApp.cpp:86).

## 4. A preset whose controls fail to parse loads as empty and reports success

**Confirmed by reading [Preset.cpp:353](../../apps/controller/src/Model/Preset.cpp:353).**

Every stage of `Preset::parse` returns `false` when it fails, except the last
one. `parseControls` failing calls `reset()` and then falls through to

```c
System::logger.write(LOG_INFO, "Preset::parse: successfully parsed preset: ...");
return (true);
```

So a preset with a malformed control array is reported as loaded, and the user
gets an empty preset with no indication of why. Given finding 3, the error
message that precedes it is discarded as well.

Adding the missing `return (false)` is the whole fix.

## 5. A control without a colour passes a null pointer to `strtol`

**Confirmed by reading [Colours.cpp:178](../../src/Graphics/Colours.cpp:178)
and its two callers.**

`Colours565::fromString` is `strtol(stringNumber, 0, 16)` with no null check.
Both callers in the preset parser pass
`jControl["color"].as<const char *>()`, which is null when the key is absent.
`strtol(NULL, ...)` is undefined behaviour.

It does not crash on this part, because address 0 is readable flash, so the
call returns garbage instead of faulting. That is luck, not safety, and it
makes `color` effectively mandatory in a preset without saying so anywhere.

## 6. The `DEBUG` build does not compile

**Confirmed by compiling with `-DDEBUG`.**

Three call sites pass a bare `ERROR` where they mean `LOG_ERROR`:
[TaskReadMidi.cpp:114](../../src/System/TaskReadMidi.cpp:114),
[TaskReadMidi.cpp:169](../../src/System/TaskReadMidi.cpp:169) and
[TaskSendMidi.cpp:67](../../src/System/TaskSendMidi.cpp:67).

One references a variable that does not exist in that scope:
[ParameterMap.cpp:166](../../apps/controller/src/Model/ParameterMap.cpp:166)
passes `origin` inside a function that has no such parameter.

```
src/System/TaskReadMidi.cpp:114:13: error: 'ERROR' was not declared in this scope
apps/controller/src/Model/ParameterMap.cpp:166:9: error: 'origin' was not declared in this scope
```

The debug logging has therefore been dead for long enough to rot. Since
finding 3 means it would produce no output even if it compiled, both are worth
fixing together or deleting together.

## 7. Nine tenths of the cost of a Lua call is the garbage collection after it

**Measured with the harness, from `baseline/v3.5.4-cc-sweep.json`.**

`luaLE_postFunctionCleanUp` forces a full collection after every single call
into Lua, at [luaIntegrationBase.cpp:138](../../src/Lua/luaIntegrationBase.cpp:138).
The code already carries a note saying this needs improving. Measured per
call, in microseconds:

| handler | the call itself | the cleanup after it |
|---|---|---|
| `formatSeven` formatter | 61 | 470 |
| `formatBend` formatter | 56 | 469 |
| `onSeven` function | 25 | 459 |
| `onNrpn` function | 27 | 461 |

So a Lua call costs about half a millisecond, of which roughly 90 per cent is
the forced collection, eight to eighteen times the cost of the work being
done. Every incoming MIDI message that touches a control with a formatter and
a function pays this twice, plus once more for `parameterMap.onChange`.

This is the single clearest lead for the performance and memory work. Anything
that collects less often, an incremental step or a collection driven by actual
allocation pressure, should be worth more than a millisecond per message.

## 8. The build silently mixes settings when a flag changes

**Confirmed by observing a firmware that was half traced and half not.**

Object files depended only on sources, not on the compiler flags. Changing
`TRACE` or `SD_FORCE_1BIT_BUS` left every object stale, and the link produced
a firmware that was a mixture of both settings, with no warning. This cost
real time during this work: a traced build ran against an untraced RTT object
and dropped every record.

Already fixed in the Makefile as part of this work; objects now depend on the
flags. Mentioned because any build made before that fix, on either side of a
flag change, cannot be trusted.

---

## Behaviour that looked wrong but is not

Recorded so nobody spends time on them twice.

**Incoming MIDI is forwarded to the other interfaces before the application
sees it.** The trace shows the outgoing copies on the DIN and USB host ports
ahead of the `MI` record. That is `readMidi` doing routing in the first queue
before handing the message to the second. Correct, if surprising in a trace.

**A control change carrying the value a parameter already holds produces no
parameter map change.** `LookupEntry::setMidiValue` reports whether the value
actually changed, and the notification is skipped when it did not.

**Parameter map changes during preset load report device 0 and message type
`none`.** At load time the entry has no destinations linked yet, so
`getMessage()` returns a default. The handler skips sending for `Origin::file`,
so nothing wrong reaches the wire. Changes from MIDI carry the real device and
type.
