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
 * @file App.cpp
 *
 * @brief A base class for custom applications.
 */

#include "App.h"

App *App::appInstance = nullptr;

App *App::get(void)
{
    return appInstance;
}

void App::set(App *newAppInstance)
{
    appInstance = newAppInstance;
}

App::App()
    : sysexPool(&Hardware::screen, SDRAM_PAGE_0, SDRAM_PAGE_SIZE),
      enableMidi(false)
{
}

void App::initialise(void)
{
}

const char *App::getApplicationName(void) const
{
    return ("default");
}

const char *App::getApplicationVersion(void) const
{
    return ("1.0.0");
}

const char *App::getApplicationSandbox(void) const
{
    return ("default");
}

// --------------------------------------------------------------------------

#include <CircularBuffer.h>

// \todo To be removed.
void App::handleElectraSysex(uint8_t port, const SysexBlock &sysexBlock)
{
}

void App::execute(const char *filename)
{
}

bool App::handleCtrlFileReceived(uint8_t port,
                                 LocalFile file,
                                 ElectraCommand::Object fileType)
{
    return (true);
}

bool App::handleCtrlFileRemoved(uint8_t bankNumber,
                                uint8_t slot,
                                ElectraCommand::Object fileType)
{
    return (true);
}

void App::runUserTask(void)
{
}

void App::runScheduled(void)
{
}

void App::yield(void)
{
}

void App::indicateMidiActivity(uint8_t port,
                               Direction direction,
                               MidiMessage::Type msgType)
{
    statusBar.indicate(MidiInterface::Type::MidiIo, port, direction, msgType);
    statusBar.indicate(
        MidiInterface::Type::MidiUsbDev, port, direction, msgType);
    if (USBDevices[port].midiDevice != NULL) {
        statusBar.indicate(
            MidiInterface::Type::MidiUsbHost, port, direction, msgType);
    }
}

void App::handleStagedFilesApplied([[maybe_unused]] uint8_t port,
                                   [[maybe_unused]] bool currentSlotWritten)
{
}

uint8_t App::getUsbHostPortAssigment(const char *productName)
{
    return (0);
}

void App::clearScreen(void)
{
    //flushRepaintGraphics();
    //Hardware::screen.clear();
    statusBar.forceRepaint();
}

void App::paintScreen(void)
{
    System::windowManager.repaintActive();
}
