#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>

#include <OrcinyCommon.h>

#include "DeviceConfig.h"

using orciny::CoreFrame;
using orciny::EffectCommand;
using orciny::CoreMode;

enum SequenceId : uint8_t {
  SEQUENCE_1 = 0,
  SEQUENCE_2,
  SEQUENCE_3,
  SEQUENCE_COUNT,
};

struct SceneProfile {
  bool sparksEnabled;
  uint8_t sparksIntensity;
  bool pulseEnabled;
  bool beamEnabled;
  bool clawEnabled;
  CoreFrame coreFrame;
};

class MomentarySwitch {
 public:
  void begin(uint8_t pin) {
    pin_ = pin;
    pinMode(pin_, INPUT_PULLUP);
    const bool initialPressed = (digitalRead(pin_) == LOW);
    stablePressed_ = initialPressed;
    rawPressed_ = initialPressed;
    lastChangeMs_ = millis();
  }

  void update(uint32_t now) {
    pressedEvent_ = false;
    releasedEvent_ = false;

    const bool sampledPressed = (digitalRead(pin_) == LOW);
    if (sampledPressed != rawPressed_) {
      rawPressed_ = sampledPressed;
      lastChangeMs_ = now;
    }

    if ((now - lastChangeMs_) < device_config::kSwitchDebounceMs) {
      return;
    }

    if (stablePressed_ == rawPressed_) {
      return;
    }

    stablePressed_ = rawPressed_;
    if (stablePressed_) {
      pressedEvent_ = true;
    } else {
      releasedEvent_ = true;
    }
  }

  bool isPressed() const { return stablePressed_; }
  bool wasPressed() const { return pressedEvent_; }
  bool wasReleased() const { return releasedEvent_; }

 private:
  uint8_t pin_ = 0;
  bool stablePressed_ = false;
  bool rawPressed_ = false;
  bool pressedEvent_ = false;
  bool releasedEvent_ = false;
  uint32_t lastChangeMs_ = 0;
};

class SparkChannel {
 public:
  void begin(uint8_t pin) {
    pin_ = pin;
    pinMode(pin_, OUTPUT);
    analogWrite(pin_, 0);
    nextEventMs_ = millis() + random(30, 180);
  }

  void update(uint32_t now, bool enabled, uint8_t intensity) {
    if (!enabled) {
      analogWrite(pin_, 0);
      active_ = false;
      nextEventMs_ = now + random(80, 200);
      return;
    }

    if (active_) {
      if (now >= flashUntilMs_) {
        active_ = false;
        analogWrite(pin_, 0);
        nextEventMs_ = now + random(25, 140);
      }
      return;
    }

    if (now < nextEventMs_) {
      return;
    }

    const int minimum = max(32, intensity / 3);
    const int peak = max(minimum + 1, static_cast<int>(intensity) + 1);
    analogWrite(pin_, random(minimum, peak));
    flashUntilMs_ = now + random(15, 70);
    active_ = true;
  }

 private:
  uint8_t pin_ = 0;
  bool active_ = false;
  uint32_t nextEventMs_ = 0;
  uint32_t flashUntilMs_ = 0;
};

class PulseEffect {
 public:
  void begin(uint8_t filamentPin) {
    filamentPin_ = filamentPin;
    pinMode(filamentPin_, OUTPUT);
    // pinMode(device_config::kPumpControlPin, OUTPUT);
    // digitalWrite(device_config::kPumpControlPin, LOW);
    analogWrite(filamentPin_, 0);
  }

  void update(uint32_t now, bool enabled) {
    if (!enabled) {
      stop();
      return;
    }

    const uint16_t phase = now % 1600;
    uint8_t wave = 0;
    if (phase < 800) {
      wave = map(phase, 0, 800, 40, 255);
    } else {
      wave = map(phase, 800, 1600, 255, 40);
    }

    analogWrite(filamentPin_, wave);
    // Pump may be removed in the updated hardware, keep action disabled.
    // digitalWrite(device_config::kPumpControlPin, wave >= 96 ? HIGH : LOW);
    active_ = true;
  }

  void stop() {
    if (!active_) {
      analogWrite(filamentPin_, 0);
      // digitalWrite(device_config::kPumpControlPin, LOW);
      return;
    }

    analogWrite(filamentPin_, 0);
    // digitalWrite(device_config::kPumpControlPin, LOW);
    active_ = false;
  }

