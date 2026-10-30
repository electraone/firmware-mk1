#include "SysexApi.h"
#include "System.h"
#include "ArduinoJson.h"
#include "MidiOutput.h"
#include "SubscribedEvents.h"
#include "FileTransfer.h"
#include "ScreenCapture.h"

SysexApi::SysexApi(MainDelegate &newDelegate,
                   CommandManager &newCommandManager)
    : delegate(newDelegate), commandManager(newCommandManager)
{
}

bool SysexApi::process(uint8_t port, const SysexBlock &sysexBlock)
{
    ElectraCommand cmd = sysexBlock.getElectraSysexCommand();
    MemoryBlock sysexPayload = sysexBlock.getElectraSysexPayload();
    ElectraCommand::Type command = cmd.getType();
    ElectraCommand::Object object = cmd.getObject();

    System::logger.write(
        LOG_INFO,
        "SysexApi::process: sysex received: command=%d, parameter=%d, byte1=%d",
        command,
        object,
        cmd.getByte1());

    /* A request nobody answers is worse than one that is refused: the host
     * cannot tell the difference between a message still in flight and one
     * that will never be answered, so its queue stops. Anything that reaches
     * the end of these chains is said "no" to. */
    bool handled = true;

    if (cmd.isFileRequest()) {
        if (object == ElectraCommand::Object::SnapshotList) {
            sendSnapshotList(port, sysexPayload);
        } else if (object == ElectraCommand::Object::FileSnapshot) {
            sendSnapshot(port, sysexPayload);
        } else if (object == ElectraCommand::Object::PresetList) {
            sendPresetList(port);
        } else if (object == ElectraCommand::Object::Location) {
            sendFileListing(port, sysexPayload);
        } else if (object == ElectraCommand::Object::File) {
            sendFile(port, sysexPayload);
        } else if (object == ElectraCommand::Object::PresetSlot) {
            sendPresetSlotInfo(port, cmd.getByte1(), cmd.getByte2());
        } else if (object == ElectraCommand::Object::UsbHostList) {
            sendUsbHostDevices(port);
        } else {
            handled = false;
        }
    } else if (cmd.isMidiLearnSwitch()) {
        if (object == ElectraCommand::Object::MidiLearnOff) {
            disableMidiLearn(port);
        } else {
            enableMidiLearn(port);
        }
    } else if (cmd.isSwitch()) {
        if (object == ElectraCommand::Object::PresetSlot) {
            switchPreset(port, cmd.getByte1(), cmd.getByte2());
        } else if (object == ElectraCommand::Object::Page) {
            switchPage(port, cmd.getByte1());
        } else if (object == ElectraCommand::Object::ControlSet) {
            switchControlSet(port, cmd.getByte1());
        } else {
            handled = false;
        }
    } else if (cmd.isUpdateRuntime()) {
        if (object == ElectraCommand::Object::Control) {
            uint16_t controlId = cmd.getByte1() | cmd.getByte2() << 7;
            updateControl(port, controlId, sysexPayload);
        } else if (object == ElectraCommand::Object::SnapshotSlot) {
            setSnapshotSlot(port, sysexPayload);
        } else if (object == ElectraCommand::Object::PresetSlot) {
            setPresetSlot(port, cmd.getByte1(), cmd.getByte2());
        } else if (object == ElectraCommand::Object::EventSubscription) {
            subscribeEvents(port, cmd.getByte1());
        } else if (object == ElectraCommand::Object::ControlPort) {
            setControlPort(port, cmd.getByte1());
        } else if (object == ElectraCommand::Object::Value) {
            uint16_t controlId = cmd.getByte1() | cmd.getByte2() << 7;
            uint8_t handleId = cmd.getByte3();
            updateControlValueLabel(port, controlId, handleId, sysexPayload);
        } else if (object == ElectraCommand::Object::StatusBar) {
            setBottomBarText(port, sysexPayload);
        } else {
            handled = false;
        }
    } else if (cmd.isUpdate()) {
        if (object == ElectraCommand::Object::SnapshotInfo) {
            updateSnapshot(port, sysexPayload);
        } else if (object == ElectraCommand::Object::PresetSlot) {
            updatePresetSlot(port, sysexPayload);
        } else {
            handled = false;
        }
    } else if (cmd.isRemove()) {
        if (object == ElectraCommand::Object::SnapshotInfo) {
            removeSnapshot(port, sysexPayload);
        } else if (object == ElectraCommand::Object::File) {
            removeFile(port, sysexPayload);
        } else if (object == ElectraCommand::Object::Location) {
            removeFileLocation(port, sysexPayload);
        } else if (object == ElectraCommand::Object::Directory) {
            removeDirectory(port, sysexPayload);
        } else {
            handled = false;
        }
    } else if (cmd.isCreate()) {
        if (object == ElectraCommand::Object::Directory) {
            createDirectory(port, sysexPayload);
        } else {
            handled = false;
        }
    } else if (cmd.isExecute()) {
        if (object == ElectraCommand::Object::PresetSlot) {
            reloadPresetSlot(port, sysexPayload);
        } else if (object == ElectraCommand::Object::Command) {
            runCommand(port, sysexPayload);
        } else {
            handled = false;
        }
    } else if (cmd.isSwap()) {
        if (object == ElectraCommand::Object::SnapshotInfo) {
            swapSnapshots(port, sysexPayload);
        } else {
            handled = false;
        }
    } else if (cmd.isEvent()) {
        if (cmd.getEvent() == ElectraCommand::Event::SnapshotBankSwitch) {
            uint8_t bankNumber = cmd.getByte1();
            System::logger.write(
                LOG_ERROR,
                "SysexApi::process: snapshot bank change event : bankNumber=%d",
                bankNumber);
            setCurrentSnapshotBank(port, bankNumber);
        }
    } else if (cmd.isSystemCall()) {
        if (object == ElectraCommand::Object::Screenshot) {
            takeScreenshot(port);
        } else {
            System::logger.write(LOG_ERROR,
                                 "SysexApi::process: application system call");
            handled = false;
        }
    } else {
        System::logger.write(LOG_ERROR,
                             "SysexApi::process: unknown sysex request");
        handled = false;
    }
    /* \todo This needs to be reviewed. Commands are taking care of ACK/NACK
     * by themselves. That is not in line with the file processing.
     */
    return (handled);
}

