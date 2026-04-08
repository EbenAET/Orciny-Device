# Orciny Device Firmware Framework

Arduino IDE scaffold for a single-controller multi-effect device built around:

- Feather RP2040

The RP2040 handles scene control, switch and USB command handling, NeoPixel core rendering, spark filament outputs, Prop-Maker LED control, and dual-servo claw motion through an 8-channel Servo FeatherWing (PCA9685).

## Folder Layout

- `arduino/rp2040_fx_controller`: primary controller sketch for RP2040 scene, core, and FX control
- `arduino/m0_core_controller`: legacy split-controller reference sketch (not used in RP2040-only wiring)
- `arduino/libraries/OrcinyCommon`: shared protocol and data structures

## Arduino Libraries

Install these in the Arduino IDE before compiling:

- Adafruit NeoPixel
- Adafruit PWM Servo Driver Library

## Wiring Assumptions

The scaffold makes a few deliberate assumptions that you should verify against your hardware:

- Main 5V input feeds only the Servo FeatherWing and Prop-Maker/peltier load domains (`+5V_BUS`)
- One PKCell LP503562 3.7V 1200mAh cell is connected directly to Feather RP2040 BAT input (Feather built-in charging path)
- A second PKCell LP503562 3.7V 1200mAh cell powers the NeoPixel strip rail (`+3V7_NEO`)
- One external Adafruit Micro Lipo charger (`259`) is used for the dedicated NeoPixel LP503562 cell
- RP2040 battery rail supplies controller logic and direct spark filament branch
- RP2040 I2C connects to an 8-channel Servo FeatherWing (PCA9685 at `0x40`) that drives two servo outputs
- RP2040 pin `GP8` can remain wired to pump-enable MOSFET gate, but pump actions are currently disabled in firmware
- RP2040 uses three momentary switches wired to ground with internal pull-ups enabled
- The 3-9W LED channels are driven from the Prop-Maker FeatherWing MOSFET-controlled LED outputs, commanded by RP2040 PWM control lines
- Four spark LED filaments are wired as direct RP2040 PWM outputs through 10 ohm series current-limiting resistors and are supplied from the main RP2040 3.7V battery domain
- RP2040 pin `GP11` drives one NeoPixel data line for Adafruit product `4865` (SK6812, 166 pixels)
- All power sources share `GND_COMMON`; keep positive rails isolated (`+5V_BUS`, `RP2040_BAT`, and `+3V7_NEO` are not tied together)
- The external Adafruit 259 charger should feed only the dedicated NeoPixel battery/rail pair (`CHG_NEO <-> LP503562 -> +3V7_NEO`)

This revision intentionally removes NeoPXL8, Motor FeatherWing, and Feather Doubler dependencies.

## How To Use

1. Open `arduino/rp2040_fx_controller/rp2040_fx_controller.ino` in Arduino IDE and select the Feather RP2040 target.
2. Adjust RP2040 `DeviceConfig.h` settings to match your actual wiring and strip length.
3. Upload the RP2040 sketch.
4. Open the RP2040 USB serial monitor at `115200` baud for operator commands and status.

## Switch Inputs

The RP2040 sketch expects three momentary switches connected from GPIO to GND and uses `INPUT_PULLUP`.

- Switch 1 on pin `2`: toggle the current sequence on or off
- Switch 2 on pin `3`: move to the previous sequence, wrapping `1 -> 3`
- Switch 3 on pin `4`: move to the next sequence, wrapping `3 -> 1`
- Hold switches 1 and 3 together for 5 seconds: reset back to sequence 1

Turning the sequence off does not lose its position. Turning it back on resumes the last selected sequence.

## Sequence Map

- Sequence 1: sparks + ember-style core
- Sequence 2: optional pump + beam + pulsing core
- Sequence 3: full show with sparks, beam, claw, and animated core

## Serial Commands

Send any of these lines from the RP2040 serial monitor:

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
- Firmware includes a NeoPixel guard rail that caps strip output to a 2A maximum equivalent draw.
- High-power LED channels and servo power should have appropriate thermal/current design outside Feather logic domains.
- Use appropriately gauged wiring for power distribution: 18 AWG minimum on +5V_MAIN/+5V_BUS trunk and 20 AWG minimum on high-current branch runs/returns (including +3V7_NEO and +3V7_BEAM feeds/returns).
- Deterministic behavior depends on stable power domains and common-ground integrity across controller, strips, and load stages.

## KiCad Adafruit Footprints

- The project-level footprint library table is at `circuit/kicad/fp-lib-table`.
- It registers Adafruit's official Eagle library (`Adafruit-Eagle-Library/adafruit.lbr`) as an Eagle-type footprint source for KiCad.
- Integration notes are in `circuit/kicad/adafruit_footprint_import.md`.
- Concrete per-subsystem footprint mapping is in `circuit/kicad/orciny_netmap_no_motor_wing.txt`.