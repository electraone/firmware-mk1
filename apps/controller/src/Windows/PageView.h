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
 * @file PageViews.h
 *
 * @brief Implements a component to display a Preset page.
 */

#pragma once

#include "ControlComponent.h"
#include "GroupControl.h"
#include "BottomBar.h"
#include "Preset.h"
#include "MainDelegate.h"
#include "UiApi.h"
#include "UiFeatures.h"

class PageView : public Component
{
public:
    PageView(Preset *preset,
             MainDelegate &newDelegate,
             UiApi &uiApi,
             const UiFeatures &newUiFeatures,
             uint8_t newPageId,
             uint8_t activeControlSetId = 0);
    virtual ~PageView();

    void setControlSet(uint8_t newControlSetId);
    void onTouchDown(const TouchEvent &touchEvent) override;
    void onPotTouchDown(const PotEvent &potEvent) override;
    void onPotTouchUp(const PotEvent &potEvent) override;
    void reassignComponent(const Control &control);
    void changePageName(const char *newName);
    void setInfoText(const char *newText);
    void setRamPercentage(uint8_t newPercentage);
    void addControl(const Control &control);
    void moveControl(const Control &control);
    void removeControl(const Control &control);
    void paint(Graphics &g) override;
    void resized(void) override;

private:
    void addControls(const Controls &controls);
    void addGroups(const Groups &groups);
    void addBottomBar(const char *presetName, const char *pageName);
    void configureGroup(GroupControl *g, const Group &group);
    void configureControl(ControlComponent *cc, const Control &control);
    void resetUsedPots(void);

    Preset *model;
    MainDelegate &delegate;
    UiApi &uiApi;
    const UiFeatures &uiFeatures;
    uint8_t pageId;
    uint8_t controlSetId;
    BottomBar *bottomBar;
    bool usedPots[NR_OF_HW_POTS];
};
