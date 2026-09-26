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
 * @file LinkSelections.h
 *
 * @brief The page and preset choosers, and the windows they open in.
 *
 * Built on the controller's own Selection, PopupWindow and BankButton at the
 * controller's own colours and geometry - so the two screens are the same
 * picture and stay so. What could not be reused is PageSelection and
 * PresetSelection themselves, which read the model and the configuration;
 * these hold the same components and fill them from what the primary sent.
 *
 * The idiom is the mk1 controller's, because that is the panel these buttons
 * are on: a chooser is a window over the bottom of the page, an item is
 * picked by touching the knob under it, and the right-hand buttons move
 * between pages, presets and the page itself.
 */

#pragma once

#include "Selection.h"
#include "PopupWindow.h"
#include "BankButton.h"
#include "src/Link/LinkView.h"
#include <vector>

class LinkNavigation;

/** @brief The pages of the preset being shown. */
class LinkPageSelection : public Selection
{
public:
    explicit LinkPageSelection(LinkNavigation &newNavigation);

    /** @brief Label the buttons from the pages the primary sent. */
    void update(const std::vector<Link::ViewPage> &pages, uint8_t shownPageId);

    /** @brief The page a button names, or zero when it names none. */
    uint8_t pageAt(uint8_t index) const;

private:
    LinkNavigation &navigation;
    uint8_t pageId[12];
};

/** @brief The presets the primary holds, by bank. */
class LinkPresetSelection : public Selection
{
public:
    explicit LinkPresetSelection(LinkNavigation &newNavigation);

    void resized(void) override;

    /** @brief Show a bank's presets. */
    void setActiveBank(uint8_t newActiveBank);
    uint8_t getActiveBank(void) const;

    /** @brief Name and colour the banks the way the owner did. */
    void setBanks(const std::vector<Link::PresetBankInfo> &banks);

    /**
     * @brief Label the buttons from the preset list the primary sent.
     *
     * A preset that is neither running nor pinned is drawn disabled: the
     * primary refuses to show one, and saying so on the button is kinder than
     * letting a person find out by pressing it.
     */
    void update(const std::vector<Link::LivePreset> &presets,
                uint8_t shownPresetId);

    /** @brief The preset a button names, or -1 when it cannot be shown. */
    int16_t presetAt(uint8_t index) const;

private:
    static constexpr uint8_t numBanks = 6;
    static constexpr uint8_t numSlots = 12;

    LinkNavigation &navigation;
    BankButton *bankButton[numBanks];
    uint8_t activeBank;
    int16_t presetId[numSlots];
    bool selectable[numSlots];

    /* Kept, so a bank change relabels without asking the link again. */
    std::vector<Link::LivePreset> known;
    uint8_t shown;
};

/** @brief The window the page chooser opens in, where the controller's does. */
class LinkPagesWindow : public PopupWindow
{
public:
    explicit LinkPagesWindow(LinkNavigation &newNavigation);
    ~LinkPagesWindow() override = default;

    void onButtonDown(uint8_t buttonId) override;
    void onButtonUp(uint8_t buttonId) override;

    LinkPageSelection &getSelection(void);

private:
    LinkNavigation &navigation;
    LinkPageSelection *selection;

    static constexpr uint32_t colour = 0x00091A;
    static constexpr uint32_t activeColour = 0x001130;
};

/** @brief The window the preset chooser opens in. */
class LinkPresetsWindow : public PopupWindow
{
public:
    explicit LinkPresetsWindow(LinkNavigation &newNavigation);
    ~LinkPresetsWindow() override = default;

    void onButtonDown(uint8_t buttonId) override;

    LinkPresetSelection &getSelection(void);

private:
    LinkNavigation &navigation;
    LinkPresetSelection *selection;

    static constexpr uint32_t colour = 0x2c0000;
    static constexpr uint32_t activeColour = 0x4a0404;
};
