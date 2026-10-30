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

#include "luaExtension.h"
#include "luaLazyGlobals.h"
#include "TextBase.h"

/*
 * What a controller script can name on top of the base firmware's, made when it
 * first does - see luaLazyGlobals.h. Sorted by strcmp(), capitals first.
 *
 * The text constants are here twice: ALIGN_CENTER, FONT_BOLD and SIZE_LARGE
 * are this firmware's own names, CENTER, BOLD and LARGE the mk2's, which is
 * what the editor writes into a Custom control's script.
 */
static const LuaLazyGlobal controllerGlobals[] = {
    LuaLazyGlobal::constant("ALIGN_CENTER", (int)TextAlign::center),
    LuaLazyGlobal::constant("ALIGN_LEFT", (int)TextAlign::left),
    LuaLazyGlobal::constant("ALIGN_RIGHT", (int)TextAlign::right),
    LuaLazyGlobal::constant("BLUE", 0x529DEC),
    LuaLazyGlobal::constant("BOLD", (int)TextFace::bold),
    LuaLazyGlobal::constant("CENTER", (int)TextAlign::center),
    LuaLazyGlobal::constant("CONTROL_SETS", 2),
    LuaLazyGlobal::constant("CONTROL_SET_1", 1),
    LuaLazyGlobal::constant("CONTROL_SET_2", 2),
    LuaLazyGlobal::constant("CONTROL_SET_3", 3),
    LuaLazyGlobal::type("Control", control_register),
    LuaLazyGlobal::type("ControlValue", value_register),
    LuaLazyGlobal::type("Device", device_register),
    LuaLazyGlobal::constant("FONT_BOLD", (int)TextFace::bold),
    LuaLazyGlobal::constant("FONT_REGULAR", (int)TextFace::regular),
    LuaLazyGlobal::constant("GREEN", 0x03A598),
    LuaLazyGlobal::type("Group", group_register),
    LuaLazyGlobal::constant("HUGE", (int)TextSize::huge),
    LuaLazyGlobal::constant("LARGE", (int)TextSize::large),
    LuaLazyGlobal::constant("LEFT", (int)TextAlign::left),
    LuaLazyGlobal::constant("MEDIUM", (int)TextSize::medium),
    LuaLazyGlobal::constant("MONOSPACED", (int)TextSpacing::monospaced),
    LuaLazyGlobal::type("Message", message_register),
    LuaLazyGlobal::constant("ORANGE", 0xF49500),
    LuaLazyGlobal::type("Overlay", overlay_register),
    LuaLazyGlobal::constant("PAGES", 1),
    LuaLazyGlobal::constant("PROPORTIONAL", (int)TextSpacing::proportional),
    LuaLazyGlobal::constant("PT_ATCHANNEL", 12),
    LuaLazyGlobal::constant("PT_ATPOLY", 11),
    LuaLazyGlobal::constant("PT_CC14", 2),
    LuaLazyGlobal::constant("PT_CC7", 1),
    LuaLazyGlobal::constant("PT_NONE", 16),
    LuaLazyGlobal::constant("PT_NOTE", 5),
    LuaLazyGlobal::constant("PT_NRPN", 3),
    LuaLazyGlobal::constant("PT_PITCHBEND", 13),
    LuaLazyGlobal::constant("PT_PROGRAM", 6),
    LuaLazyGlobal::constant("PT_RELCC", 15),
    LuaLazyGlobal::constant("PT_RPN", 4),
    LuaLazyGlobal::constant("PT_SPP", 14),
    LuaLazyGlobal::constant("PT_START", 8),
    LuaLazyGlobal::constant("PT_STOP", 9),
    LuaLazyGlobal::constant("PT_SYSEX", 7),
    LuaLazyGlobal::constant("PT_TUNE", 10),
    LuaLazyGlobal::constant("PT_VIRTUAL", 0),
    LuaLazyGlobal::constant("PURPLE", 0xC44795),
    LuaLazyGlobal::type("Page", page_register),
    LuaLazyGlobal::constant("RED", 0xF45C51),
    LuaLazyGlobal::constant("REGULAR", (int)TextFace::regular),
    LuaLazyGlobal::constant("RIGHT", (int)TextAlign::right),
    LuaLazyGlobal::constant("SIZE_HUGE", (int)TextSize::huge),
    LuaLazyGlobal::constant("SIZE_LARGE", (int)TextSize::large),
    LuaLazyGlobal::constant("SIZE_MEDIUM", (int)TextSize::medium),
    LuaLazyGlobal::constant("SIZE_SMALL", (int)TextSize::small),
    LuaLazyGlobal::constant("SMALL", (int)TextSize::small),
    LuaLazyGlobal::constant("SPACING_MONOSPACED", (int)TextSpacing::monospaced),
    LuaLazyGlobal::constant("SPACING_PROPORTIONAL", (int)TextSpacing::proportional),
    LuaLazyGlobal::constant("VT_DEFAULT", 0),
    LuaLazyGlobal::constant("VT_HIGHLIGHTED", 1),
    LuaLazyGlobal::constant("WHITE", 0xffffff),
    LuaLazyGlobal::module("commands", luaopen_commands),
    LuaLazyGlobal::module("controls", luaopen_controls),
    LuaLazyGlobal::module("devices", luaopen_devices),
    LuaLazyGlobal::module("events", luaopen_events),
    LuaLazyGlobal::module("graphics", luaopen_graphics),
    LuaLazyGlobal::module("groups", luaopen_groups),
    LuaLazyGlobal::module("helpers", luaopen_helpers),
    LuaLazyGlobal::module("info", luaopen_info),
    LuaLazyGlobal::module("overlays", luaopen_overlays),
    LuaLazyGlobal::module("pages", luaopen_pages),
    LuaLazyGlobal::module("parameterMap", luaopen_parameterMap),
    LuaLazyGlobal::module("patch", luaopen_patch),
    LuaLazyGlobal::module("preset", luaopen_preset),
    LuaLazyGlobal::module("schedule", luaopen_schedule),
};

void loadLuaLibs(void)
{
    luaLE_addLazyGlobals(controllerGlobals);

    // Clear the stack
    lua_settop(L, 0);
}

/** @todo Get rid of this global variables */
Preset *luaPreset = nullptr;
Presets *luaPresets = nullptr;
MainDelegate *luaDelegate = nullptr;