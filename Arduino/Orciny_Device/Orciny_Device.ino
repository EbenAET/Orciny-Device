// =============================================================================
// Orciny_Device.ino
// Version : V 0.2.7
// Orciny Device — Plug-and-Play FX Starter Template
// Board : Adafruit Feather RP2040
// Wings : Prop-Maker FeatherWing + 8-Channel Servo FeatherWing (PCA9685)
// -----------------------------------------------------------------------------
// HOW TO USE THIS SKETCH
//
//   1. Read through Step 1–4 below and adjust any values you need to change.
//   2. Find the section labelled "STATE BEHAVIORS" (~halfway down).
//   3. Add your effect code inside the doStateX() function for each state.
//   4. Upload and test.  The three physical switches cycle through states.
//
// PHYSICAL CONTROLS  (ready to go — no changes needed)
//   SW1 (GP27) — tap: toggle all outputs on / off
//   SW2 (GP28) — tap: step backward through states
//   SW3 (GP29) — tap: step forward  through states
//   SW1 + SW3 held 5 s — reset to State 1, outputs off
//
// USB SERIAL (115200 baud, Arduino IDE Serial Monitor)
//   Prints state changes.  Add Serial.println() calls anywhere for debugging.
//
// OUTPUTS AVAILABLE
//   Spark channels  — GP18 / GP19 / GP20 / GP24 analogWrite(pin, 0–255)
//   Beam LED Red    — GP11                       analogWrite(GP11, 0–255)
//   Beam LED Green  — GP12                       analogWrite(GP12, 0–255)
//   Beam LED Blue   — GP13                       analogWrite(GP13, 0–255)
//   Servo A (claw)  — PCA9685 channel 0          setServo(0, angle 22–120)
//   Servo B (claw)  — PCA9685 channel 1          setServo(1, angle 22–120)
//   NeoPixel strip  — GP25 (166 pixels)           use neoPixelSetAll() helper
// =============================================================================

// Core libraries
#include <Wire.h>                      // I2C bus (required for servo driver)
#include <Adafruit_PWMServoDriver.h>   // PCA9685 servo wing library
#include <Adafruit_NeoPixel.h>         // NeoPixel strip library
#include "DeviceConfig.h"              // Centralized pin and parameter definitions

// =============================================================================
// STEP 1 — PIN CONFIGURATION & TUNING PARAMETERS
// All pin and parameter definitions are now in DeviceConfig.h
// Only change these if you are building on different hardware.
// =============================================================================
// STEP 2B - COLOR PALETTE DESIGNATORS
// Use these enums to select named color palettes for beam and NeoPixels.
// To add more, be sure to create a name  in the ID field and add the corresponding RGB(W) values in the BEAM_PALETTES and NEO_PALETTES arrays below.
// =============================================================================

enum BeamPaletteId : uint8_t {
  BEAM_PALETTE_OFF = 0,
  BEAM_PALETTE_COOL,
  BEAM_PALETTE_EMBER,
  BEAM_PALETTE_CYAN,
  BEAM_PALETTE_VIOLET,
  BEAM_PALETTE_MAGENTA,
  BEAM_PALETTE_GOLDEN,
  BEAM_PALETTE_TEAL,
  BEAM_PALETTE_WHITE,
};

enum NeoPaletteId : uint8_t {
  NEO_PALETTE_OFF = 0,
  NEO_PALETTE_WARM_WHITE,
  NEO_PALETTE_EMBER,
  NEO_PALETTE_CYAN,
  NEO_PALETTE_VIOLET,
  NEO_PALETTE_MAGENTA,
  NEO_PALETTE_GOLDEN,
  NEO_PALETTE_TEAL,
  NEO_PALETTE_WHITE,
};

