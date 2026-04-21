# Orciny Device Animation Timeline

**Version:** V 0.6.5  
**Board:** Adafruit Feather RP2040  
**Wings:** Prop-Maker FeatherWing, 8-Channel Servo FeatherWing (PCA9685)

---

## Animation Sequence Timeline

### State 0: INACTIVE (Sequence 0)
- **0s+**: Sparse random single sparks every 3–5 seconds
- **Beam & NeoPixels:** Off

---

### State 1: BOOT UP (Sequence 1)
- **0–4s**: Sparse sparks; cyan chases (slow to fast)
- **4–10s**: Sparks continue; cyan chases ramp up
- **10–20s**: Orange chases begin; pincers go rigid
- **20–24s**: Core pulses cyan/orange; beam fades to golden
- **24–28s**: Servo 1 oscillates; both servos go limp at 28s
- **28s+**: Transition to Demonstrate state

---

### State 2: DEMONSTRATE (Sequence 2)
- **0s**: Core flashes cyan/orange; beam brightens to orange (90%)
- **0.5s**: Beam fades to teal; both servos rigid
- **1s**: Fluid pump and pulse filament start turning on and off at random intervals
- **1.5s**: Servos random movement (slow, then frantic)
- **2s**: Sparks start; magenta chases on core
- **3–7s**: Beam pulses teal/orange with accelerating frequency
- **7s**: Core all-off; beam turns magenta (30%)
- **7.25s**: Beam magenta fades to full; pincers freeze
- **9.5s**: Pincers freeze up
- **10.75s**: Transition to Device Failure state

---

### State 3: DEVICE FAILURE (Sequence 3)
- **0s**: Random twitching servos; sparks increase in frequency; core rapid flashes all colors
- **0–4s**: Beam flashes teal/magenta/orange
- **1.75s**: Core → bright white
- **2s**: Core fades out; beam all colors full
- **2.5s**: Beam fades out; pincers go limp
- **3s**: Fluid pump seizes, pulse filament to full
- **3.5s**: Pincers go limp
- **4s+**: Everything off, output disabled


---

## Notes
- **Physical Controls:**
  - SW1: Toggle all outputs on/off
  - SW2: Step backward through states
  - SW3: Step forward through states
  - SW1 + SW3 held 5s: Reset to State 1, outputs off
- **See** `Orciny_Device.ino` for detailed implementation.
