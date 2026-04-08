#include <Adafruit_NeoPixel.h>

#include <OrcinyCommon.h>

#include "DeviceConfig.h"

using orciny::CoreFrame;
using orciny::CoreMode;
using orciny::EffectCommand;

enum SequenceId : uint8_t {
  SEQUENCE_1 = 0,
  SEQUENCE_2,
  SEQUENCE_3,
  SEQUENCE_COUNT,
};

struct SceneProfile {
  CoreFrame coreFrame;
  EffectCommand effectCommand;
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
  bool wasReleased() const { return releasedEvent_; }

 private:
  uint8_t pin_ = 0;
  bool stablePressed_ = false;
  bool rawPressed_ = false;
  bool pressedEvent_ = false;
  bool releasedEvent_ = false;
  uint32_t lastChangeMs_ = 0;
};

Adafruit_NeoPixel leds(device_config::kTotalPixels,
                       device_config::kCoreDataPin,
                       device_config::kColorOrder);

CoreFrame currentFrame = orciny::defaultFrame();
EffectCommand currentEffectCommand = orciny::defaultEffectCommand();
SequenceId currentSequence = SEQUENCE_1;
bool outputEnabled = false;
String usbCommandBuffer;

MomentarySwitch powerSwitch;
MomentarySwitch previousSwitch;
MomentarySwitch nextSwitch;

uint32_t resetChordStartMs = 0;
bool resetChordTriggered = false;
bool suppressPowerRelease = false;
bool suppressNextRelease = false;
uint32_t lastRenderMs = 0;
uint32_t lastEffectLinkMs = 0;

uint16_t pixelIndex(uint16_t pixel) {
  // Single strand, direct pixel index
  return pixel;
}

uint8_t wave8(uint32_t now, uint16_t rate, uint16_t offset) {
  const uint32_t phase = ((now * rate) / 32U + offset) & 0x1FF;
  if (phase < 256) {
    return static_cast<uint8_t>(phase);
  }
  return static_cast<uint8_t>(511 - phase);
}

uint32_t scaledColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t scale) {
  const uint16_t r = static_cast<uint16_t>(red) * scale / 255U;
  const uint16_t g = static_cast<uint16_t>(green) * scale / 255U;
  const uint16_t b = static_cast<uint16_t>(blue) * scale / 255U;
  return leds.Color(r, g, b);
}

uint32_t colorWheel(uint8_t position) {
  position = 255 - position;
  if (position < 85) {
    return leds.Color(255 - position * 3, 0, position * 3);
  }
  if (position < 170) {
    position -= 85;
    return leds.Color(0, position * 3, 255 - position * 3);
  }
  position -= 170;
  return leds.Color(position * 3, 255 - position * 3, 0);
}

uint32_t colorForPixel(uint32_t now, uint16_t pixel) {
  // For single-strand mode, use pixel position to create pseudo-strip variation
  const uint8_t pseudoStrip = pixel / 8;  // Divide 30 pixels into 4 zones (~8 pixels each)
  
  switch (currentFrame.mode) {
    case orciny::CORE_MODE_OFF:
      return 0;

    case orciny::CORE_MODE_EMBER: {
      const uint8_t flicker = wave8(now + pseudoStrip * 17U, currentFrame.speed, pixel * 9U);
      const uint8_t level = map(flicker, 0, 255, 16, currentFrame.brightness);
      return scaledColor(currentFrame.red, currentFrame.green, currentFrame.blue, level);
    }

    case orciny::CORE_MODE_PULSE: {
      const uint8_t pulse = wave8(now, currentFrame.speed, pseudoStrip * 32U);
      const uint8_t rim = map(abs(static_cast<int>(pixel) - 15), 0, 15, 255, 80);
      const uint8_t level = static_cast<uint16_t>(pulse) * rim / 255U;
      return scaledColor(currentFrame.red, currentFrame.green, currentFrame.blue, level);
    }

    case orciny::CORE_MODE_BEAM: {
      const uint8_t front = (now / max<uint8_t>(1, 30 - (currentFrame.speed / 12))) % 30;
      const uint8_t distance = abs(static_cast<int>(pixel) - static_cast<int>(front));
      const uint8_t level = distance > 10 ? 24 : map(distance, 0, 10, currentFrame.brightness, 24);
      return scaledColor(currentFrame.red, currentFrame.green, currentFrame.blue, level);
    }

    case orciny::CORE_MODE_CLAW: {
      const uint16_t chase = (now / max<uint8_t>(1, 36 - (currentFrame.speed / 10)) + pseudoStrip * 7U) % 30;
      const uint8_t distance = abs(static_cast<int>(pixel) - static_cast<int>(chase));
      const uint8_t level = distance > 5 ? 8 : map(distance, 0, 5, currentFrame.brightness, 8);
      return scaledColor(currentFrame.red, currentFrame.green, currentFrame.blue, level);
    }

    case orciny::CORE_MODE_SHOW: {
      const uint8_t hue = static_cast<uint8_t>((now / max<uint8_t>(1, 10 - (currentFrame.speed / 32))) + pixel * 5U + pseudoStrip * 16U);
      return colorWheel(hue);
    }

    default:
      return 0;
  }
}

