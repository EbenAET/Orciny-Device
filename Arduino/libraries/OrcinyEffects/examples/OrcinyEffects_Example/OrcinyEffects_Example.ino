// =============================================================================
// OrcinyEffects_Example.ino
// Version : V 0.2.5
// Example sketch showing how to use the OrcinyEffects library
//
// This is a simplified version of rp2040_fx_starter.ino that uses
// pre-built effect scenes from OrcinyEffects instead of custom code.
//
// For detailed setup and configuration, see rp2040_fx_starter.ino
// =============================================================================

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_NeoPixel.h>
#include <ColorPalettes.h>
#include <OrcinyEffects.h>

using namespace OrcinyEffects;

// Pin configuration (same as rp2040_fx_starter.ino)
#define PROP_MAKER_PWR_PIN   10
#define BEAM_RED_PIN         11
#define BEAM_GREEN_PIN       12
#define BEAM_BLUE_PIN        13
#define SPARK_PIN_1          18
#define SPARK_PIN_2          19
#define SPARK_PIN_3          20
#define SPARK_PIN_4          24
#define NEO_DATA_PIN         25
#define NEO_PIXEL_COUNT      166
#define NEO_COLOR_ORDER      NEO_GRBW
#define SW_POWER_PIN         27
#define SW_PREV_PIN          28
#define SW_NEXT_PIN          29

#define DEBOUNCE_MS          30
#define RESET_HOLD_MS        5000
#define SERVO_I2C_ADDR       0x40
#define SERVO_PULSE_MIN      120
#define SERVO_PULSE_MAX      600

// State enumeration
enum DeviceState : uint8_t {
  STATE_OFF = 0,
  STATE_1,     // Ember
  STATE_2,     // Cyan Pulse
  STATE_3,     // Full Show
  STATE_COUNT
};

const char* STATE_NAMES[] = { "OFF", "Ember", "Cyan Pulse", "Full Show" };

// Global objects
Adafruit_PWMServoDriver servoDriver(SERVO_I2C_ADDR);
Adafruit_NeoPixel       strip(NEO_PIXEL_COUNT, NEO_DATA_PIN, NEO_COLOR_ORDER + NEO_KHZ800);

DeviceState currentState   = STATE_1;
bool        outputEnabled  = false;

// Switch state tracking
struct SwitchState {
  uint8_t  pin;
  bool     stable;
  bool     raw;
  bool     pressed;
  bool     released;
  uint32_t lastChangeMs;
};

SwitchState swPower = { SW_POWER_PIN };
SwitchState swPrev  = { SW_PREV_PIN  };
SwitchState swNext  = { SW_NEXT_PIN  };

uint32_t resetStartMs       = 0;
bool     resetFired         = false;
bool     suppressPowerEvent = false;
bool     suppressNextEvent  = false;

// Helper function declarations
void updateSwitch(SwitchState &sw, uint32_t now);
void handleSwitches(uint32_t now);
void printState();
void setServo(uint8_t channel, uint8_t angle);
void neoPixelSetAll(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);
void neoPixelOff();
void allOutputsOff();

// =============================================================================
// STATE FUNCTIONS using OrcinyEffects scenes
// =============================================================================

void doState1() {
  Scene::Ember(millis());
}

void doState2() {
  Scene::CyanPulse(millis());
}

void doState3() {
  Scene::FullShow(millis());
}

// =============================================================================
// SETUP
// =============================================================================

