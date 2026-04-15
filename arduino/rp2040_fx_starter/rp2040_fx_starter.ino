// =============================================================================
// rp2040_fx_starter.ino
// Version : V 0.2.5
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

#include <Wire.h>                      // I2C bus (required for servo driver)
#include <Adafruit_PWMServoDriver.h>   // PCA9685 servo wing library
#include <Adafruit_NeoPixel.h>         // NeoPixel strip library
#include <ColorPalettes.h>             // Beam and NeoPixel color palettes
#include <AnimationPalettes.h>         // Animation preset definitions

// =============================================================================
// STEP 1 — PIN CONFIGURATION
// These are the physical pin numbers wired on the Orciny PCB.
// Only change these if you are building on different hardware.
// =============================================================================

// Prop-Maker FeatherWing enable pin — MUST be HIGH or no outputs will work.
#define PROP_MAKER_PWR_PIN   10

// Prop-Maker MOSFET outputs (0–255 PWM, HIGH = on)
#define BEAM_RED_PIN         11
#define BEAM_GREEN_PIN       12
#define BEAM_BLUE_PIN        13

// Spark channels — current-limited GPIO outputs
#define SPARK_PIN_1          18
#define SPARK_PIN_2          19
#define SPARK_PIN_3          20
#define SPARK_PIN_4          24

// NeoPixel strip data line
#define NEO_DATA_PIN         25
#define NEO_PIXEL_COUNT      166      // Number of pixels on the strip
#define NEO_COLOR_ORDER      NEO_GRBW // SK6812 RGBW pixel order

// Momentary switches (INPUT_PULLUP: LOW = pressed)
#define SW_POWER_PIN         27       // GP27 — on/off toggle
#define SW_PREV_PIN          28       // GP28 — previous state
#define SW_NEXT_PIN          29       // GP29 — next state

// =============================================================================
// STEP 2 — TUNING PARAMETERS
// Adjust these values to change timing and behavior without touching the code.
// =============================================================================

// Debounce: ignore button transitions shorter than this (milliseconds).
// If buttons feel unresponsive, raise slightly.  If they double-trigger, raise more.
#define DEBOUNCE_MS          30

// Reset chord: hold SW1 + SW3 for this long to reset (milliseconds).
#define RESET_HOLD_MS        5000

// PCA9685 I2C address — 0x40 is the default (all address pads open).
#define SERVO_I2C_ADDR       0x40

// Servo pulse width range.  Tune these if servos don't reach full travel.
#define SERVO_PULSE_MIN      120
#define SERVO_PULSE_MAX      600

// =============================================================================
// STEP 3 — STATE DEFINITIONS
// Add or remove states here.  Update STATE_COUNT to match.
// The state names show up in Serial Monitor output.
// =============================================================================

enum DeviceState : uint8_t {
  STATE_OFF = 0,   // Outputs disabled — safe idle
  STATE_1,         // Rename these to match your scene (e.g. STATE_EMBER)
  STATE_2,
  STATE_3,
  STATE_4,
  STATE_COUNT      // Keep this last — used to wrap the selector
};

// Readable names printed to Serial Monitor when states change.
const char* STATE_NAMES[] = { "OFF", "Ember", "Cyan Pulse", "Full Show", "AnimPal Demo" };

// =============================================================================
// STEP 4 — GLOBAL OBJECTS (nothing to change here in most cases)
// =============================================================================

Adafruit_PWMServoDriver servoDriver(SERVO_I2C_ADDR);
Adafruit_NeoPixel       strip(NEO_PIXEL_COUNT, NEO_DATA_PIN, NEO_COLOR_ORDER + NEO_KHZ800);

DeviceState currentState   = STATE_1;   // Which state is active
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
void neoPixelSetAll(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);
void neoPixelOff();
void allOutputsOff();

// =============================================================================
// STATE BEHAVIORS
// Each doStateX() function is called every loop iteration while that state
// is active and outputEnabled is true.
//
// IMPORTANT: Do NOT use delay() inside these functions.  Use millis() for
//            timing.  See the non-blocking timer example in State 1 below.
// =============================================================================

// --- STATE 1 -----------------------------------------------------------------
// Replace the example code with your own effects.
//
// PALETTE EXAMPLES:
//   Beam color:     analogWrite(BEAM_RED_PIN, r); // Use ColorPalettes::kBeamEmber etc.
//   NeoPixel:       neoPixelSetAll(ColorPalettes::kNeoCyan.red, ...);
//   Available beam palettes:  kBeamCool, kBeamEmber, kBeamWarmWhite, kBeamCyan, kBeamViolet
//   Available neo palettes:   kNeoCool, kNeoEmber, kNeoWarmWhite, kNeoCyan, kNeoViolet, kNeoDeepRed
//   To add or modify palettes, please edit and define in the animationpalettes.h and colorpalettes.h
//   located in the OrcinyCommon library folder.

void doState1() {
  // ---- INSERT YOUR STATE 1 EFFECT CODE BELOW --------------------------------

  // Example: ember scene — warm spark flash + ember beam + ember NeoPixels
  static uint32_t nextFlashMs = 0;
  uint32_t now = millis();

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

  // ---- END OF STATE 1 CODE --------------------------------------------------
}

