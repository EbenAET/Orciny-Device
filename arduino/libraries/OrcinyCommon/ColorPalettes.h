// =============================================================================
// ColorPalettes.h
// Version : V 0.2.0
// Color palette definitions for Orciny beam (RGB) and NeoPixel effects.
// =============================================================================
//
// BEAM PALETTES
//   BeamPalette defines the peak RGB values for the sinusoidal beam swell.
//   The swell envelope (0–255) is modulated by these base colours.
//
//   CALLING:
//     Use with applyBeamPalette() to get scaled RGB values for direct PWM writes:
//       uint8_t r, g, b;
//       uint8_t swell = 128;  // 0–255 envelope value
//       applyBeamPalette(ColorPalettes::kBeamEmber, swell, r, g, b);
//       analogWrite(BEAM_RED_PIN, r);
//       analogWrite(BEAM_GREEN_PIN, g);
//       analogWrite(BEAM_BLUE_PIN, b);
//
//   CUSTOMIZING:
//     Edit the palette struct directly to create new colours:
//       static constexpr BeamPalette kBeamMyColor = {
//         .red   = 255,   // 0–255, red channel peak
//         .green = 128,   // 0–255, green channel peak
//         .blue  = 64,    // 0–255, blue channel peak
//       };
//     Valid peak values: 0–255. Higher values = brighter channel in the swell.
//
// NEO PALETTES
//   NeoPalette holds RGBW values for setting all NeoPixels to a solid colour.
//   Use neoPixelSetAllFromPalette() helper to apply.
//
//   CALLING:
//     Apply instantly to all pixels:
//       neoPixelSetAllFromPalette(strip, ColorPalettes::kNeoCyan);
//
//     Or use in a loop() for dynamic switching:
//       uint32_t now = millis();
//       if (now % 2000 < 1000) {
//         neoPixelSetAllFromPalette(strip, ColorPalettes::kNeoEmber);
//       } else {
//         neoPixelSetAllFromPalette(strip, ColorPalettes::kNeoCool);
//       }
//
//   CUSTOMIZING:
//     Create a new palette by adding to the namespace:
//       static constexpr NeoPalette kNeoMyColor = {
//         .red   = 200,   // 0–255, red channel
//         .green = 100,   // 0–255, green channel
//         .blue  = 50,    // 0–255, blue channel
//         .white = 30,    // 0–255, white (warm) channel for SK6812 RGBW
//       };
//     White channel tip: Use between 0–50 for subtle warmth; 0 for pure RGB.
//
// =============================================================================

#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

namespace ColorPalettes {

// =============================================================================
// BEAM PALETTES — Peak RGB values for the beam swell effect
// =============================================================================

struct BeamPalette {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

// Cool white — bright cyan-ish swell (original default)
static constexpr BeamPalette kBeamCoolWhite = {
  .red   = 40,
  .green = 220,
  .blue  = 255,
};

// Warm ember — orange-red dying embers
static constexpr BeamPalette kBeamEmber = {
  .red   = 255,
  .green = 100,
  .blue  = 20,
};

// Pure cyan — ice-like glow
static constexpr BeamPalette kBeamCyan = {
  .red   = 0,
  .green = 255,
  .blue  = 255,
};

// Violet — purple twilight
static constexpr BeamPalette kBeamViolet = {
  .red   = 200,
  .green = 100,
  .blue  = 255,
};

// Deep red — blood-like intensity
static constexpr BeamPalette kBeamDeepRed = {
  .red   = 255,
  .green = 20,
  .blue  = 40,
};

// Golden amber — warm incandescent glow
static constexpr BeamPalette kBeamGolden = {
  .red   = 255,
  .green = 180,
  .blue  = 30,
};

// Lime green — acid neon
static constexpr BeamPalette kBeamLime = {
  .red   = 150,
  .green = 255,
  .blue  = 0,
};

// Magenta — intense pink-purple
static constexpr BeamPalette kBeamMagenta = {
  .red   = 255,
  .green = 0,
  .blue  = 200,
};

// =============================================================================
// NEO PALETTES — RGBW values for NeoPixel solid colors
// =============================================================================

struct NeoPalette {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t white;
};

// Cool white — pure daylight
static constexpr NeoPalette kNeoCoolWhite = {
  .red   = 100,
  .green = 200,
  .blue  = 255,
  .white = 100,
};

// Warm white — incandescent bulb warmth
static constexpr NeoPalette kNeoWarmWhite = {
  .red   = 255,
  .green = 200,
  .blue  = 100,
  .white = 80,
};

// Ember orange — warm glowing embers
static constexpr NeoPalette kNeoEmber = {
  .red   = 255,
  .green = 100,
  .blue  = 20,
  .white = 0,
};

// Pure cyan — icy cool glow
static constexpr NeoPalette kNeoCyan = {
  .red   = 0,
  .green = 255,
  .blue  = 255,
  .white = 50,
};

// Violet — mystical purple
static constexpr NeoPalette kNeoViolet = {
  .red   = 180,
  .green = 80,
  .blue  = 255,
  .white = 0,
};

// Deep red — intense crimson
static constexpr NeoPalette kNeoDeepRed = {
  .red   = 255,
  .green = 20,
  .blue  = 40,
  .white = 0,
};

// Forest green — deep natural green
static constexpr NeoPalette kNeoForestGreen = {
  .red   = 20,
  .green = 150,
  .blue  = 60,
  .white = 0,
};

// Ocean blue — deep water
static constexpr NeoPalette kNeoOceanBlue = {
  .red   = 20,
  .green = 100,
  .blue  = 200,
  .white = 0,
};

// Golden — warm luminous glow
static constexpr NeoPalette kNeoGolden = {
  .red   = 255,
  .green = 180,
  .blue  = 0,
  .white = 80,
};

// Lime — acid-green neon
static constexpr NeoPalette kNeoLime = {
  .red   = 150,
  .green = 255,
  .blue  = 0,
  .white = 30,
};

// Magenta — hot pink-purple
static constexpr NeoPalette kNeoMagenta = {
  .red   = 255,
  .green = 0,
  .blue  = 180,
  .white = 0,
};

// Soft pink — gentle rose
static constexpr NeoPalette kNeoSoftPink = {
  .red   = 255,
  .green = 150,
  .blue  = 180,
  .white = 40,
};

// Off — all dark
static constexpr NeoPalette kNeoOff = {
  .red   = 0,
  .green = 0,
  .blue  = 0,
  .white = 0,
};

}  // namespace ColorPalettes

// =============================================================================
// HELPER FUNCTIONS — Apply palettes to effects
// =============================================================================

// Convert a BeamPalette to scoped values for direct PWM writes
inline void applyBeamPalette(const ColorPalettes::BeamPalette &palette,
                             uint8_t swell_level,
                             uint8_t &out_red, uint8_t &out_green, uint8_t &out_blue) {
  // Scale palette values by the swell envelope (0–255)
  out_red   = static_cast<uint8_t>((static_cast<uint16_t>(swell_level) * palette.red)   / 255);
  out_green = static_cast<uint8_t>((static_cast<uint16_t>(swell_level) * palette.green) / 255);
  out_blue  = static_cast<uint8_t>((static_cast<uint16_t>(swell_level) * palette.blue)  / 255);
}

// Set all NeoPixels to a solid palette color
// Requires: Adafruit_NeoPixel strip object with .setPixelColor() and .show() methods
inline void neoPixelSetAllFromPalette(Adafruit_NeoPixel &strip,
                                      const ColorPalettes::NeoPalette &palette) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(palette.red, palette.green, palette.blue, palette.white));
  }
  strip.show();
}

#endif