void setup() {
  Serial.begin(115200);

  // Initialize OrcinyEffects hardware references
  OrcinyEffects::SPARK_PIN_1 = SPARK_PIN_1;
  OrcinyEffects::SPARK_PIN_2 = SPARK_PIN_2;
  OrcinyEffects::SPARK_PIN_3 = SPARK_PIN_3;
  OrcinyEffects::SPARK_PIN_4 = SPARK_PIN_4;
  OrcinyEffects::BEAM_RED_PIN = BEAM_RED_PIN;
  OrcinyEffects::BEAM_GREEN_PIN = BEAM_GREEN_PIN;
  OrcinyEffects::BEAM_BLUE_PIN = BEAM_BLUE_PIN;
  OrcinyEffects::strip = &strip;
  OrcinyEffects::setServo = setServo;
  OrcinyEffects::neoPixelSetAll = neoPixelSetAll;
  OrcinyEffects::allOutputsOff = allOutputsOff;

  // Prop-Maker power enable
  pinMode(PROP_MAKER_PWR_PIN, OUTPUT);
  digitalWrite(PROP_MAKER_PWR_PIN, HIGH);

  // Beam LED outputs
  pinMode(BEAM_RED_PIN,   OUTPUT);
  pinMode(BEAM_GREEN_PIN, OUTPUT);
  pinMode(BEAM_BLUE_PIN,  OUTPUT);
  analogWrite(BEAM_RED_PIN,   0);
  analogWrite(BEAM_GREEN_PIN, 0);
  analogWrite(BEAM_BLUE_PIN,  0);

  // Spark channel outputs
  pinMode(SPARK_PIN_1, OUTPUT);  analogWrite(SPARK_PIN_1, 0);
  pinMode(SPARK_PIN_2, OUTPUT);  analogWrite(SPARK_PIN_2, 0);
  pinMode(SPARK_PIN_3, OUTPUT);  analogWrite(SPARK_PIN_3, 0);
  pinMode(SPARK_PIN_4, OUTPUT);  analogWrite(SPARK_PIN_4, 0);

  // NeoPixel strip
  strip.begin();
  strip.setBrightness(80);
  strip.show();

  // Servo driver
  Wire.begin();
  servoDriver.begin();
  servoDriver.setPWMFreq(50);

  // Switches
  SwitchState *switches[] = { &swPower, &swPrev, &swNext };
  for (SwitchState *sw : switches) {
    pinMode(sw->pin, INPUT_PULLUP);
    sw->stable = sw->raw = (digitalRead(sw->pin) == LOW);
    sw->lastChangeMs = millis();
  }

  randomSeed(analogRead(A0));

  Serial.println(F("Orciny FX w/ Effects ready.  SW1=on/off  SW2=prev  SW3=next"));
  printState();
}

// =============================================================================
// LOOP
// =============================================================================

void loop() {
  const uint32_t now = millis();

  handleSwitches(now);

  if (!outputEnabled) {
    allOutputsOff();
    return;
  }

  // Dispatch to scene functions (all effect code is in OrcinyEffects)
  switch (currentState) {
    case STATE_1: doState1(); break;
    case STATE_2: doState2(); break;
    case STATE_3: doState3(); break;
    default:      allOutputsOff(); break;
  }

  strip.show();
}

// =============================================================================
// HELPER IMPLEMENTATIONS
// =============================================================================

void setServo(uint8_t channel, uint8_t angle) {
  uint16_t pulse = map(angle, 0, 180, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
  servoDriver.setPWM(channel, 0, pulse);
}

void neoPixelSetAll(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(r, g, b, w));
  }
}

void neoPixelOff() {
  strip.clear();
  strip.show();
}

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

void printState() {
  Serial.print(F("Output: "));
  Serial.print(outputEnabled ? F("ON") : F("OFF"));
  Serial.print(F("  |  State: "));
  Serial.println(STATE_NAMES[currentState]);
}

void updateSwitch(SwitchState &sw, uint32_t now) {
  sw.pressed  = false;
  sw.released = false;

  const bool sampled = (digitalRead(sw.pin) == LOW);

  if (sampled != sw.raw) {
    sw.raw          = sampled;
    sw.lastChangeMs = now;
  }

  if ((now - sw.lastChangeMs) < DEBOUNCE_MS) {
    return;
  }

  if (sw.stable == sw.raw) {
    return;
  }

  sw.stable = sw.raw;

  if (sw.stable) {
    sw.pressed  = true;
  } else {
    sw.released = true;
  }
}

void handleSwitches(uint32_t now) {
  updateSwitch(swPower, now);
  updateSwitch(swPrev,  now);
  updateSwitch(swNext,  now);

  const bool chordDown = swPower.stable && swNext.stable;

  if (chordDown) {
    if (resetStartMs == 0) {
      resetStartMs       = now;
      suppressPowerEvent = true;
      suppressNextEvent  = true;
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
    resetStartMs       = 0;
    resetFired         = false;
    suppressPowerEvent = false;
    suppressNextEvent  = false;
  }

  if (swPrev.released) {
    if (currentState <= STATE_1) {
      currentState = static_cast<DeviceState>(STATE_COUNT - 1);
    } else {
      currentState = static_cast<DeviceState>(currentState - 1);
    }
    printState();
  }

  if (swPower.released && !suppressPowerEvent) {
    outputEnabled = !outputEnabled;
    if (!outputEnabled) {
      allOutputsOff();
    }
    printState();
  }

  if (swNext.released && !suppressNextEvent) {
    currentState = static_cast<DeviceState>((currentState + 1) % STATE_COUNT);
    if (currentState == STATE_OFF) {
      currentState = STATE_1;
    }
    printState();
  }
}
