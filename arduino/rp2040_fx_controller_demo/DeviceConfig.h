#pragma once

#include <Arduino.h>

// =============================================================================
// DeviceConfig.h — Hardware pin map and tuning constants for the Orciny device.
// Version : V 0.1.0
//
// All preset numbers live here. Change a value once and every part of the
// firmware that uses it automatically picks up the change.
// =============================================================================

namespace device_config {

// --- Serial communication speeds -------------------------------------------
// kUsbBaudRate   : USB serial port (Serial)  used for the debug console.
// kCoreLinkBaudRate : Hardware UART (Serial1) used to receive EffectCommands
//                    from a secondary controller board.
static constexpr uint32_t kUsbBaudRate = 115200;
static constexpr uint32_t kCoreLinkBaudRate = 115200;

// How often (ms) a new CoreFrame is expected on Serial1. Unused in the current
// standalone-effect build but preserved for the core-link path.
static constexpr uint32_t kCoreLinkPeriodMs = 40;

// --- Switch debounce & reset ------------------------------------------------
// Mechanical buttons bounce — this is the minimum stable time (ms) before a
// press or release is treated as real.
static constexpr uint16_t kSwitchDebounceMs = 30;

// Hold SW1 + SW3 together for this many milliseconds to trigger a full reset.
static constexpr uint32_t kResetHoldMs = 5000;

// --- Servo driver -----------------------------------------------------------
// I2C address of the PCA9685 on the 8-Channel Servo FeatherWing.
// Default factory address is 0x40 (all address-select pads open).
static constexpr uint8_t kServoDriverI2cAddress = 0x40;

// --- Spark channels (GP18, GP19, GP20, GP24 via 10-ohm series resistors) ----
// Each channel is a bare GPIO driven with analogWrite() (PWM) and connected
// to a spark-gap element through a current-limiting resistor.
static constexpr uint8_t kSparkCount = 4;
static constexpr uint8_t kSparkPins[kSparkCount] = {18, 19, 20, 24};

// Peltier load control MOSFET gate (GP8).
// Keep enabled briefly after beam turns off to dissipate residual heat.
static constexpr uint8_t kPeltierControlPin = 8;
static constexpr uint16_t kPeltierPostBeamHoldMs = 12000;

// --- Momentary switches (INPUT_PULLUP — LOW = pressed) ----------------------
static constexpr uint8_t kPowerSwitchPin = A1;    // GP27 — SW1 on/off toggle
static constexpr uint8_t kPreviousSwitchPin = A2;  // GP28 — SW2 previous seq.
static constexpr uint8_t kNextSwitchPin = A3;      // GP29 — SW3 next sequence

// --- Prop-Maker FeatherWing LED outputs -------------------------------------
// GP10 (D10) is the Prop-Maker PWR enable pin and MUST be driven HIGH during
// setup() or the MOSFET load outputs will not work.
//
// LED1/2/3 are the three MOSFET channels controlling the high-power RGB beam:
//   LED1 (GP11) → Red   wire (H-BEAM-PWR pin 2)
//   LED2 (GP12) → Green wire (H-BEAM-PWR pin 4)
//   LED3 (GP13) → Blue  wire (H-BEAM-PWR pin 3)
// The LED is common-anode, so writing a higher PWM value = brighter.
static constexpr uint8_t kPropMakerPwrPin = 10;
static constexpr uint8_t kPropMakerLed1Pin = 11;  // Red channel
static constexpr uint8_t kPropMakerLed2Pin = 12;  // Green channel
static constexpr uint8_t kPropMakerLed3Pin = 13;  // Blue channel

// --- Servo FeatherWing (PCA9685) --------------------------------------------
// Channel assignments: CH0 = Servo A (claw left), CH1 = Servo B (claw right).
// Servo B sweeps mirrored to A so both halves move in opposition.
static constexpr uint8_t kServoChannelA = 0;
static constexpr uint8_t kServoChannelB = 1;

// Pulse width range mapped to 0–180° by the Adafruit driver.
// These values were tuned to the specific servo model installed.
static constexpr uint16_t kServoMinPulse = 120;
static constexpr uint16_t kServoMaxPulse = 600;

// Claw travel limits (degrees). Keep away from 0° and 180° to avoid binding.
static constexpr uint8_t kServoAMinAngle = 0;
static constexpr uint8_t kServoAMaxAngle = 40;
static constexpr uint8_t kServoBMinAngle = 90;
static constexpr uint8_t kServoBMaxAngle = 125;

// How often (ms) the claw advances one degree. Lower = faster sweep.
static constexpr uint16_t kClawStepIntervalMs = 15;

// If no EffectCommand arrives from Serial1 within this window (ms), the
// firmware falls back to defaultEffectCommand() (all outputs off).
static constexpr uint16_t kEffectCommandTimeoutMs = 400;

// --- NeoPixel strip (GP25, 166 pixels, SK6812 / Adafruit 4865) ---------------
// Single strand attached to +5V_BUS via servo wing stacked connector.
// Current limiting: 166 pixels * 60 mA worst-case per pixel = 10A theoretical
// max. Firmware uses global brightness cap to stay under 2A.
// GP25 is used to avoid conflict with Spark 1 on GP7; power from +5V_BUS (same domain as
// Servo FeatherWing + Prop-Maker). The 5V input is fused at 3A (F801).
static constexpr uint8_t kNeoPixelPin = 25;
static constexpr uint16_t kNeoPixelCount = 166;
static constexpr neoPixelType kNeoPixelType = NEO_GRB + NEO_KHZ800;  // SK6812
static constexpr uint16_t kNeoPixelCurrentLimitMa = 2000;
// Worst-case 60mA per pixel; firmware caps brightness to stay under limit.
static constexpr uint16_t kNeoPixelWorstCaseMaPerPixel = 60;
static constexpr uint8_t kMaxBrightnessForCurrentLimit =
    (static_cast<uint32_t>(kNeoPixelCurrentLimitMa) * 255U) /
    (static_cast<uint32_t>(kNeoPixelCount) * kNeoPixelWorstCaseMaPerPixel);
static_assert(kMaxBrightnessForCurrentLimit > 0,
              "NeoPixel current limit too low for strip length");

// --- Beam colour palettes -------------------------------------------------
// A BeamPalette specifies the peak RGB colour for the sinusoidal beam swell.
// The swell multiplier (0–255) scales each channel proportionally, so the
// same waveform shape applies to any colour.  Values are 0–255 per channel.
struct BeamPalette {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

static constexpr BeamPalette kPaletteCoolWhite = {  51, 255, 255 };  // cyan-white (classic)
static constexpr BeamPalette kPaletteEmber     = { 255,  70,   8 };  // warm orange-red
static constexpr BeamPalette kPaletteCyan      = {   0, 220, 255 };  // pure cyan
static constexpr BeamPalette kPaletteViolet    = { 180,   0, 255 };  // purple-violet
static constexpr BeamPalette kPaletteDefault   = kPaletteCoolWhite;

}  // namespace device_config
