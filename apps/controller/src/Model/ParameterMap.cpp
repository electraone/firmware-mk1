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

#include "ParameterMap.h"
#include "ParameterMapWindow.h"
#include "ControlComponent.h"
#include "JsonTools.h"
#include "luaExtension.h"

#pragma GCC optimize("O0")

// Global pointer to current ParameterMap instance. \todo get rid of that.
ParameterMap parameterMap;

ParameterMap::ParameterMap()
    : lastRead(nullptr), lastReadHash(0), enabled(false), onReadyPending(false)
{
    memset(projectId, 0x00, sizeof(projectId));
}

void ParameterMap::setProjectId(const char *newProjectId)
{
    copyString(projectId, newProjectId, 20);
}

void ParameterMap::setAppSandbox(const char *newAppSandbox)
{
    copyString(appSandbox, newAppSandbox, 20);
}

LookupEntry *ParameterMap::getAndCache(uint32_t hash)
{
    if (lastRead && (lastReadHash == hash)) {
        return (lastRead);
    }
    auto it = entries.find(hash);
    if (it != entries.end()) {
        lastRead = &(it->second);
        lastReadHash = hash;
        return &(it->second);
    }
    return (nullptr);
}

LookupEntry *ParameterMap::getOrCreate(uint8_t deviceId,
                                       Message::Type type,
                                       uint16_t parameterNumber,
                                       ControlValue *controlValue)
{
    uint32_t hash = calculateHash(deviceId, type, parameterNumber);

    auto insertResult = entries.emplace(hash, LookupEntry());

    if (controlValue) {
        insertResult.first->second.addDestination(controlValue);
    }

    lastRead = &(insertResult.first->second);
    lastReadHash = hash;

    return &(insertResult.first->second);
}

LookupEntry *ParameterMap::get(uint8_t deviceId,
                               Message::Type type,
                               uint16_t parameterNumber)
{
    return (getAndCache(calculateHash(deviceId, type, parameterNumber)));
}

uint16_t ParameterMap::getValue(uint8_t deviceId,
                                Message::Type type,
                                uint16_t parameterNumber)
{
    LookupEntry *entry =
        getAndCache(calculateHash(deviceId, type, parameterNumber));

    if (entry) {
        return (entry->getMidiValue());
    }
    return (MIDI_VALUE_DO_NOT_SEND);
}

LookupEntry *ParameterMap::setValue(LookupEntry *entry,
                                    uint16_t midiValue,
                                    Origin origin)
{
    if (entry) {
        if (entry->setMidiValue(midiValue)) {
            if (entry->hasValidMidiValue()) {
                if (onChange) {
                    onChange(entry, origin);
                }
            }
            postEntry(entry);
        }
    }
    return (entry);
}

LookupEntry *ParameterMap::setValue(uint8_t deviceId,
                                    Message::Type type,
                                    uint16_t parameterNumber,
                                    uint16_t midiValue,
                                    Origin origin)
{
#ifdef DEBUG
    System::logger.write(
        LOG_TRACE,
        "ParameterMap::setValue: deviceId=%d, type=%d, parameterNumber=%d, "
        "midiValue=%d, origin=%d",
        deviceId,
        type,
        parameterNumber,
        midiValue,
        origin);
#endif

    // These messages are not device specific
    if (type == Message::Type::start || type == Message::Type::stop
        || type == Message::Type::tune) {
        deviceId = 0xff;
    }

    LookupEntry *entry =
        getAndCache(calculateHash(deviceId, type, parameterNumber));

    if (entry) {
        setValue(entry, midiValue, origin);
    }
    return (entry);
}

LookupEntry *ParameterMap::setValueSimple(uint8_t deviceId,
                                          Message::Type type,
                                          uint16_t parameterNumber,
                                          uint16_t midiValue)
{
#ifdef DEBUG
    System::logger.write(
        LOG_TRACE,
        "ParameterMap::setValue: deviceId=%d, type=%d, parameterNumber=%d, "
        "midiValue=%d, origin=%d",
        deviceId,
        type,
        parameterNumber,
        midiValue,
        origin);
#endif

    // These messages are not device specific
    if (type == Message::Type::start || type == Message::Type::stop
        || type == Message::Type::tune) {
        deviceId = 0xff;
    }

    LookupEntry *entry =
        getAndCache(calculateHash(deviceId, type, parameterNumber));

    if (entry) {
        entry->setMidiValue(midiValue);
    }
    return (entry);
}

