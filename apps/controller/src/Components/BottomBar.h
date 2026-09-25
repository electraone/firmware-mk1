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
 * @file BottomBar.cpp
 *
 * @brief Implements the status bar shown at the bottom of the screen.
 * 
 * The bottom bar shows the name of the current preset and the name of
 * the current page and a few other status information.
 */

#pragma once

#include "Component.h"
#include "MeterHorizontal.h"

class BottomBar : virtual public Component
{
private:
    // @todo move to common config file
    static constexpr uint8_t maxPresetNameLength = 20;
    static constexpr uint8_t maxPageNameLength = 20;
    static constexpr uint8_t maxInfoTextLength = 20;

public:
    BottomBar(const char *newPresetName, const char *newPageName);
    virtual ~BottomBar() = default;

    void paint(Graphics &g) override;
    void resized(void) override;

    void setPresetName(const char *newName);
    void setPageName(const char *newName);
    void setInfoText(const char *newText);
    void setRamPercentage(uint8_t newPercentage);
    void update(const char *newPresetName, const char *newPageName);

private:
    char presetName[maxPresetNameLength + 1];
    char pageName[maxPageNameLength + 1];
    char infoText[maxInfoTextLength + 1];

    MeterHorizontal *ramIndicator;
};
