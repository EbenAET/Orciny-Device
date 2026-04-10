// =============================================================================
// rp2040_fx_controller_demo.ino
// Orciny Device — FX Controller  |  Demo / Testing Build
// Board : Adafruit Feather RP2040
// Wings : Prop-Maker FeatherWing + 8-Channel Servo FeatherWing (PCA9685)
// -----------------------------------------------------------------------------
// PURPOSE
//   This is the fully-commented demo version of the main FX controller sketch.
//   All logic is identical to the production sketch; comments have been added
//   throughout to explain every section for anyone reviewing or testing it.
//
// CONTROL PATHS — how the hardware receives show commands
//   A) Effect-Link mode  (active by default in this sketch)
//      An upstream controller sends EffectCommand packets over the hardware
//      UART (Serial1 / TX/RX pins).  updateEffects() uses those packets
//      directly.  If no packet arrives within kEffectCommandTimeoutMs all
//      outputs shut off automatically.
//
//   B) Standalone mode  (functions exist but are NOT wired into loop() here)
//      handleSwitches(), handleUsbCommands(), and buildActiveProfile() are
//      fully implemented below.  To run without an upstream controller, call
//      those three functions inside loop() and pass the resulting SceneProfile
//      through profileToEffectCommand() into updateEffects().  See the
//      commented stub at the bottom of loop() for the exact wiring.
//
// USB SERIAL COMMANDS (115200 baud, standalone mode)
//   help            — list all commands
//   status          — print power, sequence, and effect overrides
//   on / off        — enable or disable all outputs
//   toggle          — flip current output state
//   prev / next     — step through sequences
//   seq1 / seq2 / seq3 — jump directly to a numbered sequence
//   reset           — return to sequence 1, output off
//   sparks on|off|toggle|auto — per-effect override
//   beam on|off|toggle|auto   — per-effect override
//   claw on|off|toggle|auto   — per-effect override
//   s|b|c on|off|toggle|auto  — short aliases for sparks|beam|claw
//
// PHYSICAL CONTROLS (standalone mode)
//   SW1 (A1 / GP27) — tap: toggle output on/off
//   SW2 (A2 / GP28) — tap: step to previous sequence
//   SW3 (A3 / GP29) — tap: step to next sequence
//   SW1 + SW3 held 5 s — reset to sequence 1
//
// DEMO SEQUENCES
//   1 — Ember  : sparks on, warm-orange NeoPixel ember glow
//   2 — Pulse  : beam pulsing cool-white, orange NeoPixel pulse
//   3 — Show   : sparks + beam + claw sweep + cyan NeoPixel show mode
// =============================================================================

#include <Adafruit_PWMServoDriver.h>  // PCA9685 servo wing driver
#include <Wire.h>                     // I2C bus (required by servo driver)

#include <OrcinyCommon.h>  // Shared packet types: CoreFrame, EffectCommand, etc.

#include "DeviceConfig.h"  // All pin assignments and tuning constants

// Pull frequently-used types into this file's scope so we don't have to
// prefix every usage with "orciny::".
using orciny::CoreFrame;
using orciny::EffectCommand;
using orciny::CoreMode;

// =============================================================================
// SEQUENCE IDs
// A SequenceId is just a named number.  It identifies which "scene" is active
// and drives the content of buildActiveProfile() below.
// =============================================================================
enum SequenceId : uint8_t {
  SEQUENCE_1 = 0,   // Ember scene
  SEQUENCE_2,       // Pulse scene
  SEQUENCE_3,       // Full show scene
  SEQUENCE_COUNT,   // Automatically equals the number of sequences (3)
};

// =============================================================================
// SceneProfile
// A flat data record that describes what every output subsystem should be doing
// for the currently active sequence.  buildActiveProfile() fills one of these;
// profileToEffectCommand() converts it into a wire-format EffectCommand.
// =============================================================================
struct SceneProfile {
  bool    sparksEnabled;
  uint8_t sparksIntensity;   // 0–255 PWM brightness of random spark flashes
  bool    beamEnabled;       // Prop-Maker LED beam on/off
  bool    clawEnabled;       // Servo claw sweep on/off
  CoreFrame coreFrame;       // NeoPixel core parameters (mode, color, speed…)
};

