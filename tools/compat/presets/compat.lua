-- Lua side of the compatibility test preset.
--
-- Every handler here exists to be observed, not to do anything useful. The
-- harness checks that each one is still called, with the same arguments, in
-- the same order, by a newer firmware.
--
-- Handlers deliberately stay cheap. Anything slow would change the timing of
-- the very sequence being recorded.

-- Formatter on the 7 bit control change fader.
function formatSeven(valueObject, value)
    return (string.format("S%d", value))
end

-- Formatter on the 14 bit control change fader.
function formatFourteen(valueObject, value)
    return (string.format("F%d", value))
end

-- Formatter on the pitch bend fader.
function formatBend(valueObject, value)
    return (string.format("B%d", value - 8192))
end

-- Function callback on the 7 bit control change fader.
function onSeven(valueObject, value)
    lastSeven = value
end

-- Function callback on the NRPN fader.
function onNrpn(valueObject, value)
    lastNrpn = value
end

-- Function callback on the pitch bend fader.
function onBend(valueObject, value)
    lastBend = value
end

-- Called for every parameter map change the firmware decides to publish.
parameterMap = {}
function parameterMap.onChange(controlValues, origin, value)
    lastOrigin = origin
    lastValue = value
end

-- Preset lifecycle.
preset = {}
function preset.onLoad()
    loaded = true
end

function preset.onReady()
    ready = true
end

function preset.onExit()
    ready = false
end

-- Page lifecycle.
pages = {}
function pages.onChange(newPageId, oldPageId)
    lastPage = newPageId
end

-- Generic events.
events = {}
function events.onPageChange(newPageId, oldPageId)
    lastEventPage = newPageId
end

function events.onPotTouch(potId, controlId, touched)
    lastPot = potId
end

function events.onUsbHostChange(port, eventType)
    lastUsbHost = port
end

-- Incoming MIDI callbacks.
midi = {}
function midi.onControlChange(midiInput, channel, parameterNumber, value)
    lastCc = parameterNumber
end

function midi.onNoteOn(midiInput, channel, noteNumber, velocity)
    lastNoteOn = noteNumber
end

function midi.onNoteOff(midiInput, channel, noteNumber, velocity)
    lastNoteOff = noteNumber
end

function midi.onProgramChange(midiInput, channel, programNumber)
    lastProgram = programNumber
end

function midi.onPitchBend(midiInput, channel, value)
    lastPitchBend = value
end

function midi.onSysex(midiInput, sysexBlock)
    lastSysex = true
end
