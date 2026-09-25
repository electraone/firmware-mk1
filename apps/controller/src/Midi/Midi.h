#pragma once

#include <vector>
#include <String>
#include <functional>
#include "MidiOutput.h"
#include "Device.h"
#include "Message.h"
#include "Preset.h"
#include "Cc14Detector.h"
#include "RpnDetector.h"

struct PatchRequest {
    PatchRequest() : port(0), deviceId(0)
    {
    }

    PatchRequest(uint8_t port, uint8_t deviceId, std::vector<uint8_t> data)
        : port(port), deviceId(deviceId), data(data)
    {
    }

    struct {
        uint8_t deviceId : 6;
        uint8_t port : 2;
    };
    std::vector<uint8_t> data;
};

extern CircularBuffer<PatchRequest, 32> patchRequests;

class Midi
{
public:
    explicit Midi(const Preset &preset);
    virtual ~Midi() = default;

    void sendMessage(const Message &message);
    void sendTemplatedSysex(const Device &device,
                            uint16_t parameterNumber,
                            std::vector<uint8_t> data);
    void process(const MidiInput &midiInput, const MidiMessage &midiMessage);
    void requestAllPatches(void);

private:
    uint8_t transformMessage(uint16_t parameterNumber,
                             const Device &deviceId,
                             std::vector<uint8_t> data,
                             uint8_t *dataOut);
    void runVariable(uint16_t parameterNumber,
                     uint16_t &i,
                     uint16_t &j,
                     std::vector<uint8_t> data,
                     uint8_t *dataOut,
                     const Device &device);
    void runParameter(uint16_t parameterNumber,
                      uint16_t &i,
                      uint16_t &j,
                      std::vector<uint8_t> data,
                      uint8_t *dataOut,
                      const Device &device);
    void runChecksum(uint16_t &i,
                     uint16_t &j,
                     std::vector<uint8_t> data,
                     uint8_t *dataOut,
                     const Device &device);
    void runLuaFunction(uint16_t parameterNumber,
                        uint16_t &i,
                        uint16_t &j,
                        std::vector<uint8_t> data,
                        uint8_t *dataOut,
                        const Device &device);
    void runConstant(uint16_t &i,
                     uint16_t &j,
                     std::vector<uint8_t> data,
                     uint8_t *dataOut,
                     const Device &device);
    void processStart(void);
    void processStop(void);
    void processTuneRequest(void);
    void
        processCc(uint8_t deviceId, uint8_t midiParameterId, uint8_t midiValue);
    void processNote(uint8_t deviceId,
                     MidiMessage::Type midiType,
                     uint8_t noteNumber,
                     uint8_t velocity);
    void processProgramChange(uint8_t deviceId, uint8_t programNumber);
    void processAfterTouchChannel(uint8_t deviceId, uint8_t pressure);
    void processAfterTouchPoly(uint8_t deviceId,
                               uint8_t noteNumber,
                               uint8_t pressure);
    void processPitchBend(uint8_t deviceId,
                          uint8_t valueFine,
                          uint8_t valueCoarse);
    void processSongPosition(uint8_t valueFine, uint8_t valueCoarse);
    void processSysex(const MidiMessage &midiMessage);

    bool processSysexData(const DataBytes &sysexMessage,
                          const Device &device,
                          SysexBlock &sysexBlock);
    bool processResponse(const Response &response,
                         const Device &device,
                         SysexBlock &sysexBlock);

    bool doesHeaderMatch(const SysexBlock &sysexBlock,
                         uint8_t header[],
                         uint8_t headerLength);
    void resetRulesValues(const Device &device, const Rules rules);
    void applyRulesValues(const Device &device,
                          const Rules rules,
                          const SysexBlock &sysexBlock,
                          uint16_t headerLength);

    static void sendControlChange(uint8_t port,
                                  uint8_t channel,
                                  uint8_t parameterNumber,
                                  uint8_t value);
    static void sendControlChange14Bit(uint8_t port,
                                       uint8_t channel,
                                       uint16_t parameterNumber,
                                       uint16_t midiValue,
                                       bool lsbFirst);
    static void sendNrpn(uint8_t port,
                         uint8_t channel,
                         uint16_t parameterNumber,
                         uint16_t midiValue,
                         bool lsbFirst,
                         bool resetRpn);
    static void sendRpn(uint8_t port,
                        uint8_t channel,
                        uint16_t parameterNumber,
                        uint16_t midiValue);
    static void
        sendProgramChange(uint8_t port, uint8_t channel, uint8_t programNumber);

    static void sendNoteOn(uint8_t port,
                           uint8_t channel,
                           uint8_t noteNumber,
                           uint8_t velocity);
    static void sendNoteOff(uint8_t port,
                            uint8_t channel,
                            uint8_t noteNumber,
                            uint8_t velocity);

    static void sendStart(uint8_t port);
    static void sendStop(uint8_t port);
    static void sendTuneRequest(uint8_t port);
    static void
        sendSysEx(uint8_t port, uint8_t *data, uint16_t sysexDataLength);
    static void sendSysEx(uint8_t port, SysexBlock &sysexBlock);
    static void
        sendAfterTouchChannel(uint8_t port, uint8_t channel, uint8_t pressure);
    static void sendAfterTouchPoly(uint8_t port,
                                   uint8_t channel,
                                   uint8_t noteNumber,
                                   uint8_t value);
    static void sendPitchBend(uint8_t port, uint8_t channel, uint16_t value);
    static void sendSongPosition(uint8_t port, uint16_t beats);

    const Preset &model;

    RpnDetector rpnDetector;
    Cc14Detector cc14Detector;
};
