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
 * @file PresetFunctionsWindow.h
 *
 * @brief The window that offers what a preset can be asked to do.
 */

#pragma once

#include "PopupWindow.h"
#include "PresetFunctions.h"
#include "UiApi.h"

class PresetFunctionsWindow : public PopupWindow
{
private:
    PresetFunctionsWindow(CommandManager &newCommandManager,
                          UiApi &newUiApi,
                          const char *presetName)
        : PopupWindow(new PresetFunctions(newCommandManager,
                                          newUiApi,
                                          presetName,
                                          colour,
                                          activeColour),
                      colour,
                      activeColour),
          uiApi(newUiApi)
    {
        setName("FUNCTIONS");

        /* Taller than the page and preset menus: the twelve sit where those
         * put them, under the knobs, and the row of actions goes above. */
        setBounds(0, 250, 1024, 318);
    }

public:
    virtual ~PresetFunctionsWindow() = default;

    void onButtonDown(uint8_t buttonId) override
    {
        if (buttonId == 5) {
            uiApi.presetFunctions_close();
        }
    }

    static PresetFunctionsWindow *
        createPresetFunctionsWindow(CommandManager &newCommandManager,
                                    UiApi &newUiApi,
                                    const char *presetName)
    {
        return (
            new PresetFunctionsWindow(newCommandManager, newUiApi, presetName));
    }

private:
    UiApi &uiApi;

    static constexpr uint32_t colour = 0x1A0A00;
    static constexpr uint32_t activeColour = 0x301300;
};
