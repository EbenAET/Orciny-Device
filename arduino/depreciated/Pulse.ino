#include <Adafruit_NeoPixel.h>

#define PIN 11
#define NUMPIXELS 166

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.setBrightness(100); // optional
  strip.show();
}

void loop() {
  // fadeTo(r, g, b, fadeTime(ms), holdTime(ms))

  fadeTo(255, 72, 0,   3500, 8000); // Orange: slow fade, short hold
  fadeTo(90, 8, 70,   2000, 4000);  // Purple: very slow fade, quick hold
  fadeTo(53, 184, 117,   3000, 3500); // Blue/green: fast fade, long hold
}

// Keeps track of current color
int currentR = 0;
int currentG = 0;
int currentB = 0;

void fadeTo(int targetR, int targetG, int targetB, int fadeTime, int holdTime) {
  int steps = 100; // smoothness (higher = smoother)
  int delayTime = fadeTime / steps;

  for (int i = 0; i <= steps; i++) {
    int r = currentR + (targetR - currentR) * i / steps;
    int g = currentG + (targetG - currentG) * i / steps;
    int b = currentB + (targetB - currentB) * i / steps;

    for (int p = 0; p < NUMPIXELS; p++) {
      strip.setPixelColor(p, strip.Color(r, g, b));
    }
    strip.show();
    delay(delayTime);
  }

  // Update current color
  currentR = targetR;
  currentG = targetG;
  currentB = targetB;

  // Hold the color
  delay(holdTime);
}