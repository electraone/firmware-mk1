#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}
#include "luaIntegration.h"

void runFormatter(const char *formatter,
                  const void *object,
                  int16_t value,
                  char *buffer,
                  int maxLength);
void runFunction(const char *function, const void *object, int16_t value);
uint8_t runTemplateFunction(const char *function,
                            const void *object,
                            int16_t value);
