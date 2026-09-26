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
 * @file CommandManager.cpp
 *
 * @brief Runs the things the instrument can be told to do.
 */

#include "CommandManager.h"

#include "System.h"
#include "luaHooks.h"

namespace
{
    constexpr uint8_t numPresetSlots = 72;
    constexpr uint8_t numPresetBanks = 6;
    constexpr uint8_t numSlotsPerBank = 12;
    constexpr uint8_t numControlSets = 3;
    constexpr uint8_t numSnapshotBanks = 12;
    constexpr uint8_t numSnapshotsPerBank = 36;

    /** Whether a parameter counted from one falls inside a range. */
    bool inRange(int16_t parameter, int16_t last)
    {
        return ((parameter >= 1) && (parameter <= last));
    }
} // namespace

/* Indexed by UiCommand::Type, so dispatch is a subscript. The row order in
 * Commands.inc defines both, which is what keeps them from drifting apart. */
const CommandManager::CommandHandler
    CommandManager::handlers[UiCommand::count] = {
        nullptr, /* UiCommand::Type::None */

#define E1_COMMAND(id, wire, disp, param, fn) &CommandManager::fn,
#include "Commands.inc"
#undef E1_COMMAND
    };

CommandManager::CommandManager(MainDelegate &newDelegate)
    : delegate(newDelegate), uiApi(newDelegate)
{
}

bool CommandManager::run(UiCommand::Type command, int16_t parameter)
{
    const size_t index = static_cast<size_t>(command);

    if ((index == 0) || (index >= UiCommand::count)
        || (handlers[index] == nullptr)) {
        System::logger.write(
            LOG_ERROR, "CommandManager::run: no such command: %d", (int)index);
        return (false);
    }

    System::logger.write(LOG_INFO,
                         "CommandManager::run: %s, parameter=%d",
                         UiCommand::translateToText(command),
                         (int)parameter);

    if (!(this->*handlers[index])(parameter)) {
        System::logger.write(LOG_ERROR,
                             "CommandManager::run: %s refused, parameter=%d",
                             UiCommand::translateToText(command),
                             (int)parameter);
        return (false);
    }

    return (true);
}

bool CommandManager::run(const char *name, int16_t parameter)
{
    const UiCommand::Type command = UiCommand::translate(name);

    if (command == UiCommand::Type::None) {
        System::logger.write(LOG_ERROR,
                             "CommandManager::run: no such command: %s",
                             name ? name : "(none)");
        return (false);
    }

    return (run(command, parameter));
}

/*--------------------------------------------------------------------------*/
/* Windows                                                                   */
/*--------------------------------------------------------------------------*/

bool CommandManager::openPresetsMenu([[maybe_unused]] int16_t parameter)
{
    delegate.openPresetSelection();
    return (true);
}

bool CommandManager::openPagesMenu([[maybe_unused]] int16_t parameter)
{
    delegate.openPageSelection();
    return (true);
}

bool CommandManager::openUsbHost([[maybe_unused]] int16_t parameter)
{
    delegate.openUsbHostPorts();
    return (true);
}

bool CommandManager::openSnapshots([[maybe_unused]] int16_t parameter)
{
    delegate.openSnapshots();
    return (true);
}

bool CommandManager::openPresetMenu([[maybe_unused]] int16_t parameter)
{
    delegate.openPresetFunctions();
    return (true);
}

bool CommandManager::closePopupWindow([[maybe_unused]] int16_t parameter)
{
    delegate.closeAllWindows();
    return (true);
}

/*
 * Without a parameter this is the control under the knob a finger is on, which
 * is what a button on the panel means by it; with one it is a control named by
 * its id, which is what a host means.
 */
bool CommandManager::openDetail(int16_t parameter)
{
    if (parameter <= 0) {
        delegate.openDetailOfActivePotTouch();
    } else {
        delegate.openDetail((uint16_t)parameter);
    }

    return (true);
}

bool CommandManager::closeDetail([[maybe_unused]] int16_t parameter)
{
    delegate.closeDetail();
    return (true);
}

bool CommandManager::lockDetail([[maybe_unused]] int16_t parameter)
{
    delegate.lockDetail();
    return (true);
}

/*--------------------------------------------------------------------------*/
/* Moving around the preset                                                  */
/*--------------------------------------------------------------------------*/

/* One to seventy-two, counted across the banks, which is how the configuration
 * has always named a preset. */
bool CommandManager::switchPreset(int16_t parameter)
{
    if (!inRange(parameter, numPresetSlots)) {
        return (false);
    }

    const uint8_t index = (uint8_t)(parameter - 1);

    uiApi.switchPreset(index / numSlotsPerBank, index % numSlotsPerBank);
    return (true);
}

bool CommandManager::switchPresetNext([[maybe_unused]] int16_t parameter)
{
    delegate.switchPresetNext();
    return (true);
}

bool CommandManager::switchPresetPrev([[maybe_unused]] int16_t parameter)
{
    delegate.switchPresetPrev();
    return (true);
}

