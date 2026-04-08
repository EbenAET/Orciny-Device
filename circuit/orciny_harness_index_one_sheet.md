# Orciny Harness Index (One Sheet)

Quick-reference index for harness identification during build and troubleshooting.

| Label | Harness ID | Primary Connector | From | To | Notes |
|---|---|---|---|---|---|
| 1 | H-PWR-5V | Header / terminal (project-defined) | J801 / +5V input | +5V_BUS branches | 18 AWG trunk, 20 AWG branches |
| 2 | H-BAT-NEO | 2-pin JST | LP503562 #2 + U812 output | +3V7_NEO rail | Dedicated Neo battery/charger pair |
| 3 | H-CORE | 2-pin JST (signal+GND) plus strip power pair | RP2040 GP11, +3V7_NEO, GND_COMMON | NeoPixel strip DI, +V, GND | Keep low drop on +V/GND pair |
| 4 | H-SW | 4-pin JST | SW1/SW2/SW3, GND | RP2040 GP2/GP3/GP4, GND | Low-noise routing preferred |
| 5 | H-SPARK | 5-pin DMX cable | RP2040 GP5/GP6/GP9/GP10, GND | Spark channels 1-4 (+ spare) | Pin map fixed end-to-end |
| 6 | H-SERVO | 4-pin JST | ServoWing outputs/power | Servo A/B signal + power branches | ServoWing stacks on Feather |
| 7 | H-BEAM-PWR | 4-pin JST | +5V_BUS, GND_COMMON | Beam load path | High-current; beam load only |
| 8 | H-PELTIER-PWR | Deans Micro2R | +5V_BUS, GND_COMMON | Peltier load path | Separate high-current branch |
| 9 | H-PUMP-OPT | Project-defined load/control connector | RP2040 GP8, +5V_BUS, GND_COMMON | Optional pump driver + load branch | Optional population |
| 10 | H-BAT-RP | Feather battery JST | LP503562 #1 | Feather RP2040 BAT/GND | Direct battery harness; Feather onboard charger path |

## Rail Separation Reminder

- Keep +5V_BUS, RP2040_BAT, and +3V7_NEO isolated from each other.
- Only tie grounds at GND_COMMON.
- Neo battery charger U812 remains dedicated to the Neo rail battery only.
