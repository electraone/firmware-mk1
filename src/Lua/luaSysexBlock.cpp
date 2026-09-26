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
 * @file luaSysexBlock.cpp
 *
 * @brief An implementation of the SysexBlock object for E1 Lua. 
 */

#include "luaSysexBlock.h"
#include "MemoryPool.h"
#include "luaIntegrationBase.h"
#include <set>

namespace
{
    /**
     * @brief The blocks a script made for itself, and nothing else.
     *
     * A SysexBlock reaches Lua two ways. midi.onSysex() and patch.onResponse()
     * hand over one the firmware owns, which lives on the C++ stack of the
     * handler that is calling in; SysexBlock() makes one on the heap that
     * belongs to the script. Both arrive as the same userdata, so __gc could
     * not tell them apart and freed neither - which leaked one object per
     * SysexBlock() call, without bound, on a device with a quarter of a
     * megabyte.
     *
     * Recording the script's own is what lets __gc free exactly those.
     */
    std::set<SysexBlock *> ownedBlocks;
} // namespace

int sysexBlock_create(lua_State *L)
{
    lua_settop(L, 0);
    SysexBlock *sysexBlock =
        new SysexBlock(App::get()->sysexPool.openMemoryBlock());

    if (sysexBlock) {
        ownedBlocks.insert(sysexBlock);
        luaLE_pushObject(L, "SysexBlock", sysexBlock);
        return (1);
    }

    return (luaL_error(L, "failed: no room left for a new SysexBlock"));
}

int sysexBlock_delete(lua_State *L)
{
    lua_settop(L, 1);

    /* lua_touserdata(), not luaL_checkudata(): this runs as __gc, which is
     * only reached through this metatable, and a metatable lookup during
     * collection is worth avoiding. */
    if (void *userdata = lua_touserdata(L, 1)) {
        SysexBlock *sysexBlock = *reinterpret_cast<SysexBlock **>(userdata);

        if (ownedBlocks.erase(sysexBlock) > 0) {
            delete sysexBlock;
        }
    }

    return (0);
}

void sysexBlock_releaseAll(void)
{
    for (SysexBlock *sysexBlock : ownedBlocks) {
        delete sysexBlock;
    }

    ownedBlocks.clear();
}

int sysexBlock_close(lua_State *L)
{
    lua_settop(L, 1);

    SysexBlock *sysexBlock =
        (*reinterpret_cast<SysexBlock **>(luaL_checkudata(L, 1, "SysexBlock")));

    if (!sysexBlock) {
        return (luaL_error(L, "close: invalid SysexBlock"));
    }

    sysexBlock->close();
    return (0);
}

int sysexBlock_write(lua_State *L)
{
    lua_settop(L, 2);

    SysexBlock *sysexBlock =
        (*reinterpret_cast<SysexBlock **>(luaL_checkudata(L, 1, "SysexBlock")));
    int byte = luaL_checkinteger(L, 2);

    if (!sysexBlock) {
        return (luaL_error(L, "write: invalid SysexBlock"));
    }

    if ((byte < 0) || (byte > 255)) {
        return (luaL_error(L, "write: byte is out of range: value=%d", byte));
    }

    const uint8_t data = (uint8_t)byte;

    sysexBlock->writeBytes(&data, 1);
    return (0);
}

int sysexBlock_read(lua_State *L)
{
    lua_settop(L, 1);

    SysexBlock *sysexBlock =
        (*reinterpret_cast<SysexBlock **>(luaL_checkudata(L, 1, "SysexBlock")));

    if (sysexBlock) {
        lua_pushnumber(L, sysexBlock->read());
        return (1);
    }

    return (luaL_error(L, "read: invalid SysexBlock"));
}

int sysexBlock_seek(lua_State *L)
{
    lua_settop(L, 2);

    SysexBlock *sysexBlock =
        (*reinterpret_cast<SysexBlock **>(luaL_checkudata(L, 1, "SysexBlock")));
    lua_Integer index = luaL_checkinteger(L, 2);

    if (!sysexBlock) {
        return (luaL_error(L, "seek: invalid SysexBlock"));
    }

    /* Signed on purpose: read into a size_t, a negative index became an
     * enormous positive one and was reported as being past the end rather than
     * as the mistake it is. */
    if (index < 1) {
        return (luaL_error(L, "seek: index cannot be 0 or negative"));
    }

    index--;

    if ((size_t)index >= sysexBlock->getLength()) {
        return (luaL_error(L, "seek: index is outside of the SysexBlock"));
    }

    sysexBlock->seek((uint32_t)index);

    return (0);
}

