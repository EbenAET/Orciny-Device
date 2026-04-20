// =============================================================================
// AnimationPalettes.h
// Version : V 0.3.7
// Animation preset definitions for Orciny beam, NeoPixel, and spark effects.
// =============================================================================
//
// BEAM ANIMATIONS — Swell envelope patterns
//   Defines timing and modulation for the beam breathing/pulsing effect.
//   All animation minutiae for beam actions are encapsulated here.
//
// NEO ANIMATIONS — NeoPixel animation modes
//   Preset patterns with timing and color cycling parameters.
//   All animation minutiae for NeoPixel actions are encapsulated here.
//
// SPARK ANIMATIONS — Spark burst and timing patterns
//   Controls flash intensity, frequency, and burst characteristics.
//   All animation minutiae for spark actions are encapsulated here.
// =============================================================================

#pragma once

#include <Arduino.h>

namespace AnimationPalettes {

// =============================================================================
// BEAM ANIMATIONS — Sinusoidal swell envelope presets
// =============================================================================

struct BeamAnimation {
  uint16_t cycleMs;        // Total cycle duration in milliseconds
  uint8_t  minIntensity;   // Minimum swell level (0–255)
  uint8_t  maxIntensity;   // Maximum swell level (0–255)

  // Calculate current swell envelope (0–maxIntensity) based on elapsed time
  uint8_t getCurrentSwell(uint32_t now) const {
    uint16_t phase = now % cycleMs;

    // Linear ramp: first half brightens, second half dims
    if (phase < cycleMs / 2) {
      // Ramp up: minIntensity → maxIntensity
      return map(phase, 0, cycleMs / 2, minIntensity, maxIntensity);
    } else {
      // Ramp down: maxIntensity → minIntensity
      return map(phase, cycleMs / 2, cycleMs, maxIntensity, minIntensity);
    }
  }
};

// Slow breathing — gentle, continuous swell (2.4 second cycle)
static constexpr BeamAnimation kBeamSwell = {
  .cycleMs       = 2400,
  .minIntensity  = 80,
  .maxIntensity  = 255,
};

// Slow pulse — subtle ambient glow
static constexpr BeamAnimation kBeamPulseSlow = {
  .cycleMs       = 3000,
  .minIntensity  = 100,
  .maxIntensity  = 220,
};

// Medium pulse — noticeable breathing effect
static constexpr BeamAnimation kBeamPulseMedium = {
  .cycleMs       = 1500,
  .minIntensity  = 60,
  .maxIntensity  = 255,
};

// Fast pulse — quick rhythmic flashing
static constexpr BeamAnimation kBeamPulseFast = {
  .cycleMs       = 800,
  .minIntensity  = 40,
  .maxIntensity  = 255,
};

// Heartbeat — dramatic double-pulse (ba-DUM, ba-DUM)
// Uses a custom handler outside this struct for best effect
static constexpr BeamAnimation kBeamHeartbeat = {
  .cycleMs       = 1200,
  .minIntensity  = 0,
  .maxIntensity  = 255,
};

// Strobe — intense on/off flashing
static constexpr BeamAnimation kBeamStrobe = {
  .cycleMs       = 400,
  .minIntensity  = 0,
  .maxIntensity  = 255,
};

// Subtle fade — very slow, almost imperceptible change
static constexpr BeamAnimation kBeamFadeSlow = {
  .cycleMs       = 6000,
  .minIntensity  = 120,
  .maxIntensity  = 200,
};

// =============================================================================
// NEO ANIMATIONS — NeoPixel animation mode descriptors
// =============================================================================

enum NeoAnimationMode : uint8_t {
  NEO_MODE_SOLID = 0,    // Single static color
  NEO_MODE_PULSE,        // Breathing/pulsing brightness
  NEO_MODE_CHASE,        // Running light chase
  NEO_MODE_RAINBOW,      // Cycling rainbow hues
  NEO_MODE_TWINKLE,      // Random pixel sparkle
  NEO_MODE_FLAME,        // Flickering flame-like effect
  NEO_MODE_WAVE,         // Traveling brightness wave
  NEO_MODE_FLARE,        // Bright flash then fadeout
};

struct NeoAnimation {
  NeoAnimationMode mode;
  uint16_t cycleMs;           // Animation cycle duration
  uint16_t stepIntervalMs;    // Update interval per animation step
  uint8_t  intensity;         // Peak brightness (0–255)
  uint8_t  saturation;        // For rainbow modes: 0=white, 255=vivid
};

// Solid color — no animation, static display
static constexpr NeoAnimation kNeoSolid = {
  .mode             = NEO_MODE_SOLID,
  .cycleMs          = 0,
  .stepIntervalMs   = 0,
  .intensity        = 200,
  .saturation       = 255,
};

// Gentle pulse — slow breathing of current color
static constexpr NeoAnimation kNeoPulseSlow = {
  .mode             = NEO_MODE_PULSE,
  .cycleMs          = 3000,
  .stepIntervalMs   = 30,
  .intensity        = 200,
  .saturation       = 255,
};

// Medium pulse — noticeable pulsing
static constexpr NeoAnimation kNeoPulseMedium = {
  .mode             = NEO_MODE_PULSE,
  .cycleMs          = 1500,
  .stepIntervalMs   = 20,
  .intensity        = 220,
  .saturation       = 255,
};

// Fast pulse — quick rhythmic flashing
static constexpr NeoAnimation kNeoPulseFast = {
  .mode             = NEO_MODE_PULSE,
  .cycleMs          = 600,
  .stepIntervalMs   = 10,
  .intensity        = 255,
  .saturation       = 255,
};

// Running chase — light travels down the strip
static constexpr NeoAnimation kNeoChase = {
  .mode             = NEO_MODE_CHASE,
  .cycleMs          = 2000,
  .stepIntervalMs   = 15,
  .intensity        = 220,
  .saturation       = 255,
};

// Fast chase — quick traveling light
static constexpr NeoAnimation kNeoChaseFast = {
  .mode             = NEO_MODE_CHASE,
  .cycleMs          = 800,
  .stepIntervalMs   = 8,
  .intensity        = 255,
  .saturation       = 255,
};

// Rainbow cycle — slowly cycling through spectrum
static constexpr NeoAnimation kNeoRainbow = {
  .mode             = NEO_MODE_RAINBOW,
  .cycleMs          = 5000,
  .stepIntervalMs   = 30,
  .intensity        = 200,
  .saturation       = 255,
};

// Rainbow fast — quick spectrum cycle
static constexpr NeoAnimation kNeoRainbowFast = {
  .mode             = NEO_MODE_RAINBOW,
  .cycleMs          = 2000,
  .stepIntervalMs   = 15,
  .intensity        = 220,
  .saturation       = 255,
};

// Twinkle — random pixel sparkles
static constexpr NeoAnimation kNeoTwinkle = {
  .mode             = NEO_MODE_TWINKLE,
  .cycleMs          = 3000,
  .stepIntervalMs   = 100,
  .intensity        = 200,
  .saturation       = 255,
};

// Flame — flickering fire-like effect
static constexpr NeoAnimation kNeoFlame = {
  .mode             = NEO_MODE_FLAME,
  .cycleMs          = 1000,
  .stepIntervalMs   = 50,
  .intensity        = 220,
  .saturation       = 150,  // Lower sat for warm orange tones
};

// Wave — traveling brightness wave across strip
static constexpr NeoAnimation kNeoWave = {
  .mode             = NEO_MODE_WAVE,
  .cycleMs          = 2500,
  .stepIntervalMs   = 20,
  .intensity        = 210,
  .saturation       = 255,
};

// Flare — bright flash followed by fade
static constexpr NeoAnimation kNeoFlare = {
  .mode             = NEO_MODE_FLARE,
  .cycleMs          = 1000,
  .stepIntervalMs   = 30,
  .intensity        = 255,
  .saturation       = 255,
};

// =============================================================================
// SPARK ANIMATIONS — Spark burst and timing patterns
// =============================================================================

struct SparkAnimation {
  uint8_t  peakIntensity;        // Maximum PWM brightness (0–255)
  uint16_t minFlashMs;           // Shortest flash duration
  uint16_t maxFlashMs;           // Longest flash duration
  uint16_t minGapMs;             // Shortest gap between flashes
  uint16_t maxGapMs;             // Longest gap between flashes
  uint8_t  intensityFloor;       // Minimum brightness during flash (raises floor)
};

// Steady burn — constant glow with minimal variation
static constexpr SparkAnimation kSparkSteady = {
  .peakIntensity    = 180,
  .minFlashMs       = 200,
  .maxFlashMs       = 400,
  .minGapMs         = 100,
  .maxGapMs         = 250,
  .intensityFloor   = 120,
};

// Pop — quick bright flashes with long gaps
static constexpr SparkAnimation kSparkPop = {
  .peakIntensity    = 255,
  .minFlashMs       = 20,
  .maxFlashMs       = 60,
  .minGapMs         = 300,
  .maxGapMs         = 800,
  .intensityFloor   = 0,
};

// Crackle — rapid random bursts, chaotic energy
static constexpr SparkAnimation kSparkCrackle = {
  .peakIntensity    = 240,
  .minFlashMs       = 15,
  .maxFlashMs       = 100,
  .minGapMs         = 30,
  .maxGapMs         = 200,
  .intensityFloor   = 20,
};

// Pulse — rhythmic on/off flashing
static constexpr SparkAnimation kSparkPulse = {
  .peakIntensity    = 200,
  .minFlashMs       = 100,
  .maxFlashMs       = 150,
  .minGapMs         = 150,
  .maxGapMs         = 250,
  .intensityFloor   = 40,
};

// Intense — constant high energy sparking
static constexpr SparkAnimation kSparkIntense = {
  .peakIntensity    = 255,
  .minFlashMs       = 30,
  .maxFlashMs       = 120,
  .minGapMs         = 20,
  .maxGapMs         = 80,
  .intensityFloor   = 60,
};

// Slow burn — lazy, intermittent flashing
static constexpr SparkAnimation kSparkSlowBurn = {
  .peakIntensity    = 160,
  .minFlashMs       = 300,
  .maxFlashMs       = 600,
  .minGapMs         = 500,
  .maxGapMs         = 1500,
  .intensityFloor   = 0,
};

// Strobe — intense, rapid on/off
static constexpr SparkAnimation kSparkStrobe = {
  .peakIntensity    = 255,
  .minFlashMs       = 50,
  .maxFlashMs       = 100,
  .minGapMs         = 50,
  .maxGapMs         = 100,
  .intensityFloor   = 0,
};

}  // namespace AnimationPalettes

