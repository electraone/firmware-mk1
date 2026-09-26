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
 * @file PresetFunctions.h
 *
 * @brief What a preset can be asked to do, laid out under the knobs.
 */

#pragma once

#include "ActionButton.h"
#include "Selection.h"
#include "UiApi.h"
#include "luaHooks.h"

class CommandManager;

class PresetFunctions : public Selection
{
public:
    PresetFunctions(CommandManager &newCommandManager,
                    UiApi &newUiApi,
                    const char *newPresetName,
                    uint32_t newColour,
                    uint32_t newActiveColour);
    virtual ~PresetFunctions() = default;

    void paint(Graphics &g) override;
    void resized(void) override;

private:
    static constexpr size_t maxLabelLength = 14;

    /* The four things the instrument can do to a preset as a whole, in the row
     * above the functions. Ids are above the twelve the knobs take. */
    enum ActionId {
        sendCurrentId = 100,
        requestPatchId,
        defaultsId,
        restartId
    };

    ActionButton *addAction(uint16_t id,
                            const char *label,
                            uint32_t buttonColour,
                            uint32_t buttonActiveColour);
    void addActions(void);
    void labelFunctions(void);

    CommandManager &commandManager;
    UiApi &uiApi;
    const char *presetName;

    /* Copied rather than pointed at: the label of a button outlives the
     * fetch that produced it, and what produced it was a Lua string. */
    char label[numUserFunctions][maxLabelLength + 1];

    ActionButton *sendCurrentButton;
    ActionButton *requestPatchButton;
    ActionButton *defaultsButton;
    ActionButton *restartButton;
};
