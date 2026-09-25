#include "Presets.h"
#include "MidiOutput.h"
#include "MidiCallbacks.h"
#include "luaExtension.h"

#pragma GCC optimize("O0")

Presets::Presets(const char *newAppSandbox,
                 const bool &shouldKeepPresetState,
                 const bool &shouldLoadPresetStateOnStartup)
    : appSandbox(newAppSandbox),
      currentSlot(0),
      currentBankNumber(0),
      pendingSlot(0),
      pendingBankNumber(0),
      presetChangePending(false),
      readyForPresetSwitch(true),
      keepPresetState(shouldKeepPresetState),
      loadPresetStateOnStartup(shouldLoadPresetStateOnStartup)
{
}

void Presets::assignPresetNames(void)
{
    for (uint16_t i = 0; i < NumSlots; i++) {
        char filename[MAX_FILENAME_LENGTH + 1];
        snprintf(filename, MAX_FILENAME_LENGTH, "%s/p%03d.epr", appSandbox, i);

        if (File file = Hardware::sdcard.createInputStream(filename)) {
            char presetName[Preset::MaxNameLength + 1];
            Preset::getPresetName(file, presetName, Preset::MaxNameLength);
            presetSlot[i].setPresetName(presetName);

            char projectId[Preset::MaxProjectIdLength + 1];
            Preset::getPresetProjectId(
                file, projectId, Preset::MaxProjectIdLength);
            presetSlot[i].setProjectId(projectId);
            System::logger.write(LOG_ERROR,
                                 "setting a preset name: %s, id=%d",
                                 presetSlot[i].getPresetName(),
                                 i);
            file.close();
        }
    }
}

void Presets::sendList(uint8_t port)
{
    bool firstRecord = true;
    char buf[256]; // TODO: fix buffer overflow issue

    buf[0] = 0xf0;
    buf[1] = 0x00;
    buf[2] = 0x21;
    buf[3] = 0x45;
    buf[4] = 0x01;
    buf[5] = (char)ElectraCommand::Object::PresetList;

    sprintf(buf + 6, "{\"version\":1,\"presets\":[");

    MidiOutput::sendSysExPartial(MidiInterface::Type::MidiUsbDev,
                                 port,
                                 (uint8_t *)buf,
                                 strlen(buf + 6) + 6,
                                 false);

    for (uint8_t i = 0; i < 72; i++) {
        if (strlen(presetSlot[i].getPresetName()) > 0) {
            sprintf(
                buf,
                "%s{\"slot\":%d,\"bankNumber\":%d,\"name\":\"%s\",\"projectId\":\"%s\"}",
                (firstRecord) ? "" : ",",
                i % 12,
                i / 12,
                presetSlot[i].getPresetName(),
                presetSlot[i].getProjectId());
            firstRecord = false;
            MidiOutput::sendSysExPartial(MidiInterface::Type::MidiUsbDev,
                                         port,
                                         (uint8_t *)buf,
                                         strlen(buf),
                                         false);
        }
    }
    sprintf(buf, "]}");
    buf[2] = 0xf7;

    MidiOutput::sendSysExPartial(
        MidiInterface::Type::MidiUsbDev, port, (uint8_t *)buf, 3, false);

    System::sysExBusy = false;
}

/** Load preset.
 *  Read preset file and initialize all data structures to display and run the preset.
 */
