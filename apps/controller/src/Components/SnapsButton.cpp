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

#include "SnapsButton.h"

SnapsButton::SnapsButton() : used(false)
{
}

void SnapsButton::setUsed(bool shouldBeUsed)
{
    used = shouldBeUsed;
}

bool SnapsButton::isUsed()
{
    return used;
}

void SnapsButton::setProgramInfo(const char *newProgramInfo)
{
    copyString(programInfo, newProgramInfo, maxProgramInfoLength);
}

void SnapsButton::setBankNumber(uint8_t newBankNumber)
{
    bankNumber = newBankNumber;
}

void SnapsButton::paint(Graphics &g)
{
    if (used) {
        paintUsed(g,
                  getWidth(),
                  getHeight(),
                  getColour(),
                  programInfo,
                  getLabel(),
                  isHighlighted());
    } else {
        paintUnused(g, getWidth(), getHeight(), programInfo);
    }
}

void SnapsButton::paintUsed(Graphics &g,
                            uint16_t width,
                            uint16_t height,
                            uint32_t colour,
                            const char *programInfo,
                            const char *label,
                            bool highlighted)
{
    if (highlighted) {
        colour = Colours565::lighter(colour, 0.1f);
    }

    g.setColour(colour);
    g.fillRoundRect(0, 0, width, height, 4);
    g.printText(0,
                (height / 2) - 2,
                label,
                TextStyle::mediumTransparent,
                width,
                TextAlign::center);
    printProgramInfo(g, programInfo, true);
}

void SnapsButton::paintUnused(Graphics &g,
                              uint16_t width,
                              uint16_t height,
                              const char *programInfo)
{
    g.setColour(Colours565::black);
    g.fillRect(0, 0, width, height);
    g.setColour(0x2104);
    g.drawRoundRect(0, 0, width, height, 4);

    printProgramInfo(g, programInfo, false);
}

void SnapsButton::printProgramInfo(Graphics &g,
                                   const char *programInfo,
                                   bool used)
{
    g.printText(
        5, 5, programInfo, TextStyle::smallTransparent, 40, TextAlign::left);
    if (!used) {
        g.dim(5, 5, 40, 12, Colours565::black);
    }
}