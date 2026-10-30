#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}
#include "luaIntegration.h"

class Control;
class TouchEvent;
class PotEvent;

/**
 * @brief What happened to a knob, as the script API spells it.
 *
 * The mk2 reads these off a PotEvent, which carries the kind of event with
 * it. This firmware tells them apart by which handler was called, so the
 * caller says which one it is.
 */
enum class LuaPotEvent { touchDown = 1, change = 2, touchUp = 3 };

/**
 * @brief Draws a Custom control by calling the script's paint function.
 *
 * Does nothing when the control has no paint callback, which is what a custom
 * control on a preset with no script is.
 */
void runPaintCallback(const Control &control);

/** Hands a finger on the control to the script. */
void runTouchCallback(const Control &control, const TouchEvent &touchEvent);

/** Hands a knob turn to the script. */
void runPotCallback(const Control &control,
                    const PotEvent &potEvent,
                    LuaPotEvent event);

/** Hands a finger arriving on or leaving a knob to the script. */
void runPotTouchCallback(const Control &control,
                         const PotEvent &potEvent,
                         LuaPotEvent event);

/** Hands the switch in a knob to the script, as a knob event: a press is
 *  type 1, a release type 3. */
void runSwitchCallback(const Control &control, uint8_t potId, bool pressed);

void runFormatter(const char *formatter,
                  const void *object,
                  int16_t value,
                  char *buffer,
                  int maxLength);
void runFunction(const char *function, const void *object, int16_t value);
uint8_t runTemplateFunction(const char *function,
                            const void *object,
                            int16_t value);

/*
 * The preset's own functions - preset.userFunctions in the script.
 *
 * A table of entries named after the knob that offers them, each carrying the
 * name to put on the screen, the function to call, and whether running it
 * should put the window away:
 *
 *     preset.userFunctions = {
 *         pot1 = { name = "INIT", call = function() ... end, close = true },
 *         pot4 = { name = "PANIC", call = panic }
 *     }
 *
 * Nothing is stored for them here. The table belongs to the script, and both
 * of these read it where it lives, so a preset with no script and a preset
 * that declares none cost the same: nothing.
 */

/** @brief How many functions a preset may offer: one per knob. */
constexpr uint8_t numUserFunctions = 12;

/**
 * @brief The name of one function, as the script declares it.
 *
 * @param functionId counted from zero
 * @param buffer where the name is left; emptied when there is no function
 *
 * @return false when the preset declares no function in that place
 */
bool getUserFunctionName(uint8_t functionId, char *buffer, size_t size);

/**
 * @brief Runs one function.
 *
 * @return true when the entry asked for the window that offered it to close
 */
bool runUserFunction(uint8_t functionId);
