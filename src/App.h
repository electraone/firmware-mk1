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
 * @file App.h
 *
 * @brief A base class for custom applications.
 */
#pragma once

#include "Hardware.h"
#include "System.h"
#include "LocalFile.h"
#include "MemoryPool.h"
#include "WindowManager.h"
#include "StatusBar.h"
#include "Spinner.h"
#include "luabridge.h"

#define CREATE_ELECTRA_APPLICATION(AppClass)                                   \
    void createApplication(void);                                              \
    void createApplication(void)                                               \
    {                                                                          \
        App::set(new AppClass());                                              \
    }

#define START_ELECTRA_APPLICATION(AppClass) CREATE_ELECTRA_APPLICATION(AppClass)

class App
{
protected:
    App();
    static App *appInstance;

public:
    static void set(App *newAppInstance);
    static App *get(void);

    App(App &other) = delete;
    void operator=(const App &) = delete;
    virtual ~App() = default;

    virtual void initialise(void);
    virtual const char *getApplicationName(void) const;
    virtual const char *getApplicationVersion(void) const;
    virtual const char *getApplicationSandbox(void) const;

    // \todo these will be moved elsewhere
    virtual void handleElectraSysex(uint8_t port, const SysexBlock &sysexBlock);
    virtual void execute(const char *filename);
    virtual bool handleCtrlFileReceived(uint8_t port,
                                        LocalFile file,
                                        ElectraCommand::Object fileType);
    virtual bool handleCtrlFileRemoved(uint8_t bankNumber,
                                       uint8_t slot,
                                       ElectraCommand::Object fileType);
    virtual void runUserTask(void);

    void clearScreen(void);
    void paintScreen(void);
    void indicateMidiActivity(uint8_t port,
                              Direction direction,
                              MidiMessage::Type msgType);
    virtual uint8_t getUsbHostPortAssigment(const char *productName);

    // These are kept public on purpose
public:
    bool enableMidi; // \todo enableMidi needs to be renamed and moved to System
    StatusBar statusBar;
    Spinner spinner;
    MemoryPool sysexPool;
};