// =============================================================================
// MomentarySwitch
// Wraps a single digital input with software debouncing.
//
// Why debounce?  Mechanical switches bounce — the contacts open and close
// rapidly for ~5–20 ms after a press.  Without debouncing, one physical press
// can register as many events.  This class ignores transitions shorter than
// kSwitchDebounceMs before accepting them as stable.
//
// Usage:
//   switch.begin(PIN);         // call once in setup()
//   switch.update(millis());   // call every loop iteration
//   if (switch.wasPressed())   // true for exactly one loop after press
// =============================================================================
class MomentarySwitch {
 public:
  void begin(uint8_t pin) {
    pin_ = pin;
    pinMode(pin_, INPUT_PULLUP);  // Internal pull-up: floating = HIGH, pressed = LOW
    // Sample the physical state immediately so the first update() is accurate.
    const bool initialPressed = (digitalRead(pin_) == LOW);
    stablePressed_ = initialPressed;
    rawPressed_ = initialPressed;
    lastChangeMs_ = millis();
  }

  // Call once per loop iteration with the current millis() timestamp.
  void update(uint32_t now) {
    // Reset single-shot flags at the start of every update.
    pressedEvent_ = false;
    releasedEvent_ = false;

    const bool sampledPressed = (digitalRead(pin_) == LOW);

    // If the raw reading changed, restart the debounce timer.
    if (sampledPressed != rawPressed_) {
      rawPressed_ = sampledPressed;
      lastChangeMs_ = now;
    }

    // Wait for the signal to be stable for the full debounce window.
    if ((now - lastChangeMs_) < device_config::kSwitchDebounceMs) {
      return;  // Still within bouncing window — do nothing yet.
    }

    // If the stable state already matches the raw reading, nothing changed.
    if (stablePressed_ == rawPressed_) {
      return;
    }

    // The state changed and has been stable long enough — accept it.
    stablePressed_ = rawPressed_;
    if (stablePressed_) {
      pressedEvent_ = true;   // Fires on the loop after the press settles.
    } else {
      releasedEvent_ = true;  // Fires on the loop after the release settles.
    }
  }

  bool isPressed() const  { return stablePressed_; }  // Current stable state
  bool wasPressed() const { return pressedEvent_; }   // True for one loop on press
  bool wasReleased() const { return releasedEvent_; } // True for one loop on release

 private:
  uint8_t  pin_ = 0;
  bool     stablePressed_  = false;
  bool     rawPressed_     = false;
  bool     pressedEvent_   = false;
  bool     releasedEvent_  = false;
  uint32_t lastChangeMs_   = 0;
};

// =============================================================================
// SparkChannel
// Drives one GPIO pin with random PWM bursts to simulate a spark-gap element.
//
// Each channel independently decides when to fire and for how long using
// random timing windows.  The intensity parameter scales the peak brightness
// and also raises the floor so sparks never go completely dark during a flash.
// =============================================================================
class SparkChannel {
 public:
  void begin(uint8_t pin) {
    pin_ = pin;
    pinMode(pin_, OUTPUT);
    analogWrite(pin_, 0);  // Start off
    // Stagger the first event so all four channels don't fire simultaneously.
    nextEventMs_ = millis() + random(30, 180);
  }

  // enabled   : false forces pin low immediately and reschedules.
  // intensity : 0–255 PWM ceiling for peak brightness.
  void update(uint32_t now, bool enabled, uint8_t intensity) {
    if (!enabled) {
      analogWrite(pin_, 0);
      active_ = false;
      // Longer cool-down when disabled to prevent instant re-fire on enable.
      nextEventMs_ = now + random(80, 200);
      return;
    }

    if (active_) {
      // Currently in a flash burst — check whether the burst has expired.
      if (now >= flashUntilMs_) {
        active_ = false;
        analogWrite(pin_, 0);
        nextEventMs_ = now + random(25, 140);  // Random gap before next flash
      }
      return;  // Mid-burst: nothing to do until expiry.
    }

    // Waiting between flashes — check if it's time to fire.
    if (now < nextEventMs_) {
      return;
    }

    // Fire!  Choose a random PWM value between a floor and the intensity cap.
    const int minimum = max(32, intensity / 3);   // Minimum brightness floor
    const int peak    = max(minimum + 1, static_cast<int>(intensity) + 1);
    analogWrite(pin_, random(minimum, peak));
    // Hold the flash for a random short duration (15–70 ms).
    flashUntilMs_ = now + random(15, 70);
    active_ = true;
  }

