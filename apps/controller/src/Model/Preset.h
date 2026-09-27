#pragma once

#include <ArduinoJson.h>
#include <vector>
#include <map>

#include "PersistentStorage.h"
#include "Page.h"
#include "Device.h"
#include "Overlay.h"
#include "Group.h"
#include "Control.h"
#include "Controls.h"
#include "Pool.h"

#include "Rule.h"
#include "Checksum.h"

class Preset
{
public:
    Preset();
    virtual ~Preset() = default;

    bool load(const char *filename);
    void reset(void);

    bool isValid(void) const;
    const char *getName(void) const;
    uint8_t getVersion(void) const;
    const char *getProjectId(void) const;

    Page &getPage(uint8_t pageId);
    const Page &getPage(uint8_t pageId) const;
    const Device &getDevice(uint8_t deviceId) const;
    const Device &getDevice(uint8_t port, uint8_t channel) const;
    Overlay *getOverlay(uint8_t overlayId);
    Group &getGroup(uint16_t groupId);
    const Group &getGroup(uint16_t groupId) const;
    Control &getControl(uint16_t controlId);
    const Control &getControl(uint16_t controlId) const;
    static void
        getPresetName(File &file, char *presetName, size_t maxPresetNameLength);
    static bool getPresetProjectId(File &file,
                                   char *presetProjectId,
                                   size_t maxProjectIdLength);
    uint8_t registerFunction(const char *functionName);
    void print(void) const;

    Control &moveControlToSlot(uint16_t controlId,
                               uint8_t newPageId,
                               uint8_t slotId);

    Device &addDevice(uint8_t deviceId,
                      const char *name,
                      uint8_t port,
                      uint8_t channel);

    Overlay &addOverlay(uint8_t id);

    /**
     * @brief Put a control together from parts, the way the parser does.
     *
     * For a program that is handed its controls rather than reading them
     * from a file - the Electra Link builds a page from what the
     * primary sent. The values and inputs go into the preset's pools and the
     * control is given spans over them, with every back-pointer set, so the
     * result is indistinguishable from a parsed control.
     *
     * @return the stored control, or controlNotFound when there was no room
     */
    Control &addControl(const Control &control,
                        std::vector<ControlValue> &values,
                        const std::vector<Input> &inputs);

    void resetComponents(void);

    /** Give back what the indexes grew into, once the preset is read. */
    void compactControls(void);

public:
    static constexpr uint8_t MaxNameLength = 20;
    static constexpr uint8_t MaxProjectIdLength = 20;
    static constexpr uint8_t MaxNumPages = 12;

    /** How many values a Custom control may declare. It has no fixed count -
     *  its script names them - so there has to be a ceiling somewhere. */
    static constexpr uint8_t MaxCustomValues = 16;
    static constexpr uint16_t MaxNumControls = 432;
    static constexpr uint8_t MaxNumDevices = 32;
    static constexpr uint8_t MaxNumControlSets = 3;
    static constexpr uint8_t MaxNumPots = 12;

    std::vector<std::string> luaFunctions;

private:
    // Main parser
    bool parse(File &file);
    void resetRoot(void);
    void resetControls(void);

    // Root Elements
    bool parseRoot(File &file);
    bool parsePages(File &file);
    bool parseDevices(File &file);
    bool parseOverlays(File &file);
    bool parseGroups(File &file);
    bool parseControls(File &file);

    // Pages
    Page parsePage(JsonObject jPage);

    // Devices
    Device parseDevice(JsonObject jDevice);
    void parsePatches(File &file,
                      size_t startPosition,
                      size_t endPosition,
                      Device *device);
    void parsePatch(File &file,
                    size_t startPosition,
                    size_t endPosition,
                    Device *device);
    void parsePatch(JsonObject jPatchItem, Device *device);
    void parseResponses(File &file,
                        size_t startPosition,
                        size_t endPosition,
                        Device *device);
    void parseMessages(File &file,
                       size_t startPosition,
                       size_t endPosition,
                       Device *device);
    std::vector<uint8_t> parseResponseHeader(JsonArray jResponseHeader);
    std::vector<uint8_t> parseRequest(JsonArray jRequest, uint8_t deviceId);

    // Overlays
    bool parseOverlayItems(File &file, Overlay &overlay);

