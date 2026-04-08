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

// Single NeoPixel strand (Adafruit 4865, SK6812, 166 pixels)
static constexpr uint8_t kCoreDataPin = 13;
static constexpr uint16_t kTotalPixels = 166;
static constexpr neoPixelType kColorOrder = NEO_GRB;

// Guard rail: keep worst-case full-white equivalent strip current at or below 2A.
static constexpr uint16_t kNeoPixelCurrentLimitMa = 2000;
static constexpr uint16_t kNeoPixelWorstCaseMaPerPixel = 60;
static constexpr uint8_t kMaxBrightnessForCurrentLimit =
	(static_cast<uint32_t>(kNeoPixelCurrentLimitMa) * 255U) /
	(static_cast<uint32_t>(kTotalPixels) * kNeoPixelWorstCaseMaPerPixel);
static_assert(kMaxBrightnessForCurrentLimit > 0, "NeoPixel current limit too low for strip length");

}  // namespace device_config