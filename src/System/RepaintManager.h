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
 * @file RepaintManager.h
 *
 * @brief A Component repaint system.
 */

#pragma once

#include "Graphics.h"
#include "WindowManager.h"
#include <vector>

class Component;

struct RepaintRequest {
    RepaintRequest() : component(nullptr), valid(true)
    {
    }

    RepaintRequest(Component *newComponent)
        : component(newComponent), valid(true)
    {
    }

    RepaintRequest(const Rectangle &area)
        : component(nullptr), areaToClear(area), valid(true)
    {
    }

    ~RepaintRequest() = default;

    Component *component;
    Rectangle areaToClear;
    bool valid;
};

class RepaintManager
{
public:
    RepaintManager(Graphics &newG, WindowManager &newWm);
    ~RepaintManager() = default;

    /**
     * Schedules a repaint of the given component.
     *
     * @param component The component to repaint.
     */
    void scheduleRepaint(Component *component);

    /**
     * Schedules a clear of the given area.
     *
     * @param area The area to clear.
     */
    void scheduleClear(const Rectangle &area);

    /**
     * Processes the repaint queue.
     */
    void processRepaintQueue(void);

    /**
     * Clears the repaint queue.
     */
        void clearRepaintQueue(void);

    bool isQueueEmpty(void) const
    {
        return (repaintQueue.empty());
    }

    /**
     * @brief Forgets whatever is queued for one component.
     *
     * Called when a component is destroyed. The queue holds bare pointers and
     * is walked a frame later, so anything left in it for a component that has
     * gone is a read of freed memory.
     */
    void invalidateEntries(Component *component);

private:
    void unfoldNestedComponents(void);
    void unfoldComponent(Component *component);
    void repaintQueueEntries(Window *window);
    void paintComponent(Component *component);
    void clearArea(const Rectangle &area);

    std::vector<RepaintRequest> repaintQueue;
    Graphics &g;
    WindowManager &wm;
};
