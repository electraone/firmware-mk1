#include "Midi.h"
#include "Event.h"
#include "Checksum.h"
#include "luaHooks.h"
#include "luaPatch.h"
#include "ParameterMap.h"

#include "App.h"

CircularBuffer<PatchRequest, 32> patchRequests;

/** Constructor
 *
 */
Midi::Midi(const Preset &preset) : model(preset)
{
}

/** Send Electra message to Midi outputs.
 *
 */
void Midi::sendMessage(const Message &message)
{
    Device device = model.getDevice(message.getDeviceId());

    if (!device.isValid()) {
        return;
    }

    uint8_t channel = device.getChannel();
    uint8_t port = device.getPort();
    uint16_t midiValue = message.getValue();

    // Set the timestamp of device last message to current time
    device.setTsLastMessage();

    //message.print();

    // Send the particular MIDI message
    if (message.getType() == Message::Type::cc7) {
        sendControlChange(
            port, channel, message.getParameterNumber(), midiValue);
    } else if (message.getType() == Message::Type::cc14) {
        sendControlChange14Bit(port,
                               channel,
                               message.getParameterNumber(),
                               midiValue,
                               message.getLsbFirst());
    } else if (message.getType() == Message::Type::relcc) {
        sendControlChange(
            port, channel, message.getParameterNumber(), midiValue);
    } else if (message.getType() == Message::Type::nrpn) {
        sendNrpn(port,
                 channel,
                 message.getParameterNumber(),
                 midiValue,
                 message.getLsbFirst(),
                 message.getResetRpn());
    } else if (message.getType() == Message::Type::rpn) {
        sendRpn(port, channel, message.getParameterNumber(), midiValue);
    } else if (message.getType() == Message::Type::program) {
        sendProgramChange(port, channel, midiValue);
    } else if (message.getType() == Message::Type::note) {
        if (midiValue == 127) {
            sendNoteOn(port,
                       channel,
                       message.getParameterNumber(),
                       127); //midiValue);
        } else {
            sendNoteOff(port,
                        channel,
                        message.getParameterNumber(),
                        127); //midiValue);
        }
    } else if (message.getType() == Message::Type::start) {
        sendStart(port);
    } else if (message.getType() == Message::Type::stop) {
        sendStop(port);
    } else if (message.getType() == Message::Type::tune) {
        sendTuneRequest(port);
    } else if (message.getType() == Message::Type::sysex) {
        sendTemplatedSysex(device, message.getParameterNumber(), *message.data);
    } else if (message.getType() == Message::Type::atchannel) {
        sendAfterTouchChannel(port, channel, midiValue);
    } else if (message.getType() == Message::Type::atpoly) {
        sendAfterTouchPoly(
            port, channel, message.getParameterNumber(), midiValue);
    } else if (message.getType() == Message::Type::pitchbend) {
        sendPitchBend(port, channel, midiValue);
    } else if (message.getType() == Message::Type::spp) {
        sendSongPosition(port, midiValue);
    }
}

/** Send a SysEx message with the placeholder substituted
 *
 */
void Midi::sendTemplatedSysex(const Device &device,
                              uint16_t parameterNumber,
                              std::vector<uint8_t> data)
{
    const int maxSysexSize = 512;

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());

    if (data.size() < maxSysexSize) {
        uint8_t sysexData[maxSysexSize];
        uint16_t sysexDataLength = 0;
        sysexDataLength =
            transformMessage(parameterNumber, device, data, sysexData);
        sysexBlock.writeBytes(sysexData, sysexDataLength);
        sysexBlock.close();

        sendSysEx(device.getPort(), sysexBlock);
    } else {
        System::logger.write(
            LOG_ERROR,
            "sendTemplatedSysex: message exceeds allowed maximum length: %d",
            maxSysexSize);
    }
}

/** Replace substitution variables with values
 *
 */
uint8_t Midi::transformMessage(uint16_t parameterNumber,
                               const Device &device,
                               std::vector<uint8_t> data,
                               uint8_t *dataOut)
{
    uint16_t j = 0;

    for (uint16_t i = 0; i < data.size(); i++) {
        if (data[i] == VARIABLE_DATA) {
            runVariable(parameterNumber, i, j, data, dataOut, device);
        } else if (data[i] == VARIABLE_PARAMETER) {
            runParameter(parameterNumber, i, j, data, dataOut, device);
        } else if (data[i] == CHECKSUM) {
            runChecksum(i, j, data, dataOut, device);
        } else if (data[i] == LUAFUNCTION) {
            runLuaFunction(parameterNumber, i, j, data, dataOut, device);
        } else {
            runConstant(i, j, data, dataOut, device);
        }
    }
    return (j);
}