int sysexBlock_peek(lua_State *L)
{
    lua_settop(L, 2);

    SysexBlock *sysexBlock =
        (*reinterpret_cast<SysexBlock **>(luaL_checkudata(L, 1, "SysexBlock")));
    lua_Integer index = luaL_checkinteger(L, 2);

    if (!sysexBlock) {
        return (luaL_error(L, "peek: invalid SysexBlock"));
    }

    if (index < 1) {
        return (luaL_error(L, "peek: index cannot be 0 or negative"));
    }

    index--;

    if ((size_t)index >= sysexBlock->getLength()) {
        return (luaL_error(L, "peek: index is outside of the SysexBlock"));
    }

    int byte = sysexBlock->peek((size_t)index);

    lua_pushnumber(L, byte);

    return (1);
}

/**
 * @brief sysexBlock:poke(position, byte)
 *
 * Replaces one byte of a block already stored, counted from one as peek()
 * counts. What it is for is answering a device's response with a message built
 * from it - patching a device number or a checksum - without copying the whole
 * message out into a Lua table and back.
 */
int sysexBlock_poke(lua_State *L)
{
    lua_settop(L, 3);

    SysexBlock *sysexBlock =
        (*reinterpret_cast<SysexBlock **>(luaL_checkudata(L, 1, "SysexBlock")));
    lua_Integer index = luaL_checkinteger(L, 2);
    int byte = luaL_checkinteger(L, 3);

    if (!sysexBlock) {
        return (luaL_error(L, "poke: invalid SysexBlock"));
    }

    if (index < 1) {
        return (luaL_error(L, "poke: index cannot be 0 or negative"));
    }

    index--;

    if ((size_t)index >= sysexBlock->getLength()) {
        return (luaL_error(L, "poke: index is outside of the SysexBlock"));
    }

    if ((byte < 0) || (byte > 255)) {
        return (luaL_error(L, "poke: byte is out of range: value=%d", byte));
    }

    lua_pushboolean(L, sysexBlock->poke((size_t)index, (uint8_t)byte));

    return (1);
}

int sysexBlock_getLength(lua_State *L)
{
    lua_settop(L, 1);

    SysexBlock *sysexBlock =
        (*reinterpret_cast<SysexBlock **>(luaL_checkudata(L, 1, "SysexBlock")));

    if (sysexBlock) {
        lua_pushnumber(L, sysexBlock->getLength());
        return (1);
    }

    return (luaL_error(L, "getLength: invalid SysexBlock"));
}

int sysexBlock_getManufacturerSysexId(lua_State *L)
{
    lua_settop(L, 1);

    SysexBlock *sysexBlock =
        (*reinterpret_cast<SysexBlock **>(luaL_checkudata(L, 1, "SysexBlock")));

    if (sysexBlock) {
        lua_pushnumber(L, sysexBlock->getManufacturerSysexId());
        return (1);
    }

    return (luaL_error(L, "getManufacturerSysexId: invalid SysexBlock"));
}

int sysexBlock_print(lua_State *L)
{
    lua_settop(L, 1);

    SysexBlock *sysexBlock =
        (*reinterpret_cast<SysexBlock **>(luaL_checkudata(L, 1, "SysexBlock")));

    if (!sysexBlock) {
        return (luaL_error(L, "print: invalid SysexBlock"));
    }

    sysexBlock->print(LOG_LUA);

    return (0);
}

void sysexBlock_register(lua_State *L)
{
    lua_register(L, "SysexBlock", sysexBlock_create);
    luaL_newmetatable(L, "SysexBlock");

    lua_pushcfunction(L, sysexBlock_delete);
    lua_setfield(L, -2, "__gc");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaLE_addObjectMethods(L, sysexBlock_functions);

    lua_pop(L, 1);
}
