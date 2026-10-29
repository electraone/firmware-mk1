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
 * @file SystemTasks.h
 *
 * @brief A implementation of the System Task manager.
 */
#pragma once

#define _TASK_MICRO_RES
#include "TaskSchedulerDeclarations.h"
#include <IntervalTimer.h>
#include "InstanceCallback.h"

#include "TaskReadButtons.h"
#include "TaskRunScheduled.h"
#include "TaskReadPots.h"
#include "TaskReadPotTouch.h"
#include "TaskReadLCDTouch.h"
#include "TaskScanUSBHost.h"
#include "TaskRefreshMidiIndicators.h"
#include "TaskRepaintGraphics.h"
#include "TaskRunTimer.h"
#include "TaskRunUserTask.h"
#include "TaskMonitorFreeMemory.h"
#include "TaskCollectGarbage.h"
#include "TaskReadMidi.h"
#include "TaskSendMidi.h"
#include "TaskProcessSysex.h"
#include "TaskSpinner.h"

class SystemTasks : public Scheduler
{
public:
    SystemTasks();
    ~SystemTasks() = default;

    void enableAll(void);
    void enableSleepMode();
    void enableMidi(void);
    void enableCtrlMidi(void);
    void disableMidi(void);
    void flushMidi(void);
    void enableTimer(void);
    void disableTimer(void);
    bool isTimerEnabled(void);
    unsigned long timerGetInterval(void);
    void timerSetInterval(unsigned long interval);
    void enableRepaintGraphics(void);
    void disableRepaintGraphics(void);
    void flushRepaintGraphics(void);
    void clearRepaintGraphics(void);
    void doNotUsePotTouch(void);
    void enableUserTask(void);
    void disableUserTask(void);
    void enableSpinner(void);
    void disableSpinner(void);
    void displayUpdateModeScreen(void);
    void stopWindowRepaint(void);
    void resumeWindowRepaint(void);

private:
    IntervalTimer timerReadMidi;
    IntervalTimer timerSpinner;

    Task taskMonitorFreeMemory;
    Task taskCollectGarbage;
    Task taskReadPots;
    Task taskReadButtons;
    Task taskReadPotTouch;
    Task taskReadLCDTouch;
    Task taskScanUSBHost;
    Task taskRefreshMidiIndicators;
    Task taskRepaintGraphics;
    Task taskUserTask;
    Task taskRunScheduled;
    Task taskRunTimer;
    Task taskSendMidi;
    Task taskProcessMidi;
    Task taskProcessSysex;
};
