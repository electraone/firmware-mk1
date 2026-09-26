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
 * @file Controls.h
 *
 * @brief The controls of a preset: stored in a Pool, found by id through a
 *        sorted index, walked in id order.
 *
 * This replaces a std::map, whose 88-byte nodes cost a large preset more in
 * tree than in control. The controls themselves never move, so a component
 * or a value may keep a pointer to one; the index is four bytes a control
 * and a binary search, which is also what a MIDI message or a knob pays to
 * find its control.
 */

#pragma once

#include "Control.h"
#include "Pool.h"
#include <algorithm>
#include <vector>

class Controls
{
public:
    class Iterator
    {
    public:
        explicit Iterator(std::vector<Control *>::const_iterator newIt)
            : it(newIt)
        {
        }

        Control &operator*(void) const
        {
            return (**it);
        }

        Control *operator->(void) const
        {
            return (*it);
        }

        Iterator &operator++(void)
        {
            ++it;
            return (*this);
        }

        bool operator!=(const Iterator &other) const
        {
            return (it != other.it);
        }

    private:
        std::vector<Control *>::const_iterator it;
    };

    Controls() = default;

    /** Store a copy of the control. The copy is the one the preset owns
     *  from now on, and it stays where it is. */
    Control &add(const Control &control)
    {
        Control *stored = storage.allocate(1);

        if (!stored) {
            return (notStored);
        }
        *stored = control;
        byId.insert(lowerBound(control.getId()), stored);
        return (*stored);
    }

    /** The control with the id, or nullptr. */
    Control *find(uint16_t controlId)
    {
        auto it = lowerBound(controlId);

        if ((it != byId.end()) && ((*it)->getId() == controlId)) {
            return (*it);
        }
        return (nullptr);
    }

    const Control *find(uint16_t controlId) const
    {
        return (const_cast<Controls *>(this)->find(controlId));
    }

    size_t size(void) const
    {
        return (byId.size());
    }

    bool empty(void) const
    {
        return (byId.empty());
    }

    Iterator begin(void) const
    {
        return (Iterator(byId.begin()));
    }

    Iterator end(void) const
    {
        return (Iterator(byId.end()));
    }

    void clear(void)
    {
        byId.clear();
        byId.shrink_to_fit();
        storage.clear();
    }

    /** Give back the room the index grew into, once the preset is read. */
    void compact(void)
    {
        byId.shrink_to_fit();
    }

private:
    std::vector<Control *>::iterator lowerBound(uint16_t controlId)
    {
        return (std::lower_bound(byId.begin(),
                                 byId.end(),
                                 controlId,
                                 [](const Control *control, uint16_t id) {
                                     return (control->getId() < id);
                                 }));
    }

    /* Two kilobytes a page: 36 controls, and small enough that the
     * allocator finds the room in a heap already holding a preset. */
    Pool<Control, 2048> storage;
    std::vector<Control *> byId;

    /* Answered by add() when there is no memory left, so a caller that does
     * not check still writes into something. */
    static Control notStored;
};