bool SysexApi::process(uint8_t port,
                       LocalFile &file,
                       ElectraCommand::Object fileType)
{
    bool status = false;

    if (fileType == ElectraCommand::Object::FilePreset) {
        status = loadPreset(port, file);
        if (status) {
            MidiOutput::sendPresetSlotChanged(MidiInterface::Type::MidiUsbDev,
                                              port);
        }
    } else if (fileType == ElectraCommand::Object::FileLua) {
        status = loadLua(port, file);
    } else if (fileType == ElectraCommand::Object::FileConfig) {
        status = loadConfig(port, file);
    } else if (fileType == ElectraCommand::Object::FileSnapshot) {
        status = importSnapshot(port, file);
    }
    return (status);
}

void SysexApi::sendSnapshotList(uint8_t port, MemoryBlock &sysexPayload)
{
    System::logger.write(LOG_ERROR, "SysexApi::sendSnapshotList");

    const size_t capacity = JSON_OBJECT_SIZE(1) + 500;
    StaticJsonDocument<capacity> doc;

    DeserializationError err = deserializeJson(doc, sysexPayload);

    if (err) {
        System::logger.write(
            LOG_ERROR,
            "sendSnapshotList: snapshot list parsing failed: %s",
            err.c_str());
        return;
    }

    const char *projectId = doc["projectId"].as<char *>();

    if (projectId) {
        delegate.sendSnapshotList(port, projectId);
    }
}

void SysexApi::sendSnapshot(uint8_t port, MemoryBlock &sysexPayload)
{
    System::logger.write(LOG_ERROR, "SysexApi::sendSnapshot");

    const size_t capacity = JSON_OBJECT_SIZE(1) + 500;
    StaticJsonDocument<capacity> doc;

    DeserializationError err = deserializeJson(doc, sysexPayload);

    if (err) {
        System::logger.write(LOG_ERROR,
                             "sendSnapshot: snapshot info parsing failed");
        return;
    }

    const char *projectId = doc["projectId"].as<char *>();
    uint8_t bankNumber = doc["bankNumber"].as<uint8_t>();
    uint8_t slot = doc["slot"].as<uint8_t>();

    delegate.sendSnapshot(port, projectId, bankNumber, slot);
}

