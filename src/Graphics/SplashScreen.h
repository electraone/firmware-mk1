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

/** @brief Black, so that the fade starts from nothing. */
void clearSplash(Graphics &g);

/**
 * @brief Takes the logo one step further, if a step is due.
 *
 * It fades in on the clock, painted rather than copied. Made to be called for
 * every line of the fonts as they are built, so that it runs alongside them.
 */
void fadeInSplash(void);

/** @brief Waits for whatever of the fade the fonts did not cover. */
void completeSplash(void);

/** @brief The name and the version under the logo, once the fonts exist. */
void printSplashText(Graphics &g);
