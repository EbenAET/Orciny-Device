


# Orciny Device Unified Firmware (V 0.7.0)

## Overview

This repository contains the complete, unified firmware and documentation for the Orciny Device: a single-controller, multi-effect system built around the Adafruit Feather RP2040. All device logic, state management, and effect sequencing are implemented in a single, modular program with robust configuration and documentation.

## Architecture

- **Main Program:**
	- `arduino/Orciny_Device/Orciny_Device.ino` — The only sketch you need. Implements the full state machine, effect sequencing, physical controls, and safety logic.
- **Configuration:**
	- `arduino/Orciny_Device/DeviceConfig.h` — All pin assignments and tunable parameters in one place.
- **Animation & Palette Data:**
	- `arduino/Orciny_Device/ColorPalettes.h`, `arduino/Orciny_Device/AnimationPalettes.h` — All color, timing, and animation data.
- **Shared Logic:**
	- `arduino/libraries/OrcinyCommon` — Protocols, data structures, and shared code.
- **Optional Advanced Scenes:**
	- `arduino/libraries/OrcinyEffects` — Pre-built effect scenes for advanced/hybrid workflows (not required for standard operation).
- **Documentation:**
	- Markdown files in `arduino/Orciny_Device/` detail the animation timeline, troubleshooting, and visualizations.

## How to Use

1. **Set Up Arduino IDE:**
	 - Set your Sketchbook location to the repo’s `arduino` folder.
	 - Install required libraries: Adafruit NeoPixel, Adafruit PWM Servo Driver Library.
2. **Open the Main Sketch:**
	 - Open `arduino/Orciny_Device/Orciny_Device.ino`.
	 - Select the Feather RP2040 board.
3. **Configure Hardware:**
	 - Edit `DeviceConfig.h` for your pinout and hardware specifics.
4. **Upload and Test:**
	 - Upload to your device and test with the physical switches.
5. **Reference Documentation:**
	 - See the Markdown docs for animation timing, troubleshooting, and wiring.

### Physical Controls

- SW1 (`GP27`): Toggle outputs on/off
- SW2 (`GP28`): Previous state
- SW3 (`GP29`): Next state
- Hold SW1 + SW3 for 5 seconds: Reset to State 1 with outputs off

## Project Structure

- `arduino/Orciny_Device/Orciny_Device.ino` — Main program
- `arduino/Orciny_Device/DeviceConfig.h` — Pin/parameter config
- `arduino/Orciny_Device/Orciny_Device_Animation_Timeline.md` — Detailed animation timeline
- `arduino/Orciny_Device/Orciny_Device_Animation_Timeline_Visual.md` — Visual timeline (ASCII/Mermaid)
- `arduino/Orciny_Device/Orciny_Device_Troubleshooting.md` — Troubleshooting guide
- `arduino/libraries/OrcinyCommon` — Shared code
- `arduino/libraries/OrcinyEffects` — Optional advanced scenes
- `arduino/rp2040_fx_starter` — Minimal starter sketch (for bring-up only)
- `arduino/depreciated/` — Legacy/old sketches

## Documentation

- [Animation Timeline](arduino/Orciny_Device/Orciny_Device_Animation_Timeline.md): State-by-state effect breakdown
- [Timeline Visual](arduino/Orciny_Device/Orciny_Device_Animation_Timeline_Visual.md): ASCII/Mermaid diagram
- [Troubleshooting](arduino/Orciny_Device/Orciny_Device_Troubleshooting.md): Physical device troubleshooting

## Versioning & Stability

- **Current Version:** V 0.7.0
- All configuration, palette, and animation headers are centralized.
- State-tracking and reset logic are robust and up-to-date.
- Use this version as your restore point for stable builds.

## Hardware & Wiring Notes

- All pin/parameter definitions: `DeviceConfig.h`
- Color/animation minutiae: `ColorPalettes.h`, `AnimationPalettes.h`
- Power, wiring, and hardware assumptions are detailed in the troubleshooting and timeline docs.
- This firmware is a framework: PWM levels, animation timing, and thermal limits require bench validation for your build.

## Advanced: Box & Fusion Sync, KiCad

- See `tools/sync-box-local.ps1` and `tools/sync-fusion-mcp-archive.ps1` for file and CAD archive sync workflows.
- KiCad footprint and netmap info: `circuit/kicad/`.

---
For any issues, see the troubleshooting doc or open an issue on GitHub.