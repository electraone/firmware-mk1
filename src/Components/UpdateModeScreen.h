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
 * @file UpdateModeScreen.h
 *
 * @brief A component to indicate the Update mode.
 */

#pragma once

#include "Component.h"

class UpdateModeScreen : public Component
{
public:
    UpdateModeScreen()
    {
        setBounds(0, 20, 1024, 560);
        setVisible(true);
    }

    virtual ~UpdateModeScreen() = default;

    void paint(Graphics &g)
    {
        g.setBteChromaColor(0x000000);
        g.dim(0, 0, 1024, 600, Colours565::black);
        g.dim(398, 162, 228, 205, 0x1840);
        g.copyFromToolkit(822, 78, 466, 200, 92, 80);
        g.system().printText(398,
                             320,
                             "Firmware Update mode",
                             TextStyle::mediumTransparent,
                             228,
                             TextAlign::center);
    }

private:
};
