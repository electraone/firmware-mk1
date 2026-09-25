# What else could live in the display's memory

The processor has 256 KB of RAM. The display controller has 16 MB, of which
about 6.9 MB is unused: five whole pages (`SDRAM_PAGE_3`, `5`, `6`, `7`, `8`)
are allocated in the map and never written, and the character generator region
uses 3 KB of its 800 KB.

Moving data across is not free, so this list is ordered by what it saves
against what it costs, and every size here was measured on the hardware rather
than counted on paper.

## The number that decides everything

A read from the display's memory costs about **45 µs of fixed overhead plus
0.8 µs per byte**, measured after the SPI work in
[SPI-PERFORMANCE.md](SPI-PERFORMANCE.md). A typical label of forty-odd bytes
therefore costs roughly **80 µs to fetch**.

That sets the rule cleanly:

- **Fine**: read once while painting a control. A knob turn repaints one
  control, so 80 µs disappears into a repaint that already takes milliseconds.
- **Fine**: read once while painting a page. Thirty-six controls cost about
  3 ms on a full page repaint, which happens on a page change, not per frame.
- **Not fine**: anything on the MIDI path. A parameter map lookup runs per
  incoming message and must stay in RAM.
- **Not fine**: anything read in a loop, or read more than once per paint.

The second rule has a corollary worth acting on first, below.

## Measured sizes

| Type | Bytes each | Of which is text |
|---|---|---|
| `ControlValue` | 68 | 16 |
| `Control` | 68 | 16 |
| `Group` | 72 | 41 |
| `Device` | 88 | 21 |
| `Page` | 28 | 21 |
| `LookupEntry` | 16 | 0 |

Declared ceilings, from `apps/controller/src/Model/Preset.h`: 432 controls,
32 devices, 12 pages, 72 preset slots.

## Ranked candidates

### 1. ~~Control value labels~~ — done, and measured

`ControlValue::label[16]` is now a 4 byte handle into the store, so every
value costs 12 bytes less. The flag saying whether a label is set stayed in
RAM, in padding that was already there, so a painter can ask without paying
for a transfer.

Measured on the hardware, production build, with
[make_stress_preset.py](make_stress_preset.py) and
[compare_memory.py](compare_memory.py):

| Preset | Controls | Values | Free before | Free after | Saved |
|---|---|---|---|---|---|
| stress-mixed | 288 | 576 | 54416 | 60560 | **6144** |
| stress-max | 216 | 864 | 43432 | 53808 | **10376** |

The max case is 864 × 12 = 10368 predicted against 10376 measured. The mixed
case comes out 768 short of its 6912 because the allocator rounds a block up:
a single-value control goes from 68 bytes to 56, which after rounding saves 8
rather than 12. Envelopes, being four values in one block, get the full 48.

The cost on the other side is 20 bytes of the display's memory per label that
is actually set, and the measurement above shows 384 bytes of it in use — few
values ever get a label, because only a formatter that has run writes one.

The claim that it was read only while painting was not quite right, and this
is the part that shaped the design. `callFormatter` **writes** the label, and
`ParameterMap::repaintLookupEntry` calls it on every value change of a control
that has a formatter. So the label is rewritten on the MIDI path, not written
once at load. That is why the store had to grow `reserve()` and `update()`:
the space is taken once and rewritten in place, since a bump allocator handing
out a fresh record per value change would be emptied by a single knob sweep.

The write costs a header read plus one transfer, on a path that already runs a
Lua call with a full garbage collection, so it disappears into that.

Readers, all of which copy the result immediately:
[FaderControl.cpp:243](../../apps/controller/src/Components/FaderControl.cpp:243),
`KnobControl.cpp:82`, `PadControl.cpp:148`, `RelativeControl.cpp:75`,
`ListControl.cpp:141`, `ListButtonControl.cpp:157`.

### 2. Group labels — best ratio per instance

`Group::label[41]` at
[Group.h:208](../../apps/controller/src/Model/Group.h:208), 41 of the 72 bytes
a group occupies. A busy preset with a dozen groups per page across twelve
pages saves about **5.9 KB**, and unlike controls there is no declared ceiling
to bound it.

Read at [GroupControl.cpp:31](../../apps/controller/src/Components/GroupControl.cpp:31)
while painting, and from Lua at `luaGroup.cpp:96`.

**Carries the lifetime problem below.**

### 3. Control names

`Control::name[16]` at
[Control.h:147](../../apps/controller/src/Model/Control.h:147). Up to
**6.9 KB** at 432 controls, 3.2 KB at a more usual 200.

**Carries the lifetime problem below.**

### 4. Preset slot names

`PresetSlot::presetName[21]` and `projectId[21]` at
[PresetSlot.h:76](../../apps/controller/src/Model/PresetSlot.h:76), 42 bytes
across 72 slots = **3.0 KB**.

Better than its size suggests: these are read only when the preset selection
grid is on screen, which is rare, and they are written once at startup when
the preset list is scanned. Nothing else touches them. There is no lifetime
problem here either, because the readers copy rather than borrow.

Note this data outlives a preset, so it needs a store that is not reset on
preset change. Use a second `VramStore` on one of the free pages rather than
the preset-scoped one.

### 5. Device and page names

`Device::name[21]` at [Device.h:66](../../apps/controller/src/Model/Device.h:66)
gives 672 B at 32 devices. `Page::name[21]` at
[Page.h:145](../../apps/controller/src/Model/Page.h:145) gives 252 B at 12
pages, but that is 75 % of what a `Page` costs at all.

