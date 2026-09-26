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
 * @file LinkGestureSink.h
 *
 * @brief Where a custom control's touches go.
 *
 * A custom control's input belongs to its preset's script, and the script
 * runs on the primary, so a touch on one travels raw. The component that
 * receives the touch knows nothing about the link; it hands the event to
 * this, and the program behind it stamps the view and sends it.
 *
 * On the mk2 the touches are banked from the touch thread and sent from the
 * application thread. Here there is one thread, so the hand-over is a call.
 */

#pragma once

#include "TouchEvent.h"
#include <cstdint>

class LinkGestureSink
{
public:
    virtual ~LinkGestureSink() = default;

    virtual void touchGesture(uint16_t elementId,
                              const TouchEvent &touchEvent) = 0;
};
