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
 * @file LocalFile.h
 *
 * @brief A class to handle files on the internal storage.
 */

#pragma once

#include "Hardware.h"
#include "MemoryBlock.h"

class LocalFile
{
public:
    LocalFile();
    LocalFile(const char *filepath);
    ~LocalFile();

    void setFilepath(const char *newFilepath);
    const char *getFilepath(void);

    bool createDirectory(void);
    bool exists(void);
    FileIoStream createInputStream(void);
    FileIoStream createOutputStream(oflag_t oflag = O_WRITE | O_CREAT
                                                    | O_TRUNC);
    bool deleteFile(void);
    bool removeEmptyDirectory(void);
    bool writeAll(MemoryBlock &memoryBlock);

    bool rename(const char *newFilepath);

private:
    const char *filepath;
};
