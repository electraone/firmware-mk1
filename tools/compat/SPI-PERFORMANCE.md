# The SPI link to the display: before and after

All numbers measured on the hardware by `src/Hardware/DisplayBenchmark.cpp`,
built with `BENCHMARK=1 TRACE=1` and read out over J-Link. Every transfer in
the benchmark is verified by reading it back, so throughput and correctness
are reported together.

Reproduce with:

```bash
make APPPATH=apps/controller TRACE=1 BENCHMARK=1 BUILDDIR=build-bench hex
```

then flash `build-bench/firmware.hex` and read the trace. The results arrive
as `bench ...` lines.

## The headline

| Transfer | Before | After | Gain |
|---|---|---|---|
| Block write, 4 KB | 351 KB/s | 3,305 KB/s | 9.4× |
| Block read, 4 KB | 345 KB/s | 1,220 KB/s | 3.5× |
| Single byte, either way | 53 µs | 41 µs | 1.3× |
| Transfer errors, 256 KB hammered | 0 | 0 | held |

Write throughput is now 90 % of what a 30 MHz link can physically carry.
Read throughput is 98 % of what a 10 MHz link can carry, and 10 MHz is as fast
as this display will serve a burst read (see below).

## Full measurements

Before, every path byte-at-a-time:

| Block size | Write KB/s | Read KB/s | Errors |
|---|---|---|---|
| 1 byte | 19 | 19 | 0 |
| 16 B | 166 | 164 | 0 |
| 64 B | 275 | 271 | 0 |
| 256 B | 329 | 324 | 0 |
| 1 KB | 346 | 340 | 0 |
| 4 KB | 351 | 345 | 0 |

After:

| Block size | Write KB/s | Read KB/s | Errors |
|---|---|---|---|
| 1 byte | 24 | 24 | 0 |
| 16 B | 326 | 279 | 0 |
| 64 B | 1,016 | 667 | 0 |
| 256 B | 2,151 | 1,019 | 0 |
| 1 KB | 2,985 | 1,174 | 0 |
| 4 KB | 3,305 | 1,220 | 0 |

Integrity: 64 rounds of 4 KB written and read back, 262,144 bytes, **zero bad
bytes**, zero status-wait timeouts.

## The clock is not the lever

The header asks for 32 MHz. It never got 32 MHz and never could.

The SPI peripheral derives its clock by dividing the bus clock, and the
smallest divisor is two. The bus runs at 60 MHz, so **30 MHz is the ceiling**,
and a request for anything at or above 30 MHz silently resolves to exactly
that. The request for 32 MHz was not wrong so much as meaningless.

Raising it further would mean raising the peripheral bus to 90 MHz, which
changes the timing of every peripheral on the chip, not just this one. That is
not a change to make for a display driver.

So the whole improvement comes from moving more useful bytes per clock, not
from clocking faster.

## What was actually costing the time

**Four SPI bytes per stored byte.** Writing one byte to video memory went
through `writeReg`, which issues a command cycle and then a data cycle, each
with its own chip-select pulse: two bytes on the wire for the command, two for
the data. A 4 KB block therefore cost 16 KB of wire traffic plus 8,192
chip-select transitions.

Video memory has an auto-incrementing pointer, so a whole block can travel in
one command cycle followed by one data cycle. That is what
`RA8876::writeRamBulk` and `readRamBulk` now do: **one SPI byte per stored
byte**, one chip-select pulse per block.

**One stall per byte instead of per pair.** Every `SPI.transfer()` pushes one
frame and spins until the transfer-complete flag rises. A command or data
cycle is two bytes, so it paid that stall twice. Those pairs are now sent as a
single 16-bit frame, halving the stalls. This is what lifts the single-value
accessors from 53 µs to 41 µs, and it speeds up every register access in the
firmware, which is what a screen repaint is almost entirely made of.

**The transmit FIFO was never used.** Pushing one frame and waiting leaves the
four-entry FIFO empty. The bulk path keeps it fed, which is why large blocks
approach the wire rate.

## Reads must be slower than writes, and this is not optional

Driven at the write clock, a burst read returns **five bytes in eight**. The
missing three are not flagged anywhere: the controller simply does not supply
them, and what lands in the buffer is silently wrong.

This was found by pairing each bulk direction against the known-good
single-value accessors, which attributes the corruption instead of leaving
both directions under suspicion:

| Burst length | Bulk write errors | Bulk read errors |
|---|---|---|
| 64 B … 4 KB | 0 | 37.5 % of every block |

The write path is clean at every length. Then sweeping the read clock:

| Read clock | Read KB/s | Errors in 32 KB |
|---|---|---|
| 30 MHz | 3,309 | 8,192 |
| 20 MHz | 2,294 | 8,192 |
| 15 MHz | 1,783 | 3,996 |
| 12 MHz | 1,440 | 4,005 |
| **10 MHz** | **1,220** | **0** |
| 7.5 MHz | 913 | 0 |

10 MHz is the fastest clock at which this display serves a burst read
correctly, so burst reads switch to it and switch back afterwards. Writes keep
the full 30 MHz. Note that 12 MHz and 15 MHz are *not* safe: the failure is
not gradual, and picking a clock by eye rather than by measurement would have
looked fine in casual use and corrupted data under load.

Anyone changing `RA8876_SPI_READ_SPEED` should re-run this sweep rather than
reason about it.

## Correctness fixes that came out of this

**Multi-byte register reads returned nonsense.** `readRegNBytes` re-read the
same register for every byte instead of advancing, so `readReg16` returned the
low byte twice. That corrupted `saveState`, which the video memory accessors
call on every single operation, and the cursor position accessors, for any
coordinate above 255. It now advances the register the way its write
counterpart always did.

**Bulk writes read past the end of the caller's buffer.** The old loop stepped
through the buffer sixteen bytes at a time and transferred four bytes four
times, so any length that was not a multiple of sixteen over-read. It also
passed the caller's buffer as both source and destination, so the bytes
arriving on the wire overwrote the data being sent. Both are gone.

**The status waits could hang the firmware forever.** `waitForStatus` spun
with no bound, so a display that stopped answering took the whole system down
with no trace of why. The wait is now bounded at roughly 50 ms and counts
timeouts in `RA8876::statusWaitTimeouts`, which the benchmark reports. It has
been zero in every run so far.

## What is still on the table

**DMA.** The transfers are still CPU-driven. The peripheral can do eDMA, and
the pieces are present in the board support package but stubbed out. For the
4 KB write case that would free about 20 ms of CPU per 64 KB moved without
changing the throughput, because the link is already near the wire rate.

**Text drawing.** Proportional text costs roughly 38 SPI bytes and four status
polls per character, while the fixed-width path costs about six per character.
Only the expensive one is reachable from the text drawing used by the
application.

**A 90 MHz peripheral bus** would allow 45 MHz writes. It affects every
peripheral, so it is a system decision rather than a display one, and the read
ceiling would not move.
