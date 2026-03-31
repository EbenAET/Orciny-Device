#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

namespace device_config {

static constexpr uint32_t kUsbBaudRate = 115200;
static constexpr uint32_t kCoreLinkBaudRate = 115200;
static constexpr uint32_t kEffectLinkPeriodMs = 40;
static constexpr uint16_t kSwitchDebounceMs = 30;
static constexpr uint32_t kResetHoldMs = 5000;

static constexpr uint8_t kPowerSwitchPin = 2;
static constexpr uint8_t kPreviousSwitchPin = 3;
static constexpr uint8_t kNextSwitchPin = 4;

static constexpr uint8_t kActiveCoreStrips = 4;
static constexpr uint16_t kPixelsPerStrip = 30;
static constexpr neoPixelType kColorOrder = NEO_GRB;

// This pin map follows the NeoPXL8 SAMD21 guidance for preserving Serial1.
// Validate it against the exact Feather M0 + NeoPXL8 FeatherWing routing.
static int8_t kCorePins[8] = {13, 12, 11, 10, -1, -1, -1, -1};

}  // namespace device_config