LookupEntry *ParameterMap::modulateValue(uint8_t deviceId,
                                         Message::Type type,
                                         uint16_t parameterNumber,
                                         float modulationValue,
                                         int8_t depth)
{
    LookupEntry *entry =
        getAndCache(calculateHash(deviceId, type, parameterNumber));

    /** @todo this still needs a bit of extra thinking */
    if (entry) {
        uint16_t midiValue = entry->getMidiValue() + (depth * modulationValue);
        Message message = entry->getMessage();

        midiValue =
            constrain(midiValue, message.getMidiMin(), message.getMidiMax());

        LookupEntry tmpEntry = *entry;
        tmpEntry.setMidiValue(midiValue);
        if (onChange) {
            onChange(&tmpEntry, Origin::mods);
        }
    }
    return (entry);
}

LookupEntry *ParameterMap::setRelative(uint8_t deviceId,
                                       Message::Type type,
                                       uint16_t parameterNumber,
                                       uint16_t midiValue,
                                       Origin origin)
{
#ifdef DEBUG
    System::logger.write(
        LOG_TRACE,
        "ParameterMap::setRelative: deviceId=%d, type=%d, parameterNumber=%d, "
        "midiValue=%d, origin=%d",
        deviceId,
        type,
        parameterNumber,
        midiValue,
        origin);
#endif

    LookupEntry *entry =
        getAndCache(calculateHash(deviceId, type, parameterNumber));

    if (entry) {
        entry->setRelativeMidiValue(midiValue);

        if (onChange) {
            onChange(entry, origin);
        }
        entry->resetMidiValue();
    }
    return (entry);
}

LookupEntry *ParameterMap::applyToValue(uint8_t deviceId,
                                        Message::Type type,
                                        uint16_t parameterNumber,
                                        uint16_t midiValueFragment,
                                        Origin origin)
{
    LookupEntry *entry =
        getAndCache(calculateHash(deviceId, type, parameterNumber));

    if (entry && (midiValueFragment != 0)) {
        uint16_t originalMidiValue = entry->getMidiValue();
        entry->applyToMidiValue(midiValueFragment);

        if (entry->getMidiValue() != originalMidiValue) {
            if (onChange) {
                onChange(entry, origin);
            }
            postEntry(entry);
        }
    }
    return (entry);
}

bool ParameterMap::addDestination(Message *message)
{
    bool added = false;
    LookupEntry *entry = getOrCreate(message->getDeviceId(),
                                     message->getType(),
                                     message->getParameterNumber());
    if (entry) {
        added = entry->addDestination(message->getControlValue());
    }
    return (added);
}

bool ParameterMap::removeDestination(Message *message)
{
    bool removed = false;
    LookupEntry *entry = get(message->getDeviceId(),
                             message->getType(),
                             message->getParameterNumber());
    if (entry) {
        removed = entry->removeDestination(message->getControlValue());
    }
    return (removed);
}

bool ParameterMap::removeDestination(ControlValue *value)
{
    bool removed = false;
    Message *message = &value->message;
    LookupEntry *entry = get(message->getDeviceId(),
                             message->getType(),
                             message->getParameterNumber());
    if (entry) {
        removed = entry->removeDestination(value);
    }
    return (removed);
}

void ParameterMap::resetDeviceValues(uint8_t deviceId)
{
    for (auto &[hash, lookupEntry] : entries) {
        if ((hash >> 24) == deviceId) {
            if (getType(hash) != Message::Type::none) {
                lookupEntry.resetMidiValue();
            }
        }
    }
    postRepaint();
}

void ParameterMap::clear(void)
{
    setProjectId("undefined");

    for (auto &[hash, lookupEntry] : entries) {
        lookupEntry.removeAllDestinations();
    }
    entries.clear();

    lastRead = nullptr;
}

