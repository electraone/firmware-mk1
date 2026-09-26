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
 * @file LinkCustomControl.h
 *
 * @brief A custom control drawn from a captured paint.
 *
 * On a controller a custom control is painted by its preset's Lua, and the
 * script runs only on the primary - so this draws what the primary recorded:
 * the drawing calls the paint callback made, replayed against this unit's
 * own display. The coordinates are control-relative and the display's active
 * window is this component's own during paint, so the replay lands wherever
 * this layout put the element, exactly as the script's own drawing does on
 * a controller.
 *
 * Built on ControlComponent directly rather than on the controller's
 * CustomControl: that one asks a Lua state for everything, and the link
 * program has none - linking it would have pulled the interpreter into an
 * image that never runs a script.
 */

#pragma once

#include "ControlComponent.h"
#include "LinkGestureSink.h"
#include <vector>

class LinkCustomControl : public ControlComponent
{
public:
    LinkCustomControl(const Control &control,
                      MainDelegate &newDelegate,
                      LinkGestureSink &newGestures);
    virtual ~LinkCustomControl() = default;

    /* The touches. On a controller these run the preset's Lua callbacks; the
     * script is on the primary, so here they travel raw. */
    void onTouchDown(const TouchEvent &touchEvent) override;
    void onTouchMove(const TouchEvent &touchEvent) override;
    void onTouchUp(const TouchEvent &touchEvent) override;
    void onTouchClick(const TouchEvent &touchEvent) override;
    void onTouchDoubleClick(const TouchEvent &touchEvent) override;
    void onTouchLongHold(const TouchEvent &touchEvent) override;

    void onMidiValueChange(const ControlValue &value,
                           int16_t midiValue,
                           uint8_t handle = 0) override;

    /**
     * @brief Point at the frame to replay.
     *
     * The vector belongs to the page's store and outlives this component;
     * empty means the primary has not sent a picture yet, and the control
     * paints its chrome and nothing else.
     */
    void setFrame(const std::vector<uint8_t> *newFrame);

    void paint(Graphics &g) override;

private:
    /** @brief Replay the recorded ops against this unit's display. */
    void replay(void);

    const std::vector<uint8_t> *frame;
    LinkGestureSink &gestures;
};
