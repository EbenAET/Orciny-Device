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

// Single NeoPixel strand (30 pixels)
static constexpr uint8_t kCoreDataPin = 13;
static constexpr uint16_t kTotalPixels = 30;
static constexpr neoPixelType kColorOrder = NEO_GRB;

}  // namespace device_config