# Fusion 360 Electronics Starter Package

This folder contains a first-pass import/build package for creating the Orciny electronics design in Fusion 360 Electronics.

## Files

- `orciny_fusion_parts.csv`: part blocks and expected functional components
- `orciny_fusion_nets.csv`: named nets and required connectivity
- `orciny_fusion_pin_map.csv`: controller pin assignments for wiring validation
- `orciny_fusion_adafruit_library_map.csv`: concrete mapping from Orciny design parts to bundled Adafruit EAGLE library entries and fallbacks
- `orciny_fusion_library_notes.md`: notes on exact matches, footprint proxies, and package-only substitutions
- `orciny_board_placement_plan.md`: board floorplan, connector-edge placement, and routing-priority guidance for Fusion layout

## What this gives you

- A complete connectivity spec aligned with current firmware and wiring docs
- A clean checklist source for building a Fusion schematic
- A PCB-forward set of net names you can preserve through layout
- A first-pass symbol/footprint mapping against the bundled Adafruit EAGLE library used by the project
- A first-pass physical placement strategy for the board and edge connectors

## Build in Fusion 360 Electronics

1. Create a new Electronics Design and open the schematic editor.
2. Add these core components first:
   - Adafruit Feather RP2040
   - Adafruit Feather M0
   - Adafruit 8-Channel Servo FeatherWing (PCA9685)
   - Adafruit Prop-Maker FeatherWing
3. Add connectors and discrete driver blocks matching `orciny_fusion_parts.csv`.
4. Create all named nets from `orciny_fusion_nets.csv`.
5. Connect controller pins according to `orciny_fusion_pin_map.csv`.
6. Mark optional pump nets (`PUMP_*`) as DNP/optional if pump is not populated.
7. Run ERC in schematic and resolve any unconnected critical nets.
8. Switch to board view, place power/high-current paths first, then logic/interconnect.
9. Use wider traces or pours for `PSU_5V_BUS`, load returns, and filament/power paths.
10. Re-run DRC and ERC until clean.

## Net Class Suggestions

- `POWER_HIGH`: `PSU_5V_BUS`, `PSU_3V_FILAMENT`, high-current load returns
- `POWER_LOW`: MCU and control power branches
- `CONTROL_PWM`: spark/pulse/Prop-Maker control lines, servo signals
- `LOGIC`: switches, UART, I2C

## Notes

- Pump control net is intentionally preserved but optional because firmware currently disables pump actions.
- Servo wing is modeled as PCA9685 at I2C address `0x40`, using channels CH0 and CH1.
- Keep a strict common ground strategy between M0, RP2040, wings, and load stages.
