# System exclusive: how it is stored, and what was wrong with it

A review of the receive path, the send path, and the pool in the display's
memory that both of them use. Nine bugs, all fixed; the measurements below
are from the hardware.

## How it works

A system exclusive message arrives in packets. `SysexHandler::process` opens a
`SysexBlock` on `App::sysexPool`, writes each packet into it, and pushes the
finished block onto a queue for the application to deal with.

`MemoryPool` is a bump allocator over `SDRAM_PAGE_0`, 1.2 MB of the display
controller's memory. It never frees and never resets: each message takes the
next slice, and when the end is reached it starts again at the beginning.

`MemoryBlock` is one message in that pool, with a twist worth keeping: the
first 56 bytes are held in the processor's own RAM as well. That is deliberate
and good. Everything that looks at a message looks at its first bytes — the
manufacturer, the Electra command, a device's response header — and those
questions are then answered without touching the SPI bus at all. A message of
56 bytes or less never reaches the display's memory.

`SysexBlock` adds the reading of those first bytes: is this system exclusive,
whose manufacturer identifier is it, what Electra command does it carry, and
where does its payload start.

Sending reverses it: `readBytes` streams the block back out, RAM header first,
then the pool.

## What was wrong

### 1. The pool wrote past its own end, into the labels

`MemoryPool::write` had no bounds check, and the wrap was decided in
`close()`, which tested the address of the *previous* block against the
capacity:

```c
if (newMemoryBlockAddress > capacity) {   // the block before this one
    currentAddress = 0;
}
newMemoryBlockAddress = currentAddress;
```

So a block that ran off the end was written off the end first, and the pool
wrapped afterwards. What follows `SDRAM_PAGE_0` is `SDRAM_PAGE_1`, which is
the store holding list item labels and control value labels. The pool never
resets, so this was not a rare case: it is where a long enough session ends
up. Every message in and out consumes pool space for good.

Fixed three ways. `write()` now refuses anything that would cross the end and
counts it. The wrap is decided in `openAddress()`, before a block begins, so a
block is never split around the end. And a new block starts from the beginning
while there is not a quarter of the pool, at most 64 KB, ahead of it — so an
ordinary message never comes near the boundary, and only one larger than that
is truncated. A truncated block says so, and reports only the bytes it really
holds.

### 2. Reading a message cost 41 µs per byte

Everything that examines a received message walks it a byte at a time:
`Midi::processSysexData` matching a device's template,
`Midi::doesHeaderMatch`, `Midi::applyRulesValues`, and the MIDI learn logger
printing it. Past the 56 byte RAM header, each of those bytes was its own
`readRam8` — a whole SPI transaction, which is nearly all fixed overhead.

Measured on the unit: **41 µs per byte**. A 600 byte device response cost
22 ms to scan. A 4 KB patch dump cost 160 ms, with the run loop waiting.

`MemoryPool` now keeps a 256 byte window of itself in RAM and serves
sequential byte reads from it, invalidating it on any write. One window for
the whole pool rather than one per block, because blocks are copied about
freely and have to stay cheap to copy.

The same 600 byte scan now takes **968 µs**, 1.6 µs per byte: **23 times
faster**, for 256 bytes of RAM.

### 3. A malformed message could ask for four gigabytes

`SysexBlock::getElectraSysexPayload` worked the payload length out by
subtraction:

```c
return (getRange(offset, length - offset - 1));
```

On an Electra message shorter than seven bytes that wraps round, and the block
that came back claimed to be four gigabytes long. Anything that then read it
streamed the display's memory. Reachable from any device that can send a short
malformed message.

Fixed at both ends: the length is checked before the subtraction, and
`getRange` clamps its range to what the block actually holds.

### 4. `peek()` past the end returned a plausible byte

`MemoryBlock::peek` had no bounds check. `Midi::applyRulesValues` tests its
result:

```c
int sysexByte = sysexBlock.peek(bytePosition);
if (sysexByte != -1) {              // never false
```

That guard could never fire, because `peek` widened a `uint8_t` and returned
0 to 255 whatever was asked of it. A device response shorter than its rules
expect read whatever the pool held after the message and applied it to the
parameter map as a real value. `peek` now returns -1 past the end, which is
what the caller always believed it did.

### 5. Reading the manufacturer read one byte too far

`getManufacturerSysexId` checked `length < 3` and then read `header[3]` for
the three byte form. Now the three byte form requires four bytes.

### 6. Reading the Electra command read six bytes unchecked

`getElectraSysexCommand` read `header[4]` through `header[9]` with no length
check at all. A short Electra-prefixed message built its command from whatever
followed. It now builds from zeros for the bytes that are not there.

### 7. An empty block answered from uninitialised memory

`MemoryBlock` never cleared `header`, and `SysexBlock::isSysex` is just
`header[0] == 0xF0`. A default constructed block answered that question, and
the manufacturer question, from whatever the memory happened to hold. The
header is cleared, and the length is checked first.

### 8. A message over 255 bytes could hang the firmware

`Midi::processSysexData` walked the message with a `uint8_t` counter against a
`size_t` length:

```c
for (uint8_t i = 0; i < sysexLength; i++) {
```

A message longer than 255 bytes whose terminator sits past byte 255 never
ended that loop. The template index beside it was also `uint8_t` and stepped
by up to seven at a time, running off the end of its vector into the heap.
Both are `size_t` now, and the template index is bounded before each step.

### 9. A short write was reported as a full one

`MemoryBlock::writeBytes` returned the length it was given and added it to the
block's length whatever the pool did with it. With the pool now able to refuse
a write, that would have meant a block claiming bytes it does not hold, and a
reader being handed whatever was at those addresses before. It now counts only
what was stored.

## Also changed

`MemoryBlock::getRange` transferred 56 bytes from the display's memory every
time, even when the whole message was sitting in the RAM header and nothing
had ever been written to the pool. It now takes what it needs from wherever it
actually is.

A block is streamed out in 240 byte chunks rather than 60. The chunk still
divides by three, as USB system exclusive packets need, and a read costs far
more in fixed overhead than in bytes moved.

## One that is not a bug, but reads like one

Uploading a preset deletes the slot's Lua script, deliberately, in
`SysexHandlerCtrl`. So the preset has to be uploaded first and the script
after it. The harness README said the opposite, and following it leaves the
slot with no script: the recording then has no Lua calls in it at all, which
looks exactly like a firmware that stopped calling Lua. The README is
corrected.

## Left alone

`MemoryPool::saveString`, `saveItem`, `getString` and `getItem` are the older
string storage that `VramStore` replaced. They still have unbounded reads and
a variable length array on the stack, but nothing calls them any more. They
should be deleted rather than repaired.

`MidiIo::sendSysExPartial` ignores its `complete` argument and always tells
the MIDI library the array carries its own boundaries. That is correct, since
the stored block includes the leading 0xF0 and the trailing 0xF7, but the
argument being ignored makes it look like a bug and it is worth a comment or a
signature change.

## Checking it

The benchmark build exercises all of this on the hardware at boot:

```bash
make APPPATH=apps/controller SD_FORCE_1BIT_BUS=1 TRACE=1 BENCHMARK=1 \
    BUILDDIR=build-bench hex
```

Look for the `bench sysex` line. It covers a message that fits in the RAM
header, one that spans the boundary written in uneven pieces, reading past the
end, streaming against peeking, the Electra payload, a message too short to
have one, an empty block, and the end of the pool. It also times a 600 byte
scan, which is where the 968 µs above comes from; `bench single` on the same
line of output is where the 41 µs it replaced comes from.
