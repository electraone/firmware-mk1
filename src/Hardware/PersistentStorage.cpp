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
 * @file PersistentStorage.cpp
 *
 * @brief A generic interface to non-volatile data storage.
 */

#include "PersistentStorage.h"
#include "System.h"

PersistentStorage::PersistentStorage() : mounted(false)
{
}

bool PersistentStorage::mount(void)
{
    /* Ten tries at a fifth of a second each, which is two seconds of a card
     * that is merely slow to wake and no more than that of one that never
     * will. */
    static constexpr uint8_t maxAttempts = 10;

    mounted = false;

    for (uint8_t attempt = 0; attempt < maxAttempts; attempt++) {
        if (begin(SdioConfig(FIFO_SDIO))) {
            mounted = true;
            System::logger.write(LOG_ERROR, "internal storage: Initialized");
            delay(100);
            return (true);
        }

        System::logger.write(LOG_ERROR,
                             "internal storage: not ready, attempt %d of %d: "
                             "sdError=0x%02X, data=0x%02X",
                             attempt + 1,
                             maxAttempts,
                             sdErrorCode(),
                             sdErrorData());
        delay(200);
    }

    /* Said once more at the end, because the attempts above scroll past and
     * this is the line that explains everything that follows: no preset, no
     * configuration, and every file the firmware asks for missing. */
    System::logger.write(LOG_ERROR,
                         "internal storage: FAILED to initialize, "
                         "continuing without it: sdError=0x%02X, data=0x%02X",
                         sdErrorCode(),
                         sdErrorData());

    return (false);
}

bool PersistentStorage::isMounted(void) const
{
    return (mounted);
}

bool PersistentStorage::createDirectory(const char *path, bool pFlag)
{
    return (SdFs::mkdir(path, pFlag));
}

bool PersistentStorage::exists(const char *path)
{
    return (SdFs::exists(path));
}

bool PersistentStorage::directoryExists(const char *path)
{
    return (SdFs::exists(path));
}

FileIoStream PersistentStorage::createInputStream(const char *path)
{
    return (SdFs::open(path, O_RDONLY));
}

FileIoStream PersistentStorage::createOutputStream(const char *path,
                                                   oflag_t oflag)
{
    return (SdFs::open(path, oflag));
}

bool PersistentStorage::deleteFile(const char *path)
{
    return (SdFs::remove(path));
}

bool PersistentStorage::deleteRecursively(const char *path)
{
    return (SdFs::rmdir(path));
}

bool PersistentStorage::renameFile(const char *src, const char *dest)
{
    return (SdFs::rename(src, dest));
}

bool PersistentStorage::format(void)
{
    return SdFs::format();
}

bool PersistentStorage::copyFile(const char *src, const char *dest)
{
    File srcFile = createInputStream(src);
    File destFile = createOutputStream(dest, FILE_WRITE | O_CREAT | O_TRUNC);
    bool success = false;
    constexpr size_t bufferSize = 512;

    if (srcFile) {
        if (destFile) {
            char buffer[bufferSize];
            size_t bytesRead = 0;

            while ((bytesRead = srcFile.read(buffer, bufferSize)) > 0) {
                if (destFile.write(buffer, bytesRead) != bytesRead) {
                    break;
                }
            }
            success = true;
            destFile.close();
        }
        srcFile.close();
    }

    return (success);
}