 private:
  uint8_t  pin_          = 0;
  bool     active_       = false;
  uint32_t nextEventMs_  = 0;
  uint32_t flashUntilMs_ = 0;
};

// =============================================================================
// BeamEffect
// Drives the three Prop-Maker MOSFET channels (Red/Green/Blue) with a slow
// sinusoidal swell.
//
// The swell is computed from millis() so it runs continuously whether or not
// there is an upstream controller — no state machine needed.
//
// How the swell works:
//   • phase = (now % 2400) gives a 2.4-second repeating window (0–2399).
//   • First half (0–1199): linearly ramps from 100 → 255 (brightening).
//   • Second half (1200–2399): linearly ramps from 255 → 100 (dimming).
//   • Red is attenuated to 1/5 of the swell for a cyan/white tint.
//   • Blue gets a slight boost (+30) to skew the color cooler.
// =============================================================================
class BeamEffect {
 public:
  void begin(uint8_t led1Pin, uint8_t led2Pin, uint8_t led3Pin) {
    redPin_   = led1Pin;
    greenPin_ = led2Pin;
    bluePin_  = led3Pin;
    pinMode(redPin_,   OUTPUT);
    pinMode(greenPin_, OUTPUT);
    pinMode(bluePin_,  OUTPUT);
    stop();
  }

  void update(uint32_t now, bool enabled) {
    if (!enabled) {
      stop();
      return;
    }

    const uint16_t phase = now % 2400;  // 2.4-second cycle

    // Linear ramp up then down, staying in the 100–255 range.
    const uint8_t swell = phase < 1200
                          ? map(phase, 0,    1200, 100, 255)
                          : map(phase, 1200, 2400, 255, 100);

    analogWrite(redPin_,   swell / 5);                       // dim red
    analogWrite(greenPin_, swell);                           // full green
    analogWrite(bluePin_,  min<uint16_t>(255, swell + 30));  // boosted blue
    active_ = true;
  }

  void stop() {
    analogWrite(redPin_,   0);
    analogWrite(greenPin_, 0);
    analogWrite(bluePin_,  0);
    active_ = false;
  }

 private:
  uint8_t redPin_   = 0;
  uint8_t greenPin_ = 0;
  uint8_t bluePin_  = 0;
  bool    active_   = false;
};

// =============================================================================
// ClawEffect
// Drives two servo channels on the PCA9685 in a continuous back-and-forth sweep.
//
// Servo A sweeps from minAngleA to maxAngleA and back.
// Servo B mirrors Servo A using a cross-mapped angle so both claw halves open
// and close together (one going up while the other goes down).
//
// The step interval (kClawStepIntervalMs) controls how fast the claw sweeps.
// Each update() call advances the angle by exactly one degree if the interval
// has elapsed — no delays or blocking waits.
// =============================================================================
class ClawEffect {
 public:
  void begin(Adafruit_PWMServoDriver &driver,
             uint8_t servoChannelA, uint8_t servoChannelB,
             uint8_t minAngleA,    uint8_t maxAngleA,
             uint8_t minAngleB,    uint8_t maxAngleB) {
    driver_        = &driver;
    servoChannelA_ = servoChannelA;
    servoChannelB_ = servoChannelB;
    // Ensure min < max regardless of which way the caller passed them.
    minAngleA_ = min(minAngleA, maxAngleA);
    maxAngleA_ = max(minAngleA, maxAngleA);
    minAngleB_ = min(minAngleB, maxAngleB);
    maxAngleB_ = max(minAngleB, maxAngleB);
    angle_     = minAngleA_;
    direction_ = 1;             // Start sweeping toward maxAngleA_
    writeServos(static_cast<uint8_t>(angle_));
  }

  void update(uint32_t now, bool enabled) {
    if (!enabled) {
      stop();
      return;
    }

    // Non-blocking timer: only advance one step after the interval elapses.
    if (now < nextStepMs_) {
      return;
    }

    angle_ += direction_;

    // Reverse direction at each travel limit.
    if (angle_ >= maxAngleA_) {
      angle_     = maxAngleA_;
      direction_ = -1;
    } else if (angle_ <= minAngleA_) {
      angle_     = minAngleA_;
      direction_ = 1;
    }

    writeServos(static_cast<uint8_t>(angle_));
    nextStepMs_ = now + device_config::kClawStepIntervalMs;
    active_ = true;
  }

