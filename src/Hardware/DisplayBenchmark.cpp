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
 * @file DisplayBenchmark.cpp
 *
 * @brief Measures the throughput and the integrity of the SPI link.
 */

#include "DisplayBenchmark.h"

#if defined(ELECTRA_BENCHMARK) && ELECTRA_BENCHMARK

#include "Hardware.h"
#include "System.h"

namespace DisplayBenchmark {

/** Scratch area used for the measurements.
 *
 * The last page of the video RAM map is used for screen pages the application
 * is not showing during boot, so writing there disturbs nothing that is
 * visible, and the benchmark restores nothing on purpose: the page is redrawn
 * from scratch whenever it is next used.
 */
static const uint32_t scratchAddress = SDRAM_PAGE_8;

/** Block sizes swept, in bytes. Small sizes expose per-call overhead, large
 *  ones expose the raw transfer rate. */
static const uint16_t blockSizes[] = { 16, 64, 256, 1024, 4096 };

/** Largest block the benchmark moves. Sits on the stack, so it is bounded. */
static const uint16_t maxBlock = 4096;

/** How many times each block size is transferred, chosen so that every size
 *  moves a similar number of bytes and the timing is not dominated by the
 *  clock's resolution. */
static uint16_t repeatsFor(uint16_t blockSize)
{
    uint32_t target = 64u * 1024u;
    uint32_t repeats = target / blockSize;

    if (repeats < 4) {
        repeats = 4;
    }
    if (repeats > 512) {
        repeats = 512;
    }
    return ((uint16_t)repeats);
}

/** Fills a buffer with a pattern that catches the usual transfer faults:
 *  stuck bits, duplicated bytes and lost bytes all change the readback. */
static void makePattern(uint8_t *buffer, uint16_t size, uint8_t seed)
{
    uint8_t value = seed;

    for (uint16_t i = 0; i < size; i++) {
        buffer[i] = value;
        value = (uint8_t)(value * 31u + 17u);
    }
}

static uint32_t countMismatches(const uint8_t *expected,
                                const uint8_t *actual,
                                uint16_t size)
{
    uint32_t mismatches = 0;

    for (uint16_t i = 0; i < size; i++) {
        if (expected[i] != actual[i]) {
            mismatches++;
        }
    }
    return (mismatches);
}

/** Throughput in kilobytes per second, avoiding floating point. */
static uint32_t kilobytesPerSecond(uint32_t bytes, uint32_t microseconds)
{
    if (microseconds == 0) {
        return (0);
    }
    return ((uint32_t)(((uint64_t)bytes * 1000u) / microseconds));
}

static void measureBlockSize(uint16_t blockSize)
{
    static uint8_t writeBuffer[maxBlock];
    static uint8_t readBuffer[maxBlock];

    uint16_t repeats = repeatsFor(blockSize);
    uint32_t totalBytes = (uint32_t)blockSize * repeats;
    uint32_t writeMicros = 0;
    uint32_t readMicros = 0;
    uint32_t mismatches = 0;

    makePattern(writeBuffer, blockSize, (uint8_t)(blockSize & 0xff));

    uint32_t started = micros();
    for (uint16_t i = 0; i < repeats; i++) {
        Hardware::screen.writeRamData(scratchAddress, writeBuffer, blockSize);
    }
    writeMicros = micros() - started;

    started = micros();
    for (uint16_t i = 0; i < repeats; i++) {
        Hardware::screen.readRamData(scratchAddress, readBuffer, blockSize);
    }
    readMicros = micros() - started;

    mismatches = countMismatches(writeBuffer, readBuffer, blockSize);

    System::logger.write(
        LOG_ERROR,
        "bench block=%u repeats=%u bytes=%lu writeUs=%lu readUs=%lu "
        "writeKBps=%lu readKBps=%lu errors=%lu",
        (unsigned)blockSize,
        (unsigned)repeats,
        (unsigned long)totalBytes,
        (unsigned long)writeMicros,
        (unsigned long)readMicros,
        (unsigned long)kilobytesPerSecond(totalBytes, writeMicros),
        (unsigned long)kilobytesPerSecond(totalBytes, readMicros),
        (unsigned long)mismatches);
}

/**
 * @brief Times the single value accessors.
 *
 * These carry the whole per-call setup, so they show what a byte costs when
 * it travels on its own. The string pool used to read its data this way.
 */
static void measureSingleAccess(void)
{
    static const uint16_t repeats = 512;
    uint32_t mismatches = 0;

    uint32_t started = micros();
    for (uint16_t i = 0; i < repeats; i++) {
        Hardware::screen.storeRam8(scratchAddress + i, (uint8_t)(i & 0xff));
    }
    uint32_t writeMicros = micros() - started;

    started = micros();
    for (uint16_t i = 0; i < repeats; i++) {
        if (Hardware::screen.readRam8(scratchAddress + i)
            != (uint8_t)(i & 0xff)) {
            mismatches++;
        }
    }
    uint32_t readMicros = micros() - started;

    System::logger.write(
        LOG_ERROR,
        "bench single bytes=%u writeUs=%lu readUs=%lu writeUsPerByte=%lu "
        "readUsPerByte=%lu errors=%lu",
        (unsigned)repeats,
        (unsigned long)writeMicros,
        (unsigned long)readMicros,
        (unsigned long)(writeMicros / repeats),
        (unsigned long)(readMicros / repeats),
        (unsigned long)mismatches);
}

/**
 * @brief Hammers the link to find transfer errors that only appear under load.
 *
 * A link that is fast but occasionally wrong is worse than a slow one, and a
 * marginal SPI clock shows up as a rare mismatch rather than a constant one.
 */
static void measureIntegrity(void)
{
    static uint8_t writeBuffer[maxBlock];
    static uint8_t readBuffer[maxBlock];
    static const uint16_t rounds = 64;

    uint32_t mismatches = 0;
    uint32_t roundsWithErrors = 0;

    for (uint16_t round = 0; round < rounds; round++) {
        makePattern(writeBuffer, maxBlock, (uint8_t)(round + 1));
        Hardware::screen.writeRamData(scratchAddress, writeBuffer, maxBlock);
        Hardware::screen.readRamData(scratchAddress, readBuffer, maxBlock);

        uint32_t bad = countMismatches(writeBuffer, readBuffer, maxBlock);
        if (bad) {
            roundsWithErrors++;
            mismatches += bad;
        }
    }

    System::logger.write(
        LOG_ERROR,
        "bench integrity rounds=%u bytes=%lu badRounds=%lu badBytes=%lu",
        (unsigned)rounds,
        (unsigned long)((uint32_t)rounds * maxBlock),
        (unsigned long)roundsWithErrors,
        (unsigned long)mismatches);
}

/**
 * @brief Attributes any corruption to the write path or the read path.
 *
 * The single value accessors are known to be correct, so pairing each bulk
 * direction against its slow counterpart says which one is at fault, instead
 * of leaving both under suspicion.
 */
static void measureDirections(void)
{
    static const uint16_t size = 256;
    static uint8_t writeBuffer[size];
    static uint8_t readBuffer[size];

    makePattern(writeBuffer, size, 0x5A);

    // Slow both ways. Establishes that the pattern and the scratch area are
    // sound before anything is attributed to the bulk paths.
    for (uint16_t i = 0; i < size; i++) {
        Hardware::screen.storeRam8(scratchAddress + i, writeBuffer[i]);
    }
    for (uint16_t i = 0; i < size; i++) {
        readBuffer[i] = Hardware::screen.readRam8(scratchAddress + i);
    }
    uint32_t slowSlow = countMismatches(writeBuffer, readBuffer, size);

    // Bulk write, slow read. Failures here belong to the write path.
    makePattern(writeBuffer, size, 0xA5);
    Hardware::screen.writeRamData(scratchAddress, writeBuffer, size);
    for (uint16_t i = 0; i < size; i++) {
        readBuffer[i] = Hardware::screen.readRam8(scratchAddress + i);
    }
    uint32_t bulkWrite = countMismatches(writeBuffer, readBuffer, size);

    // Slow write, bulk read. Failures here belong to the read path.
    makePattern(writeBuffer, size, 0x3C);
    for (uint16_t i = 0; i < size; i++) {
        Hardware::screen.storeRam8(scratchAddress + i, writeBuffer[i]);
    }
    Hardware::screen.readRamData(scratchAddress, readBuffer, size);
    uint32_t bulkRead = countMismatches(writeBuffer, readBuffer, size);

    System::logger.write(LOG_ERROR,
                         "bench direction size=%u slowSlow=%lu bulkWrite=%lu "
                         "bulkRead=%lu",
                         (unsigned)size,
                         (unsigned long)slowSlow,
                         (unsigned long)bulkWrite,
                         (unsigned long)bulkRead);
}

/**
 * @brief Finds the transfer length at which each bulk direction starts to
 *        lose data, by pairing it against the slow accessors.
 */
static void sweepDirectionSizes(void)
{
    static uint8_t writeBuffer[maxBlock];
    static uint8_t readBuffer[maxBlock];
    static const uint16_t sizes[] = { 64, 128, 256, 512, 1024, 2048, 4096 };

    for (unsigned s = 0; s < (sizeof(sizes) / sizeof(sizes[0])); s++) {
        uint16_t size = sizes[s];

        // Bulk write verified by slow read.
        makePattern(writeBuffer, size, (uint8_t)(size ^ 0x11));
        Hardware::screen.writeRamData(scratchAddress, writeBuffer, size);
        for (uint16_t i = 0; i < size; i++) {
            readBuffer[i] = Hardware::screen.readRam8(scratchAddress + i);
        }
        uint32_t writeErrors = countMismatches(writeBuffer, readBuffer, size);

        // Bulk read verified against a slow write.
        makePattern(writeBuffer, size, (uint8_t)(size ^ 0x77));
        for (uint16_t i = 0; i < size; i++) {
            Hardware::screen.storeRam8(scratchAddress + i, writeBuffer[i]);
        }
        Hardware::screen.readRamData(scratchAddress, readBuffer, size);
        uint32_t readErrors = countMismatches(writeBuffer, readBuffer, size);

        // Where the first difference is, which distinguishes a shift from a
        // drop.
        uint16_t firstBad = size;
        for (uint16_t i = 0; i < size; i++) {
            if (writeBuffer[i] != readBuffer[i]) {
                firstBad = i;
                break;
            }
        }

        System::logger.write(
            LOG_ERROR,
            "bench sizes size=%u bulkWriteErrors=%lu bulkReadErrors=%lu "
            "firstBadRead=%u",
            (unsigned)size,
            (unsigned long)writeErrors,
            (unsigned long)readErrors,
            (unsigned)firstBad);
    }
}

/**
 * @brief Finds the largest burst the display accepts without losing bytes.
 *
 * The bus delivers bytes faster than the controller consumes them, so beyond
 * some burst length the write FIFO overflows and bytes disappear silently.
 * Reads are verified with a burst size already known to be safe, so a failure
 * here is attributable to the write side alone.
 */
static void sweepReadClock(void)
{
    static uint8_t writeBuffer[maxBlock];
    static uint8_t readBuffer[maxBlock];
    static const uint32_t clocks[] = { 30000000, 20000000, 15000000,
                                       12000000, 10000000, 7500000 };
    static const uint16_t rounds = 8;

    uint32_t original = RA8876::readClock;

    for (unsigned c = 0; c < (sizeof(clocks) / sizeof(clocks[0])); c++) {
        uint32_t mismatches = 0;
        uint32_t elapsed = 0;

        RA8876::readClock = clocks[c];

        for (uint16_t round = 0; round < rounds; round++) {
            makePattern(writeBuffer, maxBlock, (uint8_t)(round + 3));
            Hardware::screen.writeRamData(
                scratchAddress, writeBuffer, maxBlock);

            uint32_t started = micros();
            Hardware::screen.readRamData(scratchAddress, readBuffer, maxBlock);
            elapsed += micros() - started;

            mismatches += countMismatches(writeBuffer, readBuffer, maxBlock);
        }

        System::logger.write(
            LOG_ERROR,
            "bench readClock=%lu bytes=%lu readUs=%lu readKBps=%lu errors=%lu",
            (unsigned long)clocks[c],
            (unsigned long)((uint32_t)rounds * maxBlock),
            (unsigned long)elapsed,
            (unsigned long)kilobytesPerSecond((uint32_t)rounds * maxBlock,
                                              elapsed),
            (unsigned long)mismatches);
    }

    RA8876::readClock = original;
}

void run(void)
{
    System::logger.write(LOG_ERROR,
                         "bench start spiHz=%lu fBus=%lu fCpu=%lu",
                         (unsigned long)RA8876_SPI_SPEED,
                         (unsigned long)F_BUS,
                         (unsigned long)F_CPU);

    measureDirections();
    sweepDirectionSizes();

    sweepReadClock();

    measureSingleAccess();

    for (unsigned i = 0; i < (sizeof(blockSizes) / sizeof(blockSizes[0])); i++) {
        measureBlockSize(blockSizes[i]);
    }

    measureIntegrity();

    System::logger.write(LOG_ERROR,
                         "bench done statusWaitTimeouts=%lu",
                         (unsigned long)RA8876::statusWaitTimeouts);
}

} // namespace DisplayBenchmark

#endif // ELECTRA_BENCHMARK