struct BeamPaletteColor {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

struct NeoPaletteColor {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t white;
};

const BeamPaletteColor BEAM_PALETTES[] = {
  {0,   0,   0},    // OFF
  {120, 210, 255},  // COOL
  {255, 95,  18},   // EMBER
  {0,   220, 255},  // CYAN
  {175, 70,  255},  // VIOLET
  {255, 0,   200},  // MAGENTA
  {255, 180, 30},   // GOLDEN
  {40,  220, 200},  // TEAL
  {255, 255, 255},  // WHITE
};

const NeoPaletteColor NEO_PALETTES[] = {
  {0,   0,   0,   0},   // OFF
  {140, 130, 110, 40},  // WARM_WHITE
  {220, 70,  8,   0},   // EMBER
  {0,   180, 230, 0},   // CYAN
  {140, 45,  210, 0},   // VIOLET
  {255, 0,   180, 0},   // MAGENTA
  {255, 180, 0,   80},  // GOLDEN
  {0,   200, 180, 0},   // TEAL
  {200, 200, 200, 100}, // WHITE
};

// =============================================================================
// STEP 2C - STATE PALETTE SELECTORS
// One-line per-state color selectors.  Change these values to retheme states
// without editing state logic code.
// =============================================================================

// Per-state palette selectors — edit these to retheme without touching state logic.
// Sequences now match the CSV sequence map.
const BeamPaletteId STATE_INACTIVE_BEAM_PALETTE = BEAM_PALETTE_OFF;
const BeamPaletteId STATE_BOOT_BEAM_PALETTE     = BEAM_PALETTE_GOLDEN;
const BeamPaletteId STATE_DEMO_BEAM_PALETTE     = BEAM_PALETTE_CYAN;
const BeamPaletteId STATE_FAILURE_BEAM_PALETTE  = BEAM_PALETTE_WHITE;

const NeoPaletteId STATE_INACTIVE_NEO_PALETTE   = NEO_PALETTE_OFF;
const NeoPaletteId STATE_BOOT_NEO_PALETTE       = NEO_PALETTE_CYAN;
const NeoPaletteId STATE_DEMO_NEO_PALETTE       = NEO_PALETTE_CYAN;
const NeoPaletteId STATE_FAILURE_NEO_PALETTE    = NEO_PALETTE_WHITE;

// =============================================================================
// STEP 3 — STATE DEFINITIONS
// Add or remove states here.  Update STATE_COUNT to match.
// The state names show up in Serial Monitor output.
// =============================================================================

enum DeviceState : uint8_t {
  STATE_OFF      = 0,  // Outputs disabled — safe idle
  STATE_INACTIVE,      // Sequence 0: Inactive
  STATE_BOOT,          // Sequence 1: Boot up
  STATE_DEMO,          // Sequence 2: Demonstrate
  STATE_FAILURE,       // Sequence 3: Device Failure
  STATE_COUNT          // Keep this last — used to wrap the selector
};

// Readable names printed to Serial Monitor when states change.
const char* STATE_NAMES[] = { "OFF", "Inactive", "Boot Up", "Demonstrate", "Device Failure" };

// =============================================================================
// STEP 4 — GLOBAL OBJECTS (nothing to change here in most cases)
// =============================================================================

Adafruit_PWMServoDriver servoDriver(SERVO_I2C_ADDR);
Adafruit_NeoPixel       strip(NEO_PIXEL_COUNT, NEO_DATA_PIN, NEO_COLOR_ORDER + NEO_KHZ800);

DeviceState currentState   = STATE_INACTIVE;   // Which state is active
bool        outputEnabled  = false;     // Master on/off flag

// Internal switch state tracking (one struct per button).
struct SwitchState {
  uint8_t  pin;
  bool     stable;       // Last confirmed stable reading
  bool     raw;          // Most recent raw reading
  bool     pressed;      // Single-shot: true for one loop on press
  bool     released;     // Single-shot: true for one loop on release
  uint32_t lastChangeMs;
};

SwitchState swPower = { SW_POWER_PIN };
SwitchState swPrev  = { SW_PREV_PIN  };
SwitchState swNext  = { SW_NEXT_PIN  };

// Reset chord tracking
uint32_t resetStartMs       = 0;
bool     resetFired         = false;
bool     suppressPowerEvent = false;
bool     suppressNextEvent  = false;

// =============================================================================
// HELPER FUNCTIONS — forward declarations
// These are implemented at the bottom of the file.
// =============================================================================

void updateSwitch(SwitchState &sw, uint32_t now);
void handleSwitches(uint32_t now);
void printState();
void setServo(uint8_t channel, uint8_t angle);
void setBeamPalette(BeamPaletteId paletteId, uint8_t level = 255);
void setNeoPalette(NeoPaletteId paletteId, uint8_t level = 255);
void neoPixelSetAll(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);
void neoPixelOff();
void allOutputsOff();
void neoChase(uint8_t r, uint8_t g, uint8_t b, uint8_t len, uint32_t intervalMs);
void servoIdle(uint8_t channel);

// =============================================================================
// STATE BEHAVIORS
// Each doStateX() function is called every loop iteration while that state
// is active and outputEnabled is true.
//
// IMPORTANT: Do NOT use delay() inside these functions.  Use millis() for
//            timing.  See the non-blocking timer example in State 1 below.
// =============================================================================

// --- STATE: INACTIVE (Sequence 0) -------------------------------------------
// Sparse random single sparks with 3-5 second gaps. Beam and NeoPixels off.

void doStateInactive() {
  static uint32_t nextSparkMs = 0;
  uint32_t now = millis();

  if (now >= nextSparkMs) {
    Serial.println(F("Inactive: Spark event"));
    // Pick a random spark channel
    const uint8_t pins[] = {SPARK_PIN_1, SPARK_PIN_2, SPARK_PIN_3, SPARK_PIN_4};
    uint8_t ch = pins[random(0, 4)];
    analogWrite(ch, random(140, 220));
    delay(20);
    analogWrite(ch, 0);
    nextSparkMs = now + random(3000, 5000);
  }

  // Beam and NeoPixels stay off in idle
  analogWrite(BEAM_RED_PIN, 0);
  analogWrite(BEAM_GREEN_PIN, 0);
  analogWrite(BEAM_BLUE_PIN, 0);
  neoPixelOff();
}

// --- STATE: BOOT UP (Sequence 1) ---------------------------------------------
// Timeline (approximate, perfect timing not required):
//   0–4s    : Sparse sparks; cyan chases, starting slow and speeding up
//   4–10s   : Sparks continue; cyan chases ramp up in speed and length
//   10–20s  : Orange chases begin interspersing with cyan; pincers go rigid
//   20s+    : Core pulses between cyan and orange; beam fades to golden ~30%;
//             servo 1 begins slow oscillation; then both go limp

void doStateBootUp() {
  static uint32_t stateEnteredMs  = 0;
  static uint32_t nextSparkMs     = 0;
  static uint32_t nextChaseMs     = 0;
  static uint32_t nextServoMs     = 0;
  static bool     pincersRigid    = false;
  static bool     servo1Active    = false;
  static bool     servoLimp       = false;
  static int8_t   servoDir        = 1;
  static uint8_t  servoAngle      = 22;
  static bool     cyanPulsePhase  = true;  // true = cyan, false = orange

  uint32_t now = millis();

  // Reset phase tracking when first entering this state
  if (stateEnteredMs == 0) {
    stateEnteredMs = now;
    pincersRigid   = false;
    servo1Active   = false;
    servoLimp      = false;
  }

  uint32_t elapsed = now - stateEnteredMs;

  // --- Sparks: present 0-4s, 4-10s, 18-24s, 24s+ ---
  if ((elapsed < 4000 || (elapsed >= 4000 && elapsed < 10000) ||
       (elapsed >= 18000 && elapsed < 28000) || elapsed >= 28000)) {
    if (now >= nextSparkMs) {
      Serial.print(F("BootUp: Sparks phase, elapsed="));
      Serial.println(elapsed);
      uint8_t count = (elapsed >= 28000) ? random(1, 5) : random(2, 4);
      for (uint8_t i = 0; i < count; i++) {
        const uint8_t pins[] = {SPARK_PIN_1, SPARK_PIN_2, SPARK_PIN_3, SPARK_PIN_4};
        analogWrite(pins[random(0, 4)], random(160, 240));
      }
      delay(15);
      analogWrite(SPARK_PIN_1, 0);
      analogWrite(SPARK_PIN_2, 0);
      analogWrite(SPARK_PIN_3, 0);
      analogWrite(SPARK_PIN_4, 0);
      nextSparkMs = now + random(800, 1800);
    }
  }

  // --- NeoPixel chases: ramp speed and intersperse orange after ~10s ---
  if (elapsed < 10000) {
    Serial.println(F("BootUp: Cyan chase phase"));
  } else if (elapsed < 20000) {
    Serial.println(F("BootUp: Cyan/Orange chase phase"));
  } else {
    Serial.println(F("BootUp: Core pulse phase"));
  }
  // Chase length grows from 5 to 12 pixels over time; interval shrinks.
  {
    uint16_t chaseInterval = (elapsed < 5000)  ? 600 :
                             (elapsed < 10000) ? 400 :
                             (elapsed < 15000) ? 250 : 160;
    uint8_t  chaseLen      = (elapsed < 5000)  ? 5 :
                             (elapsed < 10000) ? 7 : 10;

    if (now >= nextChaseMs) {
      // After 10s, randomly pick cyan or orange
      bool useOrange = (elapsed >= 10000) && (random(0, 2) == 0);
      if (useOrange) {
        neoChase(220, 70, 8, chaseLen, 18);
      } else {
        neoChase(0, 180, 230, chaseLen, 18);
      }
      nextChaseMs = now + chaseInterval;
    }
  }

  // --- Pincers rigid ~8s ---
  if (elapsed >= 8000 && !pincersRigid) {
    Serial.println(F("BootUp: Pincers rigid"));
    setServo(0, 71);  // Midpoint = rigid hold
    setServo(1, 71);
    pincersRigid = true;
  }

  // --- Beam: fade up to golden at ~30% after ~20s ---
  if (elapsed >= 20000 && elapsed < 24000) {
    Serial.println(F("BootUp: Beam fading up to golden"));
  } else if (elapsed >= 24000) {
    Serial.println(F("BootUp: Beam golden hold"));
  }
  if (elapsed >= 20000) {
    uint32_t fadeElapsed = elapsed - 20000;
    uint8_t  level = (fadeElapsed >= 4000) ? 76 : map(fadeElapsed, 0, 4000, 0, 76);
    setBeamPalette(STATE_BOOT_BEAM_PALETTE, level);
  } else {
    analogWrite(BEAM_RED_PIN, 0);
    analogWrite(BEAM_GREEN_PIN, 0);
    analogWrite(BEAM_BLUE_PIN, 0);
  }

  // --- Servo 1 slow oscillation ~22s; both go limp ~28s ---
  if (elapsed >= 28000 && !servoLimp) {
    Serial.println(F("BootUp: Servos limp"));
    servoIdle(0);
    servoIdle(1);
    servoLimp    = true;
    servo1Active = false;
  } else if (elapsed >= 22000 && !servoLimp) {
    Serial.println(F("BootUp: Servo 1 oscillation"));
    servo1Active = true;
  }

  if (servo1Active && !servoLimp && now >= nextServoMs) {
    servoAngle += servoDir;
    if (servoAngle >= 120) servoDir = -1;
    if (servoAngle <= 22)  servoDir =  1;
    setServo(1, servoAngle);  // Only servo 1 oscillates
    nextServoMs = now + 40;   // Slow sweep
  }

  // After 28s: loop in a steady cyan-orange pulsing core + golden beam hold
  if (elapsed >= 28000) {
    uint32_t pulsePhase = (now % 800);
    uint8_t  blend      = (pulsePhase < 400)
                          ? map(pulsePhase, 0, 400, 0, 255)
                          : map(pulsePhase, 400, 800, 255, 0);
    uint8_t r = map(blend, 0, 255, 0,   220);
    uint8_t g = map(blend, 0, 255, 180, 70);
    uint8_t b = map(blend, 0, 255, 230, 8);
    neoPixelSetAll(r, g, b, 0);
  }
}

// --- STATE: DEMONSTRATE (Sequence 2) -----------------------------------------
// Timeline (approximate):
//   0s     : Core flashing cyan/orange; beam brightens to orange 90%
//   0.5s   : Beam fades to teal; both servos rigid
//   1.5s   : Servos random movement, slow then frantic
//   2s     : Sparks start; magenta chases start on core
//   3–7s   : Beam pulses teal/orange with accelerating frequency
//   7s     : Core all-off; beam turns magenta at 30%
//   7.25s  : Beam magenta fades to full; pincers freeze
//   9.5s   : Pincers freeze up
//   Loop

void doStateDemo() {
  static uint32_t stateEnteredMs = 0;
  static uint32_t nextSparkMs    = 0;
  static uint32_t nextChaseMs    = 0;
  static uint32_t nextServoMs    = 0;
  static bool     pincersRigid   = false;
  static int8_t   servoDir0      = 1;
  static int8_t   servoDir1      = -1;
  static uint8_t  servoAngle0    = 22;
  static uint8_t  servoAngle1    = 120;

  uint32_t now = millis();

  if (stateEnteredMs == 0) {
    stateEnteredMs = now;
    pincersRigid   = false;
  }

  // Loop the sequence every 10.75s
  uint32_t elapsed = (now - stateEnteredMs) % 10750;

  // --- Beam ---
  if (elapsed < 500) {
    Serial.println(F("Demo: Beam brighten to orange"));
    // Brighten to orange 90%
    uint8_t level = map(elapsed, 0, 500, 0, 230);
    setBeamPalette(BEAM_PALETTE_EMBER, level);
  } else if (elapsed < 3000) {
    Serial.println(F("Demo: Beam fade to teal"));
    // Fade color toward teal
    setBeamPalette(BEAM_PALETTE_TEAL, 200);
  } else if (elapsed < 7000) {
    Serial.println(F("Demo: Beam teal/orange pulse"));
    // Accelerating pulse between teal and orange
    uint32_t pulseElapsed = elapsed - 3000;
    uint16_t cycleMs = map(constrain(pulseElapsed, 0, 4000), 0, 4000, 1200, 200);
    uint32_t phase   = pulseElapsed % cycleMs;
    uint8_t  swell   = (phase < cycleMs / 2)
                       ? map(phase, 0, cycleMs / 2, 40, 255)
                       : map(phase, cycleMs / 2, cycleMs, 255, 40);
    // Blend teal→orange based on swell phase
    bool useTeal = ((pulseElapsed / cycleMs) % 2 == 0);
    setBeamPalette(useTeal ? BEAM_PALETTE_TEAL : BEAM_PALETTE_EMBER, swell);
  } else if (elapsed < 7250) {
    Serial.println(F("Demo: Beam magenta 30%"));
    setBeamPalette(BEAM_PALETTE_MAGENTA, 76);  // 30%
  } else {
    Serial.println(F("Demo: Beam magenta fade up"));
    // Magenta fades up to full
    uint8_t level = map(constrain((int32_t)elapsed - 7250, 0, 2750), 0, 2750, 76, 255);
    setBeamPalette(BEAM_PALETTE_MAGENTA, level);
  }

  // --- NeoPixel core ---
  if (elapsed < 2000) {
    Serial.println(F("Demo: Core cyan/orange flash"));
    // Flash between cyan and orange quickly (~200ms each)
    bool showOrange = ((now / 200) % 2 == 0);
    if (showOrange) {
      neoPixelSetAll(220, 70, 8, 0);
    } else {
      neoPixelSetAll(0, 180, 230, 0);
    }
  } else if (elapsed < 7000) {
    Serial.println(F("Demo: Core flash + magenta chase"));
    // Cyan/orange flashing + magenta chases
    bool showOrange = ((now / 200) % 2 == 0);
    if (showOrange) {
      neoPixelSetAll(220, 70, 8, 0);
    } else {
      neoPixelSetAll(0, 180, 230, 0);
    }
    if (now >= nextChaseMs) {
      neoChase(255, 0, 180, 4, 14);
      nextChaseMs = now + random(400, 700);
    }
  } else if (elapsed < 7250) {
    Serial.println(F("Demo: Core all off"));
    // All lights off
    neoPixelOff();
  } else {
    Serial.println(F("Demo: Core magenta fade up"));
    // Magenta fade-up
    uint8_t level = map(constrain((int32_t)elapsed - 7250, 0, 2750), 0, 2750, 0, 200);
    neoPixelSetAll((255 * level) / 255, 0, (180 * level) / 255, 0);
  }

  // --- Sparks: occasional flicker 2s onward ---
  if (elapsed >= 500 && now >= nextSparkMs) {
    Serial.println(F("Demo: Spark flicker"));
    const uint8_t pins[] = {SPARK_PIN_1, SPARK_PIN_2, SPARK_PIN_3, SPARK_PIN_4};
    analogWrite(pins[random(0, 4)], random(160, 240));
    delay(12);
    analogWrite(SPARK_PIN_1, 0);
    analogWrite(SPARK_PIN_2, 0);
    analogWrite(SPARK_PIN_3, 0);
    analogWrite(SPARK_PIN_4, 0);
    nextSparkMs = now + random(300, 900);
  }

  // --- Servos ---
  if (elapsed < 500) {
    Serial.println(F("Demo: Servos free"));
    // Free (no drive)
  } else if (elapsed < 1500) {
    Serial.println(F("Demo: Servos rigid"));
    // Both rigid
    setServo(0, 71);
    setServo(1, 71);
    pincersRigid = true;
  } else if (elapsed < 9500) {
    Serial.println(F("Demo: Servos random movement"));
    // Random movement — slow early, frantic late
    uint16_t stepMs = (elapsed < 4000) ? 120 : map(elapsed, 4000, 9500, 120, 20);
    if (now >= nextServoMs) {
      setServo(0, random(22, 120));
      setServo(1, random(22, 120));
      nextServoMs = now + stepMs;
    }
  } else {
    Serial.println(F("Demo: Servos freeze"));
    // Freeze
    setServo(0, servoAngle0);
    setServo(1, servoAngle1);
  }
}

// --- STATE: DEVICE FAILURE (Sequence 3) ---------------------------------------
// Timeline (~4s, then ends in held-off state):
//   0s     : Random twitching servos; sparks increasing in frequency
//            Core rapid flashing all colors
//   0s–4s  : Beam flashes teal/magenta/orange alternating
//   1.75s  : Core → bright white
//   2s     : Core fades out; beam all colors full
//   2.5s   : Beam fades all the way out; pincers go limp
//   3.5s   : Pincers go limp
//   4s+    : Everything off, outputEnabled goes false

void doStateFailure() {
  static uint32_t stateEnteredMs = 0;
  static uint32_t nextSparkMs    = 0;
  static uint32_t nextServoMs    = 0;

  uint32_t now = millis();

  if (stateEnteredMs == 0) {
    stateEnteredMs = now;
  }

  uint32_t elapsed = now - stateEnteredMs;

  // After 4s, kill outputs and disable
  if (elapsed >= 4000) {
    Serial.println(F("Failure: Sequence end, outputs off"));
    allOutputsOff();
    outputEnabled  = false;
    stateEnteredMs = 0;  // Reset so it restarts cleanly next time
    printState();
    return;
  }

  // --- Sparks: increasing frequency ---
  uint32_t sparkInterval = map(constrain(elapsed, 0, 3500), 0, 3500, 600, 60);
  if (now >= nextSparkMs) {
    Serial.println(F("Failure: Sparks increasing frequency"));
    uint8_t count = map(constrain(elapsed, 0, 3500), 0, 3500, 1, 4);
    for (uint8_t i = 0; i < count; i++) {
      const uint8_t pins[] = {SPARK_PIN_1, SPARK_PIN_2, SPARK_PIN_3, SPARK_PIN_4};
      analogWrite(pins[random(0, 4)], random(180, 255));
    }
    delay(10);
    analogWrite(SPARK_PIN_1, 0);
    analogWrite(SPARK_PIN_2, 0);
    analogWrite(SPARK_PIN_3, 0);
    analogWrite(SPARK_PIN_4, 0);
    nextSparkMs = now + sparkInterval;
  }

  // --- Beam ---
  if (elapsed < 1750) {
    Serial.println(F("Failure: Beam flash teal/magenta/orange"));
    // Flash teal/magenta/orange at ~4 per second
    uint8_t phase = (now / 250) % 3;
    if      (phase == 0) setBeamPalette(BEAM_PALETTE_TEAL,    255);
    else if (phase == 1) setBeamPalette(BEAM_PALETTE_MAGENTA, 255);
    else                 setBeamPalette(BEAM_PALETTE_EMBER,   255);
  } else if (elapsed < 2000) {
    Serial.println(F("Failure: Beam flash magenta/teal"));
    // Flash magenta then teal
    setBeamPalette(((now / 250) % 2 == 0) ? BEAM_PALETTE_MAGENTA : BEAM_PALETTE_TEAL, 255);
  } else if (elapsed < 2500) {
    Serial.println(F("Failure: Beam all colors full"));
    // All colors full (white)
    setBeamPalette(BEAM_PALETTE_WHITE, 255);
  } else {
    Serial.println(F("Failure: Beam fade out"));
    // Fade out
    uint8_t level = map(constrain(elapsed, 2500, 4000), 2500, 4000, 255, 0);
    setBeamPalette(BEAM_PALETTE_WHITE, level);
  }

  // --- NeoPixel core ---
  if (elapsed < 1750) {
    Serial.println(F("Failure: Core rapid color flash"));
    // Rapid flashing through all colors
    uint8_t colorIdx = (now / 100) % 5;
    switch (colorIdx) {
      case 0: neoPixelSetAll(0,   180, 230, 0);   break;  // cyan
      case 1: neoPixelSetAll(220, 70,  8,   0);   break;  // orange
      case 2: neoPixelSetAll(255, 0,   180, 0);   break;  // magenta
      case 3: neoPixelSetAll(200, 200, 200, 100); break;  // white
      case 4: neoPixelSetAll(40,  220, 200, 0);   break;  // teal
    }
  } else if (elapsed < 2000) {
    Serial.println(F("Failure: Core bright white"));
    // Bright white
    neoPixelSetAll(200, 200, 200, 100);
  } else {
    Serial.println(F("Failure: Core fade out"));
    // Fade out
    uint8_t level = map(constrain(elapsed, 2000, 3000), 2000, 3000, 255, 0);
    neoPixelSetAll((200 * level) / 255, (200 * level) / 255, (200 * level) / 255, (100 * level) / 255);
  }

  // --- Servos: random twitching until ~3.5s then limp ---
  if (elapsed < 3500 && now >= nextServoMs) {
    Serial.println(F("Failure: Servos twitching"));
    uint16_t stepMs = map(constrain(elapsed, 0, 3000), 0, 3000, 200, 40);
    setServo(0, random(22, 120));
    setServo(1, random(22, 120));
    nextServoMs = now + stepMs;
  } else if (elapsed >= 3500) {
    Serial.println(F("Failure: Servos limp"));
    servoIdle(0);
    servoIdle(1);
  }
}

// =============================================================================
// SETUP — runs once at power-on
// =============================================================================

void setup() {
  Serial.begin(115200);

  // --- Prop-Maker power enable -----------------------------------------------
  // This pin MUST be driven HIGH or the MOSFET outputs won't work.
  pinMode(PROP_MAKER_PWR_PIN, OUTPUT);
  digitalWrite(PROP_MAKER_PWR_PIN, HIGH);

  // --- Beam LED outputs ------------------------------------------------------
  pinMode(BEAM_RED_PIN,   OUTPUT);
  pinMode(BEAM_GREEN_PIN, OUTPUT);
  pinMode(BEAM_BLUE_PIN,  OUTPUT);
  analogWrite(BEAM_RED_PIN,   0);
  analogWrite(BEAM_GREEN_PIN, 0);
  analogWrite(BEAM_BLUE_PIN,  0);

  // --- Spark channel outputs -------------------------------------------------
  pinMode(SPARK_PIN_1, OUTPUT);  analogWrite(SPARK_PIN_1, 0);
  pinMode(SPARK_PIN_2, OUTPUT);  analogWrite(SPARK_PIN_2, 0);
  pinMode(SPARK_PIN_3, OUTPUT);  analogWrite(SPARK_PIN_3, 0);
  pinMode(SPARK_PIN_4, OUTPUT);  analogWrite(SPARK_PIN_4, 0);

  // --- NeoPixel strip --------------------------------------------------------
  strip.begin();
  strip.setBrightness(80);  // Global brightness 0–255 (80 ≈ 31% — adjust here)
  strip.show();              // Clears strip (all off)

  // --- Servo driver (PCA9685 over I2C) --------------------------------------
  Wire.begin();
  servoDriver.begin();
  servoDriver.setPWMFreq(50);  // 50 Hz standard for hobby servos

  // --- Switches (INPUT_PULLUP: floating = HIGH, pressed = LOW) ---------------
  SwitchState *switches[] = {&swPower, &swPrev, &swNext};
  for (uint8_t i = 0; i < 3; i++) {
    SwitchState *sw = switches[i];
    pinMode(sw->pin, INPUT_PULLUP);
    sw->stable = sw->raw = (digitalRead(sw->pin) == LOW);
    sw->lastChangeMs = millis();
  }

  // --- Random seed from analog noise ----------------------------------------
  randomSeed(analogRead(A0));

  Serial.println(F("Orciny FX Starter ready.  SW1=on/off  SW2=prev  SW3=next"));
  printState();
}

// =============================================================================
// LOOP — runs continuously after setup()
// Do not add delay() here — use millis()-based timers instead.
// =============================================================================

void loop() {
  const uint32_t now = millis();

  // Read and debounce all three switches; update outputEnabled and currentState.
  handleSwitches(now);

  if (!outputEnabled) {
    // Master off — ensure everything is dark.
    allOutputsOff();
    return;
  }

  // Dispatch to the active state's behavior function.
  switch (currentState) {
    case STATE_INACTIVE: doStateInactive(); break;
    case STATE_BOOT:     doStateBootUp();   break;
    case STATE_DEMO:     doStateDemo();     break;
    case STATE_FAILURE:  doStateFailure();  break;
    default:             allOutputsOff();   break;
  }

  // Push NeoPixel changes to the strip once per loop.
  strip.show();
}

// =============================================================================
// HELPER IMPLEMENTATIONS
// =============================================================================

// Turn a servo to the specified angle (0–180°) on the given PCA9685 channel.
void setServo(uint8_t channel, uint8_t angle) {
  uint16_t pulse = map(angle, 0, 180, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
  servoDriver.setPWM(channel, 0, pulse);
}

// Apply a named RGB beam palette at a 0-255 brightness level.
void setBeamPalette(BeamPaletteId paletteId, uint8_t level) {
  const uint8_t index = static_cast<uint8_t>(paletteId);
  if (index >= (sizeof(BEAM_PALETTES) / sizeof(BEAM_PALETTES[0]))) {
    return;
  }

  const BeamPaletteColor &palette = BEAM_PALETTES[index];
  analogWrite(BEAM_RED_PIN,   (palette.red   * level) / 255);
  analogWrite(BEAM_GREEN_PIN, (palette.green * level) / 255);
  analogWrite(BEAM_BLUE_PIN,  (palette.blue  * level) / 255);
}

// Apply a named RGBW NeoPixel palette at a 0-255 brightness level.
void setNeoPalette(NeoPaletteId paletteId, uint8_t level) {
  const uint8_t index = static_cast<uint8_t>(paletteId);
  if (index >= (sizeof(NEO_PALETTES) / sizeof(NEO_PALETTES[0]))) {
    return;
  }

  const NeoPaletteColor &palette = NEO_PALETTES[index];
  neoPixelSetAll((palette.red   * level) / 255,
                 (palette.green * level) / 255,
                 (palette.blue  * level) / 255,
                 (palette.white * level) / 255);
}

// Set every NeoPixel to the same RGBW color.
// w (white channel) defaults to 0 if not provided.
void neoPixelSetAll(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(r, g, b, w));
  }
  // Caller must call strip.show() to push the update — or rely on loop().
}

