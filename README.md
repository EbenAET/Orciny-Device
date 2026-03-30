# Orciny Device Firmware Framework

Arduino IDE scaffold for a two-controller multi-effect device built around:

- Feather RP2040 + Motor FeatherWing
- Feather M0 + NeoPXL8 FeatherWing
- Feather Doubler

The framework splits responsibilities like this:

- RP2040 sketch: sparks, pulse filament, DC pump, beam RGB LED, peltier control, claw stepper, top-level scene control
- M0 sketch: four NeoPixel core banks driven through NeoPXL8, listening for scene frames from the RP2040 over Serial1

## Folder Layout

- `arduino/rp2040_fx_controller`: main effects controller sketch for the Feather RP2040
- `arduino/m0_core_controller`: NeoPXL8 core controller sketch for the Feather M0
- `arduino/libraries/OrcinyCommon`: shared protocol and data structures

## Arduino Libraries

Install these in the Arduino IDE before compiling:

- Adafruit Motor Shield V2 Library
- Adafruit NeoPixel
- Adafruit NeoPXL8
- Adafruit ZeroDMA

## Wiring Assumptions

The scaffold makes a few deliberate assumptions that you should verify against your hardware:

- RP2040 Motor FeatherWing port `M1` drives the pump
- RP2040 Motor FeatherWing stepper port `M3/M4` drives the claw stepper
- RP2040 uses three momentary switches wired to ground with internal pull-ups enabled
- The 3-9W RGB LED channels are driven through external MOSFETs from three PWM pins on the RP2040
- The peltier is switched through a separate MOSFET or relay from one RP2040 digital pin
- Four spark LED filaments and one pulse LED filament are each driven through suitable transistor stages, not directly from GPIO
- RP2040 `TX` is connected to M0 `RX` for scene updates, with shared ground

The M0 NeoPXL8 sketch uses a four-output pin set that preserves `Serial1` so the boards can communicate. Because NeoPXL8 pin muxing on SAMD21 is strict, treat the pin map in `DeviceConfig.h` as the first thing to validate on hardware.

## How To Use

1. Open `arduino/rp2040_fx_controller/rp2040_fx_controller.ino` in Arduino IDE and select the Feather RP2040 target.
2. Open `arduino/m0_core_controller/m0_core_controller.ino` in another Arduino IDE window and select the Feather M0 target.
3. Adjust both `DeviceConfig.h` files to match your actual wiring and strip lengths.
4. Upload the M0 sketch first, then the RP2040 sketch.
5. Open the RP2040 USB serial monitor at `115200` baud.

## Switch Inputs

The RP2040 sketch expects three momentary switches connected from GPIO to GND and uses `INPUT_PULLUP`.

- Switch 1 on pin `2`: toggle the current sequence on or off
- Switch 2 on pin `3`: move to the previous sequence, wrapping `1 -> 3`
- Switch 3 on pin `4`: move to the next sequence, wrapping `3 -> 1`
- Hold switches 1 and 3 together for 5 seconds: reset back to sequence 1

Turning the sequence off does not lose its position. Turning it back on resumes the last selected sequence.

## Sequence Map

- Sequence 1: sparks + ember-style core
- Sequence 2: pulse pump/filament + beam + pulsing core
- Sequence 3: full show with sparks, pulse, beam, claw, and animated core

## RP2040 Serial Commands

Send any of these lines from the serial monitor:

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
- The peltier and high-power RGB LED should have their own power design, thermal protection, and current handling outside the Feather GPIO domain.
- If you want deterministic synchronized behavior between the two boards, keep the RP2040 as the single source of truth and treat the M0 as a render node.

## KiCad Adafruit Footprints

- The project-level footprint library table is at `circuit/kicad/fp-lib-table`.
- It registers Adafruit's official Eagle library (`Adafruit-Eagle-Library/adafruit.lbr`) as an Eagle-type footprint source for KiCad.
- Integration notes are in `circuit/kicad/adafruit_footprint_import.md`.
- Concrete per-subsystem footprint mapping is in `circuit/kicad/orciny_kicad_netmap.txt`.