void Midi::runVariable(uint16_t parameterNumber,
                       uint16_t &i,
                       uint16_t &j,
                       std::vector<uint8_t> data,
                       uint8_t *dataOut,
                       const Device &device)
{
    uint8_t type = 0;
    uint8_t parameterIdLSB;
    uint8_t parameterIdMSB;
    uint16_t parameterId = 0;
    uint8_t pPos = 0;
    uint8_t bPos = 0;
    uint8_t size = 0;
    uint8_t byteToSend = 0;
    uint16_t parameterValue = 0;
    uint16_t mask = 0;

    i++;

    while ((data[i] != VARIABLE_END) && (i < 100)) {
        type = data[i];
        i++;
        parameterIdLSB = data[i];
        i++;
        parameterIdMSB = data[i];
        i++;
        parameterId = parameterIdLSB + (parameterIdMSB * 128);
        pPos = data[i];
        i++;
        bPos = data[i];
        i++;
        size = data[i];
        i++;
        mask = createMask(pPos, size);

        if (parameterId == 0) {
            parameterId = parameterNumber;
        }

        parameterValue = ((parameterMap.getValue(
                               device.getId(), (Message::Type)type, parameterId)
                           & mask)
                          >> pPos)
                         << bPos;
        byteToSend |= parameterValue;
    }

    dataOut[j] = byteToSend & 0x7F; // mask to 7bit and assign to output array
    j++;
}

void Midi::runParameter(uint16_t parameterNumber,
                        uint16_t &i,
                        uint16_t &j,
                        std::vector<uint8_t> data,
                        uint8_t *dataOut,
                        const Device &device)
{
    uint8_t pPos = 0;
    uint8_t bPos = 0;
    uint8_t size = 0;
    uint8_t byteToSend = 0;
    uint16_t parameterValue = 0;
    uint16_t mask = 0;

    i++;

    while ((data[i] != VARIABLE_END) && (i < 100)) {
        pPos = data[i];
        i++;
        bPos = data[i];
        i++;
        size = data[i];
        i++;
        mask = createMask(pPos, size);

        parameterValue = ((parameterNumber & mask) >> pPos) << bPos;
        byteToSend |= parameterValue;
    }
    dataOut[j] = byteToSend & 0x7F; // mask to 7bit and assign to output array
    j++;
}

void Midi::runChecksum(uint16_t &i,
                       uint16_t &j,
                       std::vector<uint8_t> data,
                       uint8_t *dataOut,
                       const Device &device)
{
    i++;
    ChecksumAlgorithm algorithm = (ChecksumAlgorithm)data[i];
    i++;
    uint8_t start = data[i];
    i++;
    uint8_t length = data[i];
    uint8_t checksum = 0;

    if (algorithm == ChecksumAlgorithm::ROLAND) {
        checksum = calculateChecksum(&dataOut[start], length);
    } else if (algorithm == ChecksumAlgorithm::FRACTALAUDIO) {
        checksum = calculateChecksum_fractalaudio(&dataOut[start], length);
    } else if (algorithm == ChecksumAlgorithm::WALDORF) {
        checksum = calculateChecksum_waldorf(&dataOut[start], length);
    } else {
        checksum = 0;
    }

    dataOut[j] = checksum & 0x7F;
    j++;

    System::logger.write(
        LOG_ERROR,
        "Checksum calculation: algorithm=%d, start=%d, length=%d, checksum=%d",
        algorithm,
        start,
        length,
        checksum);
}

void Midi::runLuaFunction(uint16_t parameterNumber,
                          uint16_t &i,
                          uint16_t &j,
                          std::vector<uint8_t> data,
                          uint8_t *dataOut,
                          const Device &device)
{
    uint16_t parameterValue = 0;
    uint8_t byteToSend = 0;

    i++;
    uint8_t functionId = data[i];

    System::logger.write(LOG_ERROR,
                         "function: %d (%s)",
                         functionId,
                         model.luaFunctions[functionId].c_str());

    parameterValue = parameterMap.getValue(
        device.getId(), Message::Type::sysex, parameterNumber);

    if (L && (model.luaFunctions.size() > 0)) {
        byteToSend = runTemplateFunction(model.luaFunctions[functionId].c_str(),
                                         (void *)&device,
                                         parameterValue);
    }

    dataOut[j] = byteToSend & 0x7F;
    j++;
}

