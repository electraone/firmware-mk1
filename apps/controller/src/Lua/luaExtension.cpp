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
#include "TextBase.h"

void loadLuaLibs(void)
{
    static const luaL_Reg ctrlv2libs[] = { { "parameterMap",
                                             luaopen_parameterMap },
                                           { "pages", luaopen_pages },
                                           { "groups", luaopen_groups },
                                           { "devices", luaopen_devices },
                                           { "controls", luaopen_controls },
                                           { "helpers", luaopen_helpers },
                                           { "patch", luaopen_patch },
                                           { "preset", luaopen_preset },
                                           { "info", luaopen_info },
                                           { "events", luaopen_events },
                                           { "overlays", luaopen_overlays },
                                           { "graphics", luaopen_graphics },
                                           { "commands", luaopen_commands },
                                           { "schedule", luaopen_schedule },
                                           { NULL, NULL } };

    luaLE_openEoslibs(L, ctrlv2libs);
    value_register(L);
    page_register(L);
    group_register(L);
    device_register(L);
    control_register(L);
    message_register(L);
    overlay_register(L);

    loadGlobalVariables(L);

    // Clear the stack
    lua_settop(L, 0);
}

void loadGlobalVariables(lua_State *L)
{
    // Colours
    lua_pushnumber(L, 0xffffff);
    lua_setglobal(L, "WHITE");
    lua_pushnumber(L, 0xF45C51);
    lua_setglobal(L, "RED");
    lua_pushnumber(L, 0xF49500);
    lua_setglobal(L, "ORANGE");
    lua_pushnumber(L, 0x529DEC);
    lua_setglobal(L, "BLUE");
    lua_pushnumber(L, 0x03A598);
    lua_setglobal(L, "GREEN");
    lua_pushnumber(L, 0xC44795);
    lua_setglobal(L, "PURPLE");

    // Electra Parameter types
    lua_pushnumber(L, 0);
    lua_setglobal(L, "PT_VIRTUAL");
    lua_pushnumber(L, 1);
    lua_setglobal(L, "PT_CC7");
    lua_pushnumber(L, 2);
    lua_setglobal(L, "PT_CC14");
    lua_pushnumber(L, 3);
    lua_setglobal(L, "PT_NRPN");
    lua_pushnumber(L, 4);
    lua_setglobal(L, "PT_RPN");
    lua_pushnumber(L, 5);
    lua_setglobal(L, "PT_NOTE");
    lua_pushnumber(L, 6);
    lua_setglobal(L, "PT_PROGRAM");
    lua_pushnumber(L, 7);
    lua_setglobal(L, "PT_SYSEX");
    lua_pushnumber(L, 8);
    lua_setglobal(L, "PT_START");
    lua_pushnumber(L, 9);
    lua_setglobal(L, "PT_STOP");
    lua_pushnumber(L, 10);
    lua_setglobal(L, "PT_TUNE");
    lua_pushnumber(L, 11);
    lua_setglobal(L, "PT_ATPOLY");
    lua_pushnumber(L, 12);
    lua_setglobal(L, "PT_ATCHANNEL");
    lua_pushnumber(L, 13);
    lua_setglobal(L, "PT_PITCHBEND");
    lua_pushnumber(L, 14);
    lua_setglobal(L, "PT_SPP");
    lua_pushnumber(L, 15);
    lua_setglobal(L, "PT_RELCC");
    lua_pushnumber(L, 16);
    lua_setglobal(L, "PT_NONE");

    // Control sets
    lua_pushnumber(L, 1);
    lua_setglobal(L, "CONTROL_SET_1");
    lua_pushnumber(L, 2);
    lua_setglobal(L, "CONTROL_SET_2");
    lua_pushnumber(L, 3);
    lua_setglobal(L, "CONTROL_SET_3");

    // Events
    lua_pushnumber(L, 1);
    lua_setglobal(L, "PAGES");
    lua_pushnumber(L, 2);
    lua_setglobal(L, "CONTROL_SETS");

    // Variants
    lua_pushnumber(L, 0);
    lua_setglobal(L, "VT_DEFAULT");
    lua_pushnumber(L, 1);
    lua_setglobal(L, "VT_HIGHLIGHTED");

    /* Text alignment, which graphics.print() takes as its fifth argument.
     * Without names for them a script had to write the raw 0, 1 or 2. */
    lua_pushnumber(L, (int)TextAlign::left);
    lua_setglobal(L, "ALIGN_LEFT");
    lua_pushnumber(L, (int)TextAlign::center);
    lua_setglobal(L, "ALIGN_CENTER");
    lua_pushnumber(L, (int)TextAlign::right);
    lua_setglobal(L, "ALIGN_RIGHT");

    /* The face graphics.print() may be given, and that the two measuring
     * functions take. These three axes are this model's own - the mk2's print()
     * has one face - so the names are too. */
    lua_pushnumber(L, (int)TextFace::regular);
    lua_setglobal(L, "FONT_REGULAR");
    lua_pushnumber(L, (int)TextFace::bold);
    lua_setglobal(L, "FONT_BOLD");

    lua_pushnumber(L, (int)TextSize::small);
    lua_setglobal(L, "SIZE_SMALL");
    lua_pushnumber(L, (int)TextSize::medium);
    lua_setglobal(L, "SIZE_MEDIUM");
    lua_pushnumber(L, (int)TextSize::large);
    lua_setglobal(L, "SIZE_LARGE");
    lua_pushnumber(L, (int)TextSize::huge);
    lua_setglobal(L, "SIZE_HUGE");

    lua_pushnumber(L, (int)TextSpacing::proportional);
    lua_setglobal(L, "SPACING_PROPORTIONAL");
    lua_pushnumber(L, (int)TextSpacing::monospaced);
    lua_setglobal(L, "SPACING_MONOSPACED");
}

/** @todo Get rid of this global variables */
Preset *luaPreset = nullptr;
Presets *luaPresets = nullptr;
MainDelegate *luaDelegate = nullptr;