  // Park claw at the minimum (closed) position.
  void stop() {
    if (!active_) {
      return;  // Already stopped — avoid redundant I2C writes.
    }
    writeServos(minAngleA_);
    active_ = false;
  }

 private:
  // Convert a 0–180° angle to the PCA9685 pulse-count range.
  uint16_t angleToPulse(uint8_t angle) const {
    return map(angle, 0, 180,
               device_config::kServoMinPulse,
               device_config::kServoMaxPulse);
  }

  // Mirror Servo A's angle onto Servo B's range so they move in opposition.
  uint8_t mapAngleForB(uint8_t angleA) const {
    return map(angleA, minAngleA_, maxAngleA_, maxAngleB_, minAngleB_);
  }

  // Push both servo positions to the PCA9685 over I2C.
  void writeServos(uint8_t angleA) {
    if (driver_ == nullptr) {
      return;
    }
    const uint8_t clampedA = constrain(angleA, minAngleA_, maxAngleA_);
    const uint8_t angleB   = constrain(mapAngleForB(clampedA), minAngleB_, maxAngleB_);
    driver_->setPWM(servoChannelA_, 0, angleToPulse(clampedA));
    driver_->setPWM(servoChannelB_, 0, angleToPulse(angleB));
  }

  Adafruit_PWMServoDriver *driver_ = nullptr;
  uint8_t  servoChannelA_ = 0;
  uint8_t  servoChannelB_ = 1;
  int16_t  angle_         = 0;
  uint8_t  minAngleA_     = 0;
  uint8_t  maxAngleA_     = 180;
  uint8_t  minAngleB_     = 0;
  uint8_t  maxAngleB_     = 180;
  int8_t   direction_     = 1;
  bool     active_        = false;
  uint32_t nextStepMs_    = 0;
};

// =============================================================================
// GLOBAL INSTANCES
// One object per physical subsystem.  All are initialized in setup().
// =============================================================================

SparkChannel sparks[device_config::kSparkCount];  // Four independent spark channels
BeamEffect   beamEffect;                           // Prop-Maker RGB beam
ClawEffect   clawEffect;                           // Dual-servo claw
Adafruit_PWMServoDriver servoDriver(device_config::kServoDriverI2cAddress);

MomentarySwitch powerSwitch;    // SW1 — on/off toggle
MomentarySwitch previousSwitch; // SW2 — previous sequence
MomentarySwitch nextSwitch;     // SW3 — next sequence

// =============================================================================
// GLOBAL STATE VARIABLES
// These track the runtime state; they are manipulated by the standalone-mode
// control functions (handleSwitches, handleUsbCommands).
// =============================================================================

SequenceId currentSequence      = SEQUENCE_1;  // Which scene is active
bool       outputEnabled        = false;         // Master on/off flag

enum EffectOverride : uint8_t {
  OVERRIDE_AUTO = 0,
  OVERRIDE_FORCE_OFF,
  OVERRIDE_FORCE_ON,
};

EffectOverride sparksOverride = OVERRIDE_AUTO;
EffectOverride beamOverride   = OVERRIDE_AUTO;
EffectOverride clawOverride   = OVERRIDE_AUTO;

// USB receive buffer — characters accumulate here until a newline is received.
String usbCommandBuffer;

// Serial1 receive buffer — same pattern for EffectCommand packets.
String effectLinkBuffer;

uint32_t lastCoreFrameMs     = 0;  // Timestamp of last received CoreFrame
uint32_t lastEffectCommandMs = 0;  // Timestamp of last received EffectCommand

// State for the SW1+SW3 reset chord detection.
uint32_t resetChordStartMs    = 0;
bool     resetChordTriggered  = false;
bool     suppressPowerRelease = false;  // Prevent toggle fire after chord hold
bool     suppressNextRelease  = false;  // Prevent seq advance after chord hold

// The most recently received EffectCommand from Serial1 (effect-link mode).
EffectCommand currentEffectCommand = orciny::defaultEffectCommand();

// =============================================================================
// UTILITY — status printing
// =============================================================================

// Print a list of available USB serial commands.
void printHelp() {
  Serial.println(F("Commands: help, status, on, off, toggle, prev, next, seq1, seq2, seq3, reset"));
  Serial.println(F("         sparks on|off|toggle|auto"));
  Serial.println(F("         beam   on|off|toggle|auto"));
  Serial.println(F("         claw   on|off|toggle|auto"));
  Serial.println(F("         s|b|c  on|off|toggle|auto"));
}

