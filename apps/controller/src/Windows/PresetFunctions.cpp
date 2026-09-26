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
 * @file PresetFunctions.cpp
 *
 * @brief What a preset can be asked to do, laid out under the knobs.
 */

#include "PresetFunctions.h"

#include "CommandManager.h"
#include "System.h"
#include "Window.h"

PresetFunctions::PresetFunctions(CommandManager &newCommandManager,
                                 UiApi &newUiApi,
                                 const char *newPresetName,
                                 uint32_t newColour,
                                 uint32_t newActiveColour)
    /* No entry starts out chosen: these are things to do, not a state to show,
     * so an id no button has keeps every one of them unlit. */
    : Selection(0xff, newColour, newActiveColour),
      commandManager(newCommandManager),
      uiApi(newUiApi),
      presetName(newPresetName),
      sendCurrentButton(nullptr),
      requestPatchButton(nullptr),
      defaultsButton(nullptr),
      restartButton(nullptr)
{
    setName("PresetFunctions");

    labelFunctions();
    addActions();

    for (uint8_t i = 0; i < numUserFunctions; i++) {
        if (!button[i]) {
            continue;
        }

        if (label[i][0] == '\0') {
            /* A knob with nothing behind it keeps its blank face and stops
             * listening, so turning it does not light an empty button. */
            button[i]->releasePot();
            continue;
        }

        button[i]->setLabel(label[i]);
        button[i]->onClick = [this, i]() {
            commandManager.run(UiCommand::Type::RunUserFunction,
                               (int16_t)(i + 1));
        };
    }
}

/*
 * The names the script wrote, read once when the window opens rather than on
 * every repaint: a script is free to rewrite the table, and a paint that ran
 * Lua would run it from the repaint task.
 */
void PresetFunctions::labelFunctions(void)
{
    for (uint8_t i = 0; i < numUserFunctions; i++) {
        getUserFunctionName(i, label[i], sizeof(label[i]));
    }
}

ActionButton *PresetFunctions::addAction(uint16_t id,
                                         const char *actionLabel,
                                         uint32_t buttonColour,
                                         uint32_t buttonActiveColour)
{
    ActionButton *action = new ActionButton();

    if (action) {
        action->setId(id);
        action->setName(actionLabel);
        action->setLabel(actionLabel);
        action->setColours(buttonColour, buttonActiveColour);
        addAndMakeVisible(action);
    }

    return (action);
}

/*
 * The four that apply to any preset, script or no script, so that the window
 * is worth opening on a preset that declares no functions of its own. Each is
 * run through the command manager rather than through the delegate, so that
 * what the screen does and what a button or a host asks for are the same
 * thing.
 */
void PresetFunctions::addActions(void)
{
    sendCurrentButton = addAction(sendCurrentId, "SEND VALUES", 0x162800, 0x1d3300);

    if (sendCurrentButton) {
        sendCurrentButton->onClick = [this]() {
            commandManager.run(UiCommand::Type::SendCurrent);
            uiApi.presetFunctions_close();
        };
    }

    requestPatchButton = addAction(requestPatchId, "REQUEST", 0x07021a, 0x00053c);

    if (requestPatchButton) {
        requestPatchButton->onClick = [this]() {
            commandManager.run(UiCommand::Type::RequestPatch);
            uiApi.presetFunctions_close();
        };
    }

    defaultsButton = addAction(defaultsId, "DEFAULTS", 0x270800, 0x300a00);

    if (defaultsButton) {
        defaultsButton->onClick = [this]() {
            commandManager.run(UiCommand::Type::SetAllDefaultValues);
        };
    }

    restartButton = addAction(restartId, "RESTART", 0x270800, 0x300a00);

    if (restartButton) {
        restartButton->onClick = [this]() {
            /* The window belongs to the preset that is about to be thrown
             * away, so it goes first. */
            uiApi.presetFunctions_close();
            commandManager.run(UiCommand::Type::ReloadPreset);
        };
    }
}

void PresetFunctions::paint(Graphics &g)
{
    g.fillAll(Colours565::black);

    if (presetName && (presetName[0] != '\0')) {
        g.system().printText(12,
                             8,
                             presetName,
                             TextStyle::mediumTransparent,
                             getWidth() - 24,
                             TextAlign::left);
    }
}

void PresetFunctions::resized(void)
{
    /* The twelve are placed where Selection puts them, under the knobs they
     * belong to, and the four actions take the row above. */
    Selection::resized();

    constexpr uint16_t actionWidth = 154;
    constexpr uint16_t actionHeight = 40;
    constexpr uint16_t actionY = 36;

    ActionButton *actions[] = { sendCurrentButton,
                                requestPatchButton,
                                defaultsButton,
                                restartButton };

    for (uint8_t i = 0; i < 4; i++) {
        if (actions[i]) {
            actions[i]->setBounds(
                12 + (i * (actionWidth + 15)), actionY, actionWidth, actionHeight);
        }
    }
}
