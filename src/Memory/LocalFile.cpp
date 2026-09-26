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

#include "LocalFile.h"
#include "System.h"

LocalFile::LocalFile() : filepath("")
{
}

LocalFile::LocalFile(const char *filepath) : filepath(filepath)
{
}

LocalFile::~LocalFile()
{
}

void LocalFile::setFilepath(const char *newFilepath)
{
    filepath = newFilepath;
}

const char *LocalFile::getFilepath(void)
{
    return (filepath);
}

bool LocalFile::createDirectory(void)
{
    return (Hardware::sdcard.createDirectory(filepath));
}

bool LocalFile::exists(void)
{
    return (Hardware::sdcard.exists(filepath));
}

FileIoStream LocalFile::createInputStream(void)
{
    return (Hardware::sdcard.createInputStream(filepath));
}

FileIoStream LocalFile::createOutputStream(oflag_t oflag)
{
    return (Hardware::sdcard.createOutputStream(filepath, oflag));
}

bool LocalFile::deleteFile(void)
{
    return (Hardware::sdcard.deleteFile(filepath));
}

bool LocalFile::removeEmptyDirectory(void)
{
    return (Hardware::sdcard.removeEmptyDirectory(filepath));
}

bool LocalFile::writeAll(MemoryBlock &memoryBlock)
{
    static const size_t bufferSize = 200;
    File file;

    file = Hardware::sdcard.createOutputStream(filepath,
                                               FILE_WRITE | O_CREAT | O_TRUNC);

    if (!file) {
        System::logger.write(
            LOG_ERROR, "Localfile: Cannot open file failed: %s", filepath);
        return (false);
    }

    uint8_t buffer[bufferSize];
    size_t length = 0;

    while ((length = memoryBlock.readBytes(buffer, bufferSize)) > 0) {
        //logData ((uint8_t *) buffer, length);
        file.write(buffer, length);
    }

    file.close();

    return (true);
}

bool LocalFile::rename(const char *newFilepath)
{
    File file = Hardware::sdcard.createOutputStream(filepath);

    if (!file) {
        return (false);
    }

    bool rc = file.rename(newFilepath);

    file.close();

    return rc;
}
