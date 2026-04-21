// =============================================================================
// OrcinyEffects.h
// Version : V 0.2.5
// Pre-built effect scenes and helpers for Orciny Device
// =============================================================================
//
// Provides ready-to-use effect scenes for the Orciny Device.
// Use these helpers to quickly add spark, beam, and NeoPixel effects.
//
// INCLUDED SCENES:
//   Scene::Ember()      — Warm spark flash, ember beam, ember NeoPixels
//   Scene::CyanPulse()  — Pulsing cyan beam, matching NeoPixel swell
//   Scene::FullShow()   — Sparks, violet beam, claw sweep, cyan NeoPixels
//
// USAGE:
//   #include <OrcinyEffects.h>
//   using namespace OrcinyEffects;
//   Scene::Ember(millis());
// =============================================================================

#pragma once

#include <Arduino.h>
#include <ColorPalettes.h>
#include <Adafruit_NeoPixel.h>

namespace OrcinyEffects {

// Forward declarations for external hardware objects.
// These must be initialized in your sketch before calling scene functions.
extern uint8_t  SPARK_PIN_1;
extern uint8_t  SPARK_PIN_2;
extern uint8_t  SPARK_PIN_3;
extern uint8_t  SPARK_PIN_4;
extern uint8_t  BEAM_RED_PIN;
extern uint8_t  BEAM_GREEN_PIN;
extern uint8_t  BEAM_BLUE_PIN;
extern Adafruit_NeoPixel *strip;
extern void (*setServo)(uint8_t channel, uint8_t angle);
extern void (*neoPixelSetAll)(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
extern void (*allOutputsOff)();

// =============================================================================
// SCENE DEFINITIONS
// Each scene is a self-contained effect that updates based on elapsed time.
// =============================================================================

class Scene {
 public:
  // --- EMBER SCENE ----------------------------------------------------------
  // Warm spark flash on channel 1, ember beam, ember NeoPixels
  static void Ember(uint32_t now) {
    static uint32_t nextFlashMs = 0;

    // Random spark bursts on channel 1
    if (now >= nextFlashMs) {
      analogWrite(SPARK_PIN_1, random(100, 220));
      delay(15);
      analogWrite(SPARK_PIN_1, 0);
      nextFlashMs = now + random(100, 300);
    }

    // Set beam to warm ember palette
    analogWrite(BEAM_RED_PIN,   ColorPalettes::kBeamEmber.red);
    analogWrite(BEAM_GREEN_PIN, ColorPalettes::kBeamEmber.green);
    analogWrite(BEAM_BLUE_PIN,  ColorPalettes::kBeamEmber.blue);

    // Set all NeoPixels to ember palette
    neoPixelSetAll(ColorPalettes::kNeoEmber.red,
                   ColorPalettes::kNeoEmber.green,
                   ColorPalettes::kNeoEmber.blue,
                   ColorPalettes::kNeoEmber.white);
  }

  // --- CYAN PULSE SCENE -----------------------------------------------------
  // Pulsing cyan beam with matching NeoPixel swell
  static void CyanPulse(uint32_t now) {
    uint16_t phase = now % 2400;              // 2.4-second repeating cycle
    uint8_t  swell = phase < 1200
                     ? map(phase, 0,    1200, 40, 255)
                     : map(phase, 1200, 2400, 255, 40);

    // Apply swell to cyan palette
    analogWrite(BEAM_RED_PIN,   map(swell, 0, 255, 0, ColorPalettes::kBeamCyan.red));
    analogWrite(BEAM_GREEN_PIN, map(swell, 0, 255, 0, ColorPalettes::kBeamCyan.green));
    analogWrite(BEAM_BLUE_PIN,  map(swell, 0, 255, 0, ColorPalettes::kBeamCyan.blue));

    // Match NeoPixels to beam swell
    uint8_t neo_r = map(swell, 0, 255, 0, ColorPalettes::kNeoCyan.red);
    uint8_t neo_g = map(swell, 0, 255, 0, ColorPalettes::kNeoCyan.green);
    uint8_t neo_b = map(swell, 0, 255, 0, ColorPalettes::kNeoCyan.blue);
    uint8_t neo_w = map(swell, 0, 255, 0, ColorPalettes::kNeoCyan.white);
    neoPixelSetAll(neo_r, neo_g, neo_b, neo_w);
  }

  // --- FULL SHOW SCENE ------------------------------------------------------
  // Sparks + violet beam + claw sweep + cyan NeoPixels
  static void FullShow(uint32_t now) {
    static uint32_t nextSpark = 0;
    static int8_t   servo_dir = 1;
    static uint8_t  servo_angle = 22;
    static uint32_t nextServo = 0;

    // Random spark bursts across all channels
    if (now >= nextSpark) {
      for (int i = 0; i < 3; i++) {
        analogWrite(SPARK_PIN_1, random(200, 255));
        analogWrite(SPARK_PIN_2, random(200, 255));
        delay(8);
      }
      allOutputsOff();  // Clear sparks
      nextSpark = now + random(200, 400);
    }

    // Set beam to violet palette
    analogWrite(BEAM_RED_PIN,   ColorPalettes::kBeamViolet.red);
    analogWrite(BEAM_GREEN_PIN, ColorPalettes::kBeamViolet.green);
    analogWrite(BEAM_BLUE_PIN,  ColorPalettes::kBeamViolet.blue);

    // Non-blocking claw servo sweep
    if (now >= nextServo) {
      servo_angle += servo_dir;
      if (servo_angle >= 120) servo_dir = -1;
      if (servo_angle <= 22)  servo_dir =  1;
      setServo(0, servo_angle);
      setServo(1, map(servo_angle, 22, 120, 120, 22));
      nextServo = now + 10;
    }

    // Set all NeoPixels to cyan palette
    neoPixelSetAll(ColorPalettes::kNeoCyan.red,
                   ColorPalettes::kNeoCyan.green,
                   ColorPalettes::kNeoCyan.blue,
                   ColorPalettes::kNeoCyan.white);
  }
};

}  // namespace OrcinyEffects