const __FlashStringHelper *overrideLabel(EffectOverride value) {
  switch (value) {
    case OVERRIDE_FORCE_OFF: return F("FORCE_OFF");
    case OVERRIDE_FORCE_ON:  return F("FORCE_ON");
    default:                 return F("AUTO");
  }
}

bool applyOverride(bool baseState, EffectOverride value) {
  if (value == OVERRIDE_FORCE_ON) {
    return true;
  }
  if (value == OVERRIDE_FORCE_OFF) {
    return false;
  }
  return baseState;
}

void printOverrideStatus() {
  Serial.print(F("Overrides -> sparks: "));
  Serial.print(overrideLabel(sparksOverride));
  Serial.print(F(", beam: "));
  Serial.print(overrideLabel(beamOverride));
  Serial.print(F(", claw: "));
  Serial.println(overrideLabel(clawOverride));
}

// Print the current power state and sequence number to USB serial.
void printSequenceStatus() {
  Serial.print(F("Power -> "));
  Serial.print(outputEnabled ? F("ON") : F("OFF"));
  Serial.print(F(", Sequence -> "));
  Serial.println(static_cast<uint8_t>(currentSequence) + 1);
  printOverrideStatus();
}

// =============================================================================
// SCENE PROFILES
// buildActiveProfile() is the heart of standalone mode.  It maps the current
// (outputEnabled, currentSequence) state to a full SceneProfile that describes
// what every subsystem should do.
//
// Add or modify cases here to change what each button-selectable scene does.
// NOTE: This function is called only in standalone mode.  In effect-link mode
//       the upstream controller sends EffectCommands directly over Serial1.
// =============================================================================
SceneProfile buildActiveProfile() {
  SceneProfile profile = {};
  profile.coreFrame = orciny::defaultFrame();

  // Output off — zero everything and return early.
  if (!outputEnabled) {
    profile.coreFrame.mode       = orciny::CORE_MODE_OFF;
    profile.coreFrame.brightness = 0;
    return profile;
  }

  switch (currentSequence) {

    // --- SEQUENCE 1: Ember ---------------------------------------------------
    // Sparks flash at high intensity.  NeoPixel glows a warm orange ember.
    case SEQUENCE_1:
      profile.sparksEnabled    = true;
      profile.sparksIntensity  = 220;
      profile.coreFrame.mode       = orciny::CORE_MODE_EMBER;
      profile.coreFrame.brightness = 56;
      profile.coreFrame.speed      = 90;
      profile.coreFrame.red        = 255;
      profile.coreFrame.green      = 110;
      profile.coreFrame.blue       = 24;
      return profile;

    // --- SEQUENCE 2: Pulse ---------------------------------------------------
    // Beam lamp cycles on/off.  NeoPixel pulses in a cooler orange.
    case SEQUENCE_2:
      profile.beamEnabled = true;
      profile.coreFrame.mode       = orciny::CORE_MODE_PULSE;
      profile.coreFrame.brightness = 96;
      profile.coreFrame.speed      = 110;
      profile.coreFrame.red        = 255;
      profile.coreFrame.green      = 48;
      profile.coreFrame.blue       = 0;
      return profile;

    // --- SEQUENCE 3: Full Show -----------------------------------------------
    // Everything on: sparks at max, beam running, claw sweeping, NeoPixel show.
    case SEQUENCE_3:
      profile.sparksEnabled    = true;
      profile.sparksIntensity  = 255;
      profile.beamEnabled      = true;
      profile.clawEnabled      = true;
      profile.coreFrame.mode       = orciny::CORE_MODE_SHOW;
      profile.coreFrame.brightness = 180;
      profile.coreFrame.speed      = 140;
      profile.coreFrame.red        = 60;
      profile.coreFrame.green      = 220;
      profile.coreFrame.blue       = 255;
      return profile;

    default:
      profile.coreFrame.mode       = orciny::CORE_MODE_OFF;
      profile.coreFrame.brightness = 0;
      return profile;
  }
}

