# Adafruit Footprint Source Integration

This project now includes official Adafruit CAD repositories under this folder:

- `Adafruit-Eagle-Library` (official footprint/source library, includes `adafruit.lbr`)
- `Adafruit_CAD_Parts` (official product CAD archive; does not currently expose KiCad `.pretty` libs)

## What Was Configured

- Added project-local footprint table: `fp-lib-table`
- Registered library:
  - Name: `Adafruit_Eagle`
  - Type: `Eagle`
  - URI: `${KIPRJMOD}/Adafruit-Eagle-Library/adafruit.lbr`

## Using It In KiCad

1. Open the project in KiCad from this folder.
2. Open Symbol Editor / Schematic and assign footprints as needed.
3. In Footprint assignment, select the `Adafruit_Eagle` library.
4. Map each schematic component to the closest Adafruit package footprint.

## Concrete Assignments For This Project

- RP2040 Feather controller: `Adafruit_Eagle:FEATHERWING`
- M0 Feather controller: `Adafruit_Eagle:FEATHERWING`
- SW1/SW2/SW3 momentary buttons: `Adafruit_Eagle:PUSHBUTTON_SMD_SJ`
- 5V DC input jack: `Adafruit_Eagle:DCJACK_2MM_PTH`
- Pump/Peltier 2-pin connector: `Adafruit_Eagle:JST-PH-2-THM`
- Beam terminal (4-pin): `Adafruit_Eagle:1X04-3.5MM`
- TO-220 MOSFET stages: `Adafruit_Eagle:TO220`

## Notes

- Adafruit's public `Adafruit-KiCad-Library` and `KiCad-Library` repositories are currently not available at those names.
- If Adafruit publishes/renames a native KiCad footprint repo later, update `fp-lib-table` to that `.pretty` path for a direct KiCad-native workflow.
