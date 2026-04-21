# OrcinyEffects Library

Pre-built effect scenes and helpers for the Orciny Device.

## Overview

This library provides ready-to-use effect scenes (`Ember`, `CyanPulse`, `FullShow`) so you don't have to write your own effect code. Each scene is fully implemented and uses `ColorPalettes` for consistent, polished visuals.

## When to Use This Library

✅ **Use OrcinyEffects if you:**
- Want to get up and running quickly without coding effects
- Want reliable, tested effect implementations
- Are comfortable with the `#include` and namespace syntax
- Want to customize timing or palettes without touching core effect logic

❌ **Use rp2040_fx_starter.ino directly if you:**
- Are new to Arduino and want to learn by editing code
- Need to understand how effects work line-by-line
- Plan to write completely custom effects

## Installation

The library is already in your sketchbook at:
```
arduino/libraries/OrcinyEffects/
```

Arduino IDE will discover it automatically only if your sketchbook path is set to this repo's `arduino` folder.

If the library or headers are not found, check `arduino/TROUBLESHOOTING.md` and restart Arduino IDE after correcting the sketchbook path.

## Quick Start

### 1. Include the library and initialize hardware references

```cpp
#include <OrcinyEffects.h>
#include <ColorPalettes.h>

// At the top of your sketch, set up hardware references
// (These are pointers that OrcinyEffects uses to control your device)
using namespace OrcinyEffects;

void setup() {
  // ... your normal setup code ...
  
  // Tell OrcinyEffects which pins and objects to use
  OrcinyEffects::SPARK_PIN_1 = 18;
  OrcinyEffects::SPARK_PIN_2 = 19;
  OrcinyEffects::SPARK_PIN_3 = 20;
  OrcinyEffects::SPARK_PIN_4 = 24;
  OrcinyEffects::BEAM_RED_PIN = 11;
  OrcinyEffects::BEAM_GREEN_PIN = 12;
  OrcinyEffects::BEAM_BLUE_PIN = 13;
  OrcinyEffects::strip = &strip;  // Your Adafruit_NeoPixel instance
  OrcinyEffects::setServo = setServo;  // Your servo function
  OrcinyEffects::neoPixelSetAll = neoPixelSetAll;  // Your helper
  OrcinyEffects::allOutputsOff = allOutputsOff;  // Your helper
}
```

### 2. Call scenes in your state functions

```cpp
void doState1() {
  Scene::Ember(millis());
}

void doState2() {
  Scene::CyanPulse(millis());
}

void doState3() {
  Scene::FullShow(millis());
}
```

That's it! No effect code to write—the scenes handle everything.

## Available Scenes

### `Scene::Ember(uint32_t now)`
Warm ember atmosphere: random spark flashes on channel 1, warm orange beam, and glowing ember NeoPixels.

**Colors used:**
- Beam: `ColorPalettes::kBeamEmber`
- NeoPixels: `ColorPalettes::kNeoEmber`

### `Scene::CyanPulse(uint32_t now)`
Cool, pulsing cyan scene with a breathing swell effect. Both beam and NeoPixels pulse in sync.

**Colors used:**
- Beam: `ColorPalettes::kBeamCyan`
- NeoPixels: `ColorPalettes::kNeoCyan`

### `Scene::FullShow(uint32_t now)`
Full spectacle scene: sparks burst across channels, violet beam holds steady, claw servos sweep, and cyan NeoPixels glow.

**Colors used:**
- Beam: `ColorPalettes::kBeamViolet`
- NeoPixels: `ColorPalettes::kNeoCyan`

## Customization

Each scene function is simple and can be customized by editing the library header or copying it into your sketch.

### Example: Change Ember to use a different palette

Edit `arduino/libraries/OrcinyEffects/src/OrcinyEffects.h` in the `Scene::Ember()` function:

```cpp
// Original:
analogWrite(BEAM_RED_PIN,   ColorPalettes::kBeamEmber.red);

// Change to use Cool White palette:
analogWrite(BEAM_RED_PIN,   ColorPalettes::kBeamCoolWhite.red);
```

### Example: Faster or slower pulse

Edit the cycle time in `Scene::CyanPulse()`:

```cpp
// Original (2.4 second cycle):
uint16_t phase = now % 2400;

// Make it faster (1.2 second cycle):
uint16_t phase = now % 1200;
```

## See Also

- [ColorPalettes.h](../OrcinyCommon/src/ColorPalettes.h) — Available color palettes
- [rp2040_fx_starter.ino](../../rp2040_fx_starter/rp2040_fx_starter.ino) — Starter template for learning
- [rp2040_fx_controller_demo.ino](../../rp2040_fx_controller_demo/rp2040_fx_controller_demo.ino) — Full-featured demo
- [TROUBLESHOOTING.md](../../TROUBLESHOOTING.md) — Arduino setup and common fixes