void SysexApi::sendPresetList(uint8_t port) // \todo port does not belong here
{
    System::logger.write(LOG_ERROR, "SysexApi::sendPresetList");
    delegate.sendPresetList(port);
}

void SysexApi::enableMidiLearn(uint8_t port)
{
    System::logger.write(LOG_ERROR, "SysexApi::enableMidiLearn");
    delegate.enableMidiLearn();
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::disableMidiLearn(uint8_t port)
{
    System::logger.write(LOG_ERROR, "SysexApi::disableMidiLearn");
    delegate.disableMidiLearn();
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::switchPreset(uint8_t port, uint8_t bankNumber, uint8_t slot)
{
    System::logger.write(LOG_ERROR,
                         "SysexApi::switchPreset: port=%d, bank=%d, slot=%d",
                         port,
                         bankNumber,
                         slot);
    delegate.switchPreset(bankNumber, slot);
    MidiOutput::sendPresetSwitched(
        MidiInterface::Type::MidiUsbDev, port, bankNumber, slot);
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::switchPage(uint8_t port, uint8_t pageNumber)
{
    System::logger.write(LOG_ERROR,
                         "SysexApi::switchPreset: port=%d, page=%d",
                         port,
                         pageNumber);
    if (delegate.switchPage(pageNumber + 1)) {
        if (delegate.getSubscribedEvents() & SubscribedEvents::pages) {
            MidiOutput::sendPageSwitched(
                MidiInterface::Type::MidiUsbDev, port, pageNumber);
        }
        MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
    } else {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
    }
}

void SysexApi::switchControlSet(uint8_t port, uint8_t controlSetId)
{
    System::logger.write(LOG_ERROR,
                         "SysexApi::switchControlSet: port=%d, controlSetId=%d",
                         port,
                         controlSetId);
    if (delegate.switchControlSet(controlSetId)) {
        MidiOutput::sendControlSetSwitched(
            MidiInterface::Type::MidiUsbDev, port, controlSetId);
        MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
    } else {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
    }
}

void SysexApi::updateControl(uint8_t port,
                             uint16_t controlId,
                             MemoryBlock &sysexPayload)
{
    System::logger.write(LOG_ERROR, "SysexApi::updateControl");
    const size_t capacity = JSON_OBJECT_SIZE(1) + 500;
    StaticJsonDocument<capacity> doc;

    // skip first two bytes (after command and object)
    sysexPayload.read();
    sysexPayload.read();

    DeserializationError err = deserializeJson(doc, sysexPayload);

    if (err) {
        System::logger.write(LOG_ERROR,
                             "updateControl: control info parsing failed");
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    if (!doc["name"].isNull()) {
        const char *name = doc["name"].as<char *>();
        delegate.setControlName(controlId, name);
    }

    if (!doc["color"].isNull()) {
        uint32_t colour = Colours565::fromString(doc["color"].as<char *>());
        delegate.setControlColour(controlId, colour);
    }

    if (!doc["visible"].isNull()) {
        bool shouldBeVisible = doc["visible"].as<bool>();
        delegate.setControlVisible(controlId, shouldBeVisible);
    }

    if (!doc["value"].isNull()) {
        const char *valueId = doc["value"]["id"] | "value";
        const char *text = doc["value"]["text"] | "";
        delegate.setControlValueLabel(controlId, valueId, text);
    }

    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::updateControlValueLabel(uint8_t port,
                                       uint16_t controlId,
                                       uint8_t handleId,
                                       MemoryBlock &sysexPayload)
{
    // skip first three bytes (after command and object)
    sysexPayload.read();
    sysexPayload.read();
    sysexPayload.read();

    uint8_t buffer[20];
    auto length = sysexPayload.readBytes(buffer, sizeof(buffer) - 1);
    buffer[length] = '\0';
    delegate.setControlValueLabel(controlId, handleId, (char *)buffer);
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::setSnapshotSlot(uint8_t port, MemoryBlock &sysexPayload)
{
    System::logger.write(LOG_ERROR, "SysexApi::setSnapshotSlot");

    const size_t capacity = JSON_OBJECT_SIZE(1) + 500;
    StaticJsonDocument<capacity> doc;

    DeserializationError err = deserializeJson(doc, sysexPayload);

    if (err) {
        System::logger.write(LOG_ERROR,
                             "setSnapshotSlot: snapshot info parsing failed");
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    const char *projectId = doc["projectId"].as<char *>();
    uint8_t bankNumber = doc["bankNumber"].as<uint8_t>();
    uint8_t slot = doc["slot"].as<uint8_t>();

    delegate.setSnapshotSlot(projectId, bankNumber, slot);
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

bool SysexApi::loadPreset(uint8_t port, LocalFile &file)
{
    return (delegate.loadPreset(file));
}

bool SysexApi::loadLua(uint8_t port, LocalFile &file)
{
    return (delegate.loadLua(file));
}

bool SysexApi::loadConfig(uint8_t port, LocalFile &file)
{
    // this is commented out on purpose. We do not want to load the config here yet.
    //delegate.loadConfig(file);
    //MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);;
    return (true);
}

bool SysexApi::importSnapshot(uint8_t port, LocalFile &file)
{
    return (delegate.importSnapshot(file));
}

void SysexApi::setPresetSlot(uint8_t port, uint8_t bankNumber, uint8_t slot)
{
    System::logger.write(LOG_ERROR, "SysexApi::setPresetSlot");
    delegate.setPresetSlot(bankNumber, slot);
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::updateSnapshot(uint8_t port, MemoryBlock &sysexPayload)
{
    System::logger.write(LOG_ERROR, "SysexApi::updateSnapshot");

    const size_t capacity = JSON_OBJECT_SIZE(1) + 500;
    StaticJsonDocument<capacity> doc;

    DeserializationError err = deserializeJson(doc, sysexPayload);

    if (err) {
        System::logger.write(LOG_ERROR,
                             "updateSnapshot: snapshot info parsing failed");
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    const char *projectId = doc["projectId"].as<char *>();
    uint8_t bankNumber = doc["bankNumber"].as<uint8_t>();
    uint8_t slot = doc["slot"].as<uint8_t>();
    const char *name = doc["name"].as<char *>();
    uint32_t colour = Colours565::fromString(doc["color"].as<char *>());

    delegate.updateSnapshot(projectId, bankNumber, slot, name, colour);
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::updatePresetSlot(uint8_t port, MemoryBlock &sysexPayload)
{
    System::logger.write(LOG_ERROR, "SysexApi::updatePresetSlot");

    const size_t capacity = JSON_OBJECT_SIZE(1) + 500;
    StaticJsonDocument<capacity> doc;

    DeserializationError err = deserializeJson(doc, sysexPayload);

    if (err) {
        System::logger.write(
            LOG_ERROR, "updatePresetSlot: presetSlot info parsing failed");
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    uint8_t bankNumber = doc["bankNumber"].as<uint8_t>();
    uint8_t slot = doc["slot"].as<uint8_t>();
    const char *preset = doc["preset"].as<char *>();

    if (delegate.updatePresetSlot(bankNumber, slot, preset)) {
        MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
    } else {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
    }
}

void SysexApi::removeSnapshot(uint8_t port, MemoryBlock &sysexPayload)
{
    System::logger.write(LOG_ERROR, "SysexApi::removeSnapshot");

    const size_t capacity = JSON_OBJECT_SIZE(1) + 500;
    StaticJsonDocument<capacity> doc;

    DeserializationError err = deserializeJson(doc, sysexPayload);

    if (err) {
        System::logger.write(LOG_ERROR,
                             "removeSnapshot: snapshot info parsing failed");
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    const char *projectId = doc["projectId"].as<char *>();
    uint8_t bankNumber = doc["bankNumber"].as<uint8_t>();
    uint8_t slot = doc["slot"].as<uint8_t>();

    delegate.removeSnapshot(projectId, bankNumber, slot);
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::swapSnapshots(uint8_t port, MemoryBlock &sysexPayload)
{
    const size_t capacity = JSON_OBJECT_SIZE(1) + 500;
    StaticJsonDocument<capacity> doc;

    DeserializationError err = deserializeJson(doc, sysexPayload);

    if (err) {
        System::logger.write(LOG_ERROR,
                             "swapSnapshots: snapshot info parsing failed");
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    const char *projectId = doc["projectId"].as<char *>();
    uint8_t sourceBankNumber = doc["fromBankNumber"].as<uint8_t>();
    uint8_t sourceSlot = doc["fromSlot"].as<uint8_t>();
    uint8_t destBankNumber = doc["toBankNumber"].as<uint8_t>();
    uint8_t destSlot = doc["toSlot"].as<uint8_t>();

    delegate.swapSnapshots(
        projectId, sourceBankNumber, sourceSlot, destBankNumber, destSlot);
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::setCurrentSnapshotBank(uint8_t port, uint8_t bankNumber)
{
    if ((0 <= bankNumber) && (bankNumber <= 11)) {
        System::logger.write(
            LOG_ERROR, "setCurrentSnapshotBank: new bankNumber=%d", bankNumber);
        delegate.setCurrentSnapshotBank(bankNumber);
        MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
    } else {
        System::logger.write(LOG_ERROR,
                             "setCurrentSnapshotBank: invalid bankNumber=%d",
                             bankNumber);
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
    }
}

void SysexApi::setControlPort(uint8_t port, uint8_t newControlPort)
{
    if ((0 <= newControlPort) && (newControlPort <= 2)) {
        System::logger.write(
            LOG_ERROR, "setControlPort: new controlPort=%d", newControlPort);
        delegate.setControlPort(newControlPort);
        MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
    } else {
        System::logger.write(
            LOG_ERROR,
            "setControlPort: invalid port specified controlPort=%d",
            newControlPort);
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
    }
}

uint8_t SysexApi::getControlPort(void)
{
    return (delegate.getControlPort());
}

void SysexApi::subscribeEvents(uint8_t port, uint8_t newEvents)
{
    System::logger.write(
        LOG_ERROR, "subscribeEvents: new events=%02x", newEvents);
    delegate.setSubscribedEvents(newEvents);
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

/** What one preset slot holds. See Presets::sendSlotInfo(). */
void SysexApi::sendPresetSlotInfo(uint8_t port,
                                  uint8_t bankNumber,
                                  uint8_t slot)
{
    System::logger.write(LOG_ERROR,
                         "SysexApi::sendPresetSlotInfo: bank=%d, slot=%d",
                         bankNumber,
                         slot);
    delegate.sendPresetSlotInfo(port, bankNumber, slot);
}

void SysexApi::sendUsbHostDevices(uint8_t port)
{
    System::logger.write(LOG_ERROR, "SysexApi::sendUsbHostDevices");
    MidiOutput::sendUsbHostDevices(MidiInterface::Type::MidiUsbDev, port);
}

/**
 * @brief The text in the bottom bar, set by the host rather than by a script.
 *
 * The same place info.setText() writes to, so the two cannot disagree about
 * what is showing.
 */
/**
 * @brief Runs one of the instrument's commands.
 *
 * The payload is either the command's name on its own, which is what makes
 * this usable from anything that can send a SysEx message by hand, or a JSON
 * object when the command takes a number:
 *
 *     {"command":"switchPage","parameter":3}
 *
 * A name nothing answers to, or a number the command will not take, is a NACK
 * rather than silence - the host asked for something the instrument cannot do
 * and has to be told so.
 */
void SysexApi::runCommand(uint8_t port, MemoryBlock &sysexPayload)
{
    /* Long enough for the longest name and a small JSON object around it. */
    char payload[128];
    size_t length = sysexPayload.getLength();

    if (length > (sizeof(payload) - 1)) {
        length = sizeof(payload) - 1;
    }

    sysexPayload.seek(0);
    length = sysexPayload.readBytes((uint8_t *)payload, length);
    payload[length] = '\0';

    const char *name = payload;
    int16_t parameter = 0;

    StaticJsonDocument<192> doc;

    if (payload[0] == '{') {
        DeserializationError err = deserializeJson(doc, payload);

        if (err) {
            System::logger.write(LOG_ERROR,
                                 "SysexApi::runCommand: not JSON: %s",
                                 err.c_str());
            MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
            return;
        }

        name = doc["command"] | "";
        parameter = doc["parameter"] | (int16_t)0;
    }

    if (commandManager.run(name, parameter)) {
        MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
    } else {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
    }
}

void SysexApi::setBottomBarText(uint8_t port, MemoryBlock &sysexPayload)
{
    /* Forty is what the API documents, and one more for the terminator. Longer
     * text is cut rather than refused: the host asked for something to be
     * shown, and showing the front of it is closer to that than showing
     * nothing. */
    char text[41];
    size_t length = sysexPayload.getLength();

    if (length > (sizeof(text) - 1)) {
        length = sizeof(text) - 1;
    }

    sysexPayload.seek(0);
    length = sysexPayload.readBytes((uint8_t *)text, length);
    text[length] = '\0';

    delegate.setInfoText(text);
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

/**
 * @brief Loads a slot again, whatever is in it now.
 *
 * What a host does after writing a preset into a slot: the file on the card has
 * changed and the panel has not. A request with no bank and slot reloads the one
 * that is showing, which is how the host tools ask for it most of the time.
 */
void SysexApi::reloadPresetSlot(uint8_t port, MemoryBlock &sysexPayload)
{
    if (sysexPayload.getLength() >= 2) {
        const uint8_t bankNumber = (uint8_t)sysexPayload.peek(0);
        const uint8_t slot = (uint8_t)sysexPayload.peek(1);

        System::logger.write(LOG_ERROR,
                             "SysexApi::reloadPresetSlot: bank=%d, slot=%d",
                             bankNumber,
                             slot);
        delegate.switchPreset(bankNumber, slot);
    } else {
        System::logger.write(LOG_ERROR, "SysexApi::reloadPresetSlot: current");
        delegate.reloadPreset();
    }

    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::sendFileListing(uint8_t port, MemoryBlock &sysexPayload)
{
    if (FileTransfer::namesASlot(sysexPayload)) {
        if (!FileTransfer::sendSlotListing(port, sysexPayload)) {
            MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        }
        return;
    }

    char directory[FileTransfer::maxPathLength];

    if (!FileTransfer::resolveLocation(
            directory, sizeof(directory), sysexPayload)) {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    FileTransfer::sendListing(port, directory);
}

void SysexApi::sendFile(uint8_t port, MemoryBlock &sysexPayload)
{
    char path[FileTransfer::maxPathLength];

    if (!FileTransfer::resolve(path, sizeof(path), sysexPayload)
        || !FileTransfer::sendFile(port, path)) {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    /* The file first and the acknowledgement after it, which is the shape the
     * host tools expect: the ACK is what tells them the transfer ended rather
     * than stopped. */
    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::removeFile(uint8_t port, MemoryBlock &sysexPayload)
{
    char path[FileTransfer::maxPathLength];

    if (!FileTransfer::resolve(path, sizeof(path), sysexPayload)
        || !Hardware::sdcard.deleteFile(path)) {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::removeFileLocation(uint8_t port, MemoryBlock &sysexPayload)
{
    char directory[FileTransfer::maxPathLength];

    if (!FileTransfer::resolveLocation(
            directory, sizeof(directory), sysexPayload)
        || !FileTransfer::removeLocation(directory)) {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::createDirectory(uint8_t port, MemoryBlock &sysexPayload)
{
    char directory[FileTransfer::maxPathLength];

    if (!FileTransfer::resolveLocation(
            directory, sizeof(directory), sysexPayload)
        || !FileTransfer::createDirectory(directory)) {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::removeDirectory(uint8_t port, MemoryBlock &sysexPayload)
{
    char directory[FileTransfer::maxPathLength];

    if (!FileTransfer::resolveLocation(
            directory, sizeof(directory), sysexPayload)
        || !FileTransfer::removeDirectory(directory)) {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}

void SysexApi::takeScreenshot(uint8_t port)
{
    char filename[FileTransfer::maxPathLength];

    if (!ScreenCapture::saveNext(filename, sizeof(filename))) {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        return;
    }

    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
}
