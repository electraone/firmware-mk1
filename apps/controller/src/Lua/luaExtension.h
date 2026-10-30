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
 * @file luaExtension.h
 *
 * @brief A wrapper for all E1 Lua Extension functionality
 */

#pragma once

#include "Preset.h"
#include "Presets.h"
#include "MainDelegate.h"

#include "luaExtensionBase.h"
#include "luaControl.h"
#include "luaDevice.h"
#include "luaEvents.h"
#include "luaGroup.h"
#include "luaHelpers.h"
#include "luaHooks.h"
#include "luaInfo.h"
#include "luaMessage.h"
#include "luaOverlay.h"
#include "luaPage.h"
#include "luaParameterMap.h"
#include "luaPatch.h"
#include "luaPreset.h"
#include "luaValue.h"

extern Presets *luaPresets;
extern Preset *luaPreset;
extern MainDelegate *luaDelegate;

void loadLuaLibs();
void loadGlobalVariables(lua_State *L);
