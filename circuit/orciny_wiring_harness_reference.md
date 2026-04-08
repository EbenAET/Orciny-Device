# Orciny Wiring Harness Reference

This file groups cable runs into practical harness bundles for assembly and troubleshooting.

Design intent in this reference:
- RP2040 is the primary controller.
- Main 5V rail powers only ServoWing and Prop-Maker/peltier load domains.
- One LP503562 battery feeds RP2040 BAT directly.
- One LP503562 battery feeds NeoPixel rail, charged by external Adafruit 259.

## Harness Set Overview

| Harness ID | Name | Scope |
|---|---|---|
| H-PWR-5V | 5V load backbone | Main 5V input to +5V_BUS load domain |
| H-BAT-RP | RP2040 battery harness | Main 3.7V battery to Feather BAT |
| H-BAT-NEO | NeoPixel battery + charger harness | Neo battery and external charger wiring |
| H-CORE | NeoPixel strip harness | Data and strip supply run |
| H-SW | Switch input harness | SW1/SW2/SW3 to controller inputs |
| H-SPARK | Spark output harness | RP2040 spark PWM channels to spark loads |
| H-SERVO-CTL | Servo control harness | I2C and PWM control to ServoWing |
| H-SERVO-PWR | Servo power harness | +5V/GND to ServoWing and servos |
| H-BEAM-CTL | Beam control harness | RP2040 control lines to Prop-Maker stage |
| H-BEAM-PWR | Beam/peltier load harness | +5V_BUS load-side runs |
| H-PUMP-OPT | Optional pump harness | Optional pump enable/load run |

## Harness Definitions

### H-PWR-5V - 5V Load Backbone
- Source: J801 (5V input)
- Path: Main fuse -> F801 -> +5V_BUS
- Loads: Servo FeatherWing V+, Prop-Maker load domain, optional pump/peltier load supply
- Recommended wire: 18 AWG trunk, 20 AWG branches
- Notes: Keep this harness physically separated from low-level signal bundles.

### H-BAT-RP - RP2040 Battery Harness
- Source: LP503562 cell #1
- Destination: Feather RP2040 BAT and GND
- Charging: Feather onboard charger path
- Recommended wire: 20 AWG
- Notes: Keep battery positive isolated from +5V_BUS and +3V7_NEO.

### H-BAT-NEO - NeoPixel Battery and Charger Harness
- Source: LP503562 cell #2
- Charger: U812 (Adafruit 259)
- Destination rail: +3V7_NEO
- Recommended wire: 20 AWG battery and rail runs
- Notes: U812 output must remain dedicated to this battery/rail pair only.

### H-CORE - NeoPixel Strip Harness
- Signal run: RP2040 GP11 -> strip DI
- Power run: +3V7_NEO -> strip +V
- Ground run: GND_COMMON -> strip GND
- Recommended wire: 24-26 AWG for data, 20 AWG for strip power/ground
- Notes: Keep data conductor away from high-current switching runs where possible.

### H-SW - Switch Input Harness
- SW1 -> RP2040 GP2
- SW2 -> RP2040 GP3
- SW3 -> RP2040 GP4
- Return: shared GND
- Recommended wire: 24-26 AWG
- Notes: Route this harness away from spark and servo power runs.

### H-SPARK - Spark Output Harness
- Channels: RP2040 GP5/GP6/GP9/GP10 (through series resistors) -> Spark 1-4
- Supply domain: RP2040_BAT
- Return: GND_COMMON
- Recommended wire: 22-24 AWG signal legs, 20 AWG shared feed/return where applicable
- Notes: Keep channel labeling fixed end-to-end to avoid cross-wiring.

### H-SERVO-CTL - Servo Control Harness
- RP2040 SDA/SCL -> ServoWing SDA/SCL
- PWM outputs: ServoWing CH0 -> Servo A signal, CH1 -> Servo B signal
- Recommended wire: 24-26 AWG
- Notes: Keep I2C pair short and routed together.

### H-SERVO-PWR - Servo Power Harness
- +5V_BUS -> ServoWing V+ and servo power pins
- GND_COMMON -> ServoWing/servo grounds
- Recommended wire: 20 AWG branches
- Notes: Match positive and return gauge on each servo branch.

### H-BEAM-CTL - Beam Control Harness
- RP2040 GP12/GP13/GP14 -> Prop-Maker LED control inputs
- Recommended wire: 24-26 AWG
- Notes: Control-only harness; no high-current load feed in this bundle.

### H-BEAM-PWR - Beam and Peltier Load Harness
- +5V_BUS -> Prop-Maker load domain and peltier load source path
- GND_COMMON returns from load stages
- Recommended wire: 20 AWG minimum
- Notes: Treat as high-current branch; keep runs short.

### H-PUMP-OPT - Optional Pump Harness
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
5. Verify H-SERVO-PWR polarity before connecting servos.
6. Verify U812 charger wiring only touches Neo battery rail.
