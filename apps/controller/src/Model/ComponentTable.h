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
 * @file ComponentTable.h
 *
 * @brief Which component, if any, is drawing a control or a group.
 *
 * Only the controls and groups of the page on the screen have a component -
 * a few dozen - while a preset holds hundreds of controls, and every one of
 * them used to carry a pointer for it. The pointers live here instead, in a
 * small sorted table keyed by the model object's address, which costs eight
 * bytes per component on the screen rather than four per object in the
 * preset.
 */

#pragma once

class Component;

namespace ComponentTable
{
    /** Records that @p owner is drawn by @p component; nullptr forgets it. */
    void set(const void *owner, Component *component);

    /** The component drawing @p owner, or nullptr. */
    Component *get(const void *owner);

    /** Forgets every entry. Called when a preset goes away, so that a model
     *  object later created at a freed address does not inherit a dead
     *  pointer. */
    void clear(void);
} // namespace ComponentTable
