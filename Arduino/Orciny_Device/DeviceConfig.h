// =============================================================================
// DeviceConfig.h
// Version : V 0.6.7
// Centralized pin and parameter definitions for Orciny Device
// =============================================================================
#ifndef ORCINY_DEVICE_CONFIG_H
#define ORCINY_DEVICE_CONFIG_H

// Pin assignments (update only if hardware changes)
#define PULSE_FILAMENT_PIN   0   // PWM capable (fading effect)
#define PUMP_PIN             1   // PWM/digital (MOSFET)
#define PROP_MAKER_PWR_PIN   10  // Must be HIGH for MOSFETs to work
#define BEAM_RED_PIN         11  // PWM
#define BEAM_GREEN_PIN       12  // PWM
#define BEAM_BLUE_PIN        13  // PWM
#define SPARK_PIN_1          18  // PWM
#define SPARK_PIN_2          19  // PWM
#define SPARK_PIN_3          20  // PWM
#define SPARK_PIN_4          24  // PWM
#define NEO_DATA_PIN         25  // NeoPixel strip
#define NEO_PIXEL_COUNT      166 // Number of NeoPixels
#define NEO_COLOR_ORDER      NEO_GRB
#define SW_POWER_PIN         27  // SW1
#define SW_PREV_PIN          28  // SW2
#define SW_NEXT_PIN          29  // SW3

// Tuning parameters
#define DEBOUNCE_MS          30    // ms debounce for switches
#define RESET_HOLD_MS        5000  // ms to hold SW1+SW3 for reset
#define SERVO_I2C_ADDR       0x40  // PCA9685 I2C address
#define SERVO_PULSE_MIN      120   // Servo min pulse (0 deg)
#define SERVO_PULSE_MAX      600   // Servo max pulse (180 deg)

#endif // ORCINY_DEVICE_CONFIG_H
