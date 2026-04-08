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
| H-BEAM-PWR | Beam/peltier load harness | +5V_BUS load-side runs to Prop-Maker and peltier path | Prop-Maker FeatherWing stacks on the Feather; harness is load-side wiring only. |


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
- Connector note: Battery connects directly to the Feather Battery JST port.

### H-BAT-NEO - NeoPixel Battery and Charger Harness
- Source: LP503562 cell #2
- Charger: U812 (Adafruit 259)
- Destination rail: +3V7_NEO
- Recommended wire: 20 AWG battery and rail runs
- Notes: U812 output must remain dedicated to this battery/rail pair only.
- Connector note: Use a 2-pin JST for charger power, and a separate 2-pin JST for the controller signal/ground run.
- Capacity note: Running the strip from 3.7V still requires the harness to carry strip current; size connector and wire for at least the 2A strip budget used in project docs, with margin for any brightness or pixel-count increase.

### H-CORE - NeoPixel Strip Harness
- Signal run: RP2040 GP11 -> strip DI
- Power run: +3V7_NEO -> strip +V
- Ground run: GND_COMMON -> strip GND
- Recommended wire: 24-26 AWG for data, 20 AWG for strip power/ground
- Notes: Keep data conductor away from high-current switching runs where possible.
- Capacity note: On a 3.7V rail there is less voltage-drop headroom than at 5V, so keep +V/GND path resistance low and maintain the same 2A minimum current capacity in this run.

### H-SW - Switch Input Harness
- SW1 -> RP2040 GP2
- SW2 -> RP2040 GP3
- SW3 -> RP2040 GP4
- Return: shared GND
- Recommended wire: 24-26 AWG
- Notes: Route this harness away from spark and servo power runs.
- Connector note: Use a 4-pin JST connector.

### H-SPARK - Spark Output Harness
- Channels: RP2040 GP5/GP6/GP9/GP10 (through series resistors) -> Spark 1-4
- Supply domain: RP2040_BAT
- Return: GND_COMMON
- Recommended wire: 22-24 AWG signal legs, 20 AWG shared feed/return where applicable
- Notes: Keep channel labeling fixed end-to-end to avoid cross-wiring.
- Connector note: Use a 5-pin DMX cable.

### H-SERVO - Servo Output Harness
- Stack note: ServoWing stacks directly on the Feather RP2040; SDA/SCL, control, and wing power are handled by the stack headers.
- Output runs: ServoWing CH0 -> Servo A signal, CH1 -> Servo B signal
- Power runs: ServoWing V+ and GND -> servo power branches
- Recommended wire: 24-26 AWG for signal, 20 AWG for power/ground branches
- Notes: Match positive and return gauge on each servo branch; no separate Feather-to-ServoWing harness is required.
- Connector note: Use a 4-pin JST connector.

### H-BEAM-PWR - Beam and Peltier Load Harness
- Stack note: Prop-Maker FeatherWing stacks directly on the Feather RP2040; no separate controller-to-wing harness is required.
- +5V_BUS -> Prop-Maker load domain and peltier load source path
- GND_COMMON returns from load stages
- Recommended wire: 20 AWG minimum
- Notes: Treat as high-current branch; keep runs short and consider this harness load-side only.
- Connector note: Beam uses a 4-pin JST connector; peltier uses a Deans Micro2R connector.

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
5. Verify H-SERVO polarity before connecting servos.
6. Verify U812 charger wiring only touches Neo battery rail.
