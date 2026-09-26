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
 * @file DetailEnvelope.h
 *
 * @brief The window every envelope opens: the contour, and a dial per segment.
 */

#pragma once

#include "Detail.h"
#include "ControlComponent.h"
#include <vector>

/**
 * @brief One window for all six envelopes.
 *
 * There were three of these - DetailADSR, DetailADR, DetailDx7Env - and the
 * diff between any two was a segment count and the names of its knobs. Wiring
 * the other three envelope types would have made six.
 *
 * What differs between them is how many dials there are, and that is read off
 * the control: a dial per value it declares, labelled with that value's id.
 * Up to six go in one row; more than six go in two, paired so that a DX7
 * envelope keeps its levels above its rates.
 */
class DetailEnvelope : public Detail
{
public:
    DetailEnvelope(const Control &control, MainDelegate &newDelegate);
    virtual ~DetailEnvelope() = default;

    void paint(Graphics &g) override;
    void resized(void) override;

private:
    /** How many dials go side by side, and how many rows they make. */
    uint8_t numColumns(void) const;
    uint8_t numRows(void) const;

    std::vector<ControlComponent *> knobs;
};