Small in absolute terms. Worth doing only once the mechanism is in place and
proven by the ones above.

### 6. Device SysEx templates — the largest single body of data

`Device::requests`, `Device::responses` and `Device::sysexMessages` at
[Device.h:70](../../apps/controller/src/Model/Device.h:70) are vectors of byte
vectors holding SysEx templates. Size depends entirely on the preset and can
run to several kilobytes on a device-heavy one, with heap fragmentation on top
because each is a separate allocation.

These are read when a patch request is sent, not while painting, so the cost
lands somewhere the user is already waiting. The existing `MemoryBlock` is
built for exactly this shape and already mirrors the first 56 bytes in RAM.

Harder than the labels, and the payoff is the least predictable, but on
presets that use templated SysEx heavily it is probably the biggest number on
this page.

### 7. Lua function names

`Preset::luaFunctions` at
[Preset.h:70](../../apps/controller/src/Model/Preset.h:70) is a
`std::vector<std::string>`, so every name costs a `std::string` header plus a
heap block. Written once while parsing, read once per registration. A preset
with fifty Lua functions spends on the order of 2 KB on this for no benefit.

## Do not move

**`ParameterMap::entries`** at
[ParameterMap.h:507](../../apps/controller/src/Model/ParameterMap.h:507). This
is the MIDI path. An 80 µs read per incoming message would be catastrophic,
and the map is looked up far more than once per message.

**Anything reached from an interrupt.** The transfer takes the SPI bus and the
display needs it; blocking an interrupt handler on it invites exactly the kind
of corruption the SPI work just removed.

## Two things to fix before moving anything

### ~~The paint path reads the same record twice~~ — done, and it was smaller than stated

Fixed. What was actually there, after looking properly:

`getByIndex()` and `getByValue()` do **not** touch the display's memory. Only
`getLabel()` and `getBitmap()` do. So calling `getByIndex()` twice cost a
vector copy, not a transfer, and in `LookAndFeel::paintList`,
`paintButtonList`, `ListControl` and `ListButtonControl` the label and bitmap
branches are mutually exclusive: one fetch per paint, not two. The earlier
claim of 80 µs wasted per list control was wrong.

Two things were real and are now fixed:

- **`ListDetailControl::paint` did fetch twice**, once for the label and again
  for the bitmap, for every item in the viewport that has one. `ListDataItem`
  gained `getLabelAndBitmap()`, which returns both from a single fetch, since
  they live in one record anyway.
- **`FaderControl::paint` searched the list three times.** `getByValue()` is a
  linear scan, and it was asked the same question in the bitmap test, the
  label test and the label fetch. Now looked up once.

The redundant `getByIndex()` calls in the other painters were hoisted into a
local as well. Cheap either way, but it makes the pattern uniform and removes
the trap for anyone adding a second fetch later.

### Component names are borrowed pointers, not copies

`Component::name` is a `const char *` and `setName` only stores the pointer,
at [Component.h:432](../../src/Components/Component.h:432). `GroupControl`
hands it `group.getLabel()` directly. Once that label lives in the display's
memory, `getLabel()` returns a pointer to a shared scratch buffer that the
next call overwrites, exactly as `ListDataItem::getLabel` does now.

So candidates 2 and 3 need `Component` to copy the name into its own storage,
or every `getName()` caller audited for how long it holds the pointer. That is
the real work in those two, not the storage itself.

`Button::setLabel` and `Meter::setLabel` already copy, so those are safe.

## Suggested order

1. ~~Fix the double read in the list painters.~~ Done.
2. ~~Move control value labels.~~ Done: 6144 bytes on a 288 control preset,
   10376 on one with 864 values, both measured on the unit.
3. Make `Component` own its name, then move control names and group labels.
4. Move preset slot names, on a store that survives a preset change.
5. Look at the SysEx templates, which need their own design rather than a
   label-shaped one.

Steps 3 and 4 should add roughly another 10 KB at the sizes above. Step 5 is
unbounded and preset-dependent.

## Reproducing these numbers

The type sizes come from the benchmark build, which logs them at boot:

```bash
make APPPATH=apps/controller TRACE=1 BENCHMARK=1 BUILDDIR=build-bench hex
```

Look for the `bench sizeof` line.

The saving is measured rather than counted. Build the two images, put a big
preset on the unit, and let `compare_memory.py` flash each in turn and read
the firmware's memory snapshot out of target RAM:

```bash
python3 tools/compat/make_stress_preset.py --profile max --pages 6
electraone --port "Electra CTRL" preset switch -b 0 -s 3
electraone --port "Electra CTRL" preset upload tools/compat/presets/stress-max.epr
electraone --port "Electra CTRL" lua upload tools/compat/presets/stress-max.lua
python3 tools/compat/compare_memory.py --serial <probe> \
    --before ../old/build --after build --slot 3:stress-max
```

Two things to know about the figures. They are read out of RAM rather than
the trace, because a traced build spends 56 KB on its buffer and the firmware
refuses any preset that would leave under 62000 bytes free, so a preset big
enough to be interesting will not load on one. And free RAM is bounded by the
allocator's high water mark, which never comes back down, so the absolute
figure depends on what the unit has loaded since it booted. The comparison is
valid because both images are put through the same sequence; a single reading
on its own is not.
