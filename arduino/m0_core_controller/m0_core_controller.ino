#include <Adafruit_NeoPXL8.h>

#include <OrcinyCommon.h>

#include "DeviceConfig.h"

using orciny::CoreFrame;

Adafruit_NeoPXL8 leds(device_config::kPixelsPerStrip,
                      device_config::kCorePins,
                      device_config::kColorOrder);

CoreFrame currentFrame = orciny::defaultFrame();
String coreCommandBuffer;
uint32_t lastRenderMs = 0;

uint16_t pixelIndex(uint8_t strip, uint16_t pixel) {
  return static_cast<uint16_t>(strip) * device_config::kPixelsPerStrip + pixel;
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

uint32_t colorForPixel(uint32_t now, uint8_t strip, uint16_t pixel) {
  switch (currentFrame.mode) {
    case orciny::CORE_MODE_OFF:
      return 0;

    case orciny::CORE_MODE_EMBER: {
      const uint8_t flicker = wave8(now + strip * 17U, currentFrame.speed, pixel * 9U);
      const uint8_t level = map(flicker, 0, 255, 16, currentFrame.brightness);
      return scaledColor(currentFrame.red, currentFrame.green, currentFrame.blue, level);
    }

    case orciny::CORE_MODE_PULSE: {
      const uint8_t pulse = wave8(now, currentFrame.speed, strip * 32U);
      const uint8_t rim = map(abs(static_cast<int>(pixel) - (device_config::kPixelsPerStrip / 2)),
                              0,
                              device_config::kPixelsPerStrip / 2,
                              255,
                              80);
      const uint8_t level = static_cast<uint16_t>(pulse) * rim / 255U;
      return scaledColor(currentFrame.red, currentFrame.green, currentFrame.blue, level);
    }

    case orciny::CORE_MODE_BEAM: {
      const uint8_t front = (now / max<uint8_t>(1, 30 - (currentFrame.speed / 12))) % device_config::kPixelsPerStrip;
      const uint8_t distance = abs(static_cast<int>(pixel) - static_cast<int>(front));
      const uint8_t level = distance > 10 ? 24 : map(distance, 0, 10, currentFrame.brightness, 24);
      return scaledColor(currentFrame.red, currentFrame.green, currentFrame.blue, level);
    }

    case orciny::CORE_MODE_CLAW: {
      const uint16_t chase = (now / max<uint8_t>(1, 36 - (currentFrame.speed / 10)) + strip * 7U) % device_config::kPixelsPerStrip;
      const uint8_t distance = abs(static_cast<int>(pixel) - static_cast<int>(chase));
      const uint8_t level = distance > 5 ? 8 : map(distance, 0, 5, currentFrame.brightness, 8);
      return scaledColor(currentFrame.red, currentFrame.green, currentFrame.blue, level);
    }

    case orciny::CORE_MODE_SHOW: {
      const uint8_t hue = static_cast<uint8_t>((now / max<uint8_t>(1, 10 - (currentFrame.speed / 32))) + pixel * 5U + strip * 16U);
      return colorWheel(hue);
    }

    default:
      return 0;
  }
}

void renderCore(uint32_t now) {
  leds.setBrightness(currentFrame.brightness);
  for (uint8_t strip = 0; strip < device_config::kActiveCoreStrips; ++strip) {
    for (uint16_t pixel = 0; pixel < device_config::kPixelsPerStrip; ++pixel) {
      leds.setPixelColor(pixelIndex(strip, pixel), colorForPixel(now, strip, pixel));
    }
  }

  for (uint8_t strip = device_config::kActiveCoreStrips; strip < 8; ++strip) {
    for (uint16_t pixel = 0; pixel < device_config::kPixelsPerStrip; ++pixel) {
      leds.setPixelColor(pixelIndex(strip, pixel), 0);
    }
  }

  leds.show();
}

void readCoreFrames() {
  while (Serial1.available() > 0) {
    const char incoming = static_cast<char>(Serial1.read());
    if (incoming == '\r') {
      continue;
    }
    if (incoming != '\n') {
      coreCommandBuffer += incoming;
      continue;
    }

    CoreFrame candidate = currentFrame;
    if (orciny::readCoreFrame(coreCommandBuffer, candidate)) {
      currentFrame = candidate;
    }
    coreCommandBuffer = "";
  }
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
}

void loop() {
  readCoreFrames();

  const uint32_t now = millis();
  if (now - lastRenderMs >= 16) {
    renderCore(now);
    lastRenderMs = now;
  }
}