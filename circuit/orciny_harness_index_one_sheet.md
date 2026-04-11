# Orciny Harness Index (One Sheet)

Quick-reference index for harness identification during build and troubleshooting.

| Label | Harness ID | Primary Connector | From | To | Notes |
|---|---|---|---|---|---|
| 1 | H-PWR-5V | Header / terminal (project-defined) | J801 / +5V input | +5V_BUS branches | 18 AWG trunk, 20 AWG branches |
| 2 | H-BAT-NEO | 2-pin JST | LP503562 #2 + U812 output | +3V7_NEO rail | red=+, black=-; 500-1000 uF bulk cap at strip entry |
| 3 | H-CORE | 3-pin JST HX (NeoPixel side); splits from same 4-pin cable as H-BAT-NEO | RP2040 GP25, +5V_BUS, GND_COMMON | NeoPixel strip DI, +V, GND | Pin 1 red=+V, pin 2 white=signal, pin 3 black+brown=GND; cap across pins 1-3 |
| 4 | H-SW | 4-pin JST | SW1/SW2/SW3, GND | RP2040 A1/A2/A3 (GP27/GP28/GP29), GND | Pin 1 blue=SW1, pin 2 brown=SW2, pin 3 orange=SW3, pin 4 green=GND |
| 5 | H-SPARK | 5-pin DMX cable | RP2040 GP18/GP19/GP20/GP24, GND | Spark channels 1-4 (+ spare) | green=GND; pin 1 red=SP1, pin 2 brown=SP2, pin 3 white=SP3, pin 4 blue=SP4, pin 5 black=spare |
| 6 | H-SERVO | 4-pin JST | ServoWing outputs/power | Servo A/B signal + power branches | Pin 1 white=CH0 sig, pin 2 blue=CH1 sig, pin 3 red=PWR, pin 4 black=GND |
| 7 | H-BEAM-PWR | 4-pin JST (common anode RGB LED) | +5V_BUS, GND_COMMON | Beam load path | Pin 1 white=anode, pin 2 red=R(GP11), pin 3 blue=B(GP13), pin 4 green=G(GP12); GP10 reserved for Prop-Maker PWR enable |
| 8 | H-PELTIER-PWR | Deans Micro2R | +5V_BUS, GND_COMMON | Peltier load path | Pin 1 red=PWR, pin 2 black=GND; GP8->Q9 gate; firmware ON with beam + short cooldown hold |
| 9 | H-PUMP-OPT | Project-defined load/control connector | GPIO unassigned (this revision), +5V_BUS, GND_COMMON | Optional pump driver + load branch | GP8 reassigned to peltier; optional population |
| 10 | H-BAT-RP | Feather battery JST | LP503562 #1 | Feather RP2040 BAT/GND | Direct battery harness; Feather onboard charger path |

## Rail Separation Reminder

- Keep +5V_BUS, RP2040_BAT, and +3V7_NEO isolated from each other.
- Only tie grounds at GND_COMMON.
- Neo battery charger U812 remains dedicated to the Neo rail battery only.
