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
 * @file DebugMessage.cpp
 *
 * @brief A simple text component to display debug messages.
 */

#pragma once

#include "Component.h"

class DebugMessage : public Component
{
public:
    DebugMessage(char *newBuf) : buf(newBuf)
    {
    }

    ~DebugMessage()
    {
    }

    void paint(Graphics &g)
    {
        g.fillAll(Colours565::black);
        g.system().printText(0, 0, buf, TextStyle::smallTransparent, getWidth(), TextAlign::left);
    }

private:
    char *buf;
};
