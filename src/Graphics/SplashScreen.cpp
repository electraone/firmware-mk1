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

void displaySplash(Graphics &g,
                   const char *appName,
                   const char *appVersion,
                   uint8_t hwRevision)
{
    char versionText[100];

    sprintf(versionText,
            "HW V%d.%d %s %s",
            hwRevision / 100,
            hwRevision - ((hwRevision / 100) * 100),
            appName,
            appVersion);

    g.switchWriteLayer();
    g.system().printText(0,
                         360,
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
    g.showPreparedLayer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}
