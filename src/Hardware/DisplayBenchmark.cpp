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
#include "VramStore.h"
#include "SysexBlock.h"
#include "MemoryPool.h"
#include "ListData.h"
#include "Control.h"
#include "Group.h"
#include "Device.h"
#include "Page.h"
#include "LookupEntry.h"
#include <string.h>

namespace DisplayBenchmark {

/** Scratch area used for the measurements.
 *
 * The last page of the video RAM map is used for screen pages the application
 * is not showing during boot, so writing there disturbs nothing that is
 * visible, and the benchmark restores nothing on purpose: the page is redrawn
 * from scratch whenever it is next used.
 */
/* Past the last page, in the megabyte the character generator used to have:
 * everything below page eight is the glyph atlas now, and page eight itself is
 * where the store measurements below put their pools. */
static const uint32_t scratchAddress = SDRAM_PAGE_8 + SDRAM_PAGE_SIZE;

/** Block sizes swept, in bytes. Small sizes expose per-call overhead, large
 *  ones expose the raw transfer rate. */
static const uint16_t blockSizes[] = { 16, 64, 256, 1024, 4096 };

/** Largest block the benchmark moves. */
static const uint16_t maxBlock = 4096;

/** One pair of buffers shared by every measurement. Giving each function its
 *  own pair cost tens of kilobytes of RAM in a build that is meant to measure
 *  the machine, not change it. */
static uint8_t writeBuffer[maxBlock];
static uint8_t readBuffer[maxBlock];

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

/**
 * @brief Exercises VramStore against the real display.
 *
 * Storing and reading back on the hardware is the only way to know the store
 * works: the failure modes it has to survive, dropped bytes and a full
 * region, do not exist off the device. A spare page is used so the running
 * application's own data is not touched.
 */
static void testVramStore(void)
{
    static const char *samples[] = {
        "",
        "a",
        "Cutoff",
        "a label of a more usual length",
        "0123456789012345678901234567890123456789012345678901234567890123"
    };
    static const unsigned sampleCount =
        sizeof(samples) / sizeof(samples[0]);

    VramStore store(SDRAM_PAGE_8, SDRAM_PAGE_SIZE);
    VramStore::Handle handles[sampleCount];
    char readBack[128];
    uint32_t failures = 0;

    store.assignStorageDriver(&Hardware::screen);
    store.reset();

    for (unsigned i = 0; i < sampleCount; i++) {
        handles[i] = store.storeString(samples[i]);
        if (handles[i] == VramStore::invalidHandle) {
            failures++;
        }
    }

    // Read back out of order, so a handle that happens to point at the record
    // written just before it cannot pass by accident.
    for (unsigned i = sampleCount; i-- > 0;) {
        store.loadString(handles[i], readBack, sizeof(readBack));
        if (strcmp(readBack, samples[i]) != 0) {
            failures++;
        }
    }

    // A record longer than the buffer must truncate and stay terminated
    // rather than run past the end.
    char small[8];
    store.loadString(handles[3], small, sizeof(small));
    bool truncatedSafely = (strlen(small) == sizeof(small) - 1)
                           && (strncmp(small, samples[3], sizeof(small) - 1)
                               == 0);

    // An invalid handle must read as nothing, not as somebody else's record.
    store.loadString(VramStore::invalidHandle, readBack, sizeof(readBack));
    bool invalidIsEmpty = (readBack[0] == '\0');

    size_t usedAfterStores = store.getUsed();

    // A store that fills up must refuse rather than wrap onto live records.
    VramStore tiny(SDRAM_PAGE_8 + (SDRAM_PAGE_SIZE / 2), 32);
    tiny.assignStorageDriver(&Hardware::screen);
    VramStore::Handle first = tiny.storeString("0123456789");
    VramStore::Handle second = tiny.storeString("0123456789");
    VramStore::Handle third = tiny.storeString("0123456789");
    bool refusedWhenFull = (first != VramStore::invalidHandle)
                           && (second != VramStore::invalidHandle)
                           && (third == VramStore::invalidHandle)
                           && (tiny.getRejectedCount() == 1);

    /* A reserved record is written over and over, which is what a value
     * label does. It must take its space once, hold whatever was last put in
     * it, and never reach the record that follows however long the data. */
    VramStore::Handle reserved = store.reserve(16);
    VramStore::Handle neighbour = store.storeString("neighbour");
    size_t usedAfterReserve = store.getUsed();

    store.loadString(reserved, readBack, sizeof(readBack));
    bool reserveStartsEmpty = (readBack[0] == '\0');

    store.updateString(reserved, "first");
    store.loadString(reserved, readBack, sizeof(readBack));
    bool firstWriteReads = (strcmp(readBack, "first") == 0);

    store.updateString(reserved, "a second, longer one");
    store.loadString(reserved, readBack, sizeof(readBack));
    bool overlongTruncates = (strlen(readBack) == 15)
                             && (strncmp(readBack, "a second, longe", 15) == 0);

    store.updateString(reserved, "3rd");
    store.loadString(reserved, readBack, sizeof(readBack));
    bool shorterWriteReads = (strcmp(readBack, "3rd") == 0);

    store.loadString(neighbour, readBack, sizeof(readBack));
    bool neighbourIntact = (strcmp(readBack, "neighbour") == 0);
    bool rewritesInPlace = (store.getUsed() == usedAfterReserve);

    store.reset();
    bool resetFrees = (store.getUsed() == 0);

    System::logger.write(
        LOG_ERROR,
        "bench vramstore failures=%lu used=%lu truncate=%d invalid=%d "
        "full=%d reset=%d reserve=%d%d%d%d%d%d",
        (unsigned long)failures,
        (unsigned long)usedAfterStores,
        truncatedSafely ? 1 : 0,
        invalidIsEmpty ? 1 : 0,
        refusedWhenFull ? 1 : 0,
        resetFrees ? 1 : 0,
        reserveStartsEmpty ? 1 : 0,
        firstWriteReads ? 1 : 0,
        overlongTruncates ? 1 : 0,
        shorterWriteReads ? 1 : 0,
        neighbourIntact ? 1 : 0,
        rewritesInPlace ? 1 : 0);
}

/**
 * @brief Checks that a list item still gives back the label it was made with.
 *
 * The labels live in the display's memory, so this exercises the whole path
 * the painting code uses: pack a record, store it, read it back, unpack it.
 */
static void testListItems(void)
{
    static const char *labels[] = { "Off", "Sine", "a fairly long item label" };
    static const unsigned count = sizeof(labels) / sizeof(labels[0]);

    ListData list;
    uint32_t failures = 0;

    for (unsigned i = 0; i < count; i++) {
        list.addItem((int16_t)i, labels[i]);
    }

    for (unsigned i = 0; i < count; i++) {
        ListDataItem item = list.getByIndex(i);

        if (strcmp(item.getLabel(), labels[i]) != 0) {
            failures++;
        }
        if (item.getValue() != (int16_t)i) {
            failures++;
        }
        if (!item.isBitmapEmpty()) {
            failures++;
        }
    }

    System::logger.write(LOG_ERROR,
                         "bench listitems count=%u failures=%lu poolUsed=%lu",
                         count,
                         (unsigned long)failures,
                         (unsigned long)Hardware::memory.stringPool.getUsed());
}

/**
 * @brief Reports what the model types cost in RAM.
 *
 * The counts that matter are in Preset.h; multiplying them by these sizes is
 * what decides whether moving a member to the display's memory is worth the
 * read it costs. Printed rather than assumed, because padding and the
 * embedded containers make the arithmetic on paper wrong.
 */
/**
 * @brief Checks the pool that received and sent system exclusive messages
 *        live in.
 *
 * The awkward cases are all about the boundary between the part of a message
 * kept in RAM and the part kept over here: a message shorter than the header,
 * one exactly the length of it, one that spans it, and a range taken across
 * it. Then the end of the pool, which used to be written straight past.
 */
static void testSysexBlocks(void)
{
    MemoryPool pool(&Hardware::screen, SDRAM_PAGE_8, SDRAM_PAGE_SIZE);
    uint32_t failures = 0;

    // A message that fits entirely in the RAM header.
    static const uint8_t shortMessage[] = {
        0xF0, 0x00, 0x21, 0x45, 0x02, 0x7F, 0xF7
    };
    SysexBlock small(pool.openMemoryBlock());
    small.writeBytes(shortMessage, sizeof(shortMessage));
    small.close();

    bool shortIsSysex = small.isSysex();
    bool shortIsElectra = small.isElectraSysex();
    bool shortLength = (small.getLength() == sizeof(shortMessage));
    bool shortBytes = true;

    for (size_t i = 0; i < sizeof(shortMessage); i++) {
        if (small.peek(i) != shortMessage[i]) {
            shortBytes = false;
        }
    }

    // Past the end must say so rather than hand back the memory after it.
    bool pastEndIsMinusOne = (small.peek(sizeof(shortMessage)) == -1)
                             && (small.peek(10000) == -1);

    // A message far longer than the header, written in uneven pieces so it
    // crosses the boundary mid write.
    static const size_t longLength = 600;
    uint8_t longMessage[longLength];

    longMessage[0] = 0xF0;
    longMessage[1] = 0x00;
    longMessage[2] = 0x21;
    longMessage[3] = 0x45;
    longMessage[4] = 0x7E;
    longMessage[5] = 0x7F;

    for (size_t i = 6; i < (longLength - 1); i++) {
        longMessage[i] = (uint8_t)(i & 0x7F);
    }
    longMessage[longLength - 1] = 0xF7;

    SysexBlock large(pool.openMemoryBlock());
    size_t offset = 0;
    static const size_t pieces[] = { 7, 40, 9, 100, 3, 441 };

    for (unsigned p = 0; p < (sizeof(pieces) / sizeof(pieces[0])); p++) {
        large.writeBytes(longMessage + offset, pieces[p]);
        offset += pieces[p];
    }
    large.close();

    bool longLengthOk = (large.getLength() == longLength);
    bool longBytes = true;

    for (size_t i = 0; i < longLength; i++) {
        if (large.peek(i) != longMessage[i]) {
            longBytes = false;
        }
    }

    /* What a scan of a received message costs. Everything that examines one
     * walks it a byte at a time, and every byte past the header used to be
     * its own transfer: compare this against the readUsPerByte the single
     * access measurement reports. */
    uint32_t scanStarted = micros();
    uint32_t scanSum = 0;

    for (size_t i = 0; i < longLength; i++) {
        scanSum += (uint32_t)large.peek(i);
    }
    uint32_t scanMicros = micros() - scanStarted;

    // Reading it back as a stream has to give the same thing as peeking.
    uint8_t streamed[longLength];
    memset(streamed, 0, sizeof(streamed));
    large.seek(0);
    size_t streamedLength = large.readBytes(streamed, sizeof(streamed));
    bool streamMatches = (streamedLength == longLength)
                         && (memcmp(streamed, longMessage, longLength) == 0);

    // The Electra payload starts after the manufacturer, command and object
    // bytes, and stops before the terminator.
    MemoryBlock payload = large.getElectraSysexPayload();
    bool payloadLength = (payload.getLength() == (longLength - 7));
    bool payloadBytes = true;

    for (size_t i = 0; i < payload.getLength(); i++) {
        if (payload.peek(i) != longMessage[6 + i]) {
            payloadBytes = false;
        }
    }

    // A message too short to have a payload must give an empty one rather
    // than a range that wraps round to four gigabytes.
    static const uint8_t truncated[] = { 0xF0, 0x00, 0x21, 0x45, 0x09 };
    SysexBlock stub(pool.openMemoryBlock());
    stub.writeBytes(truncated, sizeof(truncated));
    stub.close();
    bool stubPayloadEmpty = (stub.getElectraSysexPayload().getLength() == 0);

    // An empty block answers without reading memory it never wrote.
    SysexBlock empty;
    bool emptyIsSafe = (empty.isSysex() == false) && (empty.isEmpty())
                       && (empty.getManufacturerSysexId() == 0)
                       && (empty.peek(0) == -1);

    /* The end of the pool. A message larger than the whole pool is the only
     * way to reach it, because a new block starts from the beginning again
     * while there is not room ahead of it. It has to be cut short rather than
     * written on past the end, which is what used to happen and what landed
     * in the page the labels live in. */
    MemoryPool tiny(
        &Hardware::screen, SDRAM_PAGE_8 + (SDRAM_PAGE_SIZE / 2), 300);
    uint8_t filler[400];
    memset(filler, 0x5A, sizeof(filler));

    MemoryBlock tooBig(tiny.openMemoryBlock());
    tooBig.writeBytes(filler, sizeof(filler));
    tooBig.close();

    bool refusedAtEnd = (tooBig.getLength() < sizeof(filler))
                        && tooBig.isOverflowed()
                        && (tiny.getOverflowCount() > 0);

    /* Whatever it did keep still has to read back as what was written. */
    for (size_t i = 0; i < tooBig.getLength(); i++) {
        if (tooBig.peek(i) != 0x5A) {
            refusedAtEnd = false;
        }
    }

    System::logger.write(
        LOG_ERROR,
        "bench sysex failures=%lu short=%d%d%d%d end=%d long=%d%d stream=%d "
        "payload=%d%d stub=%d empty=%d refused=%d scanBytes=%u scanUs=%lu "
        "scanNsPerByte=%lu checksum=%lu",
        (unsigned long)failures,
        shortIsSysex ? 1 : 0,
        shortIsElectra ? 1 : 0,
        shortLength ? 1 : 0,
        shortBytes ? 1 : 0,
        pastEndIsMinusOne ? 1 : 0,
        longLengthOk ? 1 : 0,
        longBytes ? 1 : 0,
        streamMatches ? 1 : 0,
        payloadLength ? 1 : 0,
        payloadBytes ? 1 : 0,
        stubPayloadEmpty ? 1 : 0,
        emptyIsSafe ? 1 : 0,
        refusedAtEnd ? 1 : 0,
        (unsigned)longLength,
        (unsigned long)scanMicros,
        (unsigned long)((scanMicros * 1000u) / longLength),
        (unsigned long)scanSum);
}

static void reportModelSizes(void)
{
    System::logger.write(LOG_ERROR,
                         "bench sizeof control=%u value=%u group=%u "
                         "device=%u page=%u lookupEntry=%u",
                         (unsigned)sizeof(Control),
                         (unsigned)sizeof(ControlValue),
                         (unsigned)sizeof(Group),
                         (unsigned)sizeof(Device),
                         (unsigned)sizeof(Page),
                         (unsigned)sizeof(LookupEntry));
}

void run(void)
{
    System::logger.write(LOG_ERROR,
                         "bench start spiHz=%lu fBus=%lu fCpu=%lu",
                         (unsigned long)RA8876_SPI_SPEED,
                         (unsigned long)F_BUS,
                         (unsigned long)F_CPU);

    reportModelSizes();
    testVramStore();
    testListItems();
    testSysexBlocks();

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
