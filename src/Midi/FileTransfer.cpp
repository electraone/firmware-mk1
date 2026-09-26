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
 * @file FileTransfer.cpp
 *
 * @brief Naming, listing and fetching files over SysEx.
 */

#include "FileTransfer.h"

#include "ArduinoJson.h"
#include "Hardware.h"
#include "MemoryBlock.h"
#include "MidiOutput.h"
#include "ScreenCapture.h"
#include "System.h"

#include <cstdio>
#include <cstring>

namespace
{
    /** The one location a listing or a fetch serves, and its one type. */
    constexpr const char *tmpLocation = "tmp";
    constexpr const char *screenshotType = "screenshot";
    constexpr const char *screenshotExtension = ".bmp";

    /** The two a commit can write to, and the types they hold. */
    constexpr const char *slotsLocation = "slots";
    constexpr const char *rootLocation = "root";
    constexpr const char *presetType = "preset";
    constexpr const char *luaType = "lua";
    constexpr const char *configType = "config";

    constexpr int numberOfBanks = 6;
    constexpr int slotsPerBank = 12;

    /** As much of a listing as is sent. A hundred or so names fit. */
    constexpr size_t listingCapacity = 2048;

    const char *directoryFor(const char *location)
    {
        if (location && (strcmp(location, tmpLocation) == 0)) {
            return (ScreenCapture::directory);
        }

        return (nullptr);
    }

    const char *extensionFor(const char *type)
    {
        if (type && (strcmp(type, screenshotType) == 0)) {
            return (screenshotExtension);
        }

        return (nullptr);
    }

    /** The descriptor, parsed. Small: it is three short strings. */
    bool readDescriptor(MemoryBlock &descriptor, StaticJsonDocument<256> &doc)
    {
        DeserializationError error = deserializeJson(doc, descriptor);

        if (error) {
            System::logger.write(LOG_ERROR,
                                 "FileTransfer: the descriptor is not JSON: %s",
                                 error.c_str());
            return (false);
        }

        return (true);
    }
} // namespace

bool FileTransfer::resolveLocation(char *path,
                                   size_t size,
                                   MemoryBlock &descriptor)
{
    StaticJsonDocument<256> doc;

    if (!readDescriptor(descriptor, doc)) {
        return (false);
    }

    const char *directory = directoryFor(doc["location"]);

    if (!directory) {
        System::logger.write(LOG_ERROR,
                             "FileTransfer: no such location: location=%s",
                             doc["location"].as<const char *>()
                                 ? doc["location"].as<const char *>()
                                 : "");
        return (false);
    }

    copyString(path, directory, size - 1);

    return (true);
}

bool FileTransfer::resolve(char *path, size_t size, MemoryBlock &descriptor)
{
    StaticJsonDocument<256> doc;

    if (!readDescriptor(descriptor, doc)) {
        return (false);
    }

    const char *directory = directoryFor(doc["location"]);
    const char *extension = extensionFor(doc["type"]);
    const char *name = doc["name"];

    if (!directory || !extension || !name || (name[0] == '\0')) {
        System::logger.write(
            LOG_ERROR,
            "FileTransfer: the descriptor names nothing this serves");
        return (false);
    }

    /* A name is a name and not a path. Anything that could climb out of the
     * location it was given is refused rather than cleaned up, because the
     * cleaning is where the mistakes live. */
    if (strchr(name, '/') || strchr(name, '\\') || strstr(name, "..")) {
        System::logger.write(
            LOG_ERROR, "FileTransfer: the name is not a plain name: %s", name);
        return (false);
    }

    snprintf(path, size, "%s/%s%s", directory, name, extension);

    return (true);
}

void FileTransfer::sendListing(uint8_t port, const char *directory)
{
    StaticJsonDocument<listingCapacity> doc;

    /* The shape the API publishes, and the host tools read by name: a
     * version, the directory this answers for, whether it is there at all,
     * and then the files. The editor will not accept a listing without the
     * path - it is how it tells the answer it asked for from one it did
     * not - and a listing of nothing but files left it waiting. */
    doc["version"] = 1;
    doc["path"] = directory;

    FsFile dir;
    const bool exists = dir.open(directory, O_RDONLY);

    doc["exists"] = exists;

    JsonArray files = doc.createNestedArray("files");

    if (exists) {
        FsFile entry;
        char name[64];

        while (entry.openNext(&dir, O_RDONLY)) {
            if (!entry.isDirectory() && (entry.getName(name, sizeof(name)) > 0)
                && (doc.capacity() - doc.memoryUsage() > 128)) {
                JsonObject item = files.createNestedObject();
                /* Copied into the document rather than pointed at. A const
                 * char * is stored by reference, so every entry ended up
                 * naming this one buffer - a listing of three screenshots
                 * came back as the last name three times. A char * is
                 * copied, which is the whole difference. */
                item["name"] = (char *)name;
                item["size"] = (uint32_t)entry.fileSize();
            }
            entry.close();
        }
        dir.close();
    } else {
        /* A location with nothing in it yet is an empty listing rather than
         * an error: the directory is made when the first file is written. */
        System::logger.write(LOG_INFO,
                             "FileTransfer::sendListing: no directory yet: %s",
                             directory);
    }

    MidiOutput::sendFileListing(MidiInterface::Type::MidiUsbDev, port, doc);
}

