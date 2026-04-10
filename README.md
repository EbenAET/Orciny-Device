# Orciny Device Firmware Framework

Arduino IDE scaffold for a single-controller multi-effect device built around:

- Feather RP2040

The RP2040 handles scene control, switch and USB command handling, NeoPixel core rendering, spark filament outputs, Prop-Maker LED control, and dual-servo claw motion through an 8-channel Servo FeatherWing (PCA9685).

## Folder Layout

- `arduino/rp2040_fx_controller_demo`: primary RP2040 FX controller sketch for the current demo/test build
- `arduino/rp2040_fx_starter`: minimal RP2040 starter sketch for hardware bring-up
- `arduino/depreciated/rp2040_fx_controller`: older RP2040 controller reference sketch
- `arduino/depreciated/m0_core_controller`: legacy split-controller reference sketch
- `arduino/libraries/OrcinyCommon`: shared protocol and data structures

## Arduino Libraries

Install these in the Arduino IDE before compiling:

- Adafruit NeoPixel
- Adafruit PWM Servo Driver Library

Set the Arduino sketchbook location to the repo's `arduino` folder so custom libraries resolve correctly:

- Sketchbook location: `C:\Users\ebena\Box\Orciny Device\arduino`

`OrcinyCommon` is intended to be used from `arduino/libraries/OrcinyCommon`. Do not keep per-sketch duplicate copies of that header.

## Wiring Assumptions

The scaffold makes a few deliberate assumptions that you should verify against your hardware:

- Main 5V input feeds only the Servo FeatherWing and Prop-Maker/peltier load domains (`+5V_BUS`)
- One PKCell LP503562 3.7V 1200mAh cell is connected directly to Feather RP2040 BAT input (Feather built-in charging path)
- NeoPixel strip power is sourced from `+5V_BUS` in the current wiring profile
- Optional legacy: a dedicated NeoPixel LP503562 battery rail (`+3V7_NEO`) with external Adafruit 259 charger
- RP2040 battery rail supplies controller logic and direct spark filament branch
- RP2040 I2C connects to an 8-channel Servo FeatherWing (PCA9685 at `0x40`) that drives two servo outputs
- FeatherWing headers are electrically shared when used on a Feather Doubler/Tripler (pins are cross-connected, not isolated)
- RP2040 pin `GP8` controls the peltier MOSFET gate; firmware turns peltier on while beam output is active and holds it on briefly after beam-off for heat dissipation
- Optional pump path is currently unassigned at GPIO level in this revision (previous GP8 pump gate mapping was repurposed to peltier control)
- RP2040 uses three momentary switches wired to ground with internal pull-ups enabled
- The 3-9W LED channels are driven from the Prop-Maker FeatherWing MOSFET-controlled LED outputs, commanded by RP2040 PWM control lines
- Four spark LED filaments are wired as direct RP2040 PWM outputs through 10 ohm series current-limiting resistors and are supplied from the main RP2040 3.7V battery domain
- RP2040 pin `GP5` drives one NeoPixel data line for Adafruit product `4865` (SK6812, 166 pixels)
- RP2040 pin `GP10` is reserved for Prop-Maker PWR enable and is held HIGH in firmware
- All power sources share `GND_COMMON`; keep positive rails isolated (`+5V_BUS`, `RP2040_BAT`, and `+3V7_NEO` are not tied together)
- If the legacy Neo battery path is used, the external Adafruit 259 charger should feed only that dedicated pair (`CHG_NEO <-> LP503562 -> +3V7_NEO`)

This revision intentionally removes NeoPXL8 and Motor FeatherWing dependencies.

## Box Sync Workflow

- Use `tools/sync-box-local.ps1` to audit or sync the Box copy and the local git clone.
- Default paths are the current repo root and `~/Box/Orciny Device`.
- Run an audit first:

```powershell
.\tools\sync-box-local.ps1 -Mode Audit
```

- Copy newer or missing files from Box into the repo:

```powershell
.\tools\sync-box-local.ps1 -Mode BoxToRepo -PruneLegacyDuplicates
```

- Copy newer or missing files from the repo back into Box:

```powershell
.\tools\sync-box-local.ps1 -Mode RepoToBox -PruneLegacyDuplicates
```

- Add `-WhatIf` to preview file operations without changing anything.
- Add `-DeleteDestinationOnly` only when you explicitly want the destination side pruned to match the source.
- Add `-PruneLegacyDuplicates` to remove the old root-level duplicate library and backup folders and keep the canonical `circuit/kicad/...` layout intact.
- The sync skips nested `.git` directories, the old root-level duplicate library and backup folders, plus `.DS_Store` and `Thumbs.db` files.

## How To Use

1. In Arduino IDE, set Sketchbook location to `C:\Users\ebena\Box\Orciny Device\arduino`.
2. Open `arduino/rp2040_fx_controller_demo/rp2040_fx_controller_demo.ino` in Arduino IDE and select the Feather RP2040 target.
3. Adjust RP2040 `DeviceConfig.h` settings to match your actual wiring and strip length.
4. Upload the RP2040 sketch.
5. Open the RP2040 USB serial monitor at `115200` baud for operator commands and status.

## Switch Inputs

The RP2040 sketch expects three momentary switches connected from GPIO to GND and uses `INPUT_PULLUP`.

- Switch 1 on pin `A1` (`GP27`): toggle the current sequence on or off
- Switch 2 on pin `A2` (`GP28`): move to the previous sequence, wrapping `1 -> 3`
- Switch 3 on pin `A3` (`GP29`): move to the next sequence, wrapping `3 -> 1`
- Hold switches 1 and 3 together for 5 seconds: reset back to sequence 1

Turning the sequence off does not lose its position. Turning it back on resumes the last selected sequence.

## Sequence Map

- Sequence 1: sparks + ember-style core
- Sequence 2: beam + pulsing core (peltier auto-follows beam with cooldown hold)
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