void renderCore(uint32_t now) {
  leds.setBrightness(currentFrame.brightness);
  for (uint16_t pixel = 0; pixel < device_config::kTotalPixels; ++pixel) {
    leds.setPixelColor(pixelIndex(pixel), colorForPixel(now, pixel));
  }
  leds.show();
}

void printHelp() {
  Serial.println(F("Commands: help, on, off, toggle, prev, next, seq1, seq2, seq3, reset"));
}

void printSequenceStatus() {
  Serial.print(F("Power -> "));
  Serial.print(outputEnabled ? F("ON") : F("OFF"));
  Serial.print(F(", Sequence -> "));
  Serial.println(static_cast<uint8_t>(currentSequence) + 1);
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

SceneProfile buildActiveProfile() {
  SceneProfile profile = {orciny::defaultFrame(), orciny::defaultEffectCommand()};

  if (!outputEnabled) {
    profile.coreFrame.mode = orciny::CORE_MODE_OFF;
    profile.coreFrame.brightness = 0;
    return profile;
  }

  profile.effectCommand.outputEnabled = true;
  switch (currentSequence) {
    case SEQUENCE_1:
      profile.effectCommand.sparksEnabled = true;
      profile.effectCommand.sparksIntensity = 220;
      profile.coreFrame.mode = orciny::CORE_MODE_EMBER;
      profile.coreFrame.brightness = 56;
      profile.coreFrame.speed = 90;
      profile.coreFrame.red = 255;
      profile.coreFrame.green = 110;
      profile.coreFrame.blue = 24;
      return profile;

    case SEQUENCE_2:
      profile.effectCommand.beamEnabled = true;
      profile.coreFrame.mode = orciny::CORE_MODE_PULSE;
      profile.coreFrame.brightness = 96;
      profile.coreFrame.speed = 110;
      profile.coreFrame.red = 255;
      profile.coreFrame.green = 48;
      profile.coreFrame.blue = 0;
      return profile;

    case SEQUENCE_3:
      profile.effectCommand.sparksEnabled = true;
      profile.effectCommand.sparksIntensity = 255;
      profile.effectCommand.beamEnabled = true;
      profile.effectCommand.clawEnabled = true;
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

void writeEffectCommand(uint32_t now, const EffectCommand &command) {
  if ((now - lastEffectLinkMs) < device_config::kEffectLinkPeriodMs) {
    return;
  }

  orciny::writeEffectCommand(Serial1, command);
  lastEffectLinkMs = now;
}

void setup() {
  Serial.begin(device_config::kUsbBaudRate);
  Serial1.begin(device_config::kCoreLinkBaudRate);

  if (!leds.begin()) {
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) {
      digitalWrite(LED_BUILTIN, (millis() / 250) & 1);
    }
  }

  leds.show();

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

  const SceneProfile profile = buildActiveProfile();
  currentFrame = profile.coreFrame;
  currentEffectCommand = profile.effectCommand;
  writeEffectCommand(now, currentEffectCommand);

  if (now - lastRenderMs >= 16) {
    renderCore(now);
    lastRenderMs = now;
  }
}