 private:
  uint8_t filamentPin_ = 0;
  bool active_ = false;
};

class BeamEffect {
 public:
  void begin(uint8_t led1Pin, uint8_t led2Pin, uint8_t led3Pin) {
    redPin_ = led1Pin;
    greenPin_ = led2Pin;
    bluePin_ = led3Pin;
    pinMode(redPin_, OUTPUT);
    pinMode(greenPin_, OUTPUT);
    pinMode(bluePin_, OUTPUT);
    stop();
  }

  void update(uint32_t now, bool enabled) {
    if (!enabled) {
      stop();
      return;
    }

    const uint16_t phase = now % 2400;
    const uint8_t swell = phase < 1200 ? map(phase, 0, 1200, 100, 255)
                                       : map(phase, 1200, 2400, 255, 100);
    analogWrite(redPin_, swell / 5);
    analogWrite(greenPin_, swell);
    analogWrite(bluePin_, min<uint16_t>(255, swell + 30));
    active_ = true;
  }

  void stop() {
    analogWrite(redPin_, 0);
    analogWrite(greenPin_, 0);
    analogWrite(bluePin_, 0);
    active_ = false;
  }

 private:
  uint8_t redPin_ = 0;
  uint8_t greenPin_ = 0;
  uint8_t bluePin_ = 0;
  bool active_ = false;
};

class ClawEffect {
 public:
  void begin(Adafruit_PWMServoDriver &driver,
             uint8_t servoChannelA,
             uint8_t servoChannelB,
             uint8_t minAngleA,
             uint8_t maxAngleA,
             uint8_t minAngleB,
             uint8_t maxAngleB) {
    driver_ = &driver;
    servoChannelA_ = servoChannelA;
    servoChannelB_ = servoChannelB;
    minAngleA_ = min(minAngleA, maxAngleA);
    maxAngleA_ = max(minAngleA, maxAngleA);
    minAngleB_ = min(minAngleB, maxAngleB);
    maxAngleB_ = max(minAngleB, maxAngleB);
    angle_ = minAngleA_;
    direction_ = 1;
    writeServos(static_cast<uint8_t>(angle_));
  }

  void update(uint32_t now, bool enabled) {
    if (!enabled) {
      stop();
      return;
    }

    if (now < nextStepMs_) {
      return;
    }

    angle_ += direction_;
    if (angle_ >= maxAngleA_) {
      angle_ = maxAngleA_;
      direction_ = -1;
    } else if (angle_ <= minAngleA_) {
      angle_ = minAngleA_;
      direction_ = 1;
    }

    writeServos(static_cast<uint8_t>(angle_));
    nextStepMs_ = now + device_config::kClawStepIntervalMs;
    active_ = true;
  }

  void stop() {
    if (!active_) {
      return;
    }
    writeServos(minAngleA_);
    active_ = false;
  }

 private:
  uint16_t angleToPulse(uint8_t angle) const {
    return map(angle,
               0,
               180,
               device_config::kServoMinPulse,
               device_config::kServoMaxPulse);
  }

  uint8_t mapAngleForB(uint8_t angleA) const {
    return map(angleA, minAngleA_, maxAngleA_, maxAngleB_, minAngleB_);
  }

  void writeServos(uint8_t angleA) {
    if (driver_ == nullptr) {
      return;
    }

    const uint8_t clampedA = constrain(angleA, minAngleA_, maxAngleA_);
    const uint8_t angleB = constrain(mapAngleForB(clampedA), minAngleB_, maxAngleB_);
    driver_->setPWM(servoChannelA_, 0, angleToPulse(clampedA));
    driver_->setPWM(servoChannelB_, 0, angleToPulse(angleB));
  }

  Adafruit_PWMServoDriver *driver_ = nullptr;
  uint8_t servoChannelA_ = 0;
  uint8_t servoChannelB_ = 1;
  int16_t angle_ = 0;
  uint8_t minAngleA_ = 0;
  uint8_t maxAngleA_ = 180;
  uint8_t minAngleB_ = 0;
  uint8_t maxAngleB_ = 180;
  int8_t direction_ = 1;
  bool active_ = false;
  uint32_t nextStepMs_ = 0;
};

SparkChannel sparks[device_config::kSparkCount];
PulseEffect pulseEffect;
BeamEffect beamEffect;
ClawEffect clawEffect;
Adafruit_PWMServoDriver servoDriver(device_config::kServoDriverI2cAddress);

