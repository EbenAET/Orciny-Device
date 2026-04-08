# Orciny Board Placement Plan

This document turns the current Fusion starter package into a physical board-planning guide.

## Scope

This plan assumes a carrier/backplane style PCB that preserves the current firmware architecture:

- one Feather M0 for switches, core strip output, and serial command source
- one Feather RP2040 for FX execution
- a Servo FeatherWing function using PCA9685 on the RP2040 side
- a Prop-Maker LED output function on the RP2040 side

## Important Mechanical Constraint

The current firmware maps both the servo function and the Prop-Maker LED control to the RP2040 side.

That creates two viable physical implementations:

1. Module-stack approach:
   - RP2040 sits in one Feather footprint
   - a Feather doubler/interposer or equivalent second-wing support is required if both the Servo FeatherWing and Prop-Maker FeatherWing remain separate physical modules

2. Carrier-integration approach:
   - keep one physical FeatherWing footprint as the main module interface
   - place the PCA9685 and servo headers directly on the carrier PCB
   - route Prop-Maker control/load wiring through the remaining FeatherWing module or vice versa

For Fusion layout work, the recommended baseline is:

- use a two-controller carrier PCB
- treat the Servo FeatherWing function as integrated or doubler-dependent
- avoid assuming two full FeatherWings can occupy the same single Feather header footprint without explicit stacking hardware

## Recommended Floorplan

Use a left-to-right power-to-logic-to-load arrangement.

### Zone A: Power Entry and Protection

Preferred location:

- top-left corner or left edge

Place here:

- 5V power input connector
- main fuse
- 5V bus branch fuse
- filament-rail fuse
- 3V buck converter
- first large ground/power pours

Rules:

- keep the input connector, fuses, and buck converter tightly grouped
- keep high-current 5V and 3V rail stubs short before they branch out
- define the main star-ground anchor in this zone

### Zone B: High-Current Filament Drivers

Preferred location:

- left edge, below the power zone

Place here:

- spark and pulse MOSFET stages
- filament output headers/connectors
- 10 ohm series filament resistors

Rules:

- group all spark and pulse outputs together
- keep the `PSU_3V_FILAMENT` path wide and local to this region
- keep MOSFET drain/load routing away from MCU and I2C traces

### Zone C: RP2040 FX Controller Region

Preferred location:

- center-right area

Place here:

- Feather RP2040 module footprint
- serial link header/routing from M0
- I2C breakout to servo function
- PWM control fanout for sparks, pulse, and Prop-Maker LED control

Rules:

- orient the RP2040 so PWM-heavy pins face the high-current/output side
- keep the serial link to the M0 short and direct
- route I2C as a short, quiet pair before it enters the servo function area

### Zone D: M0 UI/Core Region

Preferred location:

- center-left or bottom-center, separated from the high-current load stages

Place here:

- Feather M0 module footprint
- switch connector headers or panel harness connector
- core NeoPixel connector

Rules:

- keep switch traces away from high-current switching loops
- place the core strip connector close to the M0 D13 side if possible
- keep the M0 physically closer to user I/O than to power switching hardware

### Zone E: Servo and LED Output Region

Preferred location:

- right edge of the board

Place here:

- servo A connector
- servo B connector
- 3-9W LED output connector
- optional pump connector
- Prop-Maker Wing footprint or integrated equivalent
- Servo FeatherWing footprint proxy or PCA9685 section

Rules:

- keep servo output connectors adjacent to the servo-control function
- keep the LED output connector adjacent to the Prop-Maker output side
- place optional pump connection at the far edge so it can be DNP without disturbing the main routing

## Connector Edge Plan

Recommended edge assignments:

- left edge: power input, fuse access, filament outputs
- right edge: servo A, servo B, 3-9W LED output, optional pump
- bottom edge: switch harness, NeoPixel/core harness, debug/service headers

This keeps:

- noisy load wiring away from user/control wiring
- service/debug access on one side
- load exits on predictable harness edges

## Recommended Connector Order

### Left edge, top to bottom

1. 5V input
2. fused 5V branch access if exposed
3. spark 1
4. spark 2
5. spark 3
6. spark 4
7. pulse filament

### Right edge, top to bottom

1. 3-9W LED output
2. servo A
3. servo B
4. optional pump

### Bottom edge, left to right

1. SW1
2. SW2
3. SW3
4. core strip connector
5. serial/debug header if used

## Routing Priorities

Priority 1:

- `PSU_5V_MAIN`
- `PSU_5V_BUS`
- `PSU_3V_FILAMENT`
- ground return paths for servo and LED loads

Priority 2:

- spark/pulse PWM control to driver gates
- Prop-Maker control lines
- servo PWM outputs from PCA9685 or wing channels

Priority 3:

- M0 to RP2040 serial link
- switch inputs
- core strip data line
- I2C SDA/SCL

## Placement Rules for Stacked Modules

If you keep off-the-shelf Feather-format modules instead of integrating functions onto the carrier:

1. Leave full outline keepouts for both Feather controllers.
2. Reserve vertical clearance for USB connectors and headers.
3. Do not place tall terminal blocks directly under overhanging Feather boards.
4. Keep the right edge clear enough for servo and LED harnesses to exit without crossing over controller boards.
5. If both RP2040-side wing functions remain discrete modules, explicitly reserve a doubler/interposer footprint area.

## Preferred First Fusion Placement Sequence

1. Place power input, fuses, and buck converter.
2. Place the RP2040 module.
3. Place the M0 module.
4. Place the servo function block and the Prop-Maker block on the RP2040 side.
5. Place all edge connectors.
6. Lock high-current connectors and power blocks first.
7. Route power and ground next.
8. Route gate/control signals next.
9. Route switch, UART, core data, and I2C last.

## Decision Checkpoint Before Layout Freeze

Before locking the board outline, confirm which of these is true:

1. Both RP2040-side wing functions remain as stacked Feather-format modules.
2. The PCA9685 servo function is absorbed into the carrier PCB and only one RP2040-side wing remains physical.
3. The RP2040-side wing functions are moved onto separate controller boards and the carrier becomes only a distribution/backplane PCB.

This choice changes mechanical keepouts more than it changes net names.