// Convert a SceneProfile into an EffectCommand so updateEffects() can consume
// it through the same path used by the Serial1 effect link.
EffectCommand profileToEffectCommand(const SceneProfile &profile) {
  EffectCommand command = orciny::defaultEffectCommand();
  command.outputEnabled   = outputEnabled;
  command.sparksEnabled   = applyOverride(profile.sparksEnabled, sparksOverride);
  command.sparksIntensity = command.sparksEnabled
                            ? (profile.sparksIntensity > 0 ? profile.sparksIntensity : 220)
                            : 0;
  command.pulseEnabled    = false;  // Not used in current hardware
  command.beamEnabled     = applyOverride(profile.beamEnabled, beamOverride);
  command.clawEnabled     = applyOverride(profile.clawEnabled, clawOverride);
  return command;
}

// =============================================================================
// SEQUENCE NAVIGATION HELPERS
// =============================================================================

SequenceId previousSequenceValue(SequenceId sequence) {
  // Wrap around: sequence 1 → wraps back to sequence 3.
  return sequence == SEQUENCE_1 ? SEQUENCE_3
                                : static_cast<SequenceId>(sequence - 1);
}

SequenceId nextSequenceValue(SequenceId sequence) {
  // Wrap around: sequence 3 → wraps back to sequence 1.
  return sequence == SEQUENCE_3 ? SEQUENCE_1
                                : static_cast<SequenceId>(sequence + 1);
}

void setSequence(SequenceId sequence) {
  currentSequence = sequence;
  printSequenceStatus();  // Echo new state to USB serial
}

void resetToBeginning() {
  currentSequence = SEQUENCE_1;
  outputEnabled = false;
  sparksOverride = OVERRIDE_AUTO;
  beamOverride = OVERRIDE_AUTO;
  clawOverride = OVERRIDE_AUTO;
  Serial.println(F("Reset -> sequence 1"));
  printSequenceStatus();
}

// =============================================================================
// STANDALONE MODE — USB SERIAL COMMAND HANDLER
// Reads characters from USB Serial one at a time and processes complete lines.
// Call this from loop() to enable USB debug control (standalone mode only).
// =============================================================================
void handleUsbCommands() {
  while (Serial.available() > 0) {
    const char incoming = static_cast<char>(Serial.read());
    if (incoming == '\r') {
      continue;  // Skip Windows carriage-return characters
    }
    if (incoming != '\n') {
      usbCommandBuffer += incoming;  // Accumulate until newline
      continue;
    }

    // Newline received — process the buffered line.
    String command = usbCommandBuffer;
    usbCommandBuffer = "";
    command.trim();
    command.toLowerCase();

    if (command.length() == 0) {
      continue;  // Ignore blank lines
    }

    if      (command == F("help"))   { printHelp(); }
    else if (command == F("status")) { printSequenceStatus(); }
    else if (command == F("on"))     { outputEnabled = true;              printSequenceStatus(); }
    else if (command == F("off"))    { outputEnabled = false;             printSequenceStatus(); }
    else if (command == F("toggle")) { outputEnabled = !outputEnabled;    printSequenceStatus(); }
    else if (command == F("prev"))   { setSequence(previousSequenceValue(currentSequence)); }
    else if (command == F("next"))   { setSequence(nextSequenceValue(currentSequence)); }
    else if (command == F("seq1"))   { setSequence(SEQUENCE_1); }
    else if (command == F("seq2"))   { setSequence(SEQUENCE_2); }
    else if (command == F("seq3"))   { setSequence(SEQUENCE_3); }
    else if (command == F("reset"))  { resetToBeginning(); }
    else if (command == F("sparks on") || command == F("s on"))     { sparksOverride = OVERRIDE_FORCE_ON;  printOverrideStatus(); }
    else if (command == F("sparks off") || command == F("s off"))    { sparksOverride = OVERRIDE_FORCE_OFF; printOverrideStatus(); }
    else if (command == F("sparks auto") || command == F("s auto"))   { sparksOverride = OVERRIDE_AUTO;      printOverrideStatus(); }
    else if (command == F("sparks toggle") || command == F("s toggle")) {
      sparksOverride = (sparksOverride == OVERRIDE_FORCE_ON) ? OVERRIDE_FORCE_OFF : OVERRIDE_FORCE_ON;
      printOverrideStatus();
    }
    else if (command == F("beam on") || command == F("b on"))       { beamOverride = OVERRIDE_FORCE_ON;    printOverrideStatus(); }
    else if (command == F("beam off") || command == F("b off"))      { beamOverride = OVERRIDE_FORCE_OFF;   printOverrideStatus(); }
    else if (command == F("beam auto") || command == F("b auto"))     { beamOverride = OVERRIDE_AUTO;        printOverrideStatus(); }
    else if (command == F("beam toggle") || command == F("b toggle")) {
      beamOverride = (beamOverride == OVERRIDE_FORCE_ON) ? OVERRIDE_FORCE_OFF : OVERRIDE_FORCE_ON;
      printOverrideStatus();
    }
    else if (command == F("claw on") || command == F("c on"))       { clawOverride = OVERRIDE_FORCE_ON;    printOverrideStatus(); }
    else if (command == F("claw off") || command == F("c off"))      { clawOverride = OVERRIDE_FORCE_OFF;   printOverrideStatus(); }
    else if (command == F("claw auto") || command == F("c auto"))     { clawOverride = OVERRIDE_AUTO;        printOverrideStatus(); }
    else if (command == F("claw toggle") || command == F("c toggle")) {
      clawOverride = (clawOverride == OVERRIDE_FORCE_ON) ? OVERRIDE_FORCE_OFF : OVERRIDE_FORCE_ON;
      printOverrideStatus();
    }
    else {
      Serial.print(F("Unknown command: "));
      Serial.println(command);
      printHelp();
    }
  }
}

