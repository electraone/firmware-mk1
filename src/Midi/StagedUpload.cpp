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
 * @file StagedUpload.cpp
 *
 * @brief The writing half of the File Transfer API: the cache transaction.
 */

#include "StagedUpload.h"

#include "App.h"
#include "ArduinoJson.h"
#include "FileTransfer.h"
#include "Hardware.h"
#include "Md5.h"
#include "MemoryBlock.h"
#include "MidiOutput.h"
#include "SysexBlock.h"
#include "System.h"

#include <cstring>

namespace
{
    /** One file a host has announced but not yet committed. */
    struct StagedFile {
        uint32_t expected;
        uint32_t received;
        uint8_t id;
        bool used;
    };

    /** The whole transaction. Twelve bytes a file, and nothing when idle. */
    struct {
        StagedFile files[StagedUpload::maxFiles];
        uint32_t receivedTotal;
        bool open;
    } transaction;

    /** How much of a chunk is moved from the pool to the card at a time. */
    constexpr size_t copyChunk = 512;

    /** Enough for a commit naming a handful of files. */
    constexpr size_t commitCapacity = 1536;

    StagedFile *find(uint8_t id)
    {
        for (uint8_t i = 0; i < StagedUpload::maxFiles; i++) {
            if (transaction.files[i].used && (transaction.files[i].id == id)) {
                return (&transaction.files[i]);
            }
        }

        return (nullptr);
    }

    void forget(void)
    {
        memset(&transaction, 0, sizeof(transaction));
    }

    void nack(uint8_t port)
    {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
    }

    void ack(uint8_t port)
    {
        MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
    }
} // namespace

void StagedUpload::discard(void)
{
    char directory[FileTransfer::maxPathLength];

    System::context.formatCacheDirectory(directory, sizeof(directory));
    FileTransfer::removeLocation(directory);
    forget();
}

void StagedUpload::open(uint8_t port)
{
    char directory[FileTransfer::maxPathLength];

    System::context.formatCacheDirectory(directory, sizeof(directory));

    /* Whatever a transaction that was never committed left behind goes now.
     * The staging area is not something to accumulate: one abandoned firmware
     * image is a megabyte of a card that also holds the presets. */
    FileTransfer::removeLocation(directory);
    forget();

    if (!FileTransfer::createDirectory(directory)) {
        System::logger.write(
            LOG_ERROR, "StagedUpload::open: no staging area: %s", directory);
        nack(port);
        return;
    }

    transaction.open = true;

    System::logger.write(LOG_INFO, "StagedUpload::open: transaction opened");
    ack(port);
}

void StagedUpload::registerFile(uint8_t port, ElectraCommand &cmd)
{
    if (!transaction.open) {
        System::logger.write(LOG_ERROR,
                             "StagedUpload::registerFile: no transaction");
        nack(port);
        return;
    }

    const uint8_t id = cmd.getByte1();

    /* Four seven bit bytes, low first: twenty-eight bits, which is more file
     * than this card will ever be asked for. */
    const uint32_t size = (uint32_t)cmd.getByte2()
                          | ((uint32_t)cmd.getByte3() << 7)
                          | ((uint32_t)cmd.getByte4() << 14);

    StagedFile *file = find(id);

    if (!file) {
        for (uint8_t i = 0; i < maxFiles; i++) {
            if (!transaction.files[i].used) {
                file = &transaction.files[i];
                break;
            }
        }
    }

    if (!file) {
        System::logger.write(
            LOG_ERROR,
            "StagedUpload::registerFile: no room for more than %d files",
            maxFiles);
        nack(port);
        return;
    }

    char path[FileTransfer::maxPathLength];

    System::context.formatCacheFilename(path, sizeof(path), id);

    /* Truncated rather than appended to, so that registering the same id twice
     * starts it again instead of doubling it. */
    FileIoStream staged = Hardware::sdcard.createOutputStream(
        path, FILE_WRITE | O_CREAT | O_TRUNC);

    if (!staged) {
        System::logger.write(
            LOG_ERROR, "StagedUpload::registerFile: cannot open: %s", path);
        nack(port);
        return;
    }

    staged.close();

    file->used = true;
    file->id = id;
    file->expected = size;
    file->received = 0;

    System::logger.write(
        LOG_INFO, "StagedUpload::registerFile: id=%d, size=%d", id, (int)size);
    ack(port);
}

