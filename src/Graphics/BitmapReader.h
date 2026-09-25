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
 * @file BitmapReader.h
 *
 * @brief A loader of bitmap data. File to videoRAM.
 */

#pragma once

#include "FrameBuffer.h"
#include "PersistentStorage.h"

/*
 * BMP parsing structures
 */
typedef struct {
    unsigned short int type; // Magic identifier
    unsigned int size; // File size in bytes
    unsigned short int reserved1, reserved2;
    unsigned int offset; // Offset to image data, bytes
} bmpHeader;

typedef struct {
    unsigned int size; // Header size in bytes
    int width, height; // Width and height of image
    unsigned short int planes; // Number of colour planes
    unsigned short int bits; // Bits per pixel
    unsigned int compression; // Compression type
    unsigned int imagesize; // Image size in bytes
    int xresolution, yresolution; // Pixels per meter
    unsigned int ncolours; // Number of colours
    unsigned int importantcolours; // Important colours
} bmpInfoHeader;

bool readUShort(FileIoStream &, unsigned short *, int);
bool readUInt(FileIoStream &, unsigned int *, int);

/** \brief a representation of the Screen - the object describing what user
 *         sees on the LCD display
 */
class BitmapReader : public virtual FrameBuffer
{
public:
    BitmapReader(const uint8_t CSp, const uint8_t RSTp);
    ~BitmapReader();

    void loadBMP(const char *filename,
                 uint16_t lines = 600,
                 bool progressBar = true);

    void initUIToolkit(void);
};