bool FileTransfer::resolveDestination(char *path,
                                      size_t size,
                                      JsonObject descriptor)
{
    const char *location = descriptor["location"];
    const char *type = descriptor["type"];

    if (!location || !type) {
        System::logger.write(
            LOG_ERROR,
            "FileTransfer: the destination needs a location and a type");
        return (false);
    }

    if (strcmp(location, slotsLocation) == 0) {
        const int bankNumber = descriptor["bankNumber"] | -1;
        const int slot = descriptor["slot"] | -1;

        if ((bankNumber < 0) || (bankNumber >= numberOfBanks) || (slot < 0)
            || (slot >= slotsPerBank)) {
            System::logger.write(LOG_ERROR,
                                 "FileTransfer: no such slot: bank=%d, slot=%d",
                                 bankNumber,
                                 slot);
            return (false);
        }

        /* A slot holds one script on this model, so the `path` a newer
         * firmware uses to put several in one slot has nowhere to go. Refused
         * rather than ignored: a host that asked for `modules/x.lua` and was
         * given `the script` would not find what it wrote. */
        if (descriptor["path"].is<const char *>()) {
            System::logger.write(
                LOG_ERROR,
                "FileTransfer: a slot holds one script, so it takes no path");
            return (false);
        }

        const uint16_t fileNumber =
            (uint16_t)((bankNumber * slotsPerBank) + slot);

        if (strcmp(type, presetType) == 0) {
            System::context.formatPresetFilename(path, size, fileNumber);
            return (true);
        }

        if (strcmp(type, luaType) == 0) {
            System::context.formatLuaFilename(path, size, fileNumber);
            return (true);
        }

        System::logger.write(
            LOG_ERROR, "FileTransfer: a slot holds no %s", type);
        return (false);
    }

    if ((strcmp(location, rootLocation) == 0)
        && (strcmp(type, configType) == 0)) {
        copyString(path, System::context.getCurrentConfigFile(), size - 1);
        return (true);
    }

    System::logger.write(LOG_ERROR,
                         "FileTransfer: nothing is kept at %s/%s on this model",
                         location,
                         type);
    return (false);
}

bool FileTransfer::namesTheLoadedSlot(const char *path)
{
    if (path == nullptr) {
        return (false);
    }

    return ((strcmp(path, System::context.getCurrentPresetFile()) == 0)
            || (strcmp(path, System::context.getCurrentLuaFile()) == 0));
}

bool FileTransfer::removeLocation(const char *directory)
{
    FsFile dir;

    if (!dir.open(directory, O_RDONLY)) {
        System::logger.write(LOG_ERROR,
                             "FileTransfer::removeLocation: no such location: "
                             "%s",
                             directory);
        return (false);
    }

    bool removedAll = true;
    FsFile entry;
    char name[FileTransfer::maxPathLength];
    char path[FileTransfer::maxPathLength];

    while (entry.openNext(&dir, O_RDONLY)) {
        const bool isFile = !entry.isDirectory();
        const bool named = (entry.getName(name, sizeof(name)) > 0);

        /* Closed before the file is removed, not after: a name is read off an
         * open entry and the entry has to be let go of before the card is told
         * to delete what it points at. */
        entry.close();

        if (isFile && named) {
            snprintf(path, sizeof(path), "%s/%s", directory, name);

            if (!Hardware::sdcard.deleteFile(path)) {
                System::logger.write(
                    LOG_ERROR,
                    "FileTransfer::removeLocation: could not remove: %s",
                    path);
                removedAll = false;
            }
        }
    }

    dir.close();

    return (removedAll);
}

bool FileTransfer::createDirectory(const char *directory)
{
    if (Hardware::sdcard.exists(directory)) {
        /* Already there is success, which is what the API says of it: a host
         * making sure a location exists should not have to ask first. */
        return (true);
    }

    if (!Hardware::sdcard.createDirectory(directory)) {
        System::logger.write(
            LOG_ERROR, "FileTransfer::createDirectory: failed: %s", directory);
        return (false);
    }

    return (true);
}

bool FileTransfer::removeDirectory(const char *directory)
{
    FsFile dir;

    if (!dir.open(directory, O_RDONLY)) {
        System::logger.write(LOG_ERROR,
                             "FileTransfer::removeDirectory: no such "
                             "directory: %s",
                             directory);
        return (false);
    }

    FsFile entry;
    const bool empty = !entry.openNext(&dir, O_RDONLY);

    entry.close();
    dir.close();

    if (!empty) {
        /* Refused rather than emptied. The API says a directory has to be
         * empty before it goes, and a recursive delete asked for by one byte on
         * the wire is not worth having. */
        System::logger.write(LOG_ERROR,
                             "FileTransfer::removeDirectory: not empty: %s",
                             directory);
        return (false);
    }

    if (!Hardware::sdcard.removeEmptyDirectory(directory)) {
        System::logger.write(
            LOG_ERROR, "FileTransfer::removeDirectory: failed: %s", directory);
        return (false);
    }

    return (true);
}

bool FileTransfer::sendFile(uint8_t port, const char *path)
{
    if (!Hardware::sdcard.exists(path)) {
        System::logger.write(
            LOG_ERROR, "FileTransfer::sendFile: no such file: %s", path);
        return (false);
    }

    return (MidiOutput::sendSysExFileBase64(
        port, path, ElectraCommand::Object::File));
}
