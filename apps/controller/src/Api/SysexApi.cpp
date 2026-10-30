#include "SysexApi.h"
#include "System.h"
#include "ArduinoJson.h"
#include "MidiOutput.h"
#include "SubscribedEvents.h"

SysexApi::SysexApi(MainDelegate &newDelegate) : delegate(newDelegate)
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

    if (cmd.isFileRequest()) {
        if (object == ElectraCommand::Object::SnapshotList) {
            sendSnapshotList(port, sysexPayload);
        } else if (object == ElectraCommand::Object::FileSnapshot) {
            sendSnapshot(port, sysexPayload);
        } else if (object == ElectraCommand::Object::PresetList) {
            sendPresetList(port);
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
        }
    } else if (cmd.isUpdate()) {
        if (object == ElectraCommand::Object::SnapshotInfo) {
            updateSnapshot(port, sysexPayload);
        } else if (object == ElectraCommand::Object::PresetSlot) {
            updatePresetSlot(port, sysexPayload);
        }
    } else if (cmd.isRemove()) {
        if (object == ElectraCommand::Object::SnapshotInfo) {
            removeSnapshot(port, sysexPayload);
        }
    } else if (cmd.isSwap()) {
        if (object == ElectraCommand::Object::SnapshotInfo) {
            swapSnapshots(port, sysexPayload);
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
        System::logger.write(LOG_ERROR,
                             "SysexApi::process: application system call");
    } else {
        System::logger.write(LOG_ERROR,
                             "SysexApi::process: unknown sysex request");
    }
    /* \todo This needs to be reviewed. Commands are taking care of ACK/NACK
     * by themselves. That is not in line with the file processing.
     */
    return (true);
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
