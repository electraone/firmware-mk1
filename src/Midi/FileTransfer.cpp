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
    /** The one location this serves, and the one type kept in it. */
    constexpr const char *tmpLocation = "tmp";
    constexpr const char *screenshotType = "screenshot";
    constexpr const char *screenshotExtension = ".bmp";

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