bool CommandManager::switchPresetBank(int16_t parameter)
{
    if (!inRange(parameter, numPresetBanks)) {
        return (false);
    }

    uiApi.switchPresetBank((uint8_t)(parameter - 1));
    return (true);
}

bool CommandManager::switchPage(int16_t parameter)
{
    if (!inRange(parameter, NR_OF_PAGES)) {
        return (false);
    }

    uiApi.switchPage((uint8_t)parameter);
    return (true);
}

bool CommandManager::switchPageNext([[maybe_unused]] int16_t parameter)
{
    delegate.switchPageNext();
    return (true);
}

bool CommandManager::switchPagePrev([[maybe_unused]] int16_t parameter)
{
    delegate.switchPagePrev();
    return (true);
}

/* Counted from one here, as every other parameter is, and from zero inside -
 * where it is an index into the page's sets rather than a name. */
bool CommandManager::switchControlSet(int16_t parameter)
{
    if (!inRange(parameter, numControlSets)) {
        return (false);
    }

    uiApi.switchControlSet((uint8_t)(parameter - 1));
    return (true);
}

bool CommandManager::switchControlSet1([[maybe_unused]] int16_t parameter)
{
    uiApi.switchControlSet(0);
    return (true);
}

bool CommandManager::switchControlSet2([[maybe_unused]] int16_t parameter)
{
    uiApi.switchControlSet(1);
    return (true);
}

bool CommandManager::switchControlSet3([[maybe_unused]] int16_t parameter)
{
    uiApi.switchControlSet(2);
    return (true);
}

/* Wrapping, because a button that stops at the last set is a button that stops
 * working. */
bool CommandManager::switchControlSetNext([[maybe_unused]] int16_t parameter)
{
    const uint8_t current = delegate.getCurrentControlSetId();

    uiApi.switchControlSet((uint8_t)((current + 1) % numControlSets));
    return (true);
}

bool CommandManager::switchControlSetPrev([[maybe_unused]] int16_t parameter)
{
    const uint8_t current = delegate.getCurrentControlSetId();

    uiApi.switchControlSet(
        (uint8_t)((current + numControlSets - 1) % numControlSets));
    return (true);
}

bool CommandManager::switchHandleNext([[maybe_unused]] int16_t parameter)
{
    delegate.switchHandleNext();
    return (true);
}

bool CommandManager::switchHandlePrev([[maybe_unused]] int16_t parameter)
{
    delegate.switchHandlePrev();
    return (true);
}

/*--------------------------------------------------------------------------*/
/* The preset as a whole                                                     */
/*--------------------------------------------------------------------------*/

bool CommandManager::reloadPreset([[maybe_unused]] int16_t parameter)
{
    delegate.reloadPreset();
    return (true);
}

bool CommandManager::requestPatch([[maybe_unused]] int16_t parameter)
{
    delegate.requestAllPatches();
    return (true);
}

bool CommandManager::sendCurrent([[maybe_unused]] int16_t parameter)
{
    delegate.sendAllControls();
    return (true);
}

bool CommandManager::setAllDefaultValues([[maybe_unused]] int16_t parameter)
{
    delegate.setAllDefaultValues();
    return (true);
}

/*
 * One of the twelve entries of preset.userFunctions. The entry says whether
 * running it should put the window away, which is what makes a function that
 * switches page usable from the menu that offered it.
 */
bool CommandManager::runUserFunction(int16_t parameter)
{
    if (!inRange(parameter, numUserFunctions)) {
        return (false);
    }

    if (::runUserFunction((uint8_t)(parameter - 1))) {
        delegate.closePresetFunctions();
    }

    return (true);
}

/*--------------------------------------------------------------------------*/
/* Snapshots                                                                 */
/*--------------------------------------------------------------------------*/

bool CommandManager::switchSnapshotBank(int16_t parameter)
{
    if (!inRange(parameter, numSnapshotBanks)) {
        return (false);
    }

    uiApi.switchSnapshotBank((uint8_t)(parameter - 1));
    return (true);
}

/* Of the bank the instrument is on, because a command carries one number and
 * the bank is already a thing that can be switched. */
bool CommandManager::loadSnapshot(int16_t parameter)
{
    if (!inRange(parameter, numSnapshotsPerBank)) {
        return (false);
    }

    uiApi.loadSnapshot(delegate.getCurrentProjectId(),
                       delegate.getCurrentSnapshotBank(),
                       (uint8_t)(parameter - 1));
    return (true);
}

/*--------------------------------------------------------------------------*/
/* The instrument                                                            */
/*--------------------------------------------------------------------------*/

bool CommandManager::enableMidiLearn([[maybe_unused]] int16_t parameter)
{
    delegate.enableMidiLearn();
    return (true);
}

bool CommandManager::disableMidiLearn([[maybe_unused]] int16_t parameter)
{
    delegate.disableMidiLearn();
    return (true);
}

bool CommandManager::sleep([[maybe_unused]] int16_t parameter)
{
    delegate.switchOff();
    return (true);
}

bool CommandManager::reboot([[maybe_unused]] int16_t parameter)
{
    delegate.reboot();
    return (true);
}