void Midi::runConstant(uint16_t &i,
                       uint16_t &j,
                       std::vector<uint8_t> data,
                       uint8_t *dataOut,
                       const Device &device)
{
    dataOut[j] = data[i];
    j++;
}

/** Process incoming MIDI messages (when MIDI learn is off)
 *
 */
void Midi::process(const MidiInput &midiInput, const MidiMessage &midiMessage)
{
    Device device =
        model.getDevice(midiInput.getPort(), midiMessage.getChannel());

    // Non-Channel messages (TODO: it ignores ports)
    if (!device.isValid()) {
        if (midiMessage.isMidiStart()) {
            processStart();
        } else if (midiMessage.isMidiStop()) {
            processStop();
        } else if (midiMessage.isMidiTuneRequest()) {
            processTuneRequest();
        } else if (midiMessage.isSongPositionPointer()) {
            processSongPosition(midiMessage.getData1(), midiMessage.getData2());
        } else if (midiMessage.isSysEx()) {
            processSysex(midiMessage);
        }
    }

    // Channel Messages
    else {
        uint8_t deviceId = device.getId();

        if (midiMessage.isController()) {
            processCc(deviceId, midiMessage.getData1(), midiMessage.getData2());
        } else if (midiMessage.isNote()) {
            processNote(deviceId,
                        midiMessage.getType(),
                        midiMessage.getData1(),
                        midiMessage.getData2());
        } else if (midiMessage.isProgramChange()) {
            processProgramChange(deviceId, midiMessage.getData1());
        } else if (midiMessage.isAftertouch()) {
            processAfterTouchPoly(
                deviceId, midiMessage.getData1(), midiMessage.getData2());
        } else if (midiMessage.isChannelPressure()) {
            processAfterTouchChannel(deviceId, midiMessage.getData1());
        } else if (midiMessage.isPitchWheel()) {
            processPitchBend(
                deviceId, midiMessage.getData1(), midiMessage.getData2());
        } else {
            System::logger.write(
                LOG_INFO,
                "Midi::processMidi: other midi message. ignoring it.");
        }
    }
}

void Midi::processStart(void)
{
    System::logger.write(LOG_ERROR, "Midi::processMidi: Start midi message");
    parameterMap.setValue(0xff, Message::Type::start, 0, 0, Origin::midi);
}

void Midi::processStop(void)
{
    System::logger.write(LOG_ERROR, "Midi::processMidi: Stop midi message");
    parameterMap.setValue(0xff, Message::Type::stop, 0, 0, Origin::midi);
}

void Midi::processTuneRequest(void)
{
    System::logger.write(LOG_ERROR, "Midi::processMidi: Tune midi message");
    parameterMap.setValue(0xff, Message::Type::tune, 0, 0, Origin::midi);
}

void Midi::processCc(uint8_t deviceId,
                     uint8_t midiParameterId,
                     uint8_t midiValue)
{
    System::logger.write(
        LOG_ERROR,
        "ElectraMidi: processMidi: Control change midi message: parameter=%d, value=%d",
        midiParameterId,
        midiValue);

    // handle RPN / NRPN
    MidiRpnMessage midiRpnMessage;

    if (rpnDetector.parseControllerMessage(
            deviceId, midiParameterId, midiValue, midiRpnMessage)) {
        System::logger.write(LOG_ERROR,
                             "Midi::processMidi: RPN detected: parameter=%d, "
                             "value=%d, isNrpn=%d, is14bit=%d",
                             midiRpnMessage.parameterNumber,
                             midiRpnMessage.value,
                             midiRpnMessage.isNrpn,
                             midiRpnMessage.is14BitValue);

        Message::Type messageType =
            (midiRpnMessage.isNrpn) ? Message::Type::nrpn : Message::Type::rpn;

        parameterMap.setValue(deviceId,
                              messageType,
                              midiRpnMessage.parameterNumber,
                              midiRpnMessage.value,
                              Origin::midi);
    }

    // handle CC14
    MidiCc14Message midiCc14Message;

    if (cc14Detector.parseControllerMessage(
            deviceId, midiParameterId, midiValue, midiCc14Message)) {
        System::logger.write(
            LOG_ERROR,
            "Midi::processMidi: CC14 detected: parameter=%d, value=%d",
            midiCc14Message.parameterNumber,
            midiCc14Message.value);
        parameterMap.setValue(deviceId,
                              Message::Type::cc14,
                              midiCc14Message.parameterNumber,
                              midiCc14Message.value,
                              Origin::midi);
    }

    // Save to the parameter map
    parameterMap.setValue(
        deviceId, Message::Type::cc7, midiParameterId, midiValue, Origin::midi);
}

