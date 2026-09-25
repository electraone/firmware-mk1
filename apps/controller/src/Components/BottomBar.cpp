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

#include "BottomBar.h"

BottomBar::BottomBar(const char *newPresetName, const char *newPageName)
    : ramIndicator(nullptr)
{
    setName("bottomBar");
    setPresetName(newPresetName);
    setPageName(newPageName);
    setInfoText("");

    ramIndicator = new MeterHorizontal();

    if (ramIndicator) {
        ramIndicator->setColour(0xAD55);
        ramIndicator->setColourInactive(Colours565::black);
        ramIndicator->setColourBackground(0x2104);
        ramIndicator->setMinimum(0.0f);
        ramIndicator->setMaximum(100.0f);
        ramIndicator->setValue(0);
        addAndMakeVisible(ramIndicator);
    }
}

void BottomBar::paint(Graphics &g)
{
    g.setColour(0x2104);
    g.fillRoundRect(0, 0, getWidth(), getHeight(), 10);
    g.printText(
        0, 4, presetName, TextStyle::mediumTransparent, 500, TextAlign::right);
    g.printText(
        528, 4, pageName, TextStyle::mediumTransparent, 400, TextAlign::left);
    g.printText(
        800, 4, infoText, TextStyle::mediumTransparent, 180, TextAlign::right);

    g.setColour(Colours565::white);
    g.fillRect(512, 9, 2, 2);
}

void BottomBar::resized(void)
{
    ramIndicator->setBounds(20, 8, 50, 8);
}

void BottomBar::setPresetName(const char *newName)
{
    copyString(presetName, newName, maxPresetNameLength);
}

void BottomBar::setPageName(const char *newName)
{
    copyString(pageName, newName, maxPageNameLength);
}

void BottomBar::setInfoText(const char *newText)
{
    copyString(infoText, newText, maxPageNameLength);
}

void BottomBar::setRamPercentage(uint8_t newPercentage)
{
    ramIndicator->setValue(newPercentage);
}

void BottomBar::update(const char *newPresetName, const char *newPageName)
{
    setPresetName(newPresetName);
    setPageName(newPageName);
    repaint();
}