    // Groups
    Group parseGroup(JsonObject jGroup);

    // Controls
    Control parseControl(JsonObject jControl);

    // Inputs: placed in controlInputs and handed to the control
    void parseInputs(File &file,
                     size_t startPosition,
                     size_t endPosition,
                     Control &control);
    Input
        parseInput(File &file, size_t startPosition, Control::Type controlType);
    Input parseInput(Control::Type controlType, JsonObject jInput);

    // Values: placed in controlValues and handed to the control
    void parseValues(File &file,
                     size_t startPosition,
                     size_t endPosition,
                     Control *control);
    /** @param ordinal the value's position in the array, which is a Custom
     *         control's only way of numbering its values. */
    ControlValue parseValue(File &file,
                            size_t startPosition,
                            Control *control,
                            uint8_t ordinal);
    ControlValue
        parseValue(Control *control, JsonObject jValue, uint8_t ordinal);

    // Messages
    std::vector<uint8_t> *registerData(JsonVariant jData);
    Message parseMessage(JsonObject jMessage, Control::Type controlType);
    std::vector<Rule>
        parseRules(File &file, size_t startPosition, size_t endPosition);
    Rule parseRule(File &file, size_t startPosition);
    Rule parseRule(JsonObject jRule);
    std::vector<uint8_t> parseData(JsonArray jData);
    void transformParameter(std::vector<uint8_t> &data, JsonArray jRules);
    void transformValue(std::vector<uint8_t> &data, JsonArray jRules);
    void transformChecksum(std::vector<uint8_t> &data, JsonVariant jByte);
    void transformFunction(std::vector<uint8_t> &data, JsonVariant jByte);
    // Bounds
    Rectangle parseBounds(File &file, size_t startPosition, size_t endPosition);
    Rectangle parseBounds(JsonArray jBounds);

    /**
     * @brief The {"size","weight","spacing"} object, as the mk2 spells it.
     *
     * A member left out keeps the default, so a preset that names only a size
     * gets the regular proportional face at that size rather than a face
     * assembled out of nothing.
     */
    Control::Font parseFont(JsonObject jFont);

    // Static functions
    static uint8_t translateValueId(Control::Type type, const char *valueId);
    static uint8_t getNumValues(Control::Type type);
    static uint8_t getDefaultbitWidth(Message::Type messageType);
    static bool getPresetNameFast(File &file,
                                  char *presetName,
                                  size_t maxPresetNameLength);

    static uint8_t constrainPageId(uint8_t pageId)
    {
        return (constrain(pageId, 1, MaxNumPages));
    }

    static uint8_t constrainControlSetId(uint8_t controlSetId)
    {
        return (constrain(controlSetId, 1, MaxNumControlSets) - 1);
    }

    static uint8_t constrainPotId(uint8_t potId)
    {
        return (constrain(potId, 1, MaxNumPots) - 1);
    }

    static uint8_t constrainPort(uint8_t port)
    {
        return (constrain(port, 1, 2) - 1);
    }

    static uint8_t constrainChannel(uint8_t channel)
    {
        return (constrain(channel, 1, 16));
    }

    static uint16_t constrainRate(uint16_t rate)
    {
        return (constrain(rate, 10, 1000));
    }

    static uint8_t constrainDeviceId(uint8_t deviceId)
    {
        return (constrain(deviceId, 1, MaxNumDevices));
    }

    uint8_t version;
    char name[MaxNameLength + 1];
    char projectId[MaxProjectIdLength + 1];
    bool valid;

    /* Every incoming channel message asks which device listens on its port
     * and channel. This answers in one lookup instead of a walk through the
     * devices; rebuilt whenever a device is added. */
    const Device *devicesByPortChannel[2][17];

    /* Every value and every input of every control. A control holds a Span
     * into these - see Span.h and Pool.h. */
    Pool<ControlValue, 2048> controlValues;
    Pool<Input, 512> controlInputs;

public: // Public on the purpose
    Pages pages;
    Devices devices;

    /** Rebuild the port/channel index after devices were added. */
    void indexDevices(void);

    Groups groups;
    Controls controls;
    Overlays overlays;

    static Page pageNotFound;
    static Device deviceNotFound;
    static Control controlNotFound;
    static Group groupNotFound;
};