// =============================================================================
// STANDALONE MODE — PHYSICAL SWITCH HANDLER
// Reads the three momentary switches, handles the SW1+SW3 reset chord, and
// updates outputEnabled / currentSequence accordingly.
// Call this from loop() to enable hardware button control (standalone mode).
// =============================================================================
void handleSwitches(uint32_t now) {
  // Update all three switch debounce state machines.
  powerSwitch.update(now);
  previousSwitch.update(now);
  nextSwitch.update(now);

  // --- Reset chord: SW1 + SW3 held for kResetHoldMs -------------------------
  const bool resetChordDown = powerSwitch.isPressed() && nextSwitch.isPressed();

  if (resetChordDown) {
    if (resetChordStartMs == 0) {
      resetChordStartMs    = now;
      suppressPowerRelease = true;  // Don't fire toggle on chord release
      suppressNextRelease  = true;  // Don't fire sequence advance on chord release
    }
    if (!resetChordTriggered &&
        (now - resetChordStartMs >= device_config::kResetHoldMs)) {
      resetChordTriggered = true;
      resetToBeginning();
    }
  } else if (!powerSwitch.isPressed() && !nextSwitch.isPressed()) {
    // Both released — clear chord tracking.
    resetChordStartMs    = 0;
    resetChordTriggered  = false;
    suppressPowerRelease = false;
    suppressNextRelease  = false;
  }

  // --- Individual button events (only fire if not part of a chord) ----------
  if (previousSwitch.wasReleased()) {
    setSequence(previousSequenceValue(currentSequence));
  }

  if (powerSwitch.wasReleased() && !suppressPowerRelease) {
    outputEnabled = !outputEnabled;
    printSequenceStatus();
  }

  if (nextSwitch.wasReleased() && !suppressNextRelease) {
    setSequence(nextSequenceValue(currentSequence));
  }
}

// =============================================================================
// EFFECT-LINK MODE — SERIAL1 PACKET RECEIVER
// Reads EffectCommand packets from the hardware UART (Serial1).
// The upstream controller sends "FX,<on>,<sparks>,<intensity>,..." lines.
// Parsed packets are stored in currentEffectCommand with a fresh timestamp.
// =============================================================================
void processEffectLink(uint32_t now) {
  while (Serial1.available() > 0) {
    const char incoming = static_cast<char>(Serial1.read());
    if (incoming == '\r') {
      continue;  // Skip carriage returns
    }

    if (incoming != '\n') {
      effectLinkBuffer += incoming;  // Accumulate until newline
      continue;
    }

    // Try to parse the buffered line as an EffectCommand.
    EffectCommand command = orciny::defaultEffectCommand();
    if (orciny::readEffectCommand(effectLinkBuffer, command)) {
      currentEffectCommand = command;
      lastEffectCommandMs  = now;  // Record arrival time for timeout check
    }
    effectLinkBuffer = "";
  }
}

