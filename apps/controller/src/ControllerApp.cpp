#include "ControllerApp.h"
#include "luaExtension.h"

void Controller::initialise(void)
{
    // Set application context
    System::context.setAppName("ctrlv2");

    // Set delegates
    luaDelegate = &delegate;

    // Get info about the last used preset
    uint8_t presetId = System::runtimeInfo.getLastActivePreset();

    // Load Configuration or use defaults
    if (!loadConfig()) {
        appConfig.useDefault();
    }

    // Special boot - do not read the default preset
    if (Hardware::buttons[BUTTON_LEFT_TOP]->process() == true) {
        System::logger.write(LOG_ERROR,
                             "Special boot mode: do not read preset");
        System::context.setLoadDefaultFiles(false);
    } else if (Hardware::buttons[BUTTON_LEFT_MIDDLE]->process() == true) {
        Hardware::sdcard.format();
        System::runtimeInfo.setLastActivePreset(0);
    } else if (Hardware::buttons[BUTTON_LEFT_BOTTOM]->process() == true) {
        System::logger.write(LOG_ERROR, "Special boot: do not use knob touch");
        System::context.setTouchEnabled(false);
        //systemTasks.doNotUsePotTouch();
    }

    // Log free RAM (before the preset is loaded)
    monitorFreeMemory();

    // Let parameterMap know what the app sandbox is
    parameterMap.setAppSandbox(getApplicationSandbox());

    // Register ParameterMap onChange callback
    parameterMap.onChange = [this](LookupEntry *entry, Origin origin) {
        if (entry) {
            if (origin != Origin::midi && origin != Origin::file) {
                // @todo taking the first message destination might not be ok
                // in some situations
                // @todo would it make sense to update the value in the model
                // and use a reference to send it out?
                Message message = entry->getMessage();
                message.setValue(entry->getMidiValue());
                midi.sendMessage(message);
            }

            // Execute only if Lua is loaded
            if (origin != Origin::mods) {
                if (L && entry) {
                    parameterMap_onChange(entry, origin);
                }
            }
        }
    };

    // Initialise list of presets stored in the controller
    model.presets.assignPresetNames();

    // load the default preset
    if (System::context.getLoadDefaultFiles()) {
        delegate.switchPreset(presetId / 12, presetId % 12);
    } else {
        delegate.initialiseEmpty();
    }

    // Log free RAM (after the preset is loaded and the default page displayed)
    monitorFreeMemory();

    // Finalise the initialisation
    System::logger.write(LOG_ERROR, "App initialisation completed");

    // Set logger status according to the saved configuration
    //System::logger.setStatus(System::runtimeInfo.getLoggerStatus());
}

/** Incoming MIDI message handler.
 *  handleIncomingMidiMessage () is called when a MIDI message arrives to Electra's
 *  MIDI ports (excluding Control port)
 */
void Controller::handleIncomingMidiMessage(const MidiInput &midiInput,
                                           const MidiMessage &midiMessage)
{
    if ((appConfig.router.usbDevToMidiControl == true)
        && (midiInput.getPort() == appConfig.router.midiControlPort)
        && ((appConfig.router.midiControlChannel == 0)
            || (midiMessage.getChannel()
                == appConfig.router.midiControlChannel))) {
        midiApi.process(midiMessage);
        if (appConfig.router.midiControlDrop) {
            return;
        }
    }

    if (System::context.getMidiLearn()) {
        midiLearn.process(midiInput, midiMessage);
    } else {
        midi.process(midiInput, midiMessage);
    }
}

/** Incoming file received handler.
 *  handleCtrlFileReceived () is called whenever file is received using
 *  the Electra upload SysEx command.
 */
bool Controller::handleCtrlFileReceived(uint8_t port,
                                        LocalFile file,
                                        ElectraCommand::Object fileType)
{
    bool status = false;

    if (fileType == ElectraCommand::Object::FileConfig) {
        status = applyChangesToConfig(file);
    } else {
        status = sysexApi.process(port, file, fileType);

        if (fileType == ElectraCommand::Object::FilePreset) {
            model.presets.assignPresetNames();
        }
    }

    return (status);
}

/** File removed handler.
 *  handleCtrlFileRemoved() is called whenever file is removed using
 *  the Electra upload SysEx command.
 */
bool Controller::handleCtrlFileRemoved(uint8_t bankNumber,
                                       uint8_t slot,
                                       ElectraCommand::Object fileType)
{
    uint8_t currentPreset = model.presets.getCurrentSlot();
    uint8_t currentPresetBank = model.presets.getCurrentBankNumber();
    uint8_t slotId =
        currentPresetBank * Presets::NumPresetsInBank + currentPreset;

    model.presets.removePreset(slotId);

    if (fileType == ElectraCommand::Object::FilePreset) {
        // If it is a current preset, reload it
        if ((currentPresetBank == bankNumber) && (currentPreset == slot)) {
            delegate.switchPreset(currentPresetBank, currentPreset);
        }
    }

    return (true);
}

/** Electra SysEx handler
 *
 */
void Controller::handleElectraSysex(uint8_t port, const SysexBlock &sysexBlock)
{
    sysexApi.process(port, sysexBlock);
}

/** User configurable task
 * Currently misused for sending Patch request messages.
 */
void Controller::runUserTask(void)
{
    PatchRequest request;

    if (patchRequests.isEmpty() != true) {
        request = patchRequests.shift();
        Device device = model.currentPreset.getDevice(request.deviceId);

        if (device.isValid()) {
            midi.sendTemplatedSysex(device, 0, request.data);
        }
    }

    if (model.presets.isPresetChangePending() == true) {
        delegate.switchPreset(model.presets.getPendingBankNumber(),
                              model.presets.getPendingSlot());
    }
}

/** Apply configuation change.
 *
 * Loads config from a (temp) file, merges it with existing, and writes
 * the new config file to the standard location
 */
bool Controller::applyChangesToConfig(LocalFile file)
{
    if (appConfig.load(file.getFilepath())) {
        configureApp();
        delegate.displayPage();
        Hardware::sdcard.deleteFile(System::context.getCurrentConfigFile());
        if (!file.rename(System::context.getCurrentConfigFile())) {
            System::logger.write(
                LOG_ERROR,
                "applyChangesToConfig: failed to update config: %s",
                System::context.getCurrentConfigFile());
        }
        return (true);
    }
    return (false);
}

/** Load configuration.
 *
 */
bool Controller::loadConfig(void)
{
    if (appConfig.load()) {
        configureApp();
        return (true);
    }
    return (false);
}

/** Reconfigure Electra at run-time
 *
 */
void Controller::configureApp(void)
{
    delegate.setActiveControlSetType(appConfig.uiFeatures.activeControlSetType);

    MidiOutput::enableThru(
        MidiInterface::Type::MidiIo, 0, appConfig.router.midiIo1Thru);
    MidiOutput::enableThru(
        MidiInterface::Type::MidiIo, 1, appConfig.router.midiIo2Thru);
}

/** USB Host port assigment callback
 * allback to determine assigment of USB device to USB Host ports.
 */
uint8_t Controller::getUsbHostPortAssigment(const char *productName)
{
    return appConfig.getUsbHostAssigment(productName);
}
