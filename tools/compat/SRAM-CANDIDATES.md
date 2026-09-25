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

### 1. Control value labels — the biggest single win

`ControlValue::label[16]` at
[ControlValue.h:139](../../apps/controller/src/Model/ControlValue.h:139).

One per value, and a control has one value for a fader, list, pad or knob,
three for an ADR envelope, four for an ADSR and eight for a DX7 envelope.

| Preset size | Saving |
|---|---|
| 200 controls, one value each | 3.2 KB |
| 432 controls, one value each | 6.9 KB |
| 432 controls, ADSR throughout | 27.6 KB |

Read only while painting:
[FaderControl.cpp:243](../../apps/controller/src/Components/FaderControl.cpp:243),
`KnobControl.cpp:82`, `PadControl.cpp:148`, `RelativeControl.cpp:75`,
`ListControl.cpp:141`, `ListButtonControl.cpp:157`. No other reader.

This one is clean: the label is fetched, drawn and forgotten.

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
2. Move control value labels. Largest clean win, no lifetime problem.
3. Make `Component` own its name, then move control names and group labels.
4. Move preset slot names, on a store that survives a preset change.
5. Look at the SysEx templates, which need their own design rather than a
   label-shaped one.

Steps 2 through 4 add up to roughly **13 KB on a 200-control preset and 21 KB
at the declared maximum**, against 256 KB total. Step 5 is unbounded and
preset-dependent.

## Reproducing these numbers

The sizes come from the benchmark build, which logs them at boot:

```bash
make APPPATH=apps/controller TRACE=1 BENCHMARK=1 BUILDDIR=build-bench hex
```

Look for the `bench sizeof` line. The firmware also logs free RAM before and
after the preset loads, which is the honest before-and-after for any of this
work.