// Return the most recent EffectCommand if it arrived within the timeout window,
// otherwise return a default (all outputs off) command.
// This safety fallback ensures all outputs go dark if the upstream controller
// stops sending (e.g. power loss or cable disconnect).
EffectCommand resolveEffectCommand(uint32_t now) {
  const bool commandIsFresh =
      (lastEffectCommandMs > 0) &&
      ((now - lastEffectCommandMs) <= device_config::kEffectCommandTimeoutMs);

  if (commandIsFresh) {
    return currentEffectCommand;
  }
  return orciny::defaultEffectCommand();  // Failsafe: all off
}

// =============================================================================
// EFFECT UPDATE — applies the active EffectCommand to all hardware outputs.
// Called every loop iteration with the current timestamp.
// =============================================================================
void updateEffects(uint32_t now, const EffectCommand &command) {

  // Gate every subsystem on the master outputEnabled flag from the command.
  const bool sparksEnabled = command.outputEnabled && command.sparksEnabled;
  const bool beamEnabled   = command.outputEnabled && command.beamEnabled;
  const bool clawEnabled   = command.outputEnabled && command.clawEnabled;

  for (uint8_t i = 0; i < device_config::kSparkCount; ++i) {
    sparks[i].update(now, sparksEnabled, command.sparksIntensity);
  }
  beamEffect.update(now, beamEnabled);
  clawEffect.update(now, clawEnabled);
}

void updateEffects(uint32_t now) {
  const EffectCommand command = resolveEffectCommand(now);
  updateEffects(now, command);
}

// =============================================================================
// SETUP
// Runs once at power-on or after a reset.  Initialises every subsystem in order.
// =============================================================================
void setup() {
  // Start USB serial console (appears on the Arduino IDE Serial Monitor).
  Serial.begin(device_config::kUsbBaudRate);

  // Start hardware UART for effect-link packets from the upstream controller.
  Serial1.begin(device_config::kCoreLinkBaudRate);

  // Initialise I2C bus required by the Servo FeatherWing.
  Wire.begin();
  servoDriver.begin();
  servoDriver.setPWMFreq(50);  // 50 Hz is standard for hobby servos

  // Seed the random number generator with an unconnected analog pin noise value.
  // This prevents the spark timing pattern from being identical every boot.
  randomSeed(analogRead(A0));

  // IMPORTANT: the Prop-Maker PWR pin must be driven HIGH before any MOSFET
  // output will work.  If this line is omitted, sparks and beam stay silent.
  pinMode(device_config::kPropMakerPwrPin, OUTPUT);
  digitalWrite(device_config::kPropMakerPwrPin, HIGH);

  // Initialise all four spark channels on their respective GPIO pins.
  for (uint8_t i = 0; i < device_config::kSparkCount; ++i) {
    sparks[i].begin(device_config::kSparkPins[i]);
  }

  // Initialise the Prop-Maker beam effect with the three MOSFET LED pins.
  beamEffect.begin(device_config::kPropMakerLed1Pin,
                   device_config::kPropMakerLed2Pin,
                   device_config::kPropMakerLed3Pin);

  // Initialise the claw with servo driver reference, channel numbers, and travel limits.
  clawEffect.begin(servoDriver,
                   device_config::kServoChannelA,
                   device_config::kServoChannelB,
                   device_config::kServoAMinAngle,
                   device_config::kServoAMaxAngle,
                   device_config::kServoBMinAngle,
                   device_config::kServoBMaxAngle);

  // Start with a clean default command (all outputs off).
  currentEffectCommand = orciny::defaultEffectCommand();
}

// =============================================================================
// LOOP
// Runs repeatedly after setup().  Keep it fast — no delay() calls.
//
// CURRENT MODE: Effect-Link (receives commands from upstream controller)
//   - processEffectLink() ingests Serial1 packets.
//   - updateEffects()     applies the latest command to hardware.
//
// TO SWITCH TO STANDALONE MODE (no upstream controller):
//   1. Uncomment the three lines in the "Standalone mode" block below.
//   2. Comment out or remove processEffectLink(now).
//   3. The switches and USB serial will then drive all outputs directly.
// =============================================================================
void loop() {
  const uint32_t now = millis();

  // --- Effect-Link mode (active) -------------------------------------------
  // processEffectLink(now);   // Read and parse Serial1 EffectCommand packets
  // updateEffects(now);        // Apply the current EffectCommand to all hardware

  // --- Standalone mode (uncomment to enable; comment out effect-link above) --
  handleUsbCommands();
  handleSwitches(now);
  EffectCommand cmd = profileToEffectCommand(buildActiveProfile());
  updateEffects(now, cmd);
}