void Midi::processNote(uint8_t deviceId,
                       MidiMessage::Type midiType,
                       uint8_t noteNumber,
                       uint8_t velocity)
{
    uint8_t translatedVelocity =
        (midiType == MidiMessage::Type::NoteOn) ? velocity : 0;
    System::logger.write(
        LOG_ERROR,
        "Midi::processMidi: note message: note=%d, velocity=%d",
        noteNumber,
        translatedVelocity);
    parameterMap.setValue(deviceId,
                          Message::Type::note,
                          noteNumber,
                          translatedVelocity,
                          Origin::midi);
}

void Midi::processProgramChange(uint8_t deviceId, uint8_t programNumber)
{
    System::logger.write(LOG_ERROR,
                         "Midi::processMidi: program message: program=%d",
                         programNumber);
    parameterMap.setValue(
        deviceId, Message::Type::program, 0, programNumber, Origin::midi);
}

void Midi::processAfterTouchChannel(uint8_t deviceId, uint8_t pressure)
{
    System::logger.write(
        LOG_ERROR,
        "Midi::processMidi: aftertouch channel message: pressure=%d",
        pressure);
    parameterMap.setValue(
        deviceId, Message::Type::atchannel, 0, pressure, Origin::midi);
}

void Midi::processAfterTouchPoly(uint8_t deviceId,
                                 uint8_t noteNumber,
                                 uint8_t pressure)
{
    System::logger.write(
        LOG_ERROR,
        "Midi::processMidi: aftertouch poly message: noteNumber=%d, pressure=%d",
        noteNumber,
        pressure);
    parameterMap.setValue(
        deviceId, Message::Type::atpoly, noteNumber, pressure, Origin::midi);
}

void Midi::processPitchBend(uint8_t deviceId,
                            uint8_t valueFine,
                            uint8_t valueCoarse)
{
    uint16_t midiValue = (valueCoarse << 7) | valueFine;
    System::logger.write(LOG_ERROR,
                         "Midi::processMidi: pitchbend message: midiValue=%d",
                         midiValue);
    parameterMap.setValue(
        deviceId, Message::Type::pitchbend, 0, midiValue, Origin::midi);
}

void Midi::processSongPosition(uint8_t valueFine, uint8_t valueCoarse)
{
    uint16_t midiValue = (valueCoarse << 7) | valueFine;
    System::logger.write(
        LOG_ERROR,
        "Midi::processMidi: song position message: midiValue=%d",
        midiValue);
    parameterMap.setValue(0xff, Message::Type::spp, 0, midiValue, Origin::midi);
    parameterMap.print();
}

void Midi::processSysex(const MidiMessage &midiMessage)
{
    SysexBlock sysexBlock = midiMessage.getSysExBlock();

    if (sysexBlock.isEmpty()) {
        return;
    }

    for (const auto &[id, device] : model.devices) {
        for (const auto &[messageId, sysexMessage] : device.sysexMessages) {
            if (id < 100) { // only for user defined messages
                if (processSysexData(sysexMessage, device, sysexBlock)) {
                    break;
                }
            }
        }

        for (const auto &response : device.responses) {
            if (processResponse(response, device, sysexBlock)) {
                break;
            }
        }
    }
}

