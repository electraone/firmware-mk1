#pragma once

/* What the nine app events used to be is now any command the instrument has -
 * see Api/Commands.inc. The names the configuration carried are all command
 * names, so nothing that was written down has stopped reading. */

enum class ActiveControlSetType { none, dim, bars, background };

/**
 * @brief Which control set a page opens on, as the mk2 has it.
 *
 * Global keeps the one set across pages, local gives every page its own, which
 * it remembers, and custom uses the default the preset gives each page.
 */
enum class ActiveControlSetMode { global, local, custom };

ActiveControlSetType translateControlSetType(const char *typeText);
ActiveControlSetMode translateControlSetMode(const char *modeText);
const char *translateControlSetTypeToText(ActiveControlSetType type);
