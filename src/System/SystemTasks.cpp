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
 * @file SystemTasks.cpp
 *
 * @brief A implementation of the System Task manager.
 */

#include "SystemTasks.h"
#include "TaskScheduler.h"
#include "System.h"
#include "UpdateModeScreen.h"

SystemTasks::SystemTasks()
    : taskMonitorFreeMemory(10000000, TASK_FOREVER, &monitorFreeMemory),
      taskCollectGarbage(10000000, TASK_FOREVER, &collectGarbage),
      taskReadPots(POT_SCAN_TICK, TASK_FOREVER, &readPots),
      taskReadButtons(15000, TASK_FOREVER, &readButtons),
      taskReadPotTouch(100000, TASK_FOREVER, &readPotTouch),
      taskReadLCDTouch(40000, TASK_FOREVER, &readLCDTouch),
      taskScanUSBHost(2000000, TASK_FOREVER, &scanUSBHost),
      taskRefreshMidiIndicators(100000, TASK_FOREVER, &refreshMidiIndicators),
      taskRepaintGraphics(25000, TASK_FOREVER, &repaintGraphics),
      taskUserTask(250000, TASK_FOREVER, &userTask),
      taskRunTimer(500000, TASK_FOREVER, &runTimer),
      taskSendMidi(2000, TASK_FOREVER, &sendMidi),
      taskProcessMidi(2000, TASK_FOREVER, &processMidi),
      taskProcessSysex(5000, TASK_FOREVER, &processSysex)
{
    init();
    addTask(taskMonitorFreeMemory);
    addTask(taskCollectGarbage);
    addTask(taskReadPots);
    addTask(taskReadButtons);
    addTask(taskReadLCDTouch);
    addTask(taskScanUSBHost);
    addTask(taskRefreshMidiIndicators);
    addTask(taskReadPotTouch);
    addTask(taskRepaintGraphics);
    addTask(taskUserTask);
    addTask(taskRunTimer);
    addTask(taskSendMidi);
    addTask(taskProcessMidi);
    addTask(taskProcessSysex);
}

void SystemTasks::enableAll()
{
    taskMonitorFreeMemory.disable();
    taskCollectGarbage.enable();
    taskReadPots.enable();
    taskReadButtons.enable();
    taskReadLCDTouch.enable();
    taskScanUSBHost.enable();
    taskRefreshMidiIndicators.enable();
    timerReadMidi.priority(150);
    timerSpinner.priority(255);

    if (System::context.getTouchEnabled()) {
        taskReadPotTouch.enable();
    } else {
        /* Nothing says which knob is in a hand, so the whole panel is swept
         * and a knob's own cadence is a twelfth of the tick. */
        taskReadPots.setInterval(POT_SCAN_TICK_ALL);
    }

    taskRepaintGraphics.enable();
    taskUserTask.enable();
    taskRunTimer.disable();
    taskSendMidi.enable();
    taskProcessMidi.enable();
    taskProcessSysex.enable();
}

void SystemTasks::enableSleepMode()
{
    taskMonitorFreeMemory.disable();
    taskCollectGarbage.disable();
    taskReadPots.disable();
    taskReadButtons.enable();
    taskReadLCDTouch.disable();
    taskScanUSBHost.disable();
    taskRefreshMidiIndicators.disable();
    timerReadMidi.priority(150);
    timerSpinner.priority(255);
    taskReadPotTouch.disable();
    taskRepaintGraphics.disable();
    taskUserTask.disable();
    taskRunTimer.disable();
    taskSendMidi.disable();
    taskProcessMidi.disable();
    taskProcessSysex.disable();
    disableMidi();
}

void SystemTasks::enableMidi(void)
{
    timerReadMidi.begin(readMidi, 500);
}

void SystemTasks::enableCtrlMidi(void)
{
    timerReadMidi.end();
    timerReadMidi.begin(readCtrlMidi, 6);
}

void SystemTasks::disableMidi(void)
{
    timerReadMidi.end();
}

void SystemTasks::flushMidi(void)
{
    sendMidi();
}

void SystemTasks::enableTimer(void)
{
    taskRunTimer.enable();
}

void SystemTasks::disableTimer(void)
{
    taskRunTimer.disable();
}

bool SystemTasks::isTimerEnabled(void)
{
    return (taskRunTimer.isEnabled());
}

unsigned long SystemTasks::timerGetInterval(void)
{
    return (taskRunTimer.getInterval());
}

void SystemTasks::timerSetInterval(unsigned long interval)
{
    taskRunTimer.setInterval(interval);
}

void SystemTasks::enableRepaintGraphics(void)
{
    taskRepaintGraphics.enable();
}

void SystemTasks::disableRepaintGraphics(void)
{
    taskRepaintGraphics.disable();
}

void SystemTasks::flushRepaintGraphics(void)
{
    System::repaintManager.processRepaintQueue();
}

void SystemTasks::clearRepaintGraphics(void)
{
    System::repaintManager.clearRepaintQueue();
}

void SystemTasks::doNotUsePotTouch(void)
{
    taskReadPotTouch.disable();
    taskReadPots.setInterval(2500);
}

void SystemTasks::enableUserTask(void)
{
    taskUserTask.enable();
}

void SystemTasks::disableUserTask(void)
{
    taskUserTask.disable();
}

void SystemTasks::enableSpinner(void)
{
    clearRepaintGraphics();
    spinnerReset();
    timerSpinner.begin(spinnerTick, 100000);
}

void SystemTasks::disableSpinner(void)
{
    timerSpinner.end();
}

void SystemTasks::displayUpdateModeScreen(void)
{
    UpdateModeScreen ums;
    clearRepaintGraphics();
    ums.repaint();
    flushRepaintGraphics();
}

void SystemTasks::stopWindowRepaint(void)
{
    disableRepaintGraphics();
    clearRepaintGraphics();
}

void SystemTasks::resumeWindowRepaint(void)
{
    clearRepaintGraphics();
    System::windowManager.repaintActive();
    enableRepaintGraphics();
}