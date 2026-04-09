# Orciny Wiring Harness Reference

This file groups cable runs into practical harness bundles for assembly and troubleshooting.

Design intent in this reference:
- RP2040 is the primary controller.
- Feather wings stack directly on the RP2040 Feather, so no interconnect harness is required between the controller and stacked wings.
- Main 5V rail powers only ServoWing and Prop-Maker/peltier load domains.
- One LP503562 battery feeds RP2040 BAT directly.
- One LP503562 battery feeds NeoPixel rail, charged by external Adafruit 259.

## Harness Set Overview

| Harness ID | Name | Scope | Notes |
|---|---|---|---|
| H-PWR-5V | 5V load backbone | Main 5V input to +5V_BUS load domain | Feeds stacked wings and other +5V load branches. |
| H-BAT-RP | RP2040 battery harness | Main 3.7V battery to Feather BAT | Feather onboard charger handles this pack. |
| H-BAT-NEO | NeoPixel battery + charger harness | Neo battery and external charger wiring | Dedicated battery/charger/rail pair only. |
| H-CORE | NeoPixel strip harness | Data and strip supply run | Keep data conductor separated from noisy load wiring. |
| H-SW | Switch input harness | SW1/SW2/SW3 to controller inputs | Shared ground return. |
| H-SPARK | Spark output harness | RP2040 spark PWM channels to spark loads | DMX cable runs down shaft; pins 1-4 are Spark 1-4, pin 5 is spare if needed, and GND is common. |
| H-SERVO | Servo output harness | ServoWing outputs and power out to servos | ServoWing stacks on the Feather, so no separate Feather-to-wing harness is needed. |
| H-BEAM-PWR | Beam load harness | +5V_BUS load-side runs to beam path | Prop-Maker FeatherWing stacks on the Feather; harness is beam load-side wiring only. |
| H-PELTIER-PWR | Peltier load harness | +5V_BUS load-side runs to peltier path | Separate high-current branch for peltier load wiring. |


## Harness Definitions

### H-PWR-5V - 5V Load Backbone
- Harness Label 1
- Source: J801 (5V input)
- Path: Main fuse -> F801 -> +5V_BUS
- Loads: Servo FeatherWing V+, Prop-Maker load domain, optional pump/peltier load supply
- Recommended wire: 18 AWG trunk, 20 AWG branches
- Notes: Keep this harness physically separated from low-level signal bundles.

### H-BAT-RP - RP2040 Battery Harness
- Harness Label 10
- Source: LP503562 cell #1
- Destination: Feather RP2040 BAT and GND
- Charging: Feather onboard charger path
- Recommended wire: 20 AWG
- Notes: Keep battery positive isolated from +5V_BUS and +3V7_NEO.
- Battery connects directly to Feather's Battery JST port

### H-BAT-NEO - NeoPixel Battery and Charger Harness
- Harness Label 2
- Source: LP503562 cell #2
- Charger: U812 (Adafruit 259)
- Destination rail: +3V7_NEO
- Recommended wire: 20 AWG battery and rail runs
- Notes: U812 output must remain dedicated to this battery/rail pair only.
- Add a 500-1000 uF bulk electrolytic capacitor across +3V7_NEO and GND at the strip power entry point.
- Use 2 pin JST for power to Charger output
- Red wire is positive, black wire is negative

### H-CORE - NeoPixel Strip Harness
- Harness Label 3
- Signal run: RP2040 GP11 -> strip DI
- Power run: +3V7_NEO -> strip +V
- Ground run: GND_COMMON -> strip GND
- Recommended wire: 24-26 AWG for data, 20 AWG for strip power/ground
- Notes: Keep data conductor away from high-current switching runs where possible.
- Place the bulk capacitor physically close to the first pixel / strip power injection point.
- Use 2 pin JST for signal + ground to microcontroller
- White wire is signal, brown wire is common

- Note for NeoPixel harnesses: I am using a 4 pin cable. On the neopixel side, I am using a 3 pin JST HX to combine signal and power. Pin one (red) is power, pin two (white) is signal, pin 3 (black and brown spliced) is common. A capacitor is bridging power and common. This is Harness 2/3. The cable splits into two sets of two and becomes harness 2 and harness 3. 

