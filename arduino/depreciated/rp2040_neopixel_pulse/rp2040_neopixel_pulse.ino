#include <Adafruit_NeoPixel.h>

namespace {

// KB2040 NeoPixel animation with two pixel groups:
// galaxy pixels pulse through cool colors, while star pixels glow warm.
constexpr uint8_t kNeoPixelPin = 10;
constexpr uint16_t kPixelCount = 100;
constexpr neoPixelType kPixelType = NEO_GRB + NEO_KHZ800;
constexpr uint16_t kFrameIntervalMs = 20;
constexpr uint16_t kGalaxyMinPulsePeriodMs = 2800;
constexpr uint16_t kGalaxyMaxPulsePeriodMs = 6400;
constexpr uint16_t kGalaxyMinColorBlendMs = 3500;
constexpr uint16_t kGalaxyMaxColorBlendMs = 9000;
constexpr uint16_t kStarMinPulsePeriodMs = 7200;
constexpr uint16_t kStarMaxPulsePeriodMs = 11000;
constexpr uint8_t kGalaxyMaxBrightness = 191;
constexpr uint8_t kStarMinBrightness = 217;
constexpr uint8_t kStarMaxBrightness = 255;

struct RgbColor {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

struct PixelState {
  bool isStar;
  uint8_t fromColorIndex;
  uint8_t toColorIndex;
  uint32_t colorBlendStartMs;
  uint16_t colorBlendDurationMs;
  uint16_t pulsePeriodMs;
  uint16_t pulseOffsetMs;
  uint8_t minLevel;
  uint8_t maxLevel;
};

constexpr RgbColor kGalaxyPalette[] = {
    {24, 90, 255},
    {20, 255, 120},
    {170, 48, 255},
};

constexpr RgbColor kStarColors[] = {
  {255, 170, 24},
  {255, 236, 120},
};

// These LEDs are rendered as warm stars; every other pixel is part of the galaxy field.
constexpr uint16_t kStarPixelIndices[] = {10, 14, 18, 22, 26, 30, 34, 38, 42};

Adafruit_NeoPixel pixels(kPixelCount, kNeoPixelPin, kPixelType);
PixelState pixelStates[kPixelCount];
uint32_t lastFrameMs = 0;

uint8_t lerp8(uint8_t start, uint8_t end, uint8_t mix) {
  return static_cast<uint8_t>(start + ((static_cast<int16_t>(end) - start) * mix) / 255);
}

RgbColor blendColor(const RgbColor &from, const RgbColor &to, uint8_t mix) {
  return {lerp8(from.red, to.red, mix),
          lerp8(from.green, to.green, mix),
          lerp8(from.blue, to.blue, mix)};
}

uint8_t scale8(uint8_t value, uint8_t scale) {
  return static_cast<uint8_t>((static_cast<uint16_t>(value) * scale) / 255U);
}

uint8_t triangleWave8(uint32_t now, uint16_t periodMs, uint16_t offsetMs) {
  const uint32_t phase = ((now + offsetMs) % periodMs) * 512UL / periodMs;
  if (phase < 256UL) {
    return static_cast<uint8_t>(phase);
  }
  return static_cast<uint8_t>(511UL - phase);
}

uint8_t smoothstep8(uint8_t input) {
  const uint32_t x = input;
  const uint32_t x2 = x * x;
  const uint32_t x3 = x2 * x;
  return static_cast<uint8_t>((3UL * x2) / 255UL - (2UL * x3) / 65025UL);
}

bool isStarPixel(uint16_t pixelIndex) {
  for (uint16_t starIndex : kStarPixelIndices) {
    if (starIndex == pixelIndex) {
      return true;
    }
  }
  return false;
}

uint8_t chooseDifferentColor(uint8_t currentIndex, uint8_t paletteSize) {
  uint8_t nextIndex = currentIndex;
  while (nextIndex == currentIndex) {
    nextIndex = static_cast<uint8_t>(random(0, paletteSize));
  }
  return nextIndex;
}

void advanceColorTarget(PixelState &state, uint32_t now) {
  state.fromColorIndex = state.toColorIndex;
  if (state.isStar) {
    // Stars alternate directly between warm endpoints instead of choosing random hues.
    state.toColorIndex = state.fromColorIndex == 0 ? 1 : 0;
    state.colorBlendDurationMs = state.pulsePeriodMs;
  } else {
    state.toColorIndex = chooseDifferentColor(
        state.fromColorIndex,
        static_cast<uint8_t>(sizeof(kGalaxyPalette) / sizeof(kGalaxyPalette[0])));
    state.colorBlendDurationMs = static_cast<uint16_t>(random(kGalaxyMinColorBlendMs, kGalaxyMaxColorBlendMs + 1));
  }
  state.colorBlendStartMs = now;
}

void initializePixelState(PixelState &state, uint16_t pixelIndex, uint32_t now) {
  state.isStar = isStarPixel(pixelIndex);

  if (state.isStar) {
    // Keep stars bright and slow so they read as foreground points of light.
    state.fromColorIndex = static_cast<uint8_t>(random(0, 2));
    state.toColorIndex = state.fromColorIndex == 0 ? 1 : 0;
    state.pulsePeriodMs = static_cast<uint16_t>(random(kStarMinPulsePeriodMs, kStarMaxPulsePeriodMs + 1));
    state.colorBlendDurationMs = state.pulsePeriodMs;
    state.minLevel = kStarMinBrightness;
    state.maxLevel = kStarMaxBrightness;
  } else {
    // Galaxy pixels stay dimmer and wander across the cool-color palette.
    state.fromColorIndex = static_cast<uint8_t>(
        random(0, static_cast<long>(sizeof(kGalaxyPalette) / sizeof(kGalaxyPalette[0]))));
    state.toColorIndex = chooseDifferentColor(
        state.fromColorIndex,
        static_cast<uint8_t>(sizeof(kGalaxyPalette) / sizeof(kGalaxyPalette[0])));
    state.pulsePeriodMs = static_cast<uint16_t>(random(kGalaxyMinPulsePeriodMs, kGalaxyMaxPulsePeriodMs + 1));
    state.colorBlendDurationMs = static_cast<uint16_t>(random(kGalaxyMinColorBlendMs, kGalaxyMaxColorBlendMs + 1));
    state.minLevel = static_cast<uint8_t>(random(18, 56));
    state.maxLevel = static_cast<uint8_t>(random(120, kGalaxyMaxBrightness + 1));
  }

  state.colorBlendStartMs = now - random(0, state.colorBlendDurationMs);
  state.pulseOffsetMs = static_cast<uint16_t>(random(0, state.pulsePeriodMs));
}

RgbColor currentColor(PixelState &state, uint32_t now) {
  uint32_t elapsedMs = now - state.colorBlendStartMs;
  while (elapsedMs >= state.colorBlendDurationMs) {
    advanceColorTarget(state, state.colorBlendStartMs + state.colorBlendDurationMs);
    elapsedMs = now - state.colorBlendStartMs;
  }

  const uint8_t mix = smoothstep8(static_cast<uint8_t>((elapsedMs * 255UL) / state.colorBlendDurationMs));
  const RgbColor *palette = state.isStar ? kStarColors : kGalaxyPalette;
  return blendColor(palette[state.fromColorIndex], palette[state.toColorIndex], mix);
}

uint8_t currentLevel(const PixelState &state, uint32_t now) {
  const uint8_t wave = smoothstep8(triangleWave8(now, state.pulsePeriodMs, state.pulseOffsetMs));
  return lerp8(state.minLevel, state.maxLevel, wave);
}

void renderFrame(uint32_t now) {
  for (uint16_t index = 0; index < kPixelCount; ++index) {
    PixelState &state = pixelStates[index];
    const RgbColor color = currentColor(state, now);
    const uint8_t level = currentLevel(state, now);

    pixels.setPixelColor(index,
                         pixels.Color(scale8(color.red, level),
                                      scale8(color.green, level),
                                      scale8(color.blue, level)));
  }

  pixels.show();
}

}  // namespace

void setup() {
  randomSeed(analogRead(A0) + micros());

  pixels.begin();
  pixels.clear();
  pixels.show();

  const uint32_t now = millis();
  for (uint16_t index = 0; index < kPixelCount; ++index) {
    initializePixelState(pixelStates[index], index, now);
  }

  renderFrame(now);
}

void loop() {
  const uint32_t now = millis();
  if ((now - lastFrameMs) < kFrameIntervalMs) {
    return;
  }

  lastFrameMs = now;
  renderFrame(now);
}
