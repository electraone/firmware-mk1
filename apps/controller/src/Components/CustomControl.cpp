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

#include "CustomControl.h"
#include "LookAndFeel.h"
#include "luaHooks.h"
#include "luaCallbacks.h"
#include "luaGraphics.h"

CustomControl::CustomControl(const Control &control, MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate)
{
}

void CustomControl::onTouchDown(const TouchEvent &touchEvent)
{
    runTouchCallback(control, touchEvent);
}

void CustomControl::onTouchMove(const TouchEvent &touchEvent)
{
    runTouchCallback(control, touchEvent);
}

void CustomControl::onTouchUp(const TouchEvent &touchEvent)
{
    runTouchCallback(control, touchEvent);
}

void CustomControl::onTouchClick(const TouchEvent &touchEvent)
{
    runTouchCallback(control, touchEvent);
}

void CustomControl::onTouchDoubleClick(const TouchEvent &touchEvent)
{
    runTouchCallback(control, touchEvent);
}

/* The long hold of ControlComponent opens a detail. A Custom control has
 * none, and the gesture is the script's if it wants it. */
void CustomControl::onTouchLongHold(const TouchEvent &touchEvent)
{
    runTouchCallback(control, touchEvent);
}

/*
 * A finger arriving on a knob, or leaving it.
 *
 * A script may subscribe to these on their own with setPotTouchCallback, and
 * subscribing takes the gesture over: the turn callback is no longer told
 * about touches either, so one gesture has one owner. Without a subscription
 * this is what it always was - the turn callback hears every kind of pot
 * event.
 *
 * The base is deliberately not called in either case: a Custom control does
 * its own highlighting and its own reporting in Lua.
 */
void CustomControl::onPotTouchDown(const PotEvent &potEvent)
{
    handlePotTouch(potEvent, LuaPotEvent::touchDown);
}

void CustomControl::onPotChange(const PotEvent &potEvent)
{
    runPotCallback(control, potEvent, LuaPotEvent::change);
}

void CustomControl::onPotTouchUp(const PotEvent &potEvent)
{
    handlePotTouch(potEvent, LuaPotEvent::touchUp);
}

bool CustomControl::handlePotTouch(const PotEvent &potEvent, LuaPotEvent event)
{
    if (hasPotTouchCallback(control.getId())) {
        runPotTouchCallback(control, potEvent, event);
        return (true);
    }

    runPotCallback(control, potEvent, event);
    return (false);
}

/*
 * What the control shows is whatever its paint callback makes of its values,
 * so a value that changes - MIDI arriving, another control on the same
 * parameter, a script - is a repaint. The mk2 does nothing here and a Custom
 * control there keeps the old value on screen until the script repaints it
 * itself. A script that also calls repaint() costs nothing extra: the repaint
 * manager keeps one request per component.
 */
void CustomControl::onMidiValueChange(
    [[maybe_unused]] const ControlValue &value,
    [[maybe_unused]] int16_t midiValue,
    [[maybe_unused]] uint8_t handle)
{
    if (hasPaintCallback(control.getId())) {
        repaint();
    }
}

/*
 * Clearing and the active underline happen whether or not Lua answers. A
 * custom control on a preset that has opened no script used to paint nothing
 * at all - not even a clear - and kept whatever pixels the last component to
 * own that area had left behind.
 */
void CustomControl::paint(Graphics &g)
{
    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);

    if (hasPaintCallback(control.getId())) {
        /* The scope is what lets the graphics library draw at all: outside a
         * paint the chip's active window belongs to some other component. */
        LuaGraphicsScope scope;

        runPaintCallback(control);
    }

    ControlComponent::paint(g);
}