// --- STATE 2 -----------------------------------------------------------------
// PALETTE TIP: Use ColorPalettes to set beam colors without manually tuning RGB values.
// Example: Reference ColorPalettes::kBeamCyan for the cyan swell below.

void doState2() {
  // ---- INSERT YOUR STATE 2 EFFECT CODE BELOW --------------------------------

  // Example: pulsing cyan beam + matching NeoPixel swell
  uint32_t now   = millis();
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

  // ---- END OF STATE 2 CODE --------------------------------------------------
}

// --- STATE 3 -----------------------------------------------------------------
// PALETTE TIP: Use ColorPalettes::kNeoEmber or similar to set all NeoPixels to a palette color.
// Example: neoPixelSetAll(ColorPalettes::kNeoEmber.red, kNeoEmber.green, kNeoEmber.blue, kNeoEmber.white);

void doState3() {
  // ---- INSERT YOUR STATE 3 EFFECT CODE BELOW --------------------------------

  // Example: full show — sparks + violet beam + claw sweep + cyan NeoPixels
  static uint32_t nextSpark = 0;
  static int8_t   servo_dir = 1;
  static uint8_t  servo_angle = 22;
  static uint32_t nextServo = 0;
  uint32_t now = millis();

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

  // ---- END OF STATE 3 CODE --------------------------------------------------
}

// --- STATE 4 -----------------------------------------------------------------
// ANIMATION PALETTE ONLY EXAMPLE:
// This state demonstrates using AnimationPalettes timing/intensity presets
// directly for beam, NeoPixel pulse, and spark cadence.

void doState4() {
  // ---- INSERT YOUR STATE 4 EFFECT CODE BELOW --------------------------------

  const auto beamAnim  = AnimationPalettes::kBeamPulseMedium;
  const auto neoAnim   = AnimationPalettes::kNeoPulseFast;
  const auto sparkAnim = AnimationPalettes::kSparkCrackle;
  const uint32_t now   = millis();

  // Beam level from animation preset envelope.
  uint8_t beamLevel = beamAnim.getCurrentSwell(now);
  analogWrite(BEAM_RED_PIN,   beamLevel / 8);
  analogWrite(BEAM_GREEN_PIN, beamLevel);
  analogWrite(BEAM_BLUE_PIN,  beamLevel);

  // Neo pulse level from animation timing + intensity presets.
  uint8_t neoLevel = neoAnim.intensity;
  if (neoAnim.mode == AnimationPalettes::NEO_MODE_PULSE && neoAnim.cycleMs > 0) {
    uint8_t phase = map(now % neoAnim.cycleMs, 0, neoAnim.cycleMs, 0, 100);
    neoLevel = map(getSineEnvelope(phase), 0, 106, 8, neoAnim.intensity);
  }
  neoPixelSetAll(0, neoLevel / 2, neoLevel, neoLevel / 4);

  // Spark cadence and intensity from spark animation preset.
  static uint32_t nextSparkStepMs = 0;
  static bool sparkOn = false;
  if (now >= nextSparkStepMs) {
    sparkOn = !sparkOn;
    if (sparkOn) {
      uint8_t sparkLevel = random(sparkAnim.intensityFloor, sparkAnim.peakIntensity + 1);
      analogWrite(SPARK_PIN_1, sparkLevel);
      analogWrite(SPARK_PIN_2, sparkLevel);
      analogWrite(SPARK_PIN_3, sparkLevel / 2);
      analogWrite(SPARK_PIN_4, sparkLevel / 2);
      nextSparkStepMs = now + random(sparkAnim.minFlashMs, sparkAnim.maxFlashMs + 1);
    } else {
      analogWrite(SPARK_PIN_1, 0);
      analogWrite(SPARK_PIN_2, 0);
      analogWrite(SPARK_PIN_3, 0);
      analogWrite(SPARK_PIN_4, 0);
      nextSparkStepMs = now + random(sparkAnim.minGapMs, sparkAnim.maxGapMs + 1);
    }
  }

  // ---- END OF STATE 4 CODE --------------------------------------------------
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
  for (SwitchState *sw : {&swPower, &swPrev, &swNext}) {
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
    case STATE_1: doState1(); break;
    case STATE_2: doState2(); break;
    case STATE_3: doState3(); break;
    case STATE_4: doState4(); break;
    default:      allOutputsOff(); break;
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

// Set every NeoPixel to the same RGBW color.
// w (white channel) defaults to 0 if not provided.
void neoPixelSetAll(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(r, g, b, w));
  }
  // Caller must call strip.show() to push the update — or rely on loop().
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
  neoPixelOff();
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
      currentState  = STATE_1;
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

  // --- SW2: step backward through states (always active) --------------------
  if (swPrev.released) {
    if (currentState <= STATE_1) {
      currentState = static_cast<DeviceState>(STATE_COUNT - 1);
    } else {
      currentState = static_cast<DeviceState>(currentState - 1);
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

  // --- SW3: step forward through states -------------------------------------
  if (swNext.released && !suppressNextEvent) {
    currentState = static_cast<DeviceState>((currentState + 1) % STATE_COUNT);
    // Skip STATE_OFF when cycling forward — OFF is only reached via SW1 toggle.
    if (currentState == STATE_OFF) {
      currentState = STATE_1;
    }
    printState();
  }
}
