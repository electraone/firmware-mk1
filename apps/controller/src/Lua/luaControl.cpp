#include "luaControl.h"
#include "luaValue.h"
#include "Preset.h"
#include "MainDelegate.h"

extern Preset *luaPreset;
extern MainDelegate *luaDelegate;

static Control *getControl(lua_State *L, uint8_t stackPosition)
{
    return (*reinterpret_cast<Control **>(
        luaL_checkudata(L, stackPosition, "Control")));
}

int luaopen_controls(lua_State *L)
{
    luaL_newlib(L, controls_functions);
    return 1;
}

int control_create(lua_State *L)
{
    lua_settop(L, 1);
    int controlId = luaLE_checkControlId(L, 1);

    const Control &control = luaPreset->getControl(controlId);

    if (control.isValid()) {
        luaLE_pushObject(L, "Control", &control);
        return (1);
    }
    return (luaL_error(L, "failed: control %d does not exist", controlId));
}

int control_delete(lua_State *L)
{
    lua_settop(L, 1);
    return (0);
}

int control_getId(lua_State *L)
{
    int id = 0;
    lua_settop(L, 1);

    if (Control *control = getControl(L, 1)) {
        id = control->getId();
        lua_pushinteger(L, id);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid control"));
}

int control_setVisible(lua_State *L)
{
    lua_settop(L, 2);
    Control *control = getControl(L, 1);
    bool shouldBeVisible = luaLE_checkBoolean(L, 2);

    if (control) {
        luaDelegate->setControlVisible(control->getId(), shouldBeVisible);
    } else {
        return (luaL_error(L, "failed: not a valid control"));
    }
    return (0);
}

int control_isVisible(lua_State *L)
{
    lua_settop(L, 1);
    bool state = false;

    if (Control *control = getControl(L, 1)) {
        state = control->isVisible();
        lua_pushboolean(L, state);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid control"));
}

int control_setName(lua_State *L)
{
    lua_settop(L, 2);
    Control *control = getControl(L, 1);
    const char *name = luaL_checkstring(L, 2);

    if (control) {
        luaDelegate->setControlName(control->getId(), name);
    } else {
        return (luaL_error(L, "failed: not a valid control"));
    }
    return (0);
}

int control_getName(lua_State *L)
{
    lua_settop(L, 1);
    const char *name = "";

    if (Control *control = getControl(L, 1)) {
        name = control->getName();
        lua_pushstring(L, name);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid control"));
}

int control_setColour(lua_State *L)
{
    lua_settop(L, 2);
    Control *control = getControl(L, 1);
    int colour = luaL_checkinteger(L, 2);

    if (control) {
        luaDelegate->setControlColour(control->getId(), colour);
    } else {
        return (luaL_error(L, "failed: not a valid control"));
    }
    return (0);
}

int control_getColour(lua_State *L)
{
    lua_settop(L, 1);
    int colour = 0;

    if (Control *control = getControl(L, 1)) {
        colour = control->getColour();
        lua_pushinteger(L, colour);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid control"));
}

int control_setPot(lua_State *L)
{
    lua_settop(L, 3);

    Control *control = getControl(L, 1);
    int controlSet = luaL_checkinteger(L, 2);
    int potId = luaL_checkinteger(L, 3);

    luaL_argcheck(L,
                  1 <= controlSet && controlSet <= 3,
                  2,
                  "failed: controlSet must be between 1 and 3");

    luaL_argcheck(L,
                  1 <= potId && potId <= 12,
                  3,
                  "failed: potId must be between 1 and 12");

    if (control) {
        luaDelegate->setControlPot(control->getId(), controlSet, potId);
    } else {
        return (luaL_error(L, "failed: not a valid control"));
    }
    return (0);
}

int control_setBounds(lua_State *L)
{
    lua_settop(L, 2);

    Control *control = getControl(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    lua_rawgeti(L, 2, 1);
    lua_rawgeti(L, 2, 2);
    lua_rawgeti(L, 2, 3);
    lua_rawgeti(L, 2, 4);
    int x = luaL_checkinteger(L, -4);
    int y = luaL_checkinteger(L, -3);
    int width = luaL_checkinteger(L, -2);
    int height = luaL_checkinteger(L, -1);

    if (control) {
        luaDelegate->setControlBounds(control->getId(),
                                      Rectangle(x, y, width, height));
    } else {
        return (luaL_error(L, "failed: not a valid control"));
    }
    return (0);
}

int control_getBounds(lua_State *L)
{
    lua_settop(L, 1);
    Rectangle bounds;

    if (Control *control = getControl(L, 1)) {
        bounds = control->getBounds();

        lua_newtable(L);
        luaLE_pushArrayInteger(L, 1, bounds.getX());
        luaLE_pushArrayInteger(L, 2, bounds.getY());
        luaLE_pushArrayInteger(L, 3, bounds.getWidth());
        luaLE_pushArrayInteger(L, 4, bounds.getHeight());

        return (1);
    }
    return (luaL_error(L, "failed: not a valid control"));
}

int control_setSlot(lua_State *L)
{
    Control *control = getControl(L, 1);
    int slot = luaL_checkinteger(L, 2);

    luaL_argcheck(
        L, 1 <= slot && slot <= 36, 2, "failed: slot must be between 1 and 36");

    // Get the optional pageId parameter, or use a default value if not provided
    int pageId = lua_isnoneornil(L, 3) ? 0 : luaL_checkinteger(L, 3);

    if (control) {
        if (pageId) {
            luaDelegate->setControlSlot(control->getId(), pageId, slot);
        } else {
            luaDelegate->setControlSlot(control->getId(), slot);
        }
    } else {
        return (luaL_error(L, "failed: not a valid control"));
    }
    return (0);
}

int control_getValueIds(lua_State *L)
{
    lua_settop(L, 1);

    if (Control *control = getControl(L, 1)) {
        uint8_t i = 1;
        lua_newtable(L);

        for (auto &value : control->values) {
            luaLE_pushArrayString(L, i, value.translateId(value.getHandle()));
            i++;
        }
        return (1);
    }
    return (luaL_error(L, "failed: not a valid control"));
}

int control_getValue(lua_State *L)
{
    int numParameters = lua_gettop(L);
    if (numParameters > 2) {
        lua_settop(L, 2);
    }
    Control *control = getControl(L, 1);
    const char *valueId =
        (numParameters == 2) ? luaL_checkstring(L, 2) : "value";

    if (control) {
        const ControlValue &value = control->getValueByValueId(valueId);
        luaLE_pushObject(L, "ControlValue", &value);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid control"));
}

int control_getValues(lua_State *L)
{
    lua_settop(L, 1);

    if (Control *control = getControl(L, 1)) {
        uint8_t i = 1;
        lua_newtable(L);

        for (auto &value : control->values) {
            luaLE_pushArrayObject(L, i, "ControlValue", &value);
            i++;
        }
        return (1);
    }
    return (luaL_error(L, "failed: not a valid control"));
}

int control_print(lua_State *L)
{
    lua_settop(L, 1);

    if (Control *control = getControl(L, 1)) {
        control->print(LOG_ERROR);
    }

    return (0);
}

void control_register(lua_State *L)
{
    lua_register(L, "Control", control_create);
    luaL_newmetatable(L, "Control");

    lua_pushcfunction(L, control_delete);
    lua_setfield(L, -2, "__gc");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaLE_addObjectMethods(L, control_functions);

    lua_pop(L, 1);
}
