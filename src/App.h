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

    /**
     * @brief A staged transaction has moved its files into place.
     *
     * The base firmware knows how to write the files and nothing about what
     * they mean, so what to do about it - refresh the preset list, load the
     * slot again if it is the one on screen - belongs to the application.
     *
     * @param currentSlotWritten whether one of them was the loaded slot's
     */
    virtual void handleStagedFilesApplied(uint8_t port,
                                          bool currentSlotWritten);
    virtual void runUserTask(void);

    /**
     * @brief A long operation on the main loop is giving the app a moment.
     *
     * A file streamed out over SysEx holds the main loop for seconds, and
     * an application that has to answer something in that time - a link
     * program answering its primary's keepalive - has nowhere else to run.
     * Called every so often from inside such loops; the base does nothing.
     * Whatever runs here must not draw and must be quick.
     */
    virtual void yield(void);

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
