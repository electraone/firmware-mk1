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

#include "luaGroup.h"
#include "Preset.h"
#include "MainDelegate.h"

extern Preset *luaPreset;
extern MainDelegate *luaDelegate;

static Group *getGroup(lua_State *L, [[maybe_unused]] uint8_t stackPosition)
{
    return (*reinterpret_cast<Group **>(luaL_checkudata(L, 1, "Group")));
}

int luaopen_groups(lua_State *L)
{
    luaL_newlib(L, groups_functions);
    return 1;
}

int group_create(lua_State *L)
{
    lua_settop(L, 1);
    int groupId = luaLE_checkGroupId(L, 1);

    const Group &group = luaPreset->getGroup(groupId);

    if (group.isValid()) {
        luaLE_pushObject(L, "Group", &group);
        return (1);
    }
    return (luaL_error(L, "failed: group %d does not exist", groupId));
}

int group_delete(lua_State *L)
{
    lua_settop(L, 1);
    return (0);
}

int group_getId(lua_State *L)
{
    lua_settop(L, 1);
    int id = 0;

    if (Group *group = getGroup(L, 1)) {
        id = group->getId();
        lua_pushinteger(L, id);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid group"));
}

int group_setLabel(lua_State *L)
{
    lua_settop(L, 2);

    Group *group = getGroup(L, 1);
    const char *label = luaL_checkstring(L, 2);

    luaL_argcheck(L, label != nullptr, 2, "failed: label must not be nil");

    if (group) {
        luaDelegate->setGroupLabel(group->getId(), label);
    } else {
        return (luaL_error(L, "failed: not a valid group"));
    }
    return (0);
}

int group_getLabel(lua_State *L)
{
    lua_settop(L, 1);
    const char *label;

    if (Group *group = getGroup(L, 1)) {
        label = group->getLabel();
        lua_pushstring(L, label);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid group"));
}

int group_setColour(lua_State *L)
{
    lua_settop(L, 2);

    Group *group = getGroup(L, 1);
    int colour = luaL_checkinteger(L, 2);

    if (group) {
        luaDelegate->setGroupColour(group->getId(), colour);
    }
    return (0);
}

int group_getColour(lua_State *L)
{
    lua_settop(L, 1);
    int colour = 0;

    if (Group *group = getGroup(L, 1)) {
        colour = group->getColour();
        lua_pushinteger(L, colour);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid group"));
}

int group_setVisible(lua_State *L)
{
    lua_settop(L, 2);

    Group *group = getGroup(L, 1);
    bool shouldBeVisible = luaLE_checkBoolean(L, 2);

    if (group) {
        luaDelegate->setGroupVisible(group->getId(), shouldBeVisible);
    }
    return (0);
}

int group_isVisible(lua_State *L)
{
    lua_settop(L, 1);
    bool state = false;

    if (Group *group = getGroup(L, 1)) {
        state = group->isVisible();
        lua_pushboolean(L, state);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid group"));
}

int group_setBounds(lua_State *L)
{
    lua_settop(L, 2);

    Group *group = getGroup(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    lua_rawgeti(L, 2, 1);
    lua_rawgeti(L, 2, 2);
    lua_rawgeti(L, 2, 3);
    lua_rawgeti(L, 2, 4);
    int x = luaL_checkinteger(L, -4);
    int y = luaL_checkinteger(L, -3);
    int width = luaL_checkinteger(L, -2);
    int height = luaL_checkinteger(L, -1);

    if (group) {
        luaDelegate->setGroupBounds(group->getId(),
                                    Rectangle(x, y, width, height));
    } else {
        return (luaL_error(L, "failed: not a valid group"));
    }
    return (0);
}

int group_getBounds(lua_State *L)
{
    lua_settop(L, 1);
    Rectangle bounds;

    if (Group *group = getGroup(L, 1)) {
        bounds = group->getBounds();
        lua_newtable(L);
        luaLE_pushArrayInteger(L, 1, bounds.getX());
        luaLE_pushArrayInteger(L, 2, bounds.getY());
        luaLE_pushArrayInteger(L, 3, bounds.getWidth());
        luaLE_pushArrayInteger(L, 4, bounds.getHeight());

        return (1);
    }
    return (luaL_error(L, "failed: not a valid group"));
}

int group_setSlot(lua_State *L)
{
    lua_settop(L, 4);

    Group *group = getGroup(L, 1);

    int slot = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);

    int height = 0;

    if (lua_isnil(L, 4)) {
        height = 0;
    } else {
        height = luaL_checkinteger(L, 4);
    }

    luaL_argcheck(
        L, 1 <= slot && slot <= 36, 2, "failed: slot must be between 1 and 36");

    luaL_argcheck(L,
                  1 <= width && width <= 6,
                  3,
                  "failed: width must be between 1 and 6");

    luaL_argcheck(L,
                  0 <= height && height <= 6,
                  4,
                  "failed: height must be between 0 and 6");

    if (group) {
        luaDelegate->setGroupSlot(group->getId(), slot, width, height);
    } else {
        return (luaL_error(L, "failed: not a valid group"));
    }
    return (0);
}

int group_setHorizontalSpan(lua_State *L)
{
    lua_settop(L, 2);

    Group *group = getGroup(L, 1);

    int width = luaL_checkinteger(L, 2);

    luaL_argcheck(L,
                  1 <= width && width <= 6,
                  2,
                  "failed: width must be between 1 and 6");

    if (group) {
        luaDelegate->setGroupHorizontalSpan(group->getId(), width);
    } else {
        return (luaL_error(L, "failed: not a valid group"));
    }
    return (0);
}

int group_setVerticalSpan(lua_State *L)
{
    lua_settop(L, 2);

    Group *group = getGroup(L, 1);

    int height = luaL_checkinteger(L, 2);

    luaL_argcheck(L,
                  0 <= height && height <= 6,
                  2,
                  "failed: height must be between 0 and 6");

    if (group) {
        luaDelegate->setGroupVerticalSpan(group->getId(), height);
    } else {
        return (luaL_error(L, "failed: not a valid group"));
    }
    return (0);
}

int group_setVariant(lua_State *L)
{
    lua_settop(L, 2);

    Group *group = getGroup(L, 1);

    int variant = luaL_checkinteger(L, 2);

    luaL_argcheck(L,
                  0 <= variant && variant <= 1,
                  2,
                  "failed: variant must be between 0 and 1");

    if (group) {
        luaDelegate->setGroupVariant(group->getId(), variant);
    } else {
        return (luaL_error(L, "failed: not a valid group"));
    }
    return (0);
}

int group_print(lua_State *L)
{
    lua_settop(L, 1);

    if (Group *group = getGroup(L, 1)) {
        group->print(LOG_LUA);
    }

    return (0);
}

void group_register(lua_State *L)
{
    lua_register(L, "Group", group_create);
    luaL_newmetatable(L, "Group");

    lua_pushcfunction(L, group_delete);
    lua_setfield(L, -2, "__gc");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaLE_addObjectMethods(L, group_functions);

    lua_pop(L, 1);
}
