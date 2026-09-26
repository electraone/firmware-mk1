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
 * @file CommandManager.h
 *
 * @brief Runs the things the instrument can be told to do.
 *
 * One entry point for every surface that can ask for something: a hardware
 * button through the configuration, an incoming MIDI message through the same,
 * a host over SysEx, and a script. Each of them used to reach into the delegate
 * on its own and know a different subset of what could be asked for; now they
 * all name a command and this decides what that means.
 *
 * Unlike the newer hardware there is no queue in front of it. Every caller here
 * is a task of one cooperative scheduler, so a command runs on the thread that
 * asked for it and is finished before the next task gets a turn - which is what
 * the MIDI path already did before any of this existed. A queue would buy
 * nothing and would need memory to hold.
 */

#pragma once

#include "MainDelegate.h"
#include "UiApi.h"
#include "UiCommand.h"

class CommandManager
{
public:
    explicit CommandManager(MainDelegate &newDelegate);
    virtual ~CommandManager() = default;

    /**
     * @brief Runs a command.
     *
     * @param command what to do
     * @param parameter what it needs, when it needs one; see
     *        UiCommand::translateToParameterName()
     *
     * @return false when the command is not one this instrument has, or its
     *         parameter is outside what the command accepts. Nothing happens
     *         in either case.
     */
    bool run(UiCommand::Type command, int16_t parameter = 0);

    /** @brief The same, by name. */
    bool run(const char *name, int16_t parameter = 0);

private:
    using CommandHandler = bool (CommandManager::*)(int16_t);

    /* One handler declaration per row in Commands.inc. The bodies are in
     * CommandManager.cpp. */
#define E1_COMMAND(id, wire, disp, param, fn) bool fn(int16_t parameter);
#include "Commands.inc"
#undef E1_COMMAND

    static const CommandHandler handlers[UiCommand::count];

    MainDelegate &delegate;

    /* Its own rather than the main window's, because a command is a user
     * action and has to be reported to the host the way the screen's own
     * actions are. UiApi holds nothing but this same reference, so a second
     * one costs a pointer. */
    UiApi uiApi;
};
