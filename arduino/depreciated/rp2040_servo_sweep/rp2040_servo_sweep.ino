#include <Servo.h>

namespace {

constexpr uint8_t kServoPin = 10;
constexpr int kMinAngle = 10;
constexpr int kMaxAngle = 35;
constexpr int kStepDelayMs = 10;

Servo servo;

}  // namespace

void setup() {
  servo.attach(kServoPin);
  servo.write(kMinAngle);
}

void loop() {
  for (int angle = kMinAngle; angle <= kMaxAngle; ++angle) {
    servo.write(angle);
    delay(kStepDelayMs);
  }

  for (int angle = kMaxAngle; angle >= kMinAngle; --angle) {
    servo.write(angle);
    delay(kStepDelayMs);
  }
}
