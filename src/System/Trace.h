/*
* Electra One firmware
* See COPYRIGHT file at the top of the source tree.
*
* This product includes software developed by the
* Electra One Project (http://electra.one/).
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.
*/

/**
 * @file Trace.h
 *
 * @brief Behavioural trace used by the compatibility test harness.
 *
 * The trace records what the firmware did and, more importantly, in which
 * order it did it: incoming MIDI, calls into Lua, parameter map changes and
 * outgoing MIDI. Records leave the device over SEGGER RTT, so the MIDI ports
 * stay untouched and the observed MIDI behaviour is the real one.
 *
 * Everything here compiles away completely unless the firmware is built with
 * TRACE=1, so production images carry neither the code nor the buffer.
 *
 * Record syntax, one per line:
 *
 *     <seq> <micros> <category> [key=value ...]
 *
 * Categories:
 *     BOOT  firmware and application identification, emitted once
 *     MI    MIDI message received
 *     MO    MIDI message sent
 *     LC    call into Lua, entering
 *     LR    call into Lua, returned
 *     LX    call into Lua that could not be made (function missing)
 *     LE    Lua raised an error
 *     PM    parameter map change notification
 *     MARK  marker injected by the test harness
 *     DROP  records lost because the RTT buffer was full
 */

#pragma once

#include <stdint.h>

/*
 * C visible part. bsp/usb/usb_midi.c is plain C and carries the one place
 * every outgoing USB device MIDI packet passes through.
 */
