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

#include "SplashScreen.h"
#include "Graphics.h"
#include "VideoRam.h"
#include "FrameBuffer.h"
#include "Colours.h"
#include <cctype>
#include <cstdio>
#include <cstring>

/* Where the logo sits: centred on the 1024 by 600 panel, with the name and
 * version below it and the address at the foot, as on the mk2. */
static constexpr uint16_t logoX = 486;
static constexpr uint16_t logoY = 228;

/* The mk2's own rectangles, verbatim. */
void paintLogo(Graphics &g, uint16_t x, uint16_t y)
{
    g.setColour(Colours565::white);
    g.fillRect(x, y, 8, 72);
    g.fillRect(x + 8, y, 44, 8);
    g.fillRect(x + 8, y + 64, 44, 8);
    g.fillRect(x + 21, y + 20, 20, 8);
    g.fillRect(x + 33, y + 28, 8, 16);
    g.fillRect(x + 21, y + 44, 31, 8);
}

void displaySplashLogo(Graphics &g)
{
    g.fillAll(Colours565::black);
    paintLogo(g, logoX, logoY);
}

/*
 * Straight onto the visible page rather than through a prepared layer: the
 * logo is already there, and showing a layer built from nothing would take
 * it down for the text. Nothing else is drawing yet.
 */
void displaySplash(Graphics &g,
                   const char *appName,
                   const char *appVersion,
                   uint8_t hwRevision)
{
    char name[32];
    char versionText[64];

    strncpy(name, appName, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';

    for (char *c = name; *c != '\0'; c++) {
        *c = (char)toupper((unsigned char)*c);
    }

    snprintf(versionText,
             sizeof(versionText),
             "VERSION %s   HW %d.%02d",
             appVersion,
             hwRevision / 100,
             hwRevision - ((hwRevision / 100) * 100));

    g.system().printText(0,
                         logoY + 100,
                         name,
                         TextStyle::mediumTransparent,
                         1024,
                         TextAlign::center);
    g.system().printText(0,
                         logoY + 134,
                         versionText,
                         TextStyle::smallTransparent,
                         1024,
                         TextAlign::center);
    g.system().printText(0,
                         500,
                         "WWW.ELECTRA.ONE",
                         TextStyle::smallTransparent,
                         1024,
                         TextAlign::center);
}
