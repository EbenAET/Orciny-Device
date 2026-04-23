# Orciny Device Unified Firmware (V 0.9.0)

## Overview

This repository contains the complete, unified firmware and documentation for the Orciny Device: a single-controller, multi-effect system built around the Adafruit Feather RP2040. All device logic, state management, and effect sequencing are implemented in a single, modular program with robust configuration and documentation.

## Backstage Instructions

- **Setup:**
	- Check that 5V battery is plugged in to device USB cable
		- Located in broken end of device
		- Be sure 5V charging cable is unplugged and detatched from device
	- Tuck Programming/Charging cable into center and secure end
		- Located between "Core" and "Rotary" setions
		- Gently feed excess cable into cavity until only painted portion remains
		- Stick velcro end of cable to receiving pad
	- Test Device
		- Press "power" button to enable outputs
			- Back end should start slowly sparking
		- Press "play/pause" button to start sequence
			- Core animation should start (slowly at first, then ramping up)
		- If full program runs, its ready to go! Set for show!
	- Handoff
		- When in standby to hand off, check that device is still in "inactive/idle" by pressing "power" button
		- If in the correct state, there should be intermittent sparks and no other animations
		- If other animations start up, let the program run through or press the "Break" button to fastforward to the device failure sequence, after which the device will return to "Idle" sequence with outputs off
			- At this point, press the "power" button to make sure outputs are on and device is in "Inactive/Idle"
- **Charging:**
	- Device has two batteries for operation
		- 5V power bank in tail
		- 3.7V LiPO internally
	- 5V battery should only need charging every couple of performances if device USB is unplugged when not in use
		- Charges with MicroUSB cable as needed
	- 3.7V battery should be charged before each performance since we can't easily check status and we can't easily disconnect it
		- Charges with programming/charging cable tucked into midsection of device
		
### Physical Controls

- SW1 (`GP27`): Toggle outputs on/off
- SW2 (`GP28`): Play/Pause
- SW3 (`GP29`): Jump to Failure
- Hold SW1 + SW3 for 5 seconds: Reset to State 1 with outputs off

## Troubleshooting

- **Check troubleshooting document**
- **Additional assistance**
	- Contact Emery Smith or Eben Alguire
	
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

## How to Program

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
	 
## Documentation

- [Animation Timeline](arduino/Orciny_Device/Orciny_Device_Animation_Timeline.md): State-by-state effect breakdown
- [Troubleshooting](arduino/Orciny_Device/Orciny_Device_Troubleshooting.md): Physical device troubleshooting

## Versioning & Stability

- **Current Version:** V 0.9.0
- All configuration, palette, and animation headers are centralized.
- State-tracking and reset logic are robust and up-to-date.
- Use this version as your restore point for stable builds.

## Hardware & Wiring Notes

- All pin/parameter definitions: `DeviceConfig.h`
- Color/animation minutiae: `ColorPalettes.h`, `AnimationPalettes.h`
- Power, wiring, and hardware are detailed in the troubleshooting and timeline docs.
- This firmware is a framework: PWM levels, animation timing, and thermal limits require bench validation for new or modified builds.

---