#include <Adafruit_MotorShield.h>
#include <Wire.h>

#include <OrcinyCommon.h>

#include "DeviceConfig.h"

using orciny::CoreFrame;
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
  void begin(Adafruit_DCMotor *pumpMotor, uint8_t filamentPin) {
    pumpMotor_ = pumpMotor;
    filamentPin_ = filamentPin;
    pinMode(filamentPin_, OUTPUT);
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
    pumpMotor_->setSpeed(map(wave, 40, 255, 120, 255));
    pumpMotor_->run(FORWARD);
    active_ = true;
  }

  void stop() {
    if (!active_) {
      analogWrite(filamentPin_, 0);
      pumpMotor_->run(RELEASE);
      return;
    }

    analogWrite(filamentPin_, 0);
    pumpMotor_->run(RELEASE);
    active_ = false;
  }

 private:
  Adafruit_DCMotor *pumpMotor_ = nullptr;
  uint8_t filamentPin_ = 0;
  bool active_ = false;
};

class BeamEffect {
 public:
  void begin(uint8_t redPin, uint8_t greenPin, uint8_t bluePin, uint8_t peltierPin) {
    redPin_ = redPin;
    greenPin_ = greenPin;
    bluePin_ = bluePin;
    peltierPin_ = peltierPin;
    pinMode(redPin_, OUTPUT);
    pinMode(greenPin_, OUTPUT);
    pinMode(bluePin_, OUTPUT);
    pinMode(peltierPin_, OUTPUT);
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
    digitalWrite(peltierPin_, HIGH);
    active_ = true;
  }

  void stop() {
    analogWrite(redPin_, 0);
    analogWrite(greenPin_, 0);
    analogWrite(bluePin_, 0);
    digitalWrite(peltierPin_, LOW);
    active_ = false;
  }

 private:
  uint8_t redPin_ = 0;
  uint8_t greenPin_ = 0;
  uint8_t bluePin_ = 0;
  uint8_t peltierPin_ = 0;
  bool active_ = false;
};

class ClawEffect {
 public:
  void begin(Adafruit_StepperMotor *stepperMotor, uint16_t travelMicrosteps) {
    stepperMotor_ = stepperMotor;
    travelMicrosteps_ = max<uint16_t>(1, travelMicrosteps);
    stepperMotor_->setSpeed(30);
  }

  void update(uint32_t now, bool enabled) {
    if (!enabled) {
      if (active_) {
        stepperMotor_->release();
      }
      active_ = false;
      return;
    }

    if (now < nextStepMs_) {
      return;
    }

    stepperMotor_->onestep(direction_ > 0 ? FORWARD : BACKWARD, MICROSTEP);
    position_ += direction_;
    if (position_ >= travelMicrosteps_) {
      position_ = travelMicrosteps_;
      direction_ = -1;
    } else if (position_ <= 0) {
      position_ = 0;
      direction_ = 1;
    }

    nextStepMs_ = now + device_config::kClawStepIntervalMs;
    active_ = true;
  }

 private:
  Adafruit_StepperMotor *stepperMotor_ = nullptr;
  uint16_t travelMicrosteps_ = 0;
  int32_t position_ = 0;
  int8_t direction_ = 1;
  bool active_ = false;
  uint32_t nextStepMs_ = 0;
};

Adafruit_MotorShield motorWing;
Adafruit_DCMotor *pumpMotor = nullptr;
Adafruit_StepperMotor *clawStepper = nullptr;

SparkChannel sparks[device_config::kSparkCount];
PulseEffect pulseEffect;
BeamEffect beamEffect;
ClawEffect clawEffect;

MomentarySwitch powerSwitch;
MomentarySwitch previousSwitch;
MomentarySwitch nextSwitch;

SequenceId currentSequence = SEQUENCE_1;
bool outputEnabled = false;
String usbCommandBuffer;
uint32_t lastCoreFrameMs = 0;
uint32_t resetChordStartMs = 0;
bool resetChordTriggered = false;
bool suppressPowerRelease = false;
bool suppressNextRelease = false;

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

void updateEffects(uint32_t now) {
  const SceneProfile profile = buildActiveProfile();
  for (uint8_t i = 0; i < device_config::kSparkCount; ++i) {
    sparks[i].update(now, profile.sparksEnabled, profile.sparksIntensity);
  }
  pulseEffect.update(now, profile.pulseEnabled);
  beamEffect.update(now, profile.beamEnabled);
  clawEffect.update(now, profile.clawEnabled);

  if (now - lastCoreFrameMs >= device_config::kCoreLinkPeriodMs) {
    orciny::writeCoreFrame(Serial1, profile.coreFrame);
    lastCoreFrameMs = now;
  }
}

void setup() {
  Serial.begin(device_config::kUsbBaudRate);
  Serial1.begin(device_config::kCoreLinkBaudRate);
  randomSeed(analogRead(A0));

  motorWing.begin();
  pumpMotor = motorWing.getMotor(device_config::kPumpMotorPort);
  clawStepper = motorWing.getStepper(device_config::kClawStepsPerRevolution,
                                     device_config::kClawStepperPort);

  for (uint8_t i = 0; i < device_config::kSparkCount; ++i) {
    sparks[i].begin(device_config::kSparkPins[i]);
  }

  pulseEffect.begin(pumpMotor, device_config::kPulseFilamentPin);
  beamEffect.begin(device_config::kBeamRedPin,
                   device_config::kBeamGreenPin,
                   device_config::kBeamBluePin,
                   device_config::kPeltierEnablePin);
  clawEffect.begin(clawStepper, device_config::kClawTravelMicrosteps);
  powerSwitch.begin(device_config::kPowerSwitchPin);
  previousSwitch.begin(device_config::kPreviousSwitchPin);
  nextSwitch.begin(device_config::kNextSwitchPin);

  delay(250);
  printHelp();
  printSequenceStatus();
}

void loop() {
  const uint32_t now = millis();
  handleUsbCommands();
  handleSwitches(now);
  updateEffects(now);
}