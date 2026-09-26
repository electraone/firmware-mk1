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

#include "LinkSelections.h"
#include "LinkNavigation.h"
#include "Colours.h"
#include <cstdio>

/*
 * LinkPageSelection
 */

LinkPageSelection::LinkPageSelection(LinkNavigation &newNavigation)
    : Selection(0, 0x00091A, 0x001130), navigation(newNavigation), pageId{}
{
    setName("PAGES");

    for (uint8_t i = 0; i < 12; i++) {
        if (button[i] != nullptr) {
            button[i]->onClick = [this, i]() {
                navigation.choosePage(pageId[i]);
            };
        }
    }
}

void LinkPageSelection::update(const std::vector<Link::ViewPage> &pages,
                               uint8_t shownPageId)
{
    for (uint8_t i = 0; i < 12; i++) {
        if (button[i] == nullptr) {
            continue;
        }

        if (i < pages.size()) {
            pageId[i] = pages[i].id;
            button[i]->setLabel(pages[i].name.c_str());
            button[i]->setDisabled(false);
            button[i]->setSelected(pages[i].id == shownPageId);
        } else {
            pageId[i] = 0;
            button[i]->setLabel(" ");
            button[i]->setDisabled(true);
            button[i]->setSelected(false);
        }
    }

    repaint();
}

uint8_t LinkPageSelection::pageAt(uint8_t index) const
{
    return ((index < 12) ? pageId[index] : 0);
}

/*
 * LinkPresetSelection
 */

LinkPresetSelection::LinkPresetSelection(LinkNavigation &newNavigation)
    : Selection(0, 0x2c0000, 0x4a0404),
      navigation(newNavigation),
      bankButton{},
      activeBank(0),
      presetId{},
      selectable{},
      known(),
      shown(0)
{
    setName("PRESETS");

    for (uint8_t i = 0; i < numBanks; i++) {
        char label[3];

        bankButton[i] = new BankButton();

        if (bankButton[i] == nullptr) {
            continue;
        }

        /* Offset, as the controller does, so a bank and a preset cannot
         * share an id. Named and coloured once the primary says how. */
        bankButton[i]->setId(i + 100);
        snprintf(label, sizeof(label), "%d", (int)(i + 1));
        bankButton[i]->setLabel(label);
        bankButton[i]->onClick = [this, i]() {
            setActiveBank(i);
        };
        addAndMakeVisible(bankButton[i]);
    }

    for (uint8_t i = 0; i < numSlots; i++) {
        presetId[i] = -1;

        if (button[i] != nullptr) {
            button[i]->onClick = [this, i]() {
                navigation.choosePreset(presetAt(i));
            };
        }
    }
}

void LinkPresetSelection::resized(void)
{
    const uint16_t segmentWidth = getWidth() / 6 - 2;
    const uint16_t segmentHeight = 70;

    for (uint8_t i = 0; i < numBanks; i++) {
        if (bankButton[i] != nullptr) {
            bankButton[i]->setBounds(
                (i % 6) * segmentWidth + 12, 0, segmentWidth - 10, segmentHeight);
        }
    }

    Selection::resized();
}

void LinkPresetSelection::setBanks(
    const std::vector<Link::PresetBankInfo> &banks)
{
    for (uint8_t i = 0; i < numBanks; i++) {
        if ((bankButton[i] == nullptr) || (i >= banks.size())) {
            continue;
        }

        bankButton[i]->setLabel(banks[i].name.c_str());

        /* The controller's own treatment of a bank's colour. */
        bankButton[i]->setColours(banks[i].colour,
                                  Colours888::darker(banks[i].colour, 0.3f));
    }
}

void LinkPresetSelection::setActiveBank(uint8_t newActiveBank)
{
    if (newActiveBank >= numBanks) {
        return;
    }

    activeBank = newActiveBank;
    update(known, shown);
}

uint8_t LinkPresetSelection::getActiveBank(void) const
{
    return (activeBank);
}

void LinkPresetSelection::update(const std::vector<Link::LivePreset> &presets,
                                 uint8_t shownPresetId)
{
    if (&presets != &known) {
        known = presets;
    }
    shown = shownPresetId;

    for (uint8_t i = 0; i < numSlots; i++) {
        const Link::LivePreset *found = nullptr;

        for (const Link::LivePreset &preset : known) {
            if ((preset.bankNumber == activeBank) && (preset.slot == i)) {
                found = &preset;
                break;
            }
        }

        presetId[i] = (found != nullptr) ? (int16_t)found->presetId : -1;
        selectable[i] =
            (found != nullptr) && (found->isLive() || found->isPinned());

        if (button[i] == nullptr) {
            continue;
        }

        button[i]->setLabel((found != nullptr) ? found->name.c_str() : " ");
        button[i]->setDisabled(!selectable[i]);
        button[i]->setSelected((found != nullptr)
                               && (found->presetId == shownPresetId));
    }

    for (uint8_t i = 0; i < numBanks; i++) {
        if (bankButton[i] != nullptr) {
            bankButton[i]->setSelected(i == activeBank);
        }
    }

    repaint();
}

int16_t LinkPresetSelection::presetAt(uint8_t index) const
{
    if ((index >= numSlots) || !selectable[index]) {
        return (-1);
    }

    return (presetId[index]);
}

/*
 * LinkPagesWindow
 */

LinkPagesWindow::LinkPagesWindow(LinkNavigation &newNavigation)
    : PopupWindow(new LinkPageSelection(newNavigation), colour, activeColour),
      navigation(newNavigation),
      selection(static_cast<LinkPageSelection *>(getOwnedContent()))
{
    setName("PAGES");
    setBounds(0, 333, 1024, 235);
}

void LinkPagesWindow::onButtonDown(uint8_t buttonId)
{
    navigation.pagesButtonDown(buttonId);
}

void LinkPagesWindow::onButtonUp(uint8_t buttonId)
{
    navigation.pagesButtonUp(buttonId);
}

LinkPageSelection &LinkPagesWindow::getSelection(void)
{
    return (*selection);
}

/*
 * LinkPresetsWindow
 */

LinkPresetsWindow::LinkPresetsWindow(LinkNavigation &newNavigation)
    : PopupWindow(new LinkPresetSelection(newNavigation), colour, activeColour),
      navigation(newNavigation),
      selection(static_cast<LinkPresetSelection *>(getOwnedContent()))
{
    setName("PRESETS");
    setBounds(0, 243, 1024, 325);
}

void LinkPresetsWindow::onButtonDown(uint8_t buttonId)
{
    navigation.presetsButtonDown(buttonId);
}

LinkPresetSelection &LinkPresetsWindow::getSelection(void)
{
    return (*selection);
}
