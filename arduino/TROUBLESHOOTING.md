
# Arduino Setup And Troubleshooting

Version: V 0.3.5


This folder contains both sketches and local libraries used by the Orciny Device Arduino code.


If a sketch fails to compile after downloading the project, check these common issues:

1. Arduino IDE is not using the correct sketchbook path (should be set to the repo's `arduino` folder).
2. Local libraries are not in the correct folder structure (see below).
3. Required third-party libraries (Adafruit NeoPixel, Adafruit PWM Servo Driver, Adafruit BusIO) are not installed.
4. Arduino IDE needs to be restarted after library changes.


## Expected Folder Structure

Arduino should be able to find the local libraries with this layout:

```text
arduino/
  libraries/
    OrcinyCommon/
      library.properties
      src/
        OrcinyCommon.h
        ColorPalettes.h
        AnimationPalettes.h
    OrcinyEffects/
      library.properties
      src/
        OrcinyEffects.h
  rp2040_fx_starter/
    rp2040_fx_starter.ino
  rp2040_fx_controller_demo/
    rp2040_fx_controller_demo.ino
```

Important rule:

All public Arduino library headers must live in the library's `src` folder. For this project, `ColorPalettes.h` and `AnimationPalettes.h` belong in `libraries/OrcinyCommon/src/`, not beside the sketch.


## Recommended Arduino IDE Setup

Set Arduino IDE's Sketchbook location to the `arduino` folder in this repo.

Example:

```text
C:\Users\USERNAME\Box\Orciny Device\arduino
```

In Arduino IDE:

1. Open `File > Preferences`.
2. Set `Sketchbook location` to this repo's `arduino` folder.
3. In `Additional Boards Manager URLs`, add:

```text
https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
```

4. Click `OK` to save Preferences.
5. Open `Tools > Board > Boards Manager`.
6. Search for `Raspberry Pi Pico/RP2040`.
7. Install the `Raspberry Pi Pico/RP2040` package by Earle F. Philhower, III.
8. Restart Arduino IDE.


Why this matters:

Arduino automatically scans the sketchbook's `libraries` folder. If the sketchbook points somewhere else, includes such as `#include <ColorPalettes.h>` and `#include <AnimationPalettes.h>` will fail even if the files exist in the repo.


## Common Problems

### Error: `fatal error: ColorPalettes.h: No such file or directory`

Cause:

Arduino cannot see the `OrcinyCommon` library.

Fix:

1. Confirm your sketchbook path points to this folder's `arduino` directory.
2. Confirm `ColorPalettes.h` exists at `arduino/libraries/OrcinyCommon/src/ColorPalettes.h`.
3. Restart Arduino IDE.

### Error: `fatal error: AnimationPalettes.h: No such file or directory`

Cause:

Same root cause as above. Arduino is not resolving the `OrcinyCommon` library correctly.

Fix:

1. Check sketchbook path.
2. Check that `AnimationPalettes.h` exists at `arduino/libraries/OrcinyCommon/src/AnimationPalettes.h`.
3. Make sure there are not duplicate copies of these headers in sketch folders.

### Error: `Adafruit_PWMServoDriver.h: No such file or directory`

Cause:

The Adafruit PCA9685 servo driver library is missing.

Fix:

Install the library from Arduino Library Manager:

1. Open `Sketch > Include Library > Manage Libraries`.
2. Search for `Adafruit PWM Servo Driver Library`.
3. Install it.

### Error: `Adafruit_NeoPixel.h: No such file or directory`

Cause:

The Adafruit NeoPixel library is missing.

Fix:

Install `Adafruit NeoPixel` from Library Manager.

### Error related to `Adafruit_I2CDevice.h` or `Adafruit_BusIO`

Cause:

`Adafruit BusIO` is missing. Some Adafruit libraries depend on it.

Fix:

Install `Adafruit BusIO` from Library Manager.

### The files are present but Arduino still will not compile

Check these in order:

1. Folder names must match the library names exactly.
2. Public headers must be in the library `src` folder.
3. `library.properties` must exist in each custom library root.
4. Arduino IDE often needs a restart after moving library files.
5. If you copied the project manually, remove stale duplicate headers from sketch folders.

## Local Libraries In This Project

These libraries are intended to be used as local sketchbook libraries:

1. `OrcinyCommon`
2. `OrcinyEffects`

At the current stage, they do not need Arduino Library Manager publication. Local installation is simpler and avoids premature packaging overhead.

## When A Packaging Pass Makes Sense

Yes, it makes sense to plan a package version later, but not necessarily publish it yet.

That future packaging pass should happen when the APIs and folder layout have mostly stabilized. The useful goal is a release-ready package, not immediate distribution.

Recommended packaging targets:

1. `OrcinyCommon` as a standalone Arduino library package.
2. `OrcinyEffects` as a standalone Arduino library package.
3. Starter sketches kept as examples or templates outside the library packages.

Recommended pre-release checklist:

1. Freeze public header names and namespaces.
2. Verify all public headers live under `src/`.
3. Add or review `library.properties` metadata.
4. Confirm example sketches compile against the packaged libraries.
5. Add a short changelog and versioning policy.
6. Create ZIP artifacts for test installs with `Add .ZIP Library`.

Practical recommendation:

Do the packaging cleanup shortly before a broader handoff, first public share, or hardware beta. Until then, keep the repo optimized for active development.

### Error related to `Adafruit_I2CDevice.h` or `Adafruit_BusIO`

Cause:

`Adafruit BusIO` is missing. Some Adafruit libraries depend on it.

Fix:

Install `Adafruit BusIO` from Library Manager.

### The files are present but Arduino still will not compile

Check these in order:

1. Folder names must match the library names exactly.
2. Public headers must be in the library `src` folder.
3. `library.properties` must exist in each custom library root.
4. Arduino IDE often needs a restart after moving library files.
5. If you copied the project manually, remove stale duplicate headers from sketch folders.

## Local Libraries In This Project

These libraries are intended to be used as local sketchbook libraries:

1. `OrcinyCommon`
2. `OrcinyEffects`

At the current stage, they do not need Arduino Library Manager publication. Local installation is simpler and avoids premature packaging overhead.

## When A Packaging Pass Makes Sense

Yes, it makes sense to plan a package version later, but not necessarily publish it yet.

That future packaging pass should happen when the APIs and folder layout have mostly stabilized. The useful goal is a release-ready package, not immediate distribution.

Recommended packaging targets:

1. `OrcinyCommon` as a standalone Arduino library package.
2. `OrcinyEffects` as a standalone Arduino library package.
3. Starter sketches kept as examples or templates outside the library packages.

Recommended pre-release checklist:

1. Freeze public header names and namespaces.
2. Verify all public headers live under `src/`.
3. Add or review `library.properties` metadata.
4. Confirm example sketches compile against the packaged libraries.
5. Add a short changelog and versioning policy.
6. Create ZIP artifacts for test installs with `Add .ZIP Library`.

Practical recommendation:

Do the packaging cleanup shortly before a broader handoff, first public share, or hardware beta. Until then, keep the repo optimized for active development.
