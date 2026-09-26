#pragma once

/* What the nine app events used to be is now any command the instrument has -
 * see Api/Commands.inc. The names the configuration carried are all command
 * names, so nothing that was written down has stopped reading. */

enum class ActiveControlSetType { none, dim, bars, background };

ActiveControlSetType translateControlSetType(const char *typeText);
const char *translateControlSetTypeToText(ActiveControlSetType type);