bool Presets::loadPreset(LocalFile file)
{
    const char *presetFile = file.getFilepath();

    // Free current preset
    reset();

    if (Hardware::sdcard.exists(presetFile)) {
        if (preset.load(presetFile) == true) {
            System::logger.write(
                LOG_INFO, "Default preset loaded: filename=%s", presetFile);
        }

        // Display the preset if valid.
        if (preset.isValid()) {
            // Initialise the parameterMap
            for (auto &[id, control] : preset.controls) {
                for (auto &value : control.values) {
                    control.setDefaultValue(value, false);
                }
            }

            if (Hardware::ram.adj_free() > 62000) {
                parameterMap.setProjectId(preset.getProjectId());

                uint8_t presetId =
                    (currentBankNumber * NumPresetsInBank) + currentSlot;

                if (!loadPresetStateOnStartup
                    && !presetSlot[presetId].hasBeenAlreadyLoaded()) {
                    parameterMap.forget();
                }
                if (keepPresetState) {
                    parameterMap.recall();
                }

                // mark reset as loaded in this session
                presetSlot[presetId].setAlreadyLoaded(true);
            } else {
                System::logger.write(
                    LOG_ERROR,
                    "Presets::loadPreset: Preset is too large: file=%s",
                    presetFile);
                preset.reset();
                return (false);
            }
        } else {
            System::logger.write(LOG_ERROR,
                                 "Presets::loadPreset: Invalid preset: file=%s",
                                 presetFile);
            preset.reset();
            return (false);
        }
    }

    return (true);
}

/** Mark preset slot
 *  Mark given preset slot as unused.
 */
void Presets::removePreset(uint8_t slotId)
{
    presetSlot[slotId].clear();
}

/** Reset preset.
 *  Re-initialize preset so that it is completely empty and ready for loading a new preset.
 */
void Presets::reset(void)
{
    // Disable the ParameterMap sync
    parameterMap.disable();
    if (keepPresetState) {
        parameterMap.keep();
    }

    // trigger Lua onExit function
    if (L) {
        preset_onExit();
    }

    // Always disable the timer when loading a preset
    System::tasks.disableTimer();

    // Reset Lua callbacks
    resetMidiCallbacks();

    // Reset Lua
    closeLua();

    // Reset preset
    preset.reset();

    // Reset parameterMap
    parameterMap.clear();

    System::logger.write(LOG_TRACE,
                         "Controller::reset: preset memory deallocated");
    monitorFreeMemory();
}

/** Load preset identified with a preset Id
 *
 */
bool Presets::loadPresetById(uint8_t presetId)
{
    if (!readyForPresetSwitch) {
        System::logger.write(
            LOG_ERROR,
            "Controller::loadPresetById: still busy with swicthing previous preset");
        return (false);
    } else {
        readyForPresetSwitch = false;
    }

    // clear all entries in the frame buffer
    parameterMap.disable();
    System::tasks.disableRepaintGraphics();
    System::tasks.clearRepaintGraphics();

    if (presetId > 71) {
        presetId = 0;
    }

    setBankNumberAndSlot(presetId);
    LocalFile file(System::context.getCurrentPresetFile());

    // Remember the preset for the next startup
    System::runtimeInfo.setLastActivePreset(presetId);

    // Try to load the preset
    bool status = false;

    if (loadPreset(file)) {
        System::logger.write(LOG_INFO,
                             "loadPresetById: preset loaded: name='%s'",
                             preset.getName());

        // Re-set Lua state and execute
        runPresetLuaScript();

        // Enable the ParameterMap sync
        parameterMap.enable(true);

        status = true;
    } else {
        reset();
        System::logger.write(LOG_ERROR,
                             "loadPresetById: preset loading failed: id=%d",
                             presetId);
    }

    System::tasks.enableRepaintGraphics();
    readyForPresetSwitch = true;

    return (status);
}

void Presets::runUploadedLuaScript(void)
{
    // clear all entries in the frame buffer
    parameterMap.disable();
    System::tasks.disableRepaintGraphics();
    System::tasks.clearRepaintGraphics();

    runPresetLuaScript();

    // Enable the ParameterMap sync
    parameterMap.enable(true);
    System::tasks.enableRepaintGraphics();
}

/** Run lua script
 *
 */
void Presets::runPresetLuaScript(void)
{
    closeLua();

    luaPreset = &preset;

    if (isLuaValid(System::context.getCurrentLuaFile())) {
        L = initLua();
        loadLuaLibs();

        executeElectraLua(System::context.getCurrentLuaFile());

        // Assign Lua callbacks
        assignLuaCallbacks();

        // Trigger Lua onLoad function
        preset_onLoad();

        // Mark map entries with Lua for processing
        parameterMap.scheduleLuaProcessing();
    }
}

