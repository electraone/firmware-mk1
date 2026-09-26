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

#include "luaOverlay.h"
#include "Preset.h"

extern Preset *luaPreset;

static Overlay *getOverlay(lua_State *L, uint8_t stackPosition)
{
    return (*reinterpret_cast<Overlay **>(
        luaL_checkudata(L, stackPosition, "Overlay")));
}

int luaopen_overlays(lua_State *L)
{
    luaL_newlib(L, overlays_functions);
    return 1;
}

int overlay_create(lua_State *L)
{
    lua_settop(L, 1);
    int overlayId = luaLE_checkListId(L, 1);

    const Overlay *overlay = luaPreset->getOverlay(overlayId);

    if (overlay) {
        luaLE_pushObject(L, "Overlay", overlay);
        return (1);
    }
    return (luaL_error(L, "failed: overlay %d does not exist", overlayId));
}

int overlay_createNew(lua_State *L)
{
    if (!lua_isinteger(L, 1)) {
        return (luaL_error(
            L, "failed: expected an integer as the first argument (id)"));
    }

    /* Both arguments are checked before the overlay is made. The table used to
     * be checked after, so a call with the wrong second argument left an empty
     * overlay behind under that id as well as raising. */
    if (!lua_istable(L, 2)) {
        return (luaL_error(L, "failed: expected a table as second argument"));
    }

    int overlayId = luaLE_checkListId(L, 1);
    auto &overlay = luaPreset->addOverlay(overlayId);

    int tableLength = luaL_len(L, 2);

    for (int i = 1; i <= tableLength; i++) {
        lua_pushinteger(L, i);
        lua_gettable(L, 2);

        if (!lua_istable(L, -1)) {
            return (luaL_error(L, "failed: item %d is not a table", i));
        }

        lua_getfield(L, -1, "value");
        int value = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        /* The label and the bitmap stay on the stack until addItem() has
         * copied them. luaL_checkstring() answers a pointer into the Lua
         * string, and popping it first leaves the collector free to take the
         * bytes addItem() is about to read. */
        lua_getfield(L, -1, "label");
        const char *label = luaL_checkstring(L, -1);

        /* Read fresh for every item. Declared once outside the loop, an item
         * without a bitmap was given the previous item's. */
        const char *bitmap = nullptr;

        lua_getfield(L, -2, "bitmap");
        if (!lua_isnil(L, -1)) {
            bitmap = luaL_checkstring(L, -1);
        }

        overlay.addItem(value, label, bitmap);

        /* The bitmap, the label and the item table. */
        lua_pop(L, 3);
    }

    luaLE_pushObject(L, "Overlay", &overlay);
    return (1);
}

int overlay_delete(lua_State *L)
{
    lua_settop(L, 1);
    return (0);
}

int overlay_print(lua_State *L)
{
    lua_settop(L, 1);

    if (Overlay *overlay = getOverlay(L, 1)) {
        overlay->print(LOG_LUA);
    }
    return (0);
}

void overlay_register(lua_State *L)
{
    lua_register(L, "Overlay", overlay_create);
    luaL_newmetatable(L, "Overlay");

    lua_pushcfunction(L, overlay_delete);
    lua_setfield(L, -2, "__gc");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    /* The object's own methods, not the module's. It used to be given
     * overlays_functions, so an Overlay carried get() and create() and did not
     * carry the print() it is documented to have. */
    luaLE_addObjectMethods(L, overlay_functions);
    lua_pop(L, 1);
}
