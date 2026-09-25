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
 * @file TaskMonitorFreeMemory.h
 *
 * @brief A free memory monitor task.
 */

#pragma once

#include <stdint.h>

void monitorFreeMemory(void);

/**
 * @brief What memory looked like the last time a stage was reached.
 *
 * Kept in a known place so a host can read it straight out of target RAM
 * over the debug probe. The logger is not a reliable way to get a number out
 * and the trace only exists in a traced build, but a measurement of how much
 * a preset costs is only worth having on the build people actually run.
 */
struct MemorySnapshot {
    char stage[16];
    int32_t freeRam;
    int32_t heapUsed;
    uint32_t vramUsed;
    uint32_t vramRejected;

    /** Incremented on every update, so a reader can tell a fresh
     *  measurement from one left over from before a restart. */
    uint32_t updates;
};

extern volatile MemorySnapshot memorySnapshot;

/**
 * @brief Records memory use under a named stage.
 *
 * Fills in the snapshot above and, in a traced build, puts the same figures
 * in the trace so they sit in order with everything else.
 *
 * Only deliberate measurement points call this. The ten second monitor task
 * traces its figures but leaves the snapshot alone, so a reader always finds
 * the last named stage rather than whatever the timer last saw.
 */
void traceMemory(const char *stage);
int getUsedMemory(void);
