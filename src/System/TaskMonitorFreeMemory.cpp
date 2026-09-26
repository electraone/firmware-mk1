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

#include "TaskMonitorFreeMemory.h"
#include "Hardware.h"
#include "System.h"
#include "compat.h"
#include "Trace.h"
#include "Profile.h"

void monitorFreeMemory(void)
{
    PROFILE_DUMP();
    Hardware::ram.run();
    System::logger.write(
        LOG_TRACE,
        "free RAM: total=%d, adj_free=%d, free=%d, heap=%d, collision=%d",
        Hardware::ram.total(),
        Hardware::ram.adj_free(),
        Hardware::ram.free(),
        Hardware::ram.heap_used(),
        Hardware::ram.warning_crash());

    /* Traced, but deliberately not put in the snapshot. The snapshot is read
     * by a host comparing two firmware images at the same point in the same
     * work, and a timer firing every ten seconds would overwrite that point
     * with whatever happened to be true later. */
    TRACE_MEMORY("periodic",
                 Hardware::ram.adj_free(),
                 (int32_t)Hardware::ram.heap_used(),
                 (uint32_t)Hardware::memory.stringPool.getUsed(),
                 Hardware::memory.stringPool.getRejectedCount());
}

volatile MemorySnapshot memorySnapshot;

void traceMemory(const char *stage)
{
    Hardware::ram.run();

    int32_t freeRam = Hardware::ram.adj_free();
    int32_t heapUsed = (int32_t)Hardware::ram.heap_used();
    uint32_t vramUsed = (uint32_t)Hardware::memory.stringPool.getUsed();
    uint32_t vramRejected = Hardware::memory.stringPool.getRejectedCount();

    copyString((char *)memorySnapshot.stage,
               stage,
               sizeof(memorySnapshot.stage) - 1);
    memorySnapshot.freeRam = freeRam;
    memorySnapshot.heapUsed = heapUsed;
    memorySnapshot.vramUsed = vramUsed;
    memorySnapshot.vramRejected = vramRejected;
    memorySnapshot.updates++;

    TRACE_MEMORY(stage, freeRam, heapUsed, vramUsed, vramRejected);
}

int getUsedMemory(void)
{
    // 185000 is memory available on top of the base system.
    // \todo - sample the actual value after the system is initialized
    return (
        constrain(100 - (Hardware::ram.adj_free() / (185000 / 100)), 0, 100));
}
