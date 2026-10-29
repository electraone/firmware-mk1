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
 * @file ScreenCapture.cpp
 *
 * @brief Writes what is on the panel to a file.
 */

#include "ScreenCapture.h"

#include "Hardware.h"
#include "System.h"

#include <cstdio>
#include <cstring>

namespace
{
    /** Where the pixels start in the file, which is the header's own length. */
    constexpr uint32_t pixelOffset = 66;
    constexpr uint16_t bytesPerRow = SCREEN_WIDTH * 2;
    constexpr uint32_t imageBytes = (uint32_t)bytesPerRow * SCREEN_HEIGHT;

    void putU16(uint8_t *at, uint16_t value)
    {
        at[0] = (uint8_t)(value & 0xFF);
        at[1] = (uint8_t)((value >> 8) & 0xFF);
    }

    void putU32(uint8_t *at, uint32_t value)
    {
        at[0] = (uint8_t)(value & 0xFF);
        at[1] = (uint8_t)((value >> 8) & 0xFF);
        at[2] = (uint8_t)((value >> 16) & 0xFF);
        at[3] = (uint8_t)((value >> 24) & 0xFF);
    }

    /**
 * @brief The 66 bytes in front of the pixels.
 *
 * A BITMAPINFOHEADER saying sixteen bits a pixel with the channel masks
 * spelled out, which is the only way to say RGB565 in a BMP - the older form
 * with no masks means five bits each and a bit thrown away, and every viewer
 * would read the picture green-shifted. The height is negative, which is how
 * a BMP says its rows run top down, as the display's do.
 */
    void writeHeader(uint8_t *header)
    {
        memset(header, 0, pixelOffset);

        header[0] = 0x42; // "BM"
        header[1] = 0x4D;
        putU32(header + 2, pixelOffset + imageBytes);
        putU32(header + 10, pixelOffset);
        putU32(header + 14, 40); // BITMAPINFOHEADER
        putU32(header + 18, SCREEN_WIDTH);
        putU32(header + 22, (uint32_t)(-(int32_t)SCREEN_HEIGHT)); // top down
        putU16(header + 26, 1); // planes
        putU16(header + 28, 16); // bits per pixel
        putU32(header + 30, 3); // BI_BITFIELDS
        putU32(header + 34, imageBytes);
        putU32(header + 38, 2835); // 72 dpi
        putU32(header + 42, 2835);
        putU32(header + 54, 0xF800); // red
        putU32(header + 58, 0x07E0); // green
        putU32(header + 62, 0x001F); // blue
    }
} // namespace

bool ScreenCapture::save(const char *filename)
{
    /* Below the SRAM bank boundary, as everything in .bss.lowram is. */
    __attribute__((section(".bss.lowram"))) static uint8_t row[bytesPerRow];

    Hardware::sdcard.createDirectory(ScreenCapture::directory);

    FileIoStream file = Hardware::sdcard.createOutputStream(
        filename, FILE_WRITE | O_CREAT | O_TRUNC);

    if (!file) {
        System::logger.write(
            LOG_ERROR, "ScreenCapture::save: cannot create %s", filename);
        return (false);
    }

    uint8_t header[pixelOffset];

    writeHeader(header);

    if (file.write(header, sizeof(header)) != sizeof(header)) {
        System::logger.write(LOG_ERROR,
                             "ScreenCapture::save: cannot write the header");
        file.close();
        return (false);
    }

    const uint32_t address = Hardware::screen.getActiveBufferAddress();

    for (uint16_t y = 0; y < SCREEN_HEIGHT; y++) {
        Hardware::screen.readRamData(
            address + ((uint32_t)y * bytesPerRow), row, bytesPerRow);

        if (file.write(row, bytesPerRow) != bytesPerRow) {
            System::logger.write(
                LOG_ERROR, "ScreenCapture::save: cannot write row %d", y);
            file.close();
            return (false);
        }
    }

    file.close();

    System::logger.write(
        LOG_INFO, "ScreenCapture::save: written: filename=%s", filename);

    return (true);
}

bool ScreenCapture::saveNext(char *filename, size_t size)
{
    /* The number is found rather than counted, so that a screenshot taken
     * after a reboot does not write over one taken before it. */
    for (uint16_t index = 0; index < 1000; index++) {
        snprintf(filename, size, "%s/scrn%03d.bmp", directory, index);

        if (!Hardware::sdcard.exists(filename)) {
            return (save(filename));
        }
    }

    System::logger.write(
        LOG_ERROR, "ScreenCapture::saveNext: the scratch directory is full");

    return (false);
}
