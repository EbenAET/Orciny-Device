

# Orciny Device Firmware Framework

Version: V 0.6.7


Firmware and documentation for a single-controller multi-effect device built around:

- Feather RP2040

The RP2040 handles scene control, switch and USB command handling, NeoPixel core rendering, spark filament outputs, Prop-Maker LED control, and dual-servo claw motion through an 8-channel Servo FeatherWing (PCA9685).


## Folder Layout

- `arduino/Orciny_Device/Orciny_Device.ino`: Main FX controller sketch (current, recommended)
- `arduino/Orciny_Device/DeviceConfig.h`: Centralized pin and parameter definitions
- `arduino/Orciny_Device/Orciny_Device_Animation_Timeline.md`: Animation sequence timeline (detailed)
- `arduino/Orciny_Device/Orciny_Device_Animation_Timeline_Visual.md`: Visual timeline (ASCII & Mermaid)
- `arduino/Orciny_Device/Orciny_Device_Troubleshooting.md`: Physical troubleshooting guide
- `arduino/rp2040_fx_starter`: Minimal starter sketch for hardware bring-up
- `arduino/depreciated/`: Older and legacy sketches
- `arduino/libraries/OrcinyCommon`: Shared protocol and data structures
- `arduino/libraries/OrcinyEffects`: Optional pre-built effect scenes (advanced users)

## Arduino Libraries

Install these in the Arduino IDE before compiling:

- Adafruit NeoPixel
- Adafruit PWM Servo Driver Library


If Arduino cannot find local headers or libraries, see `arduino/TROUBLESHOOTING.md` for setup and recovery steps.


Set the Arduino sketchbook location to the repo's `arduino` folder so custom libraries resolve correctly:

- Sketchbook location: `C:\Users\ebena\Box\Orciny Device\arduino`


`OrcinyCommon` is intended to be used from `arduino/libraries/OrcinyCommon`. Do not keep per-sketch duplicate copies of that header.



## Versioning

This repository is currently at V 0.6.7. All configuration, palette, and animation headers are centralized. State-tracking variables and reset logic are robust and up-to-date as of this version.

**This is the most current functioning and validated version. Use this as your restore point for stable builds.**



## How To Use

### Main Controller (recommended)

1. In Arduino IDE, set Sketchbook location to your repo's `arduino` folder.
2. Open `arduino/Orciny_Device/Orciny_Device.ino` and select Feather RP2040.
3. Upload and test with hardware switches.
4. Adjust `DeviceConfig.h` for your hardware.
5. See the animation timeline and troubleshooting docs for effect and debug details.

Physical switch behavior:
	- SW1 (`GP27`): toggle outputs on/off
	- SW2 (`GP28`): previous state
	- SW3 (`GP29`): next state
	- Hold SW1 + SW3 for 5 seconds: reset to State 1 with outputs off

### Starter Sketch (for bring-up)

See `arduino/rp2040_fx_starter/rp2040_fx_starter.ino` for a minimal, single-file starter.

### Demo Controller (legacy/advanced)

See `arduino/rp2040_fx_controller_demo/rp2040_fx_controller_demo.ino` for the previous demo controller with USB serial commands.

### Hybrid with OrcinyEffects

See `arduino/libraries/OrcinyEffects/examples/OrcinyEffects_Example/OrcinyEffects_Example.ino` for a minimal sketch using pre-built scenes.
- Good for rapid deployment and reusable scene architecture

### Library Roles

- `arduino/libraries/OrcinyCommon`: shared protocol structs, palette definitions, animation presets
- `arduino/libraries/OrcinyEffects`: optional pre-built scene implementations for advanced/hybrid workflows

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
- RP2040 pin `GP25` drives one NeoPixel data line for Adafruit product `4865` (SK6812, 166 pixels)
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

## Fusion MCP Archive Workflow

- Use `tools/sync-fusion-mcp-archive.ps1` to query recent Fusion docs through your local MCP adapter and export archive files into this repo.
- Default MCP URL is `http://127.0.0.1:27182/mcp`.
- Default export root is `3d Models/Archive/fusion_auto`.
- If `tools/fusion-project-map.json` exists and you do not pass `-Project`, the script automatically uses map rules to route each Fusion project into its own subfolder.

