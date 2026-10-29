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
 * @file Hardware.cpp
 *
 * @brief A wrapper class for all hardware related functionality.
 */

#include "Hardware.h"
#include "System.h"
#include "Reset.h"

void Hardware::initialise(void)
{
    // Configure input/output pins
    configurePins();

    // Initialise internal signal multiplexer
    MUX_SET_ADDRESS(0);
    MUX_DISABLE;
    System::logger.write(LOG_ERROR, "Multiplexer: initialised");

    // Configure AD convertors. Twelve bits averaged by four, as on the mk2: the
    // pot reader takes eight of these per gang and trims them, see Pot.h.
    adc.adc0->setAveraging(4);
    adc.adc1->setAveraging(4);
    adc.adc0->setResolution(12);
    adc.adc1->setResolution(12);
    adc.adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
    adc.adc1->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
    adc.adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
    adc.adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
    System::logger.write(LOG_ERROR, "AD convertor: initialised");

    // Initialise RAM monitoring
    ram.initialize();
    System::logger.write(LOG_ERROR, "SD RAM monitor: initialised");

    // Clear Serial ports
    Serial1.clear();
    Serial2.clear();

    // Initialise the LCD
    screen.initialise();
    System::logger.write(LOG_ERROR, "LCD: initialised");

    // Initialise LCD touch interface
    touch.begin();
    System::logger.write(LOG_ERROR, "LCD touch: initialided");

    // Initialise pots
    pots.initialise();
    System::logger.write(LOG_ERROR, "Pots: initialised");
}

void Hardware::mountStorage(void)
{
    // Mount the internal storage media
    Hardware::sdcard.mount();

    // Assign the storage drivers.
    memory.stringPool.assignStorageDriver(&Hardware::screen);
    memory.bitmapPool.assignStorageDriver(&Hardware::screen);
}

void Hardware::reset(void)
{
    CPU_RESTART;
}

Pots Hardware::pots;
PotTouch Hardware::potTouch;
Buttons Hardware::buttons;
ADC Hardware::adc;
Touch Hardware::touch;
Graphics Hardware::screen(RA8876_CS, RA8876_RESET);
PersistentStorage Hardware::sdcard;
RamMonitor Hardware::ram;
Memory Hardware::memory;