void Presets::setBankNumberAndSlot(uint8_t presetId)
{
    currentBankNumber = presetId / NumPresetsInBank;
    currentSlot = presetId % NumPresetsInBank;
    presetChangePending = false;
    setDefaultFiles(currentBankNumber, currentSlot);
}

void Presets::setBankNumberAndSlot(uint8_t newBankNumber, uint8_t newSlot)
{
    currentBankNumber = newBankNumber;
    currentSlot = newSlot;
    presetChangePending = false;
    setDefaultFiles(currentBankNumber, currentSlot);
}

void Presets::setPendingBankNumberAndSlot(uint8_t newBankNumber,
                                          uint8_t newSlot)
{
    pendingBankNumber = newBankNumber;
    pendingSlot = newSlot;
    presetChangePending = true;
}

uint8_t Presets::getPresetId(void) const
{
    return (currentBankNumber * NumPresetsInBank + currentSlot);
}

void Presets::setCurrentSlot(uint8_t newSlot)
{
    currentSlot = newSlot;
    presetChangePending = false;
    setDefaultFiles(currentBankNumber, currentSlot);
}

uint8_t Presets::getCurrentSlot(void) const
{
    return (currentSlot);
}

void Presets::setCurrentBankNumber(uint8_t newBankNumber)
{
    currentBankNumber = newBankNumber;
    presetChangePending = false;
    setDefaultFiles(currentBankNumber, currentSlot);
}

uint8_t Presets::getCurrentBankNumber(void) const
{
    return (currentBankNumber);
}

uint8_t Presets::getPendingSlot(void) const
{
    return (pendingSlot);
}

uint8_t Presets::getPendingBankNumber(void) const
{
    return (pendingBankNumber);
}

bool Presets::isPresetChangePending(void) const
{
    return (presetChangePending);
}

void Presets::setDefaultFiles(uint8_t newBankNumber, uint8_t newSlot)
{
    System::context.setCurrentFile(newBankNumber * NumPresetsInBank + newSlot);
}

const char *Presets::getPresetName(uint8_t slotId) const
{
    return presetSlot[slotId].getPresetName();
}

bool Presets::updateSlot(uint8_t presetId, const char *newPresetPath)
{
    bool success = false;

    char slotPresetFilename[MAX_FILENAME_LENGTH];
    char slotLuaFilename[MAX_FILENAME_LENGTH];

    // Format preset and Lua filenames based on the presetId
    System::context.formatPresetFilename(
        slotPresetFilename, MAX_FILENAME_LENGTH, presetId);
    System::context.formatLuaFilename(
        slotLuaFilename, MAX_FILENAME_LENGTH, presetId);

    // Delete existing preset and Lua files if they exist
    if (Hardware::sdcard.exists(slotPresetFilename)) {
        Hardware::sdcard.deleteFile(slotPresetFilename);
    }

    if (Hardware::sdcard.exists(slotLuaFilename)) {
        Hardware::sdcard.deleteFile(slotLuaFilename);
    }

    if (newPresetPath) {
        char tmpFilename[MAX_FILENAME_LENGTH];

        // Copy preset file
        System::context.formatPreloadedFilename(
            tmpFilename, MAX_FILENAME_LENGTH, newPresetPath, "epr");
        if (Hardware::sdcard.exists(tmpFilename)) {
            success =
                Hardware::sdcard.copyFile(tmpFilename, slotPresetFilename);
        }

        // Copy Lua file
        System::context.formatPreloadedFilename(
            tmpFilename, MAX_FILENAME_LENGTH, newPresetPath, "lua");
        if (success && Hardware::sdcard.exists(tmpFilename)) {
            success = Hardware::sdcard.copyFile(tmpFilename, slotLuaFilename);
        }
    }

    // Force preset reload, when the slot is used next time
    presetSlot[presetId].setAlreadyLoaded(false);

    return (success);
}

uint8_t Presets::convertToPresetId(uint8_t bankNumber, uint8_t slot)
{
    return (bankNumber * NumPresetsInBank) + slot;
}