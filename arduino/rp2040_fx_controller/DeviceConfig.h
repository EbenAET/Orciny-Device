#pragma once

#include <Arduino.h>

namespace device_config {

static constexpr uint32_t kUsbBaudRate = 115200;
static constexpr uint32_t kCoreLinkBaudRate = 115200;
static constexpr uint32_t kCoreLinkPeriodMs = 40;
static constexpr uint16_t kSwitchDebounceMs = 30;
static constexpr uint32_t kResetHoldMs = 5000;

static constexpr uint8_t kSparkCount = 4;
static constexpr uint8_t kSparkPins[kSparkCount] = {5, 6, 9, 10};
static constexpr uint8_t kPulseFilamentPin = 11;

static constexpr uint8_t kPowerSwitchPin = 2;
static constexpr uint8_t kPreviousSwitchPin = 3;
static constexpr uint8_t kNextSwitchPin = 4;

static constexpr uint8_t kBeamRedPin = 12;
static constexpr uint8_t kBeamGreenPin = 13;
static constexpr uint8_t kBeamBluePin = 14;
static constexpr uint8_t kPeltierEnablePin = 15;

static constexpr uint8_t kPumpControlPin = 8;
static constexpr uint8_t kClawServoPin = 7;
static constexpr uint8_t kClawMinAngle = 22;
static constexpr uint8_t kClawMaxAngle = 120;
static constexpr uint16_t kClawStepIntervalMs = 8;
static constexpr uint16_t kEffectCommandTimeoutMs = 400;

}  // namespace device_config