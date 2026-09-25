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
 * @file FrameBuffer.h
 *
 * @brief A videoRAM page repaint process management.
 */

#pragma once

#include "RA8876.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 600
#define SCREEN_MAX_X (SCREEN_WIDTH - 1)
#define SCREEN_MAX_Y (SCREEN_HEIGHT - 1)

/* Four banks of 4096 rows by 512 columns of 16 bits - see defaultSdramInfo. */
#define VIDEO_RAM_SIZE 0x01000000

/*
 * Video RAM map
 */
#if DEFAULT_BPP == 24
#define FRAME_BUFFER_1 0x00000000
#define FRAME_BUFFER_2 0x00258000
#define FRAME_UI_TOOLKIT 0x004B0000
#define FRAME_MASK 0x00708000
#define SDRAM_PAGE_0 0x00960000
#define SDRAM_PAGE_1 0x00BB8000
#define SDRAM_PAGE_2 0x00708000
#define SDRAM_PAGE_3 0x00E10000
#define SDRAM_PAGE_4 0x00E10000 // on purpose
#define SDRAM_PAGE_5 0x00E10000
#define SDRAM_PAGE_6 0x00E10000
#define SDRAM_PAGE_7 0x00E10000
#define SDRAM_PAGE_8 0x00E10000
#define SDRAM_PAGE_SIZE 0x00258000
#define FRAME_GLYPHS 0x00A8C000
#define FRAME_GLYPHS_END VIDEO_RAM_SIZE
#else
#define FRAME_BUFFER_1 0x00000000
#define FRAME_BUFFER_2 0x0012C000
#define FRAME_UI_TOOLKIT 0x00258000
#define FRAME_MASK 0x00384000
#define SDRAM_PAGE_0 0x004B0000
#define SDRAM_PAGE_1 0x005DC000
#define SDRAM_PAGE_2 0x00708000
#define SDRAM_PAGE_8 0x00E10000
#define SDRAM_PAGE_SIZE 0x0012C000

/*
 * The glyph atlas and the masks beside it, which do not fit in a page and so
 * do not get one: they run from where page three used to start to where page
 * eight does, six megabytes, of which about four and three quarters are in
 * use. Sixteen user faces in six colours, two system faces in three, and one
 * bit masks of all sixteen is what that buys.
 *
 * Pages three to seven are gone rather than left named, because a name is an
 * invitation: anything that took one would be writing over the text.
 * TextAtlas.cpp fails the build if the atlas outgrows what is here.
 */
#define FRAME_GLYPHS 0x00834000
#define FRAME_GLYPHS_END SDRAM_PAGE_8
#endif

/**
 * @brief implements a video driver with buffered graphics handling.
 *        The frame buffer makes it possible to have smooth graphic objects
 *        transitions.
 */
class FrameBuffer : public RA8876
{
public:
    int writeLayer;

public:
    /** Constructor */
    FrameBuffer(const uint8_t CSp, const uint8_t RSTp);

    /** Destructor */
    virtual ~FrameBuffer() = default;

    /**
     * switches the layer for writing the graphics, ie. drawing functions
		 * will write to that layer.
		 */
    void switchWriteLayer(void);

    /**
     * switches to the prepared (invisible) layer and synchronize both layers.
		 *  @param x X coordinate of the beginning of the synchornized area.
		 *  @param y Y coordinate.
		 *  @param width width of the synchornized area.
		 *  @param height height of the synchornized area.
		 */
    void showPreparedLayer(int x, int y, int width, int height);

    /**
     * Returns the address of active FrameBuffer.
     *
     * @return address of the current FrameBuffer.
		 */
    uint32_t getActiveBufferAddress(void);

    /**
     * Clears screen using the Frame buffer.
     */
    void clear(void);

private:
    void showPreparedLayer(void);
    void syncLayers(int x, int y, int width, int height);
};
