/*
* Electra One MIDI Controller Firmware
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
 * @file Logger.cpp
 *
 * @brief An implemention of a USB MIDI Sysex based logger.
 */

#include "Logger.h"
#include "MidiOutput.h"
#include "Trace.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>

volatile bool loggerEnabled;

Logger::Logger() : minimumLevel(LOG_ERROR), port(2), enabled(true)
{
}

void Logger::setPort(uint8_t newPort)
{
    if (newPort <= 2) {
        port = newPort;
    }
}

void Logger::setLevel(uint8_t newLevel)
{
    minimumLevel = (newLevel <= 3) ? newLevel : 3;
}

void Logger::enable(void)
{
    enabled = true;
}

void Logger::disable(void)
{
    enabled = false;
}

void Logger::setStatus(bool newStatus)
{
    enabled = newStatus;
}

void Logger::write(uint8_t level, const char *format, ...)
{
#if defined(ELECTRA_TRACE) && ELECTRA_TRACE
    /* The trace wants every message the firmware produced, independently of
     * whether the logger itself decides to put it on the wire. The message is
     * formatted once and shared, so tracing costs no extra stack in what can
     * be an interrupt context. */
    const bool traceWanted = true;
#else
    const bool traceWanted = false;
#endif

    if (traceWanted || (enabled && (level < minimumLevel))
        || (level == LOG_LUA)) {
        char buf[MaxLogMessageSize];
        va_list ap;

        va_start(ap, format);
        vsnprintf(buf + 5, sizeof(buf) - 5, format, ap);
        va_end(ap);

        TRACE_LOG(level, buf + 5);

        if (!((enabled && (level < minimumLevel)) || (level == LOG_LUA))) {
            return;
        }

        buf[0] = 0x00;
        buf[1] = 0x21;
        buf[2] = 0x45;
        buf[3] = 0x7f;
        buf[4] = 0x00;

        for (uint16_t i = 5; (i < (sizeof(buf) - 5)) && (buf[i] != '\0'); i++) {
            if ((buf[i] < 32) || (buf[i] > 126)) {
                buf[i] = '#';
            }
        }

        usbMIDI.sendSysEx(
            strlen(buf + 5) + 5, (const uint8_t *)buf, false, port);
    }
}