void ParameterMap::print(uint8_t logLevel)
{
    System::logger.write(logLevel,
                         "--[Parameter Map]---------------------------------");
    for (auto &[hash, entry] : entries) {
        if (getType(hash) != Message::Type::none) {
            System::logger.write(
                logLevel,
                "ParameterMap entry: deviceId=%d, type=%d, parameterNumber=%d,"
                " midiValue=%d, hasDestinations=%d",
                getDeviceId(hash),
                getType(hash),
                getParameterNumber(hash),
                entry.getMidiValue(),
                entry.hasDestinations());
            for (auto md : entry.getDestinations()) {
                System::logger.write(
                    logLevel,
                    "    - linked control: name=%s, "
                    "value handle=%s, "
                    "display value=%d",
                    md->getControl()->getName(),
                    md->getId(),
                    md->translateMidiValue(entry.getMidiValue()));
            }
        }
    }
    System::logger.write(logLevel, "--");
}

void ParameterMap::keep(void)
{
    createMapsDir();

    char mapStateFilename[MAX_FILENAME_LENGTH + 1];
    prepareMapStateFilename(mapStateFilename, MAX_FILENAME_LENGTH);
    System::logger.write(
        LOG_INFO, "ParameterMap::keep: filename=%s", mapStateFilename);
    save(mapStateFilename);
}

void ParameterMap::save(const char *filename)
{
    File file = Hardware::sdcard.createOutputStream(
        filename, FILE_WRITE | O_CREAT | O_TRUNC);

    if (!file) {
        System::logger.write(
            LOG_ERROR,
            "ParameterMap::serialize: cannot open the patch file for writing");
        return;
    }

    file.print("{");
    serializeRoot(file);
    serializeMap(file);
    file.print("}");

    file.close();
}

void ParameterMap::serializeMap(File &file)
{
    bool firstRecord = true;

    file.print(",\"parameters\":[");

    for (const auto &[hash, entry] : entries) {
        const auto deviceId = getDeviceId(hash);
        const auto messageType = getType(hash);
        const auto parameterNumber = getParameterNumber(hash);
        const auto midiValue = entry.getMidiValue();

        if (messageType != Message::Type::none
            && midiValue != MIDI_VALUE_DO_NOT_SEND) {
            if (!firstRecord) {
                file.print(",");
            } else {
                firstRecord = false;
            }

            file.print("{\"deviceId\":");
            file.print(deviceId);
            file.print(",\"messageType\":");
            file.print(messageType);
            file.print(",\"parameterNumber\":");
            file.print(parameterNumber);
            file.print(",\"midiValue\":");
            file.print(midiValue);
            file.print("}");

            System::logger.write(
                LOG_TRACE,
                "ParameterMap::serializeMap: entry: deviceI=%d, type=%d, "
                "parameterNumber=%d, midiValue=%d",
                deviceId,
                messageType,
                parameterNumber,
                midiValue);
        }
    }
    file.print("]");
}

void ParameterMap::serializeRoot(File &file)
{
    file.print("\"version\":1,");
    file.print("\"projectId\":\"");
    file.print(projectId);
    file.print("\"");
}

bool ParameterMap::recall(void)
{
    bool status = false;
    char mapStateFilename[MAX_FILENAME_LENGTH + 1];
    prepareMapStateFilename(mapStateFilename, MAX_FILENAME_LENGTH);
    System::logger.write(
        LOG_INFO, "ParameterMap::recall: filename=%s", mapStateFilename);
    if (Hardware::sdcard.exists(mapStateFilename)) {
        status = load(mapStateFilename);
    }
    return (status);
}

void ParameterMap::forget(void)
{
    char mapStateFilename[MAX_FILENAME_LENGTH + 1];
    prepareMapStateFilename(mapStateFilename, MAX_FILENAME_LENGTH);
    if (Hardware::sdcard.deleteFile(mapStateFilename)) {
        System::logger.write(LOG_ERROR,
                             "ParameterMap::forget: cannot remove file: %s",
                             mapStateFilename);
    }
}

bool ParameterMap::load(const char *filename)
{
    System::logger.write(
        LOG_INFO, "ParameterMap::load: file: filename=%s", filename);

    File file = Hardware::sdcard.createInputStream(filename);

    if (!file) {
        System::logger.write(LOG_ERROR,
                             "ParameterMap::load: cannot open setup file: %s",
                             filename);
        return (false);
    }

    file.setTimeout(100);

    if (!parse(file)) {
        System::logger.write(
            LOG_ERROR,
            "ParameterMap::load: cannot parse setup: filename=%s",
            filename);
        file.close();
        return (false);
    } else {
        file.close();
    }

    return (true);
}

