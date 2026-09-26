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
 * @file LinkNavigation.h
 *
 * @brief Moving around from this unit's own panel.
 *
 * The mk1 controller's idiom, mirrored rather than invented, because a
 * person holding one should not have to learn a second one for the same six
 * buttons:
 *
 *   right bottom   pages - and, from a chooser, back to the page
 *   right middle   presets
 *   left column    which row of the page the knobs drive
 *
 * A chooser is a window over the bottom of the page and an item is picked by
 * touching the knob under it. Holding the pages button while picking keeps
 * the chooser up, as the controller does, so several pages can be looked at
 * in one go.
 *
 * Only what concerns pages and presets is here. Snapshots, patch requests,
 * the USB host and sleep are the controller's, and a unit that is a second
 * screen has none of them.
 *
 * Nothing here decides anything. Choosing a page or a preset sends a
 * request and the screen changes when, and only when, the primary sends a
 * view back.
 */

#pragma once

#include "src/Link/LinkClient.h"
#include <cstdint>

class LinkPagesWindow;
class LinkPresetsWindow;

class LinkNavigation
{
public:
    explicit LinkNavigation(LinkClient &newLink);

    /** @brief A button went down on the page. True when it was ours. */
    bool handleButtonDown(uint8_t buttonId);

    /** @brief And came up. */
    void handleButtonUp(uint8_t buttonId);

    /** @brief Bring whichever chooser is open up to date with the primary. */
    void refresh(void);

    /** @brief Put every chooser away. */
    void closeAll(void);

    /* From the choosers and their windows. */
    void choosePage(uint8_t pageId);
    void choosePreset(int16_t presetId);
    void pagesButtonDown(uint8_t buttonId);
    void pagesButtonUp(uint8_t buttonId);
    void presetsButtonDown(uint8_t buttonId);

private:
    void openPages(void);
    void closePages(void);
    void openPresets(void);
    void closePresets(void);

    /** @brief Ask for a row of the page, from one of the left buttons. */
    void requestControlSet(uint8_t buttonId);

    LinkClient &link;
    LinkPagesWindow *pages;
    LinkPresetsWindow *presets;

    /** Whether a page was picked while the pages button was held. */
    bool pagesBrowsed;
};
