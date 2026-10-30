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

#include "luaPage.h"
#include "Preset.h"
#include "MainDelegate.h"

extern Preset *luaPreset;
extern MainDelegate *luaDelegate;

static Page *getPage(lua_State *L, uint8_t stackPosition)
{
    return (
        *reinterpret_cast<Page **>(luaL_checkudata(L, stackPosition, "Page")));
}

int luaopen_pages(lua_State *L)
{
    luaL_newlib(L, pages_functions);
    return 1;
}

int pages_get(lua_State *L)
{
    lua_settop(L, 1);
    int pageId = luaLE_checkPageId(L, 1);

    const Page &page = luaPreset->getPage(pageId);

    if (page.isValid()) {
        luaLE_pushObject(L, "Page", &page);
        return (1);
    }
    return (luaL_error(L, "failed: page %d does not exist", pageId));
}

int pages_getActive(lua_State *L)
{
    lua_settop(L, 0);

    int pageId = luaDelegate->getCurrentPageId();
    const Page &page = luaPreset->getPage(pageId);

    if (page.isValid()) {
        luaLE_pushObject(L, "Page", &page);
        return (1);
    }
    return (luaL_error(L, "failed: page %d does not exist", pageId));
}

int pages_display(lua_State *L)
{
    int numParameters = lua_gettop(L);
    int pageId = 1;
    int controlSetId = 0;

    if (numParameters > 0) {
        pageId = luaLE_checkPageId(L, 1);
    }

    if (numParameters > 1) {
        controlSetId = luaLE_checkControlSetId(L, 2);
    }

    luaDelegate->switchPage(pageId, controlSetId);
    return (0);
}

int pages_setActiveControlSet(lua_State *L)
{
    lua_settop(L, 1);
    int controlSetId = luaLE_checkControlSetId(L, 1);
    controlSetId--;
    luaDelegate->switchControlSet(controlSetId);
    return (0);
}

int pages_getActiveControlSet(lua_State *L)
{
    lua_settop(L, 0);
    lua_pushinteger(L, luaDelegate->getCurrentControlSetId() + 1);

    return (1);
}

int page_delete(lua_State *L)
{
    lua_settop(L, 1);
    return (0);
}

int page_getId(lua_State *L)
{
    lua_settop(L, 1);
    int id = 0;

    if (Page *page = getPage(L, 1)) {
        id = page->getId();
        lua_pushinteger(L, id);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid page"));
}

int page_setName(lua_State *L)
{
    lua_settop(L, 2);

    Page *page = getPage(L, 1);
    const char *name = luaL_checkstring(L, 2);

    if (page) {
        luaDelegate->setPageName(page->getId(), name);
    } else {
        return (luaL_error(L, "failed: not a valid page"));
    }
    return (0);
}

int page_getName(lua_State *L)
{
    lua_settop(L, 1);

    if (Page *page = getPage(L, 1)) {
        lua_pushstring(L, page->getName());
        return (1);
    }
    return (luaL_error(L, "failed: not a valid page"));
}

int page_print(lua_State *L)
{
    lua_settop(L, 1);

    if (Page *page = getPage(L, 1)) {
        page->print(LOG_LUA);
    }

    return (0);
}

void page_register(lua_State *L)
{
    lua_register(L, "Page", pages_get);
    luaL_newmetatable(L, "Page");

    lua_pushcfunction(L, page_delete);
    lua_setfield(L, -2, "__gc");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaLE_addObjectMethods(L, page_functions);

    lua_pop(L, 1);
}

void pages_onChange(uint8_t newPageId, uint8_t oldPageId)
{
    const char *function = "onChange";
    luaLE_getModuleFunction("pages", function);

    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, newPageId);
        lua_pushnumber(L, oldPageId);
        if (lua_pcall(L, 2, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
    luaLE_postFunctionCleanUp(L);
}