// Chase a short run of pixels from pixel 0 toward the end of the strip.
// r/g/b: chase color; len: length of the lit run; stepMs: ms per pixel advance.
// Non-blocking — advances one step per call if stepMs has elapsed.
void neoChase(uint8_t r, uint8_t g, uint8_t b, uint8_t len, uint32_t stepMs) {
  static uint16_t head   = 0;
  static uint32_t nextMs = 0;
  uint32_t now = millis();
  if (now < nextMs) return;
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);
  }
  for (int i = 0; i < len; i++) {
    int px = head - i;
    if (px >= 0 && px < strip.numPixels()) {
      uint8_t fade = map(i, 0, len, 255, 30);
      strip.setPixelColor(px, strip.Color((r * fade) / 255,
                                          (g * fade) / 255,
                                          (b * fade) / 255, 0));
    }
  }
  strip.show();
  head   = (head + 1) % strip.numPixels();
  nextMs = now + stepMs;
}

// Turn all NeoPixels off immediately.
void neoPixelOff() {
  strip.clear();
  strip.show();
}

// Shut down every output subsystem.
void allOutputsOff() {
  analogWrite(BEAM_RED_PIN,   0);
  analogWrite(BEAM_GREEN_PIN, 0);
  analogWrite(BEAM_BLUE_PIN,  0);
  analogWrite(SPARK_PIN_1, 0);
  analogWrite(SPARK_PIN_2, 0);
  analogWrite(SPARK_PIN_3, 0);
  analogWrite(SPARK_PIN_4, 0);
  servoIdle(0);
  servoIdle(1);
  neoPixelOff();
}