MomentarySwitch powerSwitch;
MomentarySwitch previousSwitch;
MomentarySwitch nextSwitch;

SequenceId currentSequence = SEQUENCE_1;
bool outputEnabled = false;
String usbCommandBuffer;
String effectLinkBuffer;
uint32_t lastCoreFrameMs = 0;
uint32_t lastEffectCommandMs = 0;
uint32_t resetChordStartMs = 0;
bool resetChordTriggered = false;
bool suppressPowerRelease = false;
bool suppressNextRelease = false;
EffectCommand currentEffectCommand = orciny::defaultEffectCommand();

void printHelp() {
  Serial.println(F("Commands: help, on, off, toggle, prev, next, seq1, seq2, seq3, reset"));
}

void printSequenceStatus() {
  Serial.print(F("Power -> "));
  Serial.print(outputEnabled ? F("ON") : F("OFF"));
  Serial.print(F(", Sequence -> "));
  Serial.println(static_cast<uint8_t>(currentSequence) + 1);
}

SceneProfile buildActiveProfile() {
  SceneProfile profile = {};
  profile.coreFrame = orciny::defaultFrame();

  if (!outputEnabled) {
    profile.coreFrame.mode = orciny::CORE_MODE_OFF;
    profile.coreFrame.brightness = 0;
    return profile;
  }

  switch (currentSequence) {
    case SEQUENCE_1:
      profile.sparksEnabled = true;
      profile.sparksIntensity = 220;
      profile.coreFrame.mode = orciny::CORE_MODE_EMBER;
      profile.coreFrame.brightness = 56;
      profile.coreFrame.speed = 90;
      profile.coreFrame.red = 255;
      profile.coreFrame.green = 110;
      profile.coreFrame.blue = 24;
      return profile;

    case SEQUENCE_2:
      profile.pulseEnabled = true;
      profile.beamEnabled = true;
      profile.coreFrame.mode = orciny::CORE_MODE_PULSE;
      profile.coreFrame.brightness = 96;
      profile.coreFrame.speed = 110;
      profile.coreFrame.red = 255;
      profile.coreFrame.green = 48;
      profile.coreFrame.blue = 0;
      return profile;

    case SEQUENCE_3:
      profile.sparksEnabled = true;
      profile.sparksIntensity = 255;
      profile.pulseEnabled = true;
      profile.beamEnabled = true;
      profile.clawEnabled = true;
      profile.coreFrame.mode = orciny::CORE_MODE_SHOW;
      profile.coreFrame.brightness = 180;
      profile.coreFrame.speed = 140;
      profile.coreFrame.red = 60;
      profile.coreFrame.green = 220;
      profile.coreFrame.blue = 255;
      return profile;

    default:
      profile.coreFrame.mode = orciny::CORE_MODE_OFF;
      profile.coreFrame.brightness = 0;
      return profile;
  }
}

EffectCommand profileToEffectCommand(const SceneProfile &profile) {
  EffectCommand command = orciny::defaultEffectCommand();
  command.outputEnabled = outputEnabled;
  command.sparksEnabled = profile.sparksEnabled;
  command.sparksIntensity = profile.sparksIntensity;
  command.pulseEnabled = profile.pulseEnabled;
  command.beamEnabled = profile.beamEnabled;
  command.clawEnabled = profile.clawEnabled;
  return command;
}

SequenceId previousSequenceValue(SequenceId sequence) {
  return sequence == SEQUENCE_1 ? SEQUENCE_3
                                : static_cast<SequenceId>(sequence - 1);
}

SequenceId nextSequenceValue(SequenceId sequence) {
  return sequence == SEQUENCE_3 ? SEQUENCE_1
                                : static_cast<SequenceId>(sequence + 1);
}

void setSequence(SequenceId sequence) {
  currentSequence = sequence;
  printSequenceStatus();
}

void resetToBeginning() {
  currentSequence = SEQUENCE_1;
  Serial.println(F("Reset -> sequence 1"));
  printSequenceStatus();
}

