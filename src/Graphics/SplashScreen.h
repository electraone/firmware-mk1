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
 * @brief Takes the boot screen one step further, if a step is due.
 *
 * The logo fades in on the clock, painted rather than copied, and the address
 * and version go up as soon as the small font has arrived. Made to be called
 * for every line of the UI assets, so that the fade runs alongside them.
 */
void fadeInSplash(void);

/** @brief Waits for whatever of the fade the UI assets did not cover. */
void completeSplash(void);

/** @brief Puts the logo up at once, when there are no UI assets to wait on. */
void finishLogo(void);
