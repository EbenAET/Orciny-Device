# Adafruit Library Mapping Notes

This file explains how to use the bundled Adafruit EAGLE library with the Fusion starter package.

## Confirmed exact library entries

The bundled library at `circuit/kicad/Adafruit-Eagle-Library/adafruit.lbr` contains confirmed device support for:

- `FEATHERWING` deviceset with `FEATHERWING` and `FEATHERWING_DIM` packages
- `PCA9685` deviceset with `TSSOP28` package
- `SWITCH_PUSHBUTTON` deviceset with `SOFTTOUCHSMD_SJ` device and `PUSHBUTTON_SMD_SJ` package
- `DCBARREL` deviceset with `PTH` device and `DCJACK_2MM_PTH` package
- `JST_2PIN` deviceset with `-THM` device and `JST-PH-2-THM` package
- `JST_3PIN` deviceset with `JSTPH3` package
- `PINHD-1X3`, `PINHD-1X6`, and `PINHD-2X4` devicesets for generic headers

## Known gaps in the bundled library

The bundled Adafruit library does not appear to include exact board devicesets for:

- Adafruit Feather RP2040
- Adafruit Feather M0
- Adafruit Prop-Maker FeatherWing
- Adafruit 8-Channel Servo FeatherWing as a board-level module

For these boards, the recommended Fusion workflow is:

1. Use the `FEATHERWING` footprint as a board/module placement proxy.
2. Preserve the named nets from the Fusion CSV files.
3. Model board-specific functionality at the connectivity level rather than expecting an exact ready-made board symbol in the library.

## Practical recommendation

Use the mapping CSV as the source of truth:

- `exact-device`: safe to place directly from the bundled Adafruit library
- `footprint-proxy-only`: use for board/module outline and connector compatibility, not exact board semantics
- `package-only`: package exists, but you may need to pair it with your own schematic symbol or a generic Fusion symbol
- `package-supported`: package is present and suitable, but a generic connector symbol may still be simpler inside Fusion