void StagedUpload::receiveChunk(uint8_t port, const SysexBlock &sysexBlock)
{
    if (!transaction.open) {
        System::logger.write(LOG_ERROR,
                             "StagedUpload::receiveChunk: no transaction");
        nack(port);
        return;
    }

    MemoryBlock payload = sysexBlock.getElectraSysexPayload();

    if (payload.getLength() < 1) {
        System::logger.write(LOG_ERROR,
                             "StagedUpload::receiveChunk: no file id");
        nack(port);
        return;
    }

    const uint8_t id = (uint8_t)payload.peek(0);
    StagedFile *file = find(id);

    if (!file) {
        System::logger.write(
            LOG_ERROR, "StagedUpload::receiveChunk: unknown file: id=%d", id);
        nack(port);
        return;
    }

    const size_t length = payload.getLength() - 1;

    if ((file->received + length) > file->expected) {
        /* More than was announced. Refused rather than written: the digest
         * would catch it at the commit, but by then the card has taken bytes
         * that were never going to be wanted. */
        System::logger.write(LOG_ERROR,
                             "StagedUpload::receiveChunk: id=%d overruns its "
                             "registered size: %d + %d > %d",
                             id,
                             (int)file->received,
                             (int)length,
                             (int)file->expected);
        nack(port);
        return;
    }

    char path[FileTransfer::maxPathLength];

    System::context.formatCacheFilename(path, sizeof(path), id);

    FileIoStream staged =
        Hardware::sdcard.createOutputStream(path, FILE_WRITE | O_APPEND);

    if (!staged) {
        System::logger.write(
            LOG_ERROR, "StagedUpload::receiveChunk: cannot open: %s", path);
        nack(port);
        return;
    }

    /* Moved a piece at a time rather than through one buffer the size of the
     * chunk: a host may send whatever chunk size it likes, and a stack frame
     * that grows with it is a stack frame that eventually does not fit. */
    uint8_t buffer[copyChunk];
    size_t position = 1;
    bool written = true;

    while (position < payload.getLength()) {
        size_t wanted = payload.getLength() - position;

        if (wanted > sizeof(buffer)) {
            wanted = sizeof(buffer);
        }

        payload.seek(position);

        const size_t read = payload.readBytes(buffer, wanted);

        if ((read == 0) || (staged.write(buffer, read) != read)) {
            written = false;
            break;
        }

        position += read;
    }

    staged.close();

    if (!written) {
        System::logger.write(
            LOG_ERROR, "StagedUpload::receiveChunk: write failed: %s", path);
        nack(port);
        return;
    }

    file->received += length;
    transaction.receivedTotal += length;

    /* What the host draws its progress bar from. It goes out before the
     * acknowledgement, because the acknowledgement is what releases the next
     * chunk. */
    MidiOutput::sendStagedProgress(
        MidiInterface::Type::MidiUsbDev, port, transaction.receivedTotal);
    ack(port);
}

void StagedUpload::commit(uint8_t port, MemoryBlock &sysexPayload)
{
    if (!transaction.open) {
        System::logger.write(LOG_ERROR, "StagedUpload::commit: no transaction");
        nack(port);
        return;
    }

    StaticJsonDocument<commitCapacity> doc;
    DeserializationError error = deserializeJson(doc, sysexPayload);

    if (error) {
        System::logger.write(LOG_ERROR,
                             "StagedUpload::commit: the commit is not JSON: %s",
                             error.c_str());
        nack(port);
        return;
    }

    JsonArray files = doc["files"].as<JsonArray>();

    if (files.isNull() || (files.size() == 0)) {
        System::logger.write(LOG_ERROR, "StagedUpload::commit: no files named");
        nack(port);
        return;
    }

    /*
     * Two passes. The first says whether every file is whole, goes where this
     * model has somewhere to put it, and matches the digest the host sent; the
     * second moves them. Nothing is moved until every one of them has passed,
     * which is what makes a transaction of it: a slot is never left holding a
     * new preset and an old script.
     */
    char cachePath[FileTransfer::maxPathLength];
    char destination[FileTransfer::maxPathLength];
    char digest[Md5::hexLength + 1];

    for (JsonObject entry : files) {
        const uint8_t id = entry["id"].as<uint8_t>();
        StagedFile *file = find(id);

        if (!file) {
            System::logger.write(
                LOG_ERROR, "StagedUpload::commit: id=%d was never sent", id);
            nack(port);
            return;
        }

        if (file->received != file->expected) {
            System::logger.write(LOG_ERROR,
                                 "StagedUpload::commit: id=%d is short: %d of "
                                 "%d bytes",
                                 id,
                                 (int)file->received,
                                 (int)file->expected);
            nack(port);
            return;
        }

        if (!FileTransfer::resolveDestination(
                destination, sizeof(destination), entry)) {
            nack(port);
            return;
        }

        const char *wanted = entry["md5"];

        if (!wanted) {
            System::logger.write(
                LOG_ERROR, "StagedUpload::commit: id=%d has no digest", id);
            nack(port);
            return;
        }

        System::context.formatCacheFilename(cachePath, sizeof(cachePath), id);

        if (!Md5::ofFile(cachePath, digest, sizeof(digest))) {
            nack(port);
            return;
        }

        if (!Md5::hexEquals(digest, wanted)) {
            System::logger.write(LOG_ERROR,
                                 "StagedUpload::commit: id=%d does not match: "
                                 "have %s, want %s",
                                 id,
                                 digest,
                                 wanted);
            nack(port);
            return;
        }

        System::logger.write(LOG_INFO,
                             "StagedUpload::commit: id=%d verified for %s",
                             id,
                             destination);
    }

    bool currentSlotWritten = false;

    for (JsonObject entry : files) {
        const uint8_t id = entry["id"].as<uint8_t>();

        System::context.formatCacheFilename(cachePath, sizeof(cachePath), id);
        FileTransfer::resolveDestination(
            destination, sizeof(destination), entry);

        /* Removed first: a rename onto a name that is taken fails on this
         * card's driver rather than replacing it. */
        if (Hardware::sdcard.exists(destination)) {
            Hardware::sdcard.deleteFile(destination);
        }

        if (!Hardware::sdcard.renameFile(cachePath, destination)) {
            /* Past the point where this can be undone. Everything was checked
             * first so that arriving here means the card itself failed, and
             * the host is told the commit did not succeed. */
            System::logger.write(LOG_ERROR,
                                 "StagedUpload::commit: could not move %s to "
                                 "%s",
                                 cachePath,
                                 destination);
            discard();
            nack(port);
            return;
        }

        if (FileTransfer::namesTheLoadedSlot(destination)) {
            currentSlotWritten = true;
        }
    }

    System::logger.write(LOG_INFO,
                         "StagedUpload::commit: %d file(s) applied",
                         (int)files.size());

    discard();
    ack(port);

    /* After the acknowledgement: reloading a preset takes long enough that a
     * host waiting on the commit would give up on it. */
    App::get()->handleStagedFilesApplied(port, currentSlotWritten);
}