void handleUsbCommands() {
  while (Serial.available() > 0) {
    const char incoming = static_cast<char>(Serial.read());
    if (incoming == '\r') {
      continue;
    }
    if (incoming != '\n') {
      usbCommandBuffer += incoming;
      continue;
    }

    String command = usbCommandBuffer;
    usbCommandBuffer = "";
    command.trim();
    command.toLowerCase();

    if (command.length() == 0) {
      continue;
    }

    if (command == F("help")) {
      printHelp();
    } else if (command == F("on")) {
      outputEnabled = true;
      printSequenceStatus();
    } else if (command == F("off")) {
      outputEnabled = false;
      printSequenceStatus();
    } else if (command == F("toggle")) {
      outputEnabled = !outputEnabled;
      printSequenceStatus();
    } else if (command == F("prev")) {
      setSequence(previousSequenceValue(currentSequence));
    } else if (command == F("next")) {
      setSequence(nextSequenceValue(currentSequence));
    } else if (command == F("seq1")) {
      setSequence(SEQUENCE_1);
    } else if (command == F("seq2")) {
      setSequence(SEQUENCE_2);
    } else if (command == F("seq3")) {
      setSequence(SEQUENCE_3);
    } else if (command == F("reset")) {
      resetToBeginning();
    }
  }
}

void handleSwitches(uint32_t now) {
  powerSwitch.update(now);
  previousSwitch.update(now);
  nextSwitch.update(now);

  const bool resetChordDown = powerSwitch.isPressed() && nextSwitch.isPressed();

  if (resetChordDown) {
    if (resetChordStartMs == 0) {
      resetChordStartMs = now;
      suppressPowerRelease = true;
      suppressNextRelease = true;
    }
    if (!resetChordTriggered && (now - resetChordStartMs >= device_config::kResetHoldMs)) {
      resetChordTriggered = true;
      resetToBeginning();
    }
  } else if (!powerSwitch.isPressed() && !nextSwitch.isPressed()) {
    resetChordStartMs = 0;
    resetChordTriggered = false;
    suppressPowerRelease = false;
    suppressNextRelease = false;
  }

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

void processEffectLink(uint32_t now) {
  while (Serial1.available() > 0) {
    const char incoming = static_cast<char>(Serial1.read());
    if (incoming == '\r') {
      continue;
    }

    if (incoming != '\n') {
      effectLinkBuffer += incoming;
      continue;
    }

    EffectCommand command = orciny::defaultEffectCommand();
    if (orciny::readEffectCommand(effectLinkBuffer, command)) {
      currentEffectCommand = command;
      lastEffectCommandMs = now;
    }
    effectLinkBuffer = "";
  }
}

EffectCommand resolveEffectCommand(uint32_t now) {
  const bool commandIsFresh = (lastEffectCommandMs > 0) &&
                              ((now - lastEffectCommandMs) <= device_config::kEffectCommandTimeoutMs);
  if (commandIsFresh) {
    return currentEffectCommand;
  }
  return orciny::defaultEffectCommand();
}

void updateEffects(uint32_t now) {
  const EffectCommand command = resolveEffectCommand(now);

  const bool sparksEnabled = command.outputEnabled && command.sparksEnabled;
  const bool pulseEnabled = command.outputEnabled && command.pulseEnabled;
  const bool beamEnabled = command.outputEnabled && command.beamEnabled;
  const bool clawEnabled = command.outputEnabled && command.clawEnabled;

  for (uint8_t i = 0; i < device_config::kSparkCount; ++i) {
    sparks[i].update(now, sparksEnabled, command.sparksIntensity);
  }
  pulseEffect.update(now, pulseEnabled);
  beamEffect.update(now, beamEnabled);
  clawEffect.update(now, clawEnabled);
}

void setup() {
  Serial.begin(device_config::kUsbBaudRate);
  Serial1.begin(device_config::kCoreLinkBaudRate);
  Wire.begin();
  servoDriver.begin();
  servoDriver.setPWMFreq(50);
  randomSeed(analogRead(A0));

  for (uint8_t i = 0; i < device_config::kSparkCount; ++i) {
    sparks[i].begin(device_config::kSparkPins[i]);
  }

  pulseEffect.begin(device_config::kPulseFilamentPin);
  beamEffect.begin(device_config::kPropMakerLed1Pin,
                   device_config::kPropMakerLed2Pin,
                   device_config::kPropMakerLed3Pin);
  clawEffect.begin(servoDriver,
                   device_config::kServoChannelA,
                   device_config::kServoChannelB,
                   device_config::kServoAMinAngle,
                   device_config::kServoAMaxAngle,
                   device_config::kServoBMinAngle,
                   device_config::kServoBMaxAngle);
  currentEffectCommand = orciny::defaultEffectCommand();
}

void loop() {
  const uint32_t now = millis();
  processEffectLink(now);
  updateEffects(now);
}