- Run an audit first to see what would match and export:

```powershell
.\tools\sync-fusion-mcp-archive.ps1 -Mode Audit -Project "City and the City - Orciny Device"
```

- Export selected files by project and name pattern:

```powershell
.\tools\sync-fusion-mcp-archive.ps1 -Mode Export -Project "City and the City - Orciny Device" -NameLike "*Pincer*" -Formats f3d,step,stl
```

- Add `-IncludeStlOneFilePerBody` if you want STL outputs split by body.
- Add `-WhatIf` to preview actions without opening or exporting any document.
- Add `-ReportPath` to write a machine-readable run report.
- Add `-DisableProjectMap` to bypass the map and use manual `-Project` / `-NameLike` filters.

- Audit all mapped projects:

```powershell
.\tools\sync-fusion-mcp-archive.ps1 -Mode Audit
```

- Export all mapped projects into project-specific archive folders:

```powershell
.\tools\sync-fusion-mcp-archive.ps1 -Mode Export
```

- Map file location and format: `tools/fusion-project-map.json`
- Each rule can include:
	- `project` or `projectId`
	- `outputSubfolder`
	- `formats` (`f3d`, `step`, `stl`)
	- optional `nameLike`

- Chain export and Box upload in one command:

```powershell
.\tools\sync-fusion-mcp-archive.ps1 -Mode Export -Project "City and the City - Orciny Device" -Formats f3d,step -RunBoxSync
```

- The script creates a timestamped run folder under `3d Models/Archive/fusion_auto` so every run is non-destructive.

## How To Use

### Path A: Starter Sketch (most users)

1. In Arduino IDE, set Sketchbook location to `C:\Users\ebena\Box\Orciny Device\arduino`.
2. Open `arduino/rp2040_fx_starter/rp2040_fx_starter.ino` and select Feather RP2040.
3. Upload and test with hardware switches.
4. Customize `doState1()` to `doState4()`.

Starter switch behavior:

- SW1 (`GP27`): toggle outputs on/off
- SW2 (`GP28`): previous state
- SW3 (`GP29`): next state
- Hold SW1 + SW3 for 5 seconds: reset to State 1 with outputs off

### Path B: Demo Controller (advanced)

1. Open `arduino/rp2040_fx_controller_demo/rp2040_fx_controller_demo.ino`.
2. Adjust `arduino/rp2040_fx_controller_demo/DeviceConfig.h` for your hardware.
3. Upload and open USB serial monitor at `115200` baud.

Demo controller uses three sequences:

- Sequence 1: ember-focused profile
- Sequence 2: pulse/beam-focused profile
- Sequence 3: full show profile

Common serial commands:

- `help`, `status`
- `on`, `off`, `toggle`
- `prev`, `next`, `seq1`, `seq2`, `seq3`, `reset`
- `beam palette cool|ember|cyan|violet|auto`
- `neo on|off|auto`

### Path C: Hybrid with OrcinyEffects

1. Open `arduino/libraries/OrcinyEffects/examples/OrcinyEffects_Example/OrcinyEffects_Example.ino`.
2. Upload as-is for quick validation.
3. Replace scene calls or tune library scene internals in `arduino/libraries/OrcinyEffects/src/OrcinyEffects.h`.



## Documentation

- [Orciny_Device_Animation_Timeline.md](arduino/Orciny_Device/Orciny_Device_Animation_Timeline.md): Detailed animation sequence timeline
- [Orciny_Device_Animation_Timeline_Visual.md](arduino/Orciny_Device/Orciny_Device_Animation_Timeline_Visual.md): Visual timeline (ASCII & Mermaid)
- [Orciny_Device_Troubleshooting.md](arduino/Orciny_Device/Orciny_Device_Troubleshooting.md): Physical troubleshooting guide

## Notes

- As of V 0.6.7, all pin/parameter definitions are centralized in `DeviceConfig.h`, and all color/animation minutiae are in `ColorPalettes.h` and `AnimationPalettes.h`.
- State-tracking variables are global and reset logic is robust (see `resetStateStatics()` in the main sketch).
- Use this version as a restore point for stable, maintainable builds.

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