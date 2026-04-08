# Orciny Device Firmware Framework

Arduino IDE scaffold for a two-controller multi-effect device built around:

- Feather RP2040
- Feather M0 Basic Proto

The framework splits responsibilities like this:

- M0 sketch: primary scene control, switch and USB command handling, single-strand NeoPixel core rendering, and outbound effect commands over Serial1
- RP2040 sketch: effect executor for sparks, pulse filament, Prop-Maker LED channel control, and dual-servo claw motion through an 8-channel Servo FeatherWing (PCA9685)

## Folder Layout

- `arduino/rp2040_fx_controller`: main effects controller sketch for the Feather RP2040
- `arduino/m0_core_controller`: single-strand NeoPixel core controller sketch for the Feather M0
- `arduino/libraries/OrcinyCommon`: shared protocol and data structures

## Arduino Libraries

Install these in the Arduino IDE before compiling:

- Adafruit NeoPixel
- Adafruit PWM Servo Driver Library

## Wiring Assumptions

The scaffold makes a few deliberate assumptions that you should verify against your hardware:

- Main input is a 5V source split into a fused `+5V_BUS` rail and a fused `+3V_FILAMENT` buck-fed rail
- RP2040 I2C connects to an 8-channel Servo FeatherWing (PCA9685 at `0x40`) that drives two servo outputs
- RP2040 pin `GP8` can remain wired to pump-enable MOSFET gate, but pump actions are currently disabled in firmware
- M0 uses three momentary switches wired to ground with internal pull-ups enabled
- The 3-9W LED channels are driven from the Prop-Maker FeatherWing MOSFET-controlled LED outputs, commanded by RP2040 PWM control lines
- Four spark LED filaments and one pulse LED filament are each driven through suitable transistor stages and 10 ohm series current-limiting resistors
- M0 `TX` is connected to RP2040 `RX` for effect command updates, with shared ground
- M0 pin `D13` drives one WS2812/NeoPixel data line for a 30-pixel strip

This revision intentionally removes NeoPXL8, Motor FeatherWing, and Feather Doubler dependencies.

## How To Use

1. Open `arduino/rp2040_fx_controller/rp2040_fx_controller.ino` in Arduino IDE and select the Feather RP2040 target.
2. Open `arduino/m0_core_controller/m0_core_controller.ino` in another Arduino IDE window and select the Feather M0 target.
3. Adjust both `DeviceConfig.h` files to match your actual wiring and strip lengths.
4. Upload the M0 sketch first, then the RP2040 sketch.
5. Open the M0 USB serial monitor at `115200` baud for operator commands and status.

## Switch Inputs

The M0 sketch expects three momentary switches connected from GPIO to GND and uses `INPUT_PULLUP`.

- Switch 1 on pin `2`: toggle the current sequence on or off
- Switch 2 on pin `3`: move to the previous sequence, wrapping `1 -> 3`
- Switch 3 on pin `4`: move to the next sequence, wrapping `3 -> 1`
- Hold switches 1 and 3 together for 5 seconds: reset back to sequence 1

Turning the sequence off does not lose its position. Turning it back on resumes the last selected sequence.

## Sequence Map

- Sequence 1: sparks + ember-style core
- Sequence 2: pulse pump/filament + beam + pulsing core
- Sequence 3: full show with sparks, pulse, beam, claw, and animated core

## M0 Serial Commands

Send any of these lines from the M0 serial monitor:

- `help`
- `on`
- `off`
- `toggle`
- `prev`
- `next`
- `seq1`
- `seq2`
- `seq3`
- `reset`

## Notes

- This is a framework, not a final tuned show controller. PWM levels, animation timing, motor speeds, and thermal limits all need bench validation.
- High-power LED channels and servo power should have appropriate thermal/current design outside Feather logic domains.
- Deterministic behavior depends on reliable M0->RP2040 FX command delivery; RP2040 now applies a timeout-safe shutdown when command frames go stale.

## KiCad Adafruit Footprints

- The project-level footprint library table is at `circuit/kicad/fp-lib-table`.
- It registers Adafruit's official Eagle library (`Adafruit-Eagle-Library/adafruit.lbr`) as an Eagle-type footprint source for KiCad.
- Integration notes are in `circuit/kicad/adafruit_footprint_import.md`.
- Concrete per-subsystem footprint mapping is in `circuit/kicad/orciny_netmap_no_motor_wing.txt`.