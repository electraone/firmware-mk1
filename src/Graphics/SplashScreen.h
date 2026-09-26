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
 * @file SplashScreen.h
 *
 * @brief A function to display a splash screen.
 */

#pragma once

#include "Graphics.h"

/**
 * @brief The E1 logo, as the mk2 draws it: six rectangles, 52 by 72.
 *
 * Drawn with nothing but rectangles, so it needs no fonts - it can be on
 * the screen the moment the display is up, before the card has mounted or
 * a glyph has been built, which on this model is the long part of a boot.
 */
void paintLogo(Graphics &g, uint16_t x, uint16_t y);

/**
 * @brief The first thing on the screen: a black ground and the logo.
 *
 * Called as soon as the display is initialised. Everything that follows in
 * the boot draws under it rather than over it.
 */
void displaySplashLogo(Graphics &g);

/**
 * @brief The name and version under the logo, once the fonts exist.
 *
 * @param appName what the program calls itself, printed in capitals
 */
void displaySplash(Graphics &g,
                   const char *appName,
                   const char *appVersion,
                   uint8_t hwRevision);