bool ParameterMap::parse(File &file)
{
    parseParameters(file);
    return (true);
}

bool ParameterMap::parseParameters(File &file)
{
    const size_t capacity = JSON_OBJECT_SIZE(3) + 512;
    StaticJsonDocument<capacity> doc;

    uint8_t deviceId;
    uint8_t messageType;
    uint16_t parameterNumber;
    uint16_t midiValue;

    if (findElement(file, "\"parameters\"", ARRAY) == false) {
        System::logger.write(
            LOG_ERROR,
            "ParameterMap::parseParameters: parameters array not found");
        return (false);
    }

    do {
        DeserializationError err = deserializeJson(doc, file);

        if (err && (err.code() != DeserializationError::InvalidInput)) {
            System::logger.write(
                LOG_TRACE,
                "ParameterMap::parseParameters: deserializeJson() failed: %s",
                err.c_str());
            return (false);
        }

        JsonObject item = doc.as<JsonObject>();

        if (item) {
            deviceId = item["deviceId"].as<uint8_t>();
            messageType = item["messageType"].as<uint8_t>();
            parameterNumber = item["parameterNumber"].as<uint16_t>();
            midiValue = item["midiValue"].as<uint16_t>();

            setValue(deviceId,
                     (Message::Type)messageType,
                     parameterNumber,
                     midiValue,
                     Origin::file);

            System::logger.write(
                LOG_TRACE,
                "ParameterMap::parseParameters: deviceId=%d, messageType=%d, "
                "parameterNumber=%d, midiValue=%d",
                deviceId,
                messageType,
                parameterNumber,
                midiValue);
        } else {
            break;
        }
    } while (file.findUntil(",", "]"));

    return (true);
}

void ParameterMap::addWindow(ParameterMapWindow *windowToAdd)
{
    System::logger.write(LOG_TRACE,
                         "ParameterMap::addWindow: window=%s, address=%x",
                         windowToAdd->getName(),
                         windowToAdd);
    windows.push_back(windowToAdd);
}

void ParameterMap::removeWindow(ParameterMapWindow *windowToRemove)
{
    System::logger.write(LOG_TRACE,
                         "ParameterMap::removeWindow: window=%s, address=%x",
                         windowToRemove->getName(),
                         windowToRemove);

    auto it = std::remove(windows.begin(), windows.end(), windowToRemove);

    if (it != windows.end()) {
        System::logger.write(LOG_TRACE,
                             "ParameterMap::removeWindow: window removed");
        windows.erase(it, windows.end());
    }
}

void ParameterMap::listWindows(void)
{
    uint8_t index = 0;

    for (const auto &window : windows) {
        System::logger.write(
            LOG_TRACE,
            "ParameterMap::listWindows: index=%d, name=%s, address=%x",
            index,
            window->getName(),
            window);
        index++;
    }
}

void ParameterMap::enable(bool newPresetLoaded)
{
    if (newPresetLoaded) {
        onReadyPending = true;
    }
    InstanceCallback<void(void)>::callbackFunction =
        std::bind(&ParameterMap::repaintParameterMap, this);
    TaskCallback repaintTaskCallback =
        static_cast<TaskCallback>(InstanceCallback<void(void)>::callback);

    System::tasks.addTask(repaintParameterMapTask);
    repaintParameterMapTask.set(25000, TASK_FOREVER, repaintTaskCallback);
    repaintParameterMapTask.enable();
}

void ParameterMap::disable(void)
{
    System::tasks.disableRepaintGraphics();
    System::tasks.deleteTask(repaintParameterMapTask);
    System::tasks.clearRepaintGraphics();
    System::tasks.enableRepaintGraphics();
}

void ParameterMap::scheduleLuaProcessing(void)
{
    for (auto &[hash, mapEntry] : entries) {
        for (auto &messageDestination : mapEntry.getDestinations()) {
            if (messageDestination->hasLua()) {
                if (messageDestination->message.getType()
                    == Message::Type::none) {
                    mapEntry.markForRepaintWithoutFunction();
                } else {
                    mapEntry.markForFullRepaint();
                }
            }
        }
    }
}