bool Midi::processSysexData(const DataBytes &sysexMessage,
                            const Device &device,
                            SysexBlock &sysexBlock)
{
    auto sysexLength = sysexBlock.getLength();
    uint8_t j = 0;
    uint16_t parameterNumber = 0;
    uint16_t value = 0;

    uint8_t pPos = 0;
    uint8_t bPos = 0;
    uint8_t size = 0;
    uint16_t mask = 0;
    bool match = true;

    for (uint8_t i = 0; i < sysexLength; i++) {
        uint8_t receivedByte = sysexBlock.peek(i);

        if (sysexMessage[j] == VARIABLE_PARAMETER) {
            pPos = sysexMessage[j + 1];
            bPos = sysexMessage[j + 2];
            size = sysexMessage[j + 3];
            mask = createMask(bPos, size);
            System::logger.write(LOG_TRACE,
                                 "pPos=%d, bPos=%d, size=%d, mask=%02x",
                                 pPos,
                                 bPos,
                                 size,
                                 mask);
            parameterNumber |= (((receivedByte & mask) >> bPos) << pPos);
            j += 4;
        } else if (sysexMessage[j] == VARIABLE_DATA) {
            pPos = sysexMessage[j + 4];
            bPos = sysexMessage[j + 5];
            size = sysexMessage[j + 6];
            mask = createMask(bPos, size);
            System::logger.write(LOG_TRACE,
                                 "pPos=%d, bPos=%d, size=%d, mask=%02x",
                                 pPos,
                                 bPos,
                                 size,
                                 mask);
            value |= (((receivedByte & mask) >> bPos) << pPos);
            j += 7;
        } else if (receivedByte == 0xF7) {
            break;
        } else if (receivedByte != sysexMessage[j]) {
            match = false;
            return (false);
        }
        j++;
    }
    if (match == true) {
        parameterMap.setValue(device.getId(),
                              Message::Type::sysex,
                              parameterNumber,
                              value,
                              Origin::midi);
        System::logger.write(
            LOG_ERROR,
            "Midi::processSysexData: updating parameter value: "
            "parameterNumber=%d, value=%d",
            parameterNumber,
            value);
        return (true);
    }
    return (false);
}

bool Midi::processResponse(const Response &response,
                           const Device &device,
                           SysexBlock &sysexBlock)
{
    // \todo this can easily overflow the buffer
    uint8_t header[64];
    uint8_t headerLength = 0;
    // \todo the parameterNumber 0 does not belong here
    headerLength = transformMessage(0, device, response.headers, header);

    if (doesHeaderMatch(sysexBlock, header, headerLength) == true) {
        System::logger.write(
            LOG_ERROR,
            "Midi::processSysex: matched response: responseId=%d",
            response.getId());

        resetRulesValues(device, response.rules);
        applyRulesValues(device, response.rules, sysexBlock, headerLength);

        // Run Lua onResponse function
        if (L) {
            runOnResponse(device, response.getId(), sysexBlock);
        }
        return (true);
    }
    return (false);
}

bool Midi::doesHeaderMatch(const SysexBlock &sysexBlock,
                           uint8_t header[],
                           uint8_t headerLength)
{
    bool match = true;
    auto sysexLength = sysexBlock.getLength();

    if (sysexLength > headerLength) {
        for (uint8_t i = 0; i < headerLength; i++) {
            if (header[i] != sysexBlock.peek(i)) {
                match = false;
                break;
            }
        }
    } else {
        match = false;
    }
    return (match);
}

void Midi::resetRulesValues(const Device &device, const Rules rules)
{
    for (const auto &rule : rules) {
        parameterMap.setValue(device.getId(),
                              rule.getType(),
                              rule.getParameterNumber(),
                              0,
                              Origin::midi);
    }
}

void Midi::applyRulesValues(const Device &device,
                            const Rules rules,
                            const SysexBlock &sysexBlock,
                            uint16_t headerLength)
{
    for (const auto &rule : rules) {
        uint16_t bytePosition = rule.getByte() + headerLength;
        int sysexByte = sysexBlock.peek(bytePosition);

        // If the sysexBlock byte exists
        if (sysexByte != -1) {
            uint16_t mask =
                createMask(rule.getByteBitPosition(), rule.getBitWidth());
            uint16_t parameterValue = (((sysexByte & mask) >> getShift(mask))
                                       << rule.getParameterBitPosition());
            LookupEntry *entry =
                parameterMap.applyToValue(device.getId(),
                                          rule.getType(),
                                          rule.getParameterNumber(),
                                          parameterValue,
                                          Origin::midi);

            if (entry) {
                System::logger.write(
                    LOG_ERROR,
                    "Midi::applyRulesValues: applying extraction rule: byte=%d, "
                    "byteValue=%d, extractedValue=%d to parameterNumber=%d, type=%s "
                    "resulting in parameterValue=%d",
                    rule.getByte(),
                    sysexByte,
                    parameterValue,
                    rule.getParameterNumber(),
                    Message::translateType(rule.getType()),
                    entry->getMidiValue());
            }
        }
    }
}

