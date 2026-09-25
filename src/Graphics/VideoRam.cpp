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

#include "VideoRam.h"
#include "helpers.h"

VideoRam::VideoRam(const uint8_t CSp, const uint8_t RSTp)
    : FrameBuffer(CSp, RSTp)
{
}

void VideoRam::storeRam(uint32_t address, ByteSize size, uint32_t value)
{
#ifdef DEBUG
    System::logger.write(
        ERROR, "storeRam: address=%d, size=%d, value=%d", address, size, value);
#endif

    saveState();

    setMemoryMode(MemoryMode::LinearBpp8);
    setAddress(address);

    if (size == ByteSize::Uint8) {
        writeRAM8(value & 0xFF);
    } else if (size == ByteSize::Uint16) {
        writeRAM16(value & 0xFFFF);
    } else if (size == ByteSize::Uint32) {
        writeRAM32(value);
    }

    restoreState();
    setMemoryMode(DEFAULT_BPP);
}

void VideoRam::storeRam8(uint32_t address, uint8_t value)
{
    storeRam(address, ByteSize::Uint8, value);
}

void VideoRam::storeRam16(uint32_t address, uint16_t value)
{
    storeRam(address, ByteSize::Uint16, value);
}

void VideoRam::storeRam32(uint32_t address, uint32_t value)
{
    storeRam(address, ByteSize::Uint32, value);
}

uint32_t VideoRam::readRam(uint32_t address, ByteSize size)
{
    uint32_t value = 0;

    saveState();

    setMemoryMode(MemoryMode::LinearBpp8);
    setAddress(address);

    if (size == ByteSize::Uint8) {
        value = readRAM8() & 0xFF;
    } else if (size == ByteSize::Uint16) {
        value = readRAM16() & 0xFFFF;
    } else if (size == ByteSize::Uint32) {
        value = readRAM32();
    }

    restoreState();
    setMemoryMode(DEFAULT_BPP);

#ifdef DEBUG
    System::logger.write(
        ERROR, "readRam: address=%d, size=%d, value=%d", address, size, value);
#endif
    return (value);
}

uint8_t VideoRam::readRam8(uint32_t address)
{
    return (readRam(address, ByteSize::Uint8));
}

uint16_t VideoRam::readRam16(uint32_t address)
{
    return (readRam(address, ByteSize::Uint16));
}

uint32_t VideoRam::readRam32(uint32_t address)
{
    return (readRam(address, ByteSize::Uint32));
}

void VideoRam::readRamData(uint32_t address, uint8_t *data, uint16_t size)
{
#ifdef DEBUG
    System::logger.write(LOG_ERROR, "readRamData: address=%d", address);
#endif

    saveState();

    setMemoryMode(MemoryMode::LinearBpp8);
    setAddress(address);
    readRamBulk(data, size);

    restoreState();
    setMemoryMode(DEFAULT_BPP);

#ifdef DEBUG
    logData(data, size, "data read:");
#endif
}

void VideoRam::writeRamData(uint32_t address,
                            const uint8_t *data,
                            uint16_t size)
{
#ifdef DEBUG
    System::logger.write(LOG_ERROR, "writeRamData: address=%d", address);
#endif

    saveState();

    setMemoryMode(MemoryMode::LinearBpp8);
    setAddress(address);
    writeRamBulk(data, size);

    restoreState();
    setMemoryMode(DEFAULT_BPP);
}

void VideoRam::resetExtRam(void)
{
    noInterrupts();
    setForegroundColor(0);
    setCanvasAddress(SDRAM_PAGE_0);
    fillRect(0, 0, 1024, 600);
    setCanvasAddress(SDRAM_PAGE_1);
    fillRect(0, 0, 1024, 600);
    setCanvasAddress(SDRAM_PAGE_2);
    fillRect(0, 0, 1024, 600);
    setCanvasAddress(SDRAM_PAGE_3);
    fillRect(0, 0, 1024, 600);

#if DEFAULT_BPP < 24
    setCanvasAddress(SDRAM_PAGE_4);
    fillRect(0, 0, 1024, 600);
    setCanvasAddress(SDRAM_PAGE_5);
    fillRect(0, 0, 1024, 600);
    setCanvasAddress(SDRAM_PAGE_6);
    fillRect(0, 0, 1024, 600);
    setCanvasAddress(SDRAM_PAGE_7);
    fillRect(0, 0, 1024, 600);
    setCanvasAddress(SDRAM_PAGE_8);
    fillRect(0, 0, 1024, 600);
    setCanvasAddress(SDRAM_CGRAM);
    fillRect(0, 0, 1024, 600);
#endif
    interrupts();
}
