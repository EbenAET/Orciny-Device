// =============================================================================
// OrcinyEffects_PumpPulse.h
// Version : V 0.1.0
// Pump and Pulse Filament Animation for Orciny Device
// =============================================================================
#pragma once
#include <Arduino.h>

namespace OrcinyEffects {

// Call this in your state logic to animate pump and pulse filament
// - pumpPin: digital pin for pump MOSFET
// - filamentPin: PWM pin for pulse filament
// - startMs: animation start time (millis() when animation begins)
// - now: current millis()
// - durationMs: how long to run the animation (0 = infinite)
//
// Returns true if animation is still running, false if complete (if duration > 0)
inline bool PumpPulseAnimation(uint8_t pumpPin, uint8_t filamentPin, uint32_t startMs, uint32_t now, uint32_t durationMs = 0) {
  uint32_t elapsed = now - startMs;
  if (durationMs && elapsed >= durationMs) {
    digitalWrite(pumpPin, LOW);
    analogWrite(filamentPin, 0);
    return false;
  }
  // Pump: 3s on, 3s off
  uint32_t pumpPhase = (elapsed / 3000) % 2;
  digitalWrite(pumpPin, pumpPhase ? HIGH : LOW);
  // Pulse Filament: 8s fade up/down (more gradual)
  uint32_t filamentPhase = elapsed % 8000;
  uint8_t filamentLevel = (filamentPhase < 4000)
    ? map(filamentPhase, 0, 4000, 0, 255)
    : map(filamentPhase, 4000, 8000, 255, 0);
  analogWrite(filamentPin, filamentLevel);
  return true;
}

// Call this to hold the filament at full after animation
inline void PumpPulseHold(uint8_t pumpPin, uint8_t filamentPin) {
  digitalWrite(pumpPin, LOW);
  analogWrite(filamentPin, 255);
}

// Call this to turn off both outputs
inline void PumpPulseOff(uint8_t pumpPin, uint8_t filamentPin) {
  digitalWrite(pumpPin, LOW);
  analogWrite(filamentPin, 0);
}

} // namespace OrcinyEffects
