#pragma once

#include <cstdint>

enum class SignMode { noSign, twosComplement, signBit, signBit2, binOffset };

SignMode translateSignMode(const char *signModeText);

uint16_t getTwosComplementMidiValue(uint8_t bitWidth,
                                    int16_t value,
                                    int16_t minValue,
                                    int16_t maxValue);
uint16_t getSignBitMidiValue(uint8_t bitWidth,
                             int16_t value,
                             int16_t minValue,
                             int16_t maxValue);
uint16_t getUnsignedMidiValue(int16_t value,
                              int16_t minValue,
                              int16_t maxValue,
                              uint16_t midiMin,
                              uint16_t midiMax);
uint16_t translateValueToMidiValue(SignMode signMode,
                                   uint8_t bitWidth,
                                   int16_t value,
                                   int16_t minValue,
                                   int16_t maxValue,
                                   uint16_t midiMin,
                                   uint16_t midiMax);

int16_t getTwosComplementValue(uint8_t bitWidth,
                               uint16_t midiValue,
                               int16_t minValue,
                               int16_t maxValue);
int16_t getSignBitValue(uint8_t bitWidth,
                        uint16_t midiValue,
                        int16_t minValue,
                        int16_t maxValue);
int16_t getUnsignedValue(uint16_t midiValue,
                         uint16_t midiMin,
                         uint16_t midiMax,
                         int16_t minValue,
                         int16_t maxValue);
int16_t translateMidiValueToValue(SignMode signMode,
                                  uint8_t bitWidth,
                                  uint16_t midiValue,
                                  uint16_t midiMin,
                                  uint16_t midiMax,
                                  int16_t minValue,
                                  int16_t maxValue);
