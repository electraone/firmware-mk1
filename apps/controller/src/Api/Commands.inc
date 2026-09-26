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
 * @file Commands.inc
 *
 * @brief The one place a user interface command is declared.
 *
 * A command is one thing the instrument can be told to do, named so that
 * something outside the firmware can ask for it: a hardware button through the
 * configuration, an incoming MIDI message through the same, a host over SysEx,
 * or a script. Written the usual way, each of those surfaces would carry its
 * own list of names and its own switch, and the lists would drift.
 *
 * There is one row per command here and the tables are generated from it, so
 * they cannot disagree. Adding a command means adding a row and writing its
 * handler body in CommandManager.cpp - nothing else.
 *
 *   id       UiCommand::Type enumerator. Order defines the numeric values, and
 *            nothing persists them - every surface names a command in text - so
 *            the order is only a matter of reading.
 *   wire     the name every surface uses.
 *   display  a human readable label, or nullptr to fall back to the wire name.
 *   param    what the command's parameter means, or nullptr when it takes none.
 *   handler  the CommandManager member function that runs it.
 *
 * UiCommand::Type::None is deliberately not listed: it is the zero value, the
 * one an unknown name translates to, and it has no handler and no name.
 *
 * This file is included several times with different E1_COMMAND definitions
 * and has no include guard on purpose.
 */

/* clang-format off */

/* Windows */
E1_COMMAND(OpenPresetsMenu,      "openPresetsMenu",      "Open presets",            nullptr,             openPresetsMenu)
E1_COMMAND(OpenPagesMenu,        "openPagesMenu",        "Open pages",              nullptr,             openPagesMenu)
E1_COMMAND(OpenUsbHost,          "openUsbHost",          "Open USB host",           nullptr,             openUsbHost)
E1_COMMAND(OpenSnapshots,        "openSnapshots",        "Open snapshots",          nullptr,             openSnapshots)
E1_COMMAND(OpenPresetMenu,       "openPresetMenu",       "Open preset functions",   nullptr,             openPresetMenu)
E1_COMMAND(ClosePopupWindow,     "closePopupWindow",     "Close window",            nullptr,             closePopupWindow)
E1_COMMAND(OpenDetail,           "openDetail",           "Open detail",             "control id",        openDetail)
E1_COMMAND(CloseDetail,          "closeDetail",          "Close detail",            nullptr,             closeDetail)
E1_COMMAND(LockDetail,           "lockDetail",           "Lock detail",             nullptr,             lockDetail)

/* Moving around the preset */
E1_COMMAND(SwitchPreset,         "switchPreset",         "Switch to preset",        "preset, 1 to 72",   switchPreset)
E1_COMMAND(SwitchPresetNext,     "switchPresetNext",     "Next preset",             nullptr,             switchPresetNext)
E1_COMMAND(SwitchPresetPrev,     "switchPresetPrev",     "Previous preset",         nullptr,             switchPresetPrev)
E1_COMMAND(SwitchPresetBank,     "switchPresetBank",     "Switch preset bank",      "bank, 1 to 6",      switchPresetBank)
E1_COMMAND(SwitchPage,           "switchPage",           "Switch to page",          "page, 1 to 12",     switchPage)
E1_COMMAND(SwitchPageNext,       "switchPageNext",       "Next page",               nullptr,             switchPageNext)
E1_COMMAND(SwitchPagePrev,       "switchPagePrev",       "Previous page",           nullptr,             switchPagePrev)
E1_COMMAND(SwitchControlSet,     "switchControlSet",     "Set control set",         "set, 1 to 3",       switchControlSet)
E1_COMMAND(SwitchControlSet1,    "switchControlSet1",    "Control set 1",           nullptr,             switchControlSet1)
E1_COMMAND(SwitchControlSet2,    "switchControlSet2",    "Control set 2",           nullptr,             switchControlSet2)
E1_COMMAND(SwitchControlSet3,    "switchControlSet3",    "Control set 3",           nullptr,             switchControlSet3)
E1_COMMAND(SwitchControlSetNext, "switchControlSetNext", "Next control set",        nullptr,             switchControlSetNext)
E1_COMMAND(SwitchControlSetPrev, "switchControlSetPrev", "Previous control set",    nullptr,             switchControlSetPrev)
E1_COMMAND(SwitchHandleNext,     "switchHandleNext",     "Next handle",             nullptr,             switchHandleNext)
E1_COMMAND(SwitchHandlePrev,     "switchHandlePrev",     "Previous handle",         nullptr,             switchHandlePrev)

/* The preset as a whole */
E1_COMMAND(ReloadPreset,         "reloadPreset",         "Restart preset",          nullptr,             reloadPreset)
E1_COMMAND(RequestPatch,         "requestPatch",         "Request patch",           nullptr,             requestPatch)
E1_COMMAND(SendCurrent,          "sendCurrent",          "Send all values",         nullptr,             sendCurrent)
E1_COMMAND(SetAllDefaultValues,  "setAllDefaultValues",  "Set all default values",  nullptr,             setAllDefaultValues)
E1_COMMAND(RunUserFunction,      "runUserFunction",      "Run a preset function",   "function, 1 to 12", runUserFunction)

/* Snapshots */
E1_COMMAND(SwitchSnapshotBank,   "switchSnapshotBank",   "Switch snapshot bank",    "bank, 1 to 12",     switchSnapshotBank)
E1_COMMAND(LoadSnapshot,         "loadSnapshot",         "Load snapshot",           "snapshot, 1 to 36", loadSnapshot)

/* The instrument */
E1_COMMAND(EnableMidiLearn,      "enableMidiLearn",      "MIDI learn on",           nullptr,             enableMidiLearn)
E1_COMMAND(DisableMidiLearn,     "disableMidiLearn",     "MIDI learn off",          nullptr,             disableMidiLearn)
E1_COMMAND(Sleep,                "sleep",                "Sleep",                   nullptr,             sleep)
E1_COMMAND(Reboot,               "reboot",               "Reboot",                  nullptr,             reboot)

/* clang-format on */