#ifdef __cplusplus
extern "C" {
#endif

#if defined(ELECTRA_TRACE) && ELECTRA_TRACE
/** Records one 32 bit USB MIDI event packet as it goes on the wire. */
void traceUsbMidiPacket(uint32_t packed);
#define TRACE_USB_MIDI_PACKET(packed) traceUsbMidiPacket(packed)
#else
#define TRACE_USB_MIDI_PACKET(packed) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#if defined(ELECTRA_TRACE) && ELECTRA_TRACE

namespace Trace {

/** Number of leading MIDI bytes written out verbatim. Anything longer is
 *  additionally identified by its total length and a hash of all its bytes. */
static const uint16_t maxInlineBytes = 48;

/**
 * @brief Builds one trace record on the stack and emits it in one piece.
 *
 * Records are assembled without printf so that tracing stays cheap enough to
 * not reorder the very events it observes.
 */
class Record
{
public:
    explicit Record(const char *category);

    Record &text(const char *value);
    Record &num(const char *key, int32_t value);
    Record &str(const char *key, const char *value);
    Record &bytes(const char *key, const uint8_t *data, uint16_t length);

    void emit(void);

private:
    void append(const char *value);
    void appendChar(char c);
    void appendNum(int32_t value);
    void appendUnsigned(uint32_t value);
    void appendEscaped(const char *value);

    char buffer[224];
    uint16_t length;
};

/** Sets up the RTT channel. Safe to call before the heap is up. */
void initialise(void);

/** Identifies the image. Call once the application name is known. */
void boot(const char *firmwareVersion, const char *applicationName);

/** Records a marker so a host script can delimit test steps. */
void mark(const char *label);

/** Records a message the firmware logged, whatever the logger does with it. */
void logged(uint8_t level, const char *message);

/**
 * @brief Records how much memory is in use, on both sides.
 *
 * The point of moving data into the display's memory is a number, and this
 * is where that number comes from. Taken at named stages, so a host script
 * can compare the same stage across two firmware images.
 */
void memory(const char *stage,
            int32_t freeRam,
            int32_t heapUsed,
            uint32_t vramUsed,
            uint32_t vramRejected);

/** Number of records lost so far because the host was not reading fast
 *  enough. A non-zero value invalidates a comparison run. */
uint32_t droppedRecords(void);

} // namespace Trace

#define TRACE_INIT() Trace::initialise()
#define TRACE_BOOT(version, app) Trace::boot((version), (app))
#define TRACE_MARK(label) Trace::mark(label)
#define TRACE_LOG(level, message) Trace::logged((level), (message))
#define TRACE_MEMORY(stage, freeRam, heapUsed, vramUsed, vramRejected)         \
    Trace::memory(                                                            \
        (stage), (freeRam), (heapUsed), (vramUsed), (vramRejected))

/** A MIDI message received, described by its parsed fields. */
#define TRACE_MIDI_IN(iface, port, type, channel, data1, data2)                \
    Trace::Record("MI")                                                        \
        .str("if", (iface))                                                    \
        .num("port", (port))                                                   \
        .num("type", (int32_t)(type))                                          \
        .num("ch", (channel))                                                  \
        .num("d1", (data1))                                                    \
        .num("d2", (data2))                                                    \
        .emit()

/** A MIDI message handed to an interface, described by its parsed fields. */
#define TRACE_MIDI_OUT(iface, port, kind, channel, data1, data2)               \
    Trace::Record("MO")                                                        \
        .str("if", (iface))                                                    \
        .num("port", (port))                                                   \
        .str("k", (kind))                                                      \
        .num("ch", (channel))                                                  \
        .num("d1", (int32_t)(data1))                                           \
        .num("d2", (int32_t)(data2))                                           \
        .emit()

/** A system exclusive message, identified by its bytes. */
#define TRACE_SYSEX_IN(iface, port, data, len)                                 \
    Trace::Record("MI")                                                        \
        .str("if", (iface))                                                    \
        .num("port", (port))                                                   \
        .str("k", "SysEx")                                                     \
        .bytes("d", (data), (len))                                             \
        .emit()

#define TRACE_SYSEX_OUT(iface, port, data, len)                                \
    Trace::Record("MO")                                                        \
        .str("if", (iface))                                                    \
        .num("port", (port))                                                   \
        .str("k", "SysEx")                                                     \
        .bytes("d", (data), (len))                                             \
        .emit()

/** Entering a call into Lua. */
#define TRACE_LUA_ENTER(kind, name)                                            \
    Trace::Record("LC").str("kind", (kind)).str("fn", (name)).emit()

#define TRACE_LUA_ENTER_VALUE(kind, name, value)                               \
    Trace::Record("LC")                                                        \
        .str("kind", (kind))                                                   \
        .str("fn", (name))                                                     \
        .num("val", (int32_t)(value))                                          \
        .emit()

#define TRACE_LUA_RETURN_STR(kind, name, result)                               \
    Trace::Record("LR")                                                        \
        .str("kind", (kind))                                                   \
        .str("fn", (name))                                                     \
        .str("ret", (result))                                                  \
        .emit()

#define TRACE_LUA_RETURN_NUM(kind, name, result)                               \
    Trace::Record("LR")                                                        \
        .str("kind", (kind))                                                   \
        .str("fn", (name))                                                     \
        .num("ret", (int32_t)(result))                                         \
        .emit()

#define TRACE_LUA_RETURN_VOID(kind, name)                                      \
    Trace::Record("LR").str("kind", (kind)).str("fn", (name)).emit()

/** A Lua call that could not be made because the function is not defined. */
#define TRACE_LUA_MISSING(kind, name)                                          \
    Trace::Record("LX").str("kind", (kind)).str("fn", (name)).emit()

#define TRACE_LUA_ERROR(kind, name, message)                                   \
    Trace::Record("LE")                                                        \
        .str("kind", (kind))                                                   \
        .str("fn", (name))                                                     \
        .str("err", (message))                                                 \
        .emit()

/** A parameter map change notification. */
#define TRACE_PMAP_CHANGE(origin, deviceId, type, number, value)               \
    Trace::Record("PM")                                                        \
        .num("org", (int32_t)(origin))                                         \
        .num("dev", (int32_t)(deviceId))                                       \
        .num("type", (int32_t)(type))                                          \
        .num("no", (int32_t)(number))                                          \
        .num("val", (int32_t)(value))                                          \
        .emit()

#else // ELECTRA_TRACE

#define TRACE_INIT() ((void)0)
#define TRACE_BOOT(version, app) ((void)0)
#define TRACE_MARK(label) ((void)0)
#define TRACE_LOG(level, message) ((void)0)
#define TRACE_MEMORY(stage, freeRam, heapUsed, vramUsed, vramRejected)         \
    ((void)0)
#define TRACE_MIDI_IN(iface, port, type, channel, data1, data2) ((void)0)
#define TRACE_MIDI_OUT(iface, port, kind, channel, data1, data2) ((void)0)
#define TRACE_SYSEX_IN(iface, port, data, len) ((void)0)
#define TRACE_SYSEX_OUT(iface, port, data, len) ((void)0)
#define TRACE_LUA_ENTER(kind, name) ((void)0)
#define TRACE_LUA_ENTER_VALUE(kind, name, value) ((void)0)
#define TRACE_LUA_RETURN_STR(kind, name, result) ((void)0)
#define TRACE_LUA_RETURN_NUM(kind, name, result) ((void)0)
#define TRACE_LUA_RETURN_VOID(kind, name) ((void)0)
#define TRACE_LUA_MISSING(kind, name) ((void)0)
#define TRACE_LUA_ERROR(kind, name, message) ((void)0)
#define TRACE_PMAP_CHANGE(origin, deviceId, type, number, value) ((void)0)

#endif // ELECTRA_TRACE

#endif // __cplusplus
