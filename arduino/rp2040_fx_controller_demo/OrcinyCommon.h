#pragma once

#include <Arduino.h>

namespace orciny {

enum CoreMode : uint8_t {
  CORE_MODE_OFF = 0,
  CORE_MODE_EMBER = 1,
  CORE_MODE_PULSE = 2,
  CORE_MODE_BEAM = 3,
  CORE_MODE_CLAW = 4,
  CORE_MODE_SHOW = 5,
};

struct CoreFrame {
  CoreMode mode;
  uint8_t brightness;
  uint8_t speed;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

struct EffectCommand {
  bool outputEnabled;
  bool sparksEnabled;
  uint8_t sparksIntensity;
  bool pulseEnabled;
  bool beamEnabled;
  bool clawEnabled;
};

inline CoreFrame defaultFrame() {
  CoreFrame frame = {CORE_MODE_OFF, 24, 80, 255, 96, 16};
  return frame;
}

inline EffectCommand defaultEffectCommand() {
  EffectCommand command = {false, false, 0, false, false, false};
  return command;
}

inline uint8_t clampByte(int value) {
  if (value < 0) {
    return 0;
  }
  if (value > 255) {
    return 255;
  }
  return static_cast<uint8_t>(value);
}

inline void writeCoreFrame(Stream &stream, const CoreFrame &frame) {
  stream.print(F("CORE,"));
  stream.print(static_cast<uint8_t>(frame.mode));
  stream.print(',');
  stream.print(frame.brightness);
  stream.print(',');
  stream.print(frame.speed);
  stream.print(',');
  stream.print(frame.red);
  stream.print(',');
  stream.print(frame.green);
  stream.print(',');
  stream.println(frame.blue);
}

inline bool readCoreFrame(const String &line, CoreFrame &frame) {
  String trimmed = line;
  trimmed.trim();
  if (!trimmed.startsWith(F("CORE,"))) {
    return false;
  }

  int values[6] = {0};
  int start = 5;
  for (uint8_t i = 0; i < 6; ++i) {
    int comma = trimmed.indexOf(',', start);
    String token;
    if (comma >= 0) {
      token = trimmed.substring(start, comma);
      start = comma + 1;
    } else {
      token = trimmed.substring(start);
      start = trimmed.length();
    }

    if (token.length() == 0) {
      return false;
    }
    values[i] = token.toInt();
    if ((comma < 0) && (i < 5)) {
      return false;
    }
  }

  frame.mode = static_cast<CoreMode>(clampByte(values[0]));
  frame.brightness = clampByte(values[1]);
  frame.speed = clampByte(values[2]);
  frame.red = clampByte(values[3]);
  frame.green = clampByte(values[4]);
  frame.blue = clampByte(values[5]);
  return true;
}

inline void writeEffectCommand(Stream &stream, const EffectCommand &command) {
  stream.print(F("FX,"));
  stream.print(command.outputEnabled ? 1 : 0);
  stream.print(',');
  stream.print(command.sparksEnabled ? 1 : 0);
  stream.print(',');
  stream.print(command.sparksIntensity);
  stream.print(',');
  stream.print(command.pulseEnabled ? 1 : 0);
  stream.print(',');
  stream.print(command.beamEnabled ? 1 : 0);
  stream.print(',');
  stream.println(command.clawEnabled ? 1 : 0);
}

inline bool readEffectCommand(const String &line, EffectCommand &command) {
  String trimmed = line;
  trimmed.trim();
  if (!trimmed.startsWith(F("FX,"))) {
    return false;
  }

  int values[6] = {0};
  int start = 3;
  for (uint8_t i = 0; i < 6; ++i) {
    int comma = trimmed.indexOf(',', start);
    String token;
    if (comma >= 0) {
      token = trimmed.substring(start, comma);
      start = comma + 1;
    } else {
      token = trimmed.substring(start);
      start = trimmed.length();
    }

    if (token.length() == 0) {
      return false;
    }
    values[i] = token.toInt();
    if ((comma < 0) && (i < 5)) {
      return false;
    }
  }

  command.outputEnabled = values[0] > 0;
  command.sparksEnabled = values[1] > 0;
  command.sparksIntensity = clampByte(values[2]);
  command.pulseEnabled = values[3] > 0;
  command.beamEnabled = values[4] > 0;
  command.clawEnabled = values[5] > 0;
  return true;
}

}  // namespace orciny