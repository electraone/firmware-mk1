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
 * @file main.cpp
 *
 * @brief Firmware entry function.
 */

#include "Hardware.h"
#include "System.h"
#include "App.h"
#include "SplashScreen.h"
#include "MidiOutput.h"
#include "Trace.h"
#include "Profile.h"
#include "DisplayBenchmark.h"

/*
 * Prototype of the application creation function.
 * Required by START_ELECTRA_APPLICATION() macro.
 */
extern App *createApplication(void);

void initialise(void)
{
        // Bring the behavioural trace up before anything else can be traced.
    TRACE_INIT();
    PROFILE_INIT();

    createApplication();

    TRACE_BOOT(App::get()->getApplicationVersion(),
               App::get()->getApplicationName());

    // Initialise hardware components
    Hardware::initialise();

    /* The logo, the moment there is a screen to put it on. The card mount
     * and the fonts that follow are the slow part of a boot, and a panel
     * that stays dark through them looks like one that is not coming. */
    displaySplashLogo(Hardware::screen);

    Hardware::mountStorage();

    // Measure the SPI link before the application starts competing for it.
    DISPLAY_BENCHMARK_RUN();

    // Initilise the base system
    System::initialise();

    // Enable Logger
    System::logger.enable();

    // Create the sandbox dir. \todo sandboxing needs to be improved
    Hardware::sdcard.createDirectory(App::get()->getApplicationSandbox());

    // Set the brightness of the LCD \todo belongs to hardware
    uint16_t electraInfoBrightness =
        System::runtimeInfo.getElectraInfoBrightness();

    if (electraInfoBrightness != 0) {
        Hardware::screen.setBacklightbrightness(electraInfoBrightness);
    }

    /* The faces are built here rather than loaded. There is no asset file:
     * the glyphs live in flash as coverage and the coloured bands are
     * composited into the display's memory now. */
    uint32_t fontsStartedAt = millis();

    Hardware::screen.system().uploadFonts();
    Hardware::screen.user().uploadFonts();
    displaySplash(Hardware::screen,
                  App::get()->getApplicationName(),
                  App::get()->getApplicationVersion(),
                  System::runtimeInfo.getElectraInfoHwRevision());

    /* Worth a number rather than a note: it is the longest thing the unit
     * does before it can show anything, and it is the SPI link rather than
     * the compositing - a face added or dropped moves it. */
    System::logger.write(
        LOG_INFO, "UI fonts: built in %d ms", (int)(millis() - fontsStartedAt));

    // Initialize MIDI interfaces
    MidiInterface::get(MidiInterface::Type::MidiIo)->initialise();
    MidiInterface::get(MidiInterface::Type::MidiUsbDev)->initialise();
    MidiInterface::get(MidiInterface::Type::MidiUsbHost)->initialise();
    System::logger.write(LOG_INFO, "MIDI interfaces: initialised");

    // Clear the repaint queue \todo does not beliong here
    System::tasks.clearRepaintGraphics();

    // Initialise the user application (App)
    App::get()->initialise();

    // Clear and repaint the application screen.
    App::get()->clearScreen();
    App::get()->paintScreen();

    // Enable standard system tasks
    System::tasks.enableAll();
    System::logger.write(LOG_INFO,
                         "kernel task manager: enable software tasks");

    // Enable MIDI
    System::tasks.enableMidi();
    System::logger.write(LOG_INFO, "setup: enable readMidi task");

    // Disable logger by default
    System::logger.setStatus(false);

    App::get()->enableMidi = true;
    MidiOutput::sendAvailable(MidiInterface::Type::MidiUsbDev, 2);
}

void processEvents(void)
{
    System::tasks.execute();
}
