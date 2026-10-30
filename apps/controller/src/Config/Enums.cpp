#include "Enums.h"
#include <cstring>

ActiveControlSetType translateControlSetType(const char *typeText)
{
    if (typeText) {
        if (strcmp(typeText, "none") == 0) {
            return (ActiveControlSetType::none);
        } else if (strcmp(typeText, "dim") == 0) {
            return (ActiveControlSetType::dim);
        } else if (strcmp(typeText, "bars") == 0) {
            return (ActiveControlSetType::bars);
        } else if (strcmp(typeText, "background") == 0) {
            return (ActiveControlSetType::background);
        }
    }
    return (ActiveControlSetType::dim);
}

ActiveControlSetMode translateControlSetMode(const char *modeText)
{
    if (modeText) {
        if (strcmp(modeText, "local") == 0) {
            return (ActiveControlSetMode::local);
        } else if (strcmp(modeText, "custom") == 0) {
            return (ActiveControlSetMode::custom);
        }
    }
    return (ActiveControlSetMode::global);
}

const char *translateControlSetTypeToText(ActiveControlSetType type)
{
    if (type == ActiveControlSetType::none) {
        return "none";
    } else if (type == ActiveControlSetType::dim) {
        return "dim";
    } else if (type == ActiveControlSetType::bars) {
        return "bars";
    } else if (type == ActiveControlSetType::background) {
        return "background";
    }

    return ("dim");
}