### H-SW - Switch Input Harness
- Harness Label 4
- SW1 -> RP2040 GP2
- SW2 -> RP2040 GP3
- SW3 -> RP2040 GP4
- Return: shared GND
- Recommended wire: 24-26 AWG
- Notes: Route this harness away from spark and servo power runs.
- use 4 pin JST connector
- Pin 1 (blue) is SW1, pin 2 (brown) is SW2, pin 3 (orange) is SW3, and pin 4 (green) is common

### H-SPARK - Spark Output Harness
- Harness Label 5
- Channels: RP2040 GP5/GP6/GP9/GP10 (through series resistors) -> Spark 1-4
- Supply domain: RP2040_BAT
- Return: GND_COMMON
- Recommended wire: 22-24 AWG signal legs, 20 AWG shared feed/return where applicable
- Notes: Keep channel labeling fixed end-to-end to avoid cross-wiring.
- Use 5 pin DMX cable
- Green is common, pins 1 (red), 2 (brown), 3 (white), and 4 (blue) are the individual positive connections for the sparks, and pin 5 (black) is a spare

### H-SERVO - Servo Output Harness
- Harness Label 6
- Stack note: ServoWing stacks directly on the Feather RP2040; SDA/SCL, control, and wing power are handled by the stack headers.
- Output runs: ServoWing CH0 -> Servo A signal, CH1 -> Servo B signal
- Power runs: ServoWing V+ and GND -> servo power branches
- Recommended wire: 24-26 AWG for signal, 20 AWG for power/ground branches
- Notes: Match positive and return gauge on each servo branch; no separate Feather-to-ServoWing harness is required.
- Use 4 pin JST connector
- Pin 1 (white) is CH0 signal, Pin 2 (blue) is CH1 signal, Pin 3 (red) is power, pin 4 (black) is common

### H-BEAM-PWR - Beam Load Harness
- Harness Label 7
- Stack note: Prop-Maker FeatherWing stacks directly on the Feather RP2040; no separate controller-to-wing harness is required.
- +5V_BUS -> Prop-Maker load domain and beam load source path
- GND_COMMON return from beam load stage
- Recommended wire: 20 AWG minimum
- Notes: Treat as high-current branch and keep runs short; this harness is beam load-side only.
- Beam should have 4 pin JST connector
- Beam LED chip is common anode. Connector pins are pin 1 (white) as common anode, pin 2 (red) as red channel, pin 3 (blue) as blue channel, pin 4 (green) as green channel

### H-PELTIER-PWR - Peltier Load Harness
- Harness Label 8
- Stack note: Prop-Maker FeatherWing stacks directly on the Feather RP2040; no separate controller-to-wing harness is required.
- +5V_BUS -> peltier load source path
- GND_COMMON return from peltier load stage
- Recommended wire: 20 AWG minimum
- Notes: Keep this as a separate high-current branch for thermal control load wiring.
- Peltier uses Deans Micro2R connector
- Pin 1 (red) is power, pin 2 (black) is common

### H-PUMP-OPT - Optional Pump Harness
- Harness Label 9
- Gate control: RP2040 GP8 -> pump driver gate (if populated)
- Load power: +5V_BUS and GND_COMMON pump branch
- Recommended wire: 24-26 AWG control, 20 AWG load branch
- Notes: Firmware currently keeps pump actions disabled.

## Labeling Convention

Use heat-shrink or tags at both ends:
- Harness ID (example: H-CORE)
- Net name (example: GP11_DI, +3V7_NEO, GND)
- Channel index for multi-channel runs (SP1..SP4)

## Bring-Up Continuity Checklist

1. Verify no short between +5V_BUS, RP2040_BAT, and +3V7_NEO.
2. Verify all grounds share continuity to GND_COMMON.
3. Verify H-CORE data continuity from RP2040 GP11 to strip DI.
4. Verify H-SPARK channel mapping (GP5/6/9/10 to Spark 1/2/3/4).
5. Verify H-SERVO polarity before connecting servos.
6. Verify H-BEAM-PWR and H-PELTIER-PWR are not cross-connected.
7. Verify U812 charger wiring only touches Neo battery rail.