void ParameterMap::repaintParameterMap(void)
{
    for (auto &[hash, mapEntry] : entries) {
        if (mapEntry.isDirty() && mapEntry.hasValidMidiValue()) {
            System::logger.write(
                LOG_TRACE,
                "repaintParameterMap: dirty entry found: device=%d, type=%d, "
                "parameterNumber=%d, midiValue=%d",
                getDeviceId(hash),
                getType(hash),
                getParameterNumber(hash),
                mapEntry.getMidiValue());
            repaintLookupEntry(&mapEntry);
        }
    }

    if (onReadyPending) {
        if (L) {
            preset_onReady();
        }
        onReadyPending = false;
    }
}

void ParameterMap::repaintLookupEntry(LookupEntry *mapEntry)
{
    for (auto &messageDestination : mapEntry->getDestinations()) {
        if (mapEntry->hasValidMidiValue()) {
            // Process User Lua callbacks assigned to the ControlValue
            if (mapEntry->isForFullRepaint()
                && messageDestination->isFunctionAssigned()) {
                messageDestination->callFunction(
                    messageDestination->translateMidiValue(
                        mapEntry->getMidiValue()));
            }
            if (messageDestination->isFormatterAssigned()) {
                messageDestination->callFormatter(
                    messageDestination->translateMidiValue(
                        mapEntry->getMidiValue()));
            }
        }

        // Repaint affected Controls that are part of active windows
        for (const auto &window : windows) {
            Component *rc = window->getOwnedContent();
            Component *c =
                rc->findChildById(messageDestination->getControl()->getId());

            if (c) {
                System::logger.write(
                    LOG_TRACE,
                    "repaintParameterMap: repainting component: "
                    "component: %s, controlId=%d, valueId=%s",
                    c->getName(),
                    messageDestination->getControl()->getId(),
                    messageDestination->getId());

                // @todo get rid of dynamic_cast. Replace it with polymorphism.
                ControlComponent *cc = dynamic_cast<ControlComponent *>(c);

                if (cc) {
                    cc->onMidiValueChange(*messageDestination,
                                          mapEntry->getMidiValue(),
                                          messageDestination->getHandle());
                }
            }
        }
    }

    mapEntry->markAsProcessed();
}

void ParameterMap::prepareMapStateFilename(char *buffer, size_t maxLength)
{
    snprintf(buffer, maxLength, "%s/maps/%s.map", appSandbox, projectId);
}

void ParameterMap::postEntry(LookupEntry *entry)
{
    if (enabled) {
        postMessage(entry, RepaintLookupEntry);
    }
}

void ParameterMap::postRepaint(void)
{
    if (enabled) {
        postMessage(nullptr, RepaintParameterMap);
    }
}

void ParameterMap::postMessage(LookupEntry *entry,
                               [[maybe_unused]] RepaintAction repaintAction)
{
}

bool ParameterMap::createMapsDir(void)
{
    char dirname[MAX_DIRNAME_LENGTH + 1];
    snprintf(dirname, MAX_DIRNAME_LENGTH, "%s/maps", appSandbox);

    if (Hardware::sdcard.directoryExists(dirname) == false) {
        if (Hardware::sdcard.createDirectory(dirname) == false) {
            System::logger.write(
                LOG_ERROR,
                "Snapshots::createSnapshotDir: cannot create snaps directory: %s",
                dirname);
            return (false);
        }
    }
    return (true);
}

inline uint32_t ParameterMap::calculateHash(uint8_t deviceId,
                                            Message::Type type,
                                            uint16_t parameterNumber)
{
    return (parameterNumber + (((uint8_t)type) << 16) + (deviceId << 24));
}

inline uint8_t ParameterMap::getDeviceId(uint32_t hash)
{
    return (hash >> 24);
}

inline uint8_t ParameterMap::getType(uint32_t hash)
{
    return ((hash >> 16) & 0xFF);
}

inline uint16_t ParameterMap::getParameterNumber(uint32_t hash)
{
    return (hash & 0xFFFF);
}