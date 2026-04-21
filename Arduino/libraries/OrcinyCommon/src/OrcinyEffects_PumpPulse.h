// Animate pump and pulse filament outputs with semi-random timing and power.
//   pumpPin: digital pin for pump MOSFET (HIGH=on, LOW=off)
//   filamentPin: PWM pin for pulse filament (0–255)
//   startMs: millis() when animation begins
//   now: current millis()
//   durationMs: animation duration (0 = infinite)
// Returns true if animation is running, false if complete (if duration > 0)
inline bool PumpPulseAnimationRandomized(uint8_t pumpPin, uint8_t filamentPin, uint32_t startMs, uint32_t now, uint32_t durationMs = 0) {
  uint32_t elapsed = now - startMs;
  if (durationMs && elapsed >= durationMs) {
    digitalWrite(pumpPin, LOW);
    analogWrite(filamentPin, 0);
    return false;
  }

  // --- Pump: semi-random ON/OFF durations (2-4s each), random power 60-100% ---
  static uint32_t pumpPhaseStartMs = 0;
  static bool pumpOn = false;
  static uint16_t pumpPhaseDuration = 2000;
  static uint8_t pumpPower = 255;
  if (elapsed == 0) {
    // Reset on new animation
    pumpPhaseStartMs = 0;
    pumpOn = false;
    pumpPhaseDuration = 2000;
    pumpPower = 255;
  }
  if (now - pumpPhaseStartMs >= pumpPhaseDuration) {
    pumpOn = !pumpOn;
    pumpPhaseStartMs = now;
    pumpPhaseDuration = random(2000, 4001); // 2-4s
    if (pumpOn) {
      pumpPower = random(153, 256); // 60%-100% (153/255 ~ 60%)
    }
  }
  if (pumpOn) {
    analogWrite(pumpPin, pumpPower);
  } else {
    analogWrite(pumpPin, 0);
  }


  // --- Filament: flicker effect (random short intervals and brightness) ---
  static uint32_t filamentFlickerStartMs = 0;
  static uint16_t filamentFlickerDuration = 50;
  static uint8_t filamentLevel = 128;
  if (elapsed == 0) {
    filamentFlickerStartMs = 0;
    filamentFlickerDuration = 50;
    filamentLevel = 128;
  }
  if (now - filamentFlickerStartMs >= filamentFlickerDuration) {
    filamentFlickerStartMs = now;
    filamentFlickerDuration = random(30, 120); // Flicker interval: 30–120ms
    filamentLevel = random(80, 255); // Flicker brightness: 80–255
  }
  analogWrite(filamentPin, filamentLevel);

  return true;
}
// =============================================================================
// OrcinyEffects_PumpPulse.h
// Version : V 0.7.0
// Modular pump and pulse filament animation for Orciny Device
// =============================================================================
#pragma once
#include <Arduino.h>

namespace OrcinyEffects {

// Animate pump and pulse filament outputs.
//   pumpPin: digital pin for pump MOSFET (HIGH=on, LOW=off)
//   filamentPin: PWM pin for pulse filament (0–255)
//   startMs: millis() when animation begins
//   now: current millis()
//   durationMs: animation duration (0 = infinite)
// Returns true if animation is running, false if complete (if duration > 0)
inline bool PumpPulseAnimation(uint8_t pumpPin, uint8_t filamentPin, uint32_t startMs, uint32_t now, uint32_t durationMs = 0) {
  uint32_t elapsed = now - startMs;
  if (durationMs && elapsed >= durationMs) {
    digitalWrite(pumpPin, LOW);
    analogWrite(filamentPin, 0);
    return false;
  }
  // Pump: 3s on, 3s off cycle
  uint32_t pumpPhase = (elapsed / 3000) % 2;
  digitalWrite(pumpPin, pumpPhase ? HIGH : LOW);
  // Pulse Filament: 8s fade up/down (gradual)
  uint32_t filamentPhase = elapsed % 8000;
  uint8_t filamentLevel = (filamentPhase < 4000)
    ? map(filamentPhase, 0, 4000, 0, 255)
    : map(filamentPhase, 4000, 8000, 255, 0);
  analogWrite(filamentPin, filamentLevel);
  return true;
}

// Hold filament at full brightness, pump off
inline void PumpPulseHold(uint8_t pumpPin, uint8_t filamentPin) {
  digitalWrite(pumpPin, LOW);
  analogWrite(filamentPin, 255);
}

// Turn off both pump and filament outputs
inline void PumpPulseOff(uint8_t pumpPin, uint8_t filamentPin) {
  digitalWrite(pumpPin, LOW);
  analogWrite(filamentPin, 0);
}

} // namespace OrcinyEffects
