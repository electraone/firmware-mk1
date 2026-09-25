#pragma once

#include "stdint.h"

enum class ChecksumAlgorithm { UNKNOWN = 0, ROLAND, WALDORF, FRACTALAUDIO };

uint8_t calculateChecksum(uint8_t *payload, uint8_t length);
uint8_t calculateChecksum_fractalaudio(uint8_t *payload, uint8_t length);
uint8_t calculateChecksum_waldorf(uint8_t *payload, uint8_t length);

ChecksumAlgorithm translateAlgorithm(const char *algorithm);
