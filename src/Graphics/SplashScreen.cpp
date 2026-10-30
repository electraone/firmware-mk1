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
 * @file SplashScreen.cpp
 *
 * @brief A function to display a splash screen.
 */

#include "Graphics.h"
#include "VideoRam.h"
#include "FrameBuffer.h"
#include "Colours.h"
#include "Hardware.h"
#include "settings.h"

namespace
{
    /* The E1 mark as an outline: the one-pixel edge of the 52 by 72 shape,
     * drawn inside it so that the outline is the size the mark is. */
    constexpr uint8_t logoWidth = 52;
    constexpr uint8_t logoHeight = 72;
    constexpr uint16_t logoX = (SCREEN_WIDTH - logoWidth) / 2;
    constexpr uint16_t logoY = (SCREEN_HEIGHT - logoHeight) / 2 - 25;

    const uint8_t logoOutline[][4] = {
        { 0, 0, 1, 72 },   { 1, 0, 51, 1 },   { 51, 1, 1, 7 },
        { 7, 7, 1, 58 },   { 8, 7, 43, 1 },   { 21, 20, 20, 1 },
        { 21, 21, 1, 7 },  { 40, 21, 1, 24 }, { 22, 27, 12, 1 },
        { 33, 28, 1, 17 }, { 21, 44, 12, 1 }, { 41, 44, 11, 1 },
        { 21, 45, 1, 7 },  { 51, 45, 1, 7 },  { 22, 51, 29, 1 },
        { 8, 64, 44, 1 },  { 51, 65, 1, 7 },  { 1, 71, 50, 1 },
    };

    /* From black to white in a second, started as the UI assets start to
     * load, so that it runs alongside them rather than after them. */
    constexpr uint8_t fadeSteps = 40;
    constexpr uint8_t fadeStepMs = 25;

    /* The small font is the top of the UI assets. */
    constexpr uint16_t smallFontLines = 13;

    /* A grey in RGB565. Squared so that the steps are even to the eye: in a
     * linear ramp almost all of what can be seen change is at the bottom. */
    uint16_t fadeColour(uint8_t step)
    {
        const uint32_t level = (255u * step * step) / (fadeSteps * fadeSteps);

        return ((uint16_t)(((level >> 3) << 11) | ((level >> 2) << 5)
                           | (level >> 3)));
    }

    uint32_t fadeStart = 0;
    uint8_t fadeStep = 0;
    uint16_t linesLoaded = 0;

    /* Under the logo, centred on it, a line apart. */
    constexpr uint16_t textWidth = 200;
    constexpr uint16_t textHeight = 13;
    constexpr uint16_t textGap = 11;
    constexpr uint16_t addressY = logoY + logoHeight + 16;
    constexpr uint16_t versionY = addressY + textHeight + textGap;
    constexpr uint8_t versionDimPasses = 1;

    void paintLogo(Graphics &g, uint16_t colour)
    {
        g.setForegroundColor(colour);

        for (const auto &rect : logoOutline) {
            g.fillRect(logoX + rect[0], logoY + rect[1], rect[2], rect[3]);
        }
    }
} // namespace

static void printSplashText(Graphics &g)
{
    g.printText(0,
                addressY,
                "electra one mk1",
                TextStyle::smallWhiteOnBlack,
                SCREEN_WIDTH,
                TextAlign::center);
    g.printText(0,
                versionY,
                FIRMWARE_VERSION,
                TextStyle::smallWhiteOnBlack,
                SCREEN_WIDTH,
                TextAlign::center);

    /* Dimmed as the interface dims: a blend towards black, which leaves black
     * as it is and darkens the text. One pass takes off about a quarter. */
    for (uint8_t i = 0; i < versionDimPasses; i++) {
        g.dim((SCREEN_WIDTH - textWidth) / 2,
              versionY,
              textWidth,
              textHeight,
              Colours565::black);
    }
}

void fadeInSplash(void)
{
    Graphics &g = Hardware::screen;

    if (linesLoaded == 0) {
        fadeStart = millis();
    }

    if (++linesLoaded == smallFontLines) {
        g.switchWriteLayer();
        printSplashText(g);
        g.switchWriteLayer();
    }

    const uint32_t due = (millis() - fadeStart) / fadeStepMs;

    if ((fadeStep < fadeSteps) && (due > fadeStep)) {
        fadeStep = (due < fadeSteps) ? (uint8_t)due : fadeSteps;

        /* The layer on the screen, not the one being prepared. */
        g.switchWriteLayer();
        paintLogo(g, fadeColour(fadeStep));
        g.switchWriteLayer();
    }
}

void completeSplash(void)
{
    while (fadeStep < fadeSteps) {
        fadeInSplash();
        delay(1);
    }
}

void finishLogo(void)
{
    Graphics &g = Hardware::screen;

    g.switchWriteLayer();
    paintLogo(g, fadeColour(fadeSteps));
    g.switchWriteLayer();
}
