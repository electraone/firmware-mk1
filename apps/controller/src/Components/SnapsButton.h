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
 * @file SnapsButton.h
 *
 * @brief Implements an on-screen button representing a Snapshot.
 */

#pragma once

#include "Components/Button.h"

class SnapsButton : public Button
{
public:
    SnapsButton();
    virtual ~SnapsButton() = default;

    void setUsed(bool shouldBeUsed);
    bool isUsed();
    void setProgramInfo(const char *newProgramInfo);
    void setBankNumber(uint8_t newBankNumber);
    void paint(Graphics &g) override;

    static const uint8_t maxLabelLength = Component::MaxNameLength;
    static const uint8_t maxProgramInfoLength = 7;

private:
    static void paintUsed(Graphics &g,
                          uint16_t width,
                          uint16_t height,
                          uint32_t colour,
                          const char *programInfo,
                          const char *label,
                          bool highlighted);
    static void paintUnused(Graphics &g,
                            uint16_t width,
                            uint16_t height,
                            const char *programInfo);
    static void
        printProgramInfo(Graphics &g, const char *programInfo, bool used);

    char programInfo[maxProgramInfoLength + 1];

    struct {
        uint8_t bankNumber : 4;
        bool used : 1;
    };
};
