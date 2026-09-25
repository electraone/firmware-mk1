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
 * @file EnvelopeControl.h
 *
 * @brief The on-screen Control for every kind of envelope.
 */

#pragma once

#include "Control.h"
#include "ControlComponent.h"
#include "MultiValue.h"
#include "EnvelopeShape.h"

/**
 * @brief One control for every envelope.
 *
 * There used to be three classes - ADSRControl, ADRControl, Dx7EnvControl -
 * and the diff between any two of them was a segment count and a min/max
 * wiring table. Their detail versions were three more, and the diff between
 * any two of those was their names.
 *
 * They were not written that way out of carelessness. Each of them inherited
 * ControlComponent on one side and its envelope on the other, and since
 * Component was a virtual base of both, the leaf had to re-declare every event
 * handler just to name a winner.
 *
 * The shape is composed rather than inherited now, as it is on the mk2: an
 * EnvelopeShape is geometry, knows nothing about the screen, and is a
 * stateless singleton shared by every control that draws it.
 */
class EnvelopeControl : public ControlComponent, public MultiValue
{
public:
    EnvelopeControl(const Control &control,
                    MainDelegate &newDelegate,
                    const EnvelopeShape &newShape);
    virtual ~EnvelopeControl() = default;

    void syncComponentProperties(void) override;

    void onTouchDown(const TouchEvent &touchEvent) override;
    void onTouchMove(const TouchEvent &touchEvent) override;
    void onTouchUp(const TouchEvent &touchEvent) override;
    void onPotTouchDown(const PotEvent &potEvent) override;
    void onPotChange(const PotEvent &potEvent) override;
    void onPotTouchUp(const PotEvent &potEvent) override;
    void onMidiValueChange(const ControlValue &value,
                           int16_t midiValue,
                           uint8_t handle = 0) override;

    uint8_t getNumHandles(void) const override;
    void paint(Graphics &g) override;

protected:
    /** How tall the contour is drawn. A control on a page leaves room for its
     *  name underneath; a detail uses the whole window. */
    virtual uint16_t getContourHeight(void) const;

    /** The name and the active underline, which a detail does not draw. */
    virtual void paintDecorations(Graphics &g);

    void setValueRanges(void);

    const EnvelopeShape &shape;
    std::vector<Point> points;
    int16_t baselineY;
};

/**
 * @brief An EnvelopeControl that knows which shape it draws.
 *
 * The factory builds a control from (control, delegate) like every other type,
 * so the shape has to arrive some other way. This is that way: one stateless
 * instance per shape, shared by every control of that kind.
 *
 * @tparam Shape an EnvelopeShape
 * @tparam Base EnvelopeControl, or EnvelopeDetailControl
 */
template <class Shape, class Base>
class ShapedEnvelope final : public Base
{
public:
    ShapedEnvelope(const Control &control, MainDelegate &newDelegate)
        : Base(control, newDelegate, instance)
    {
    }

private:
    static inline const Shape instance{};
};
