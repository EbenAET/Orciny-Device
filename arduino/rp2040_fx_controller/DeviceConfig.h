#pragma once

#include <Arduino.h>

namespace device_config {

static constexpr uint32_t kUsbBaudRate = 115200;
static constexpr uint32_t kCoreLinkBaudRate = 115200;
static constexpr uint32_t kCoreLinkPeriodMs = 40;
static constexpr uint16_t kSwitchDebounceMs = 30;
static constexpr uint32_t kResetHoldMs = 5000;
static constexpr uint8_t kServoDriverI2cAddress = 0x40;

static constexpr uint8_t kSparkCount = 4;
static constexpr uint8_t kSparkPins[kSparkCount] = {5, 6, 9, 10};
// static constexpr uint8_t kPumpControlPin = 8;

static constexpr uint8_t kPowerSwitchPin = 2;
static constexpr uint8_t kPreviousSwitchPin = 3;
static constexpr uint8_t kNextSwitchPin = 4;

// Prop-Maker FeatherWing MOSFET LED outputs for high-power lighting.
static constexpr uint8_t kPropMakerLed1Pin = 12;
static constexpr uint8_t kPropMakerLed2Pin = 13;
static constexpr uint8_t kPropMakerLed3Pin = 14;

// 8-channel Servo FeatherWing (PCA9685) channels/pulse timing.
static constexpr uint8_t kServoChannelA = 0;
static constexpr uint8_t kServoChannelB = 1;
static constexpr uint16_t kServoMinPulse = 120;
static constexpr uint16_t kServoMaxPulse = 600;
static constexpr uint8_t kServoAMinAngle = 22;
static constexpr uint8_t kServoAMaxAngle = 120;
static constexpr uint8_t kServoBMinAngle = 22;
static constexpr uint8_t kServoBMaxAngle = 120;
static constexpr uint16_t kClawStepIntervalMs = 8;
static constexpr uint16_t kEffectCommandTimeoutMs = 400;

}  // namespace device_config