void Midi::requestAllPatches(void)
{
    for (auto &[id, device] : model.devices) {
        for (const auto &request : device.requests) {
            patchRequests.push(
                PatchRequest(device.getPort(), device.getId(), request));
        }

        if (L) {
            runOnRequest(device);
        }
    }
}

void Midi::sendControlChange(uint8_t port,
                             uint8_t channel,
                             uint8_t parameterNumber,
                             uint8_t value)
{
    MidiOutput::sendControlChange(
        MidiInterface::Type::MidiAll, port, channel, parameterNumber, value);
}

void Midi::sendControlChange14Bit(uint8_t port,
                                  uint8_t channel,
                                  uint16_t parameterNumber,
                                  uint16_t midiValue,
                                  bool lsbFirst)
{
    MidiOutput::sendControlChange14Bit(MidiInterface::Type::MidiAll,
                                       port,
                                       channel,
                                       parameterNumber,
                                       midiValue,
                                       lsbFirst);
}

void Midi::sendNrpn(uint8_t port,
                    uint8_t channel,
                    uint16_t parameterNumber,
                    uint16_t midiValue,
                    bool lsbFirst,
                    bool resetRpn)
{
    MidiOutput::sendNrpn(MidiInterface::Type::MidiAll,
                         port,
                         channel,
                         parameterNumber,
                         midiValue,
                         lsbFirst,
                         resetRpn);
}

void Midi::sendRpn(uint8_t port,
                   uint8_t channel,
                   uint16_t parameterNumber,
                   uint16_t midiValue)
{
    MidiOutput::sendRpn(MidiInterface::Type::MidiAll,
                        port,
                        channel,
                        parameterNumber,
                        midiValue);
}

void Midi::sendProgramChange(uint8_t port,
                             uint8_t channel,
                             uint8_t programNumber)
{
    MidiOutput::sendProgramChange(
        MidiInterface::Type::MidiAll, port, channel, programNumber);
}

void Midi::sendNoteOn(uint8_t port,
                      uint8_t channel,
                      uint8_t noteNumber,
                      uint8_t velocity)
{
    MidiOutput::sendNoteOn(
        MidiInterface::Type::MidiAll, port, channel, noteNumber, velocity);
}

void Midi::sendNoteOff(uint8_t port,
                       uint8_t channel,
                       uint8_t noteNumber,
                       uint8_t velocity)
{
    MidiOutput::sendNoteOff(
        MidiInterface::Type::MidiAll, port, channel, noteNumber, velocity);
}

void Midi::sendStart(uint8_t port)
{
    MidiOutput::sendStart(MidiInterface::Type::MidiAll, port);
}

void Midi::sendStop(uint8_t port)
{
    MidiOutput::sendStop(MidiInterface::Type::MidiAll, port);
}

void Midi::sendTuneRequest(uint8_t port)
{
    MidiOutput::sendTuneRequest(MidiInterface::Type::MidiAll, port);
}

void Midi::sendSysEx(uint8_t port, uint8_t *sysexData, uint16_t sysexDataLength)
{
    MidiOutput::sendSysEx(
        MidiInterface::Type::MidiAll, port, sysexData, sysexDataLength);
}

void Midi::sendSysEx(uint8_t port, SysexBlock &sysexBlock)
{
    MidiOutput::sendSysEx(MidiInterface::Type::MidiAll, port, sysexBlock);
}

void Midi::sendAfterTouchChannel(uint8_t port,
                                 uint8_t channel,
                                 uint8_t pressure)
{
    MidiOutput::sendAfterTouchChannel(
        MidiInterface::Type::MidiAll, port, channel, pressure);
}

void Midi::sendAfterTouchPoly(uint8_t port,
                              uint8_t channel,
                              uint8_t noteNumber,
                              uint8_t value)
{
    MidiOutput::sendAfterTouchPoly(
        MidiInterface::Type::MidiAll, port, channel, noteNumber, value);
}

void Midi::sendPitchBend(uint8_t port, uint8_t channel, uint16_t value)
{
    MidiOutput::sendPitchBend(
        MidiInterface::Type::MidiAll, port, channel, value);
}

void Midi::sendSongPosition(uint8_t port, uint16_t beats)
{
    MidiOutput::sendSongPosition(MidiInterface::Type::MidiAll, port, beats);
}
