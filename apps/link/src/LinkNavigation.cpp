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

#include "LinkNavigation.h"
#include "LinkSelections.h"
#include "Hardware.h"
#include "Buttons.h"
#include "Window.h"

LinkNavigation::LinkNavigation(LinkClient &newLink)
    : link(newLink), pages(nullptr), presets(nullptr), pagesBrowsed(false)
{
}

bool LinkNavigation::handleButtonDown(uint8_t buttonId)
{
    switch (buttonId) {
        case BUTTON_LEFT_TOP:
        case BUTTON_LEFT_MIDDLE:
        case BUTTON_LEFT_BOTTOM:
            requestControlSet(buttonId);
            return (true);

        case BUTTON_RIGHT_MIDDLE:
            openPresets();
            return (true);

        case BUTTON_RIGHT_BOTTOM:
            openPages();
            return (true);

        default:
            /* The patch request button. Not a page or a preset, so not a
             * thing this panel does. */
            return (false);
    }
}

void LinkNavigation::handleButtonUp([[maybe_unused]] uint8_t buttonId)
{
}

void LinkNavigation::requestControlSet(uint8_t buttonId)
{
    if (!link.isConnected()) {
        return;
    }

    /* The left buttons are the rows, top to bottom, as on the controller;
     * the protocol counts sets from one. */
    link.requestControlSet((uint8_t)(buttonId - BUTTON_LEFT_TOP + 1));
}

void LinkNavigation::openPages(void)
{
    if (pages != nullptr) {
        return;
    }

    closePresets();
    pagesBrowsed = false;
    pages = new LinkPagesWindow(*this);
    refresh();
}

void LinkNavigation::closePages(void)
{
    if (pages == nullptr) {
        return;
    }

    LinkPagesWindow *window = pages;

    pages = nullptr;
    Window::close(window);
}

void LinkNavigation::openPresets(void)
{
    if (presets != nullptr) {
        return;
    }

    closePages();
    presets = new LinkPresetsWindow(*this);

    /* Open on the bank of the preset being shown, as the controller does. */
    const uint8_t shownPresetId = link.getView().getPresetId();

    presets->getSelection().setActiveBank((uint8_t)(shownPresetId / 12));
    refresh();
}

void LinkNavigation::closePresets(void)
{
    if (presets == nullptr) {
        return;
    }

    LinkPresetsWindow *window = presets;

    presets = nullptr;
    Window::close(window);
}

void LinkNavigation::closeAll(void)
{
    closePages();
    closePresets();
}

void LinkNavigation::refresh(void)
{
    if (pages != nullptr) {
        pages->getSelection().update(link.getView().getPages(),
                                     link.getView().getPageNumber());
    }

    if (presets != nullptr) {
        presets->getSelection().setBanks(link.getBanks());
        presets->getSelection().update(link.getPresets(),
                                       link.getView().getPresetId());
    }
}

void LinkNavigation::choosePage(uint8_t pageId)
{
    /* A button that names nothing is not a choice and leaves the chooser
     * where it is. */
    if (pageId == 0) {
        return;
    }

    link.requestPage(pageId);

    /* Choosing closes the chooser, as it does on the controller - unless the
     * pages button is being held, which is how several pages are looked at in
     * one go. The release closes it then. */
    if (!Hardware::buttons[BUTTON_RIGHT_BOTTOM]->isPressed()) {
        closePages();
    } else {
        pagesBrowsed = true;

        if (pages != nullptr) {
            pages->getSelection().update(link.getView().getPages(), pageId);
        }
    }
}

void LinkNavigation::choosePreset(int16_t presetId)
{
    /* -1 for a slot holding nothing, and for one that is neither running
     * nor pinned: the primary would refuse it, and the button already
     * says so. */
    if (presetId < 0) {
        return;
    }

    link.requestPreset((uint8_t)presetId);
    closePresets();
}

void LinkNavigation::pagesButtonDown(uint8_t buttonId)
{
    switch (buttonId) {
        case BUTTON_LEFT_TOP:
        case BUTTON_LEFT_MIDDLE:
        case BUTTON_LEFT_BOTTOM:
            requestControlSet(buttonId);
            break;

        case BUTTON_RIGHT_MIDDLE:
            openPresets();
            break;

        case BUTTON_RIGHT_BOTTOM:
            closePages();
            break;

        default:
            break;
    }
}

void LinkNavigation::pagesButtonUp(uint8_t buttonId)
{
    if ((buttonId == BUTTON_RIGHT_BOTTOM) && pagesBrowsed) {
        closePages();
    }
}

void LinkNavigation::presetsButtonDown(uint8_t buttonId)
{
    switch (buttonId) {
        case BUTTON_LEFT_TOP:
        case BUTTON_LEFT_MIDDLE:
        case BUTTON_LEFT_BOTTOM:
            requestControlSet(buttonId);
            break;

        case BUTTON_RIGHT_MIDDLE:
            closePresets();
            break;

        case BUTTON_RIGHT_BOTTOM:
            openPages();
            break;

        default:
            break;
    }
}