// Stop driving a servo — removes the PWM signal so the servo goes limp.
void servoIdle(uint8_t channel) {
  servoDriver.setPWM(channel, 0, 0);
}

// Print current state to Serial Monitor.
void printState() {
  Serial.print(F("Output: "));
  Serial.print(outputEnabled ? F("ON") : F("OFF"));
  Serial.print(F("  |  State: "));
  Serial.println(STATE_NAMES[currentState]);
}

// Debounce one switch.  Call every loop with the current millis() value.
// Populates sw.pressed and sw.released as single-shot flags.
void updateSwitch(SwitchState &sw, uint32_t now) {
  sw.pressed  = false;
  sw.released = false;

  const bool sampled = (digitalRead(sw.pin) == LOW);

  if (sampled != sw.raw) {
    sw.raw          = sampled;
    sw.lastChangeMs = now;
  }

  if ((now - sw.lastChangeMs) < DEBOUNCE_MS) {
    return;  // Still bouncing — wait
  }

  if (sw.stable == sw.raw) {
    return;  // No change
  }

  sw.stable = sw.raw;

  if (sw.stable) {
    sw.pressed  = true;
  } else {
    sw.released = true;
  }
}

// Process all three switches and update outputEnabled / currentState.
void handleSwitches(uint32_t now) {
  updateSwitch(swPower, now);
  updateSwitch(swPrev,  now);
  updateSwitch(swNext,  now);

  // --- Reset chord: SW1 + SW3 held together for RESET_HOLD_MS ---------------
  const bool chordDown = swPower.stable && swNext.stable;

  if (chordDown) {
    if (resetStartMs == 0) {
      resetStartMs       = now;
      suppressPowerEvent = true;  // Block the toggle event on release
      suppressNextEvent  = true;  // Block the sequence advance on release
    }
    if (!resetFired && (now - resetStartMs >= RESET_HOLD_MS)) {
      resetFired    = true;
      outputEnabled = false;
      currentState  = STATE_INACTIVE;
      allOutputsOff();
      Serial.println(F("RESET"));
      printState();
    }
  } else if (!swPower.stable && !swNext.stable) {
    // Both buttons fully released — clear chord tracking.
    resetStartMs       = 0;
    resetFired         = false;
    suppressPowerEvent = false;
    suppressNextEvent  = false;
  }

  // --- SW2: step forward through states (now always active) -----------------
  if (swPrev.released) {
    currentState = static_cast<DeviceState>((currentState + 1) % STATE_COUNT);
    // Skip STATE_OFF when cycling forward — OFF is only reached via SW1 toggle.
    if (currentState == STATE_OFF) {
      currentState = STATE_INACTIVE;
    }
    printState();
  }

  // --- SW1: toggle output on/off --------------------------------------------
  if (swPower.released && !suppressPowerEvent) {
    outputEnabled = !outputEnabled;
    if (!outputEnabled) {
      allOutputsOff();
    }
    printState();
  }

  // --- SW3: step backward through states ------------------------------------
  if (swNext.released && !suppressNextEvent) {
    if (currentState <= STATE_INACTIVE) {
      currentState = static_cast<DeviceState>(STATE_COUNT - 1);
    } else {
      currentState = static_cast<DeviceState>(currentState - 1);
    }
    printState();
  }
}