// =============================================================================
// HELPER FUNCTIONS — Apply animations to effects
// =============================================================================

// Calculate a sinusoidal brightness envelope (0–255) for smooth pulsing.
// phase: 0–100 representing position within a cycle
inline uint8_t getSineEnvelope(uint8_t phase) {
  // Simple lookup table for sine approximation (0–100 -> 0–255)
  static const uint8_t SineTable[101] = {
    0,   3,   6,   9,   12,  15,  18,  21,  24,  27,  30,  33,  36,  39,  42,
    45,  48,  51,  54,  57,  59,  62,  65,  67,  70,  73,  75,  77,  80,  82,
    84,  86,  89,  91,  93,  94,  96,  98,  99, 101, 102, 103, 104, 105, 106,
    106, 106, 106, 105, 104, 103, 102, 101,  99,  98,  96,  94,  93,  91,  89,
    86,  84,  82,  80,  77,  75,  73,  70,  67,  65,  62,  59,  57,  54,  51,
    48,  45,  42,  39,  36,  33,  30,  27,  24,  21,  18,  15,  12,   9,   6,
    3,   0
  };

  phase = constrain(phase, 0, 100);
  return SineTable[phase];
}

// Get rainbow hue (0–255 hue value) as a function of time in a cycle
inline uint8_t getRainbowHue(uint32_t now, uint16_t cycleMs) {
  uint16_t phase = now % cycleMs;
  return map(phase, 0, cycleMs, 0, 255);
}