// =============================================================================
// DeviceConfig.h
// Version : V 0.3.7
// Centralized configuration for Orciny Device
// =============================================================================
#ifndef ORCINY_DEVICE_CONFIG_H
#define ORCINY_DEVICE_CONFIG_H

// Pin assignments
#define PROP_MAKER_PWR_PIN   10
#define BEAM_RED_PIN         11
#define BEAM_GREEN_PIN       12
#define BEAM_BLUE_PIN        13
#define SPARK_PIN_1          18
#define SPARK_PIN_2          19
#define SPARK_PIN_3          20
#define SPARK_PIN_4          24
#define NEO_DATA_PIN         25
#define NEO_PIXEL_COUNT      166
#define NEO_COLOR_ORDER      NEO_GRBW
#define SW_POWER_PIN         27
#define SW_PREV_PIN          28
#define SW_NEXT_PIN          29

// Tuning parameters
#define DEBOUNCE_MS          30
#define RESET_HOLD_MS        5000
#define SERVO_I2C_ADDR       0x40
#define SERVO_PULSE_MIN      120
#define SERVO_PULSE_MAX      600

#endif // ORCINY_DEVICE_CONFIG_H
