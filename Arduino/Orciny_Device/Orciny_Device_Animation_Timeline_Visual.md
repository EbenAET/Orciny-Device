# Orciny Device Animation Timeline — Visual Overview

**Version:** V 0.7.0

---

## Timeline Graph (ASCII)

```
Time (s) →   0      1      2      3      4      5      6      7      8      9     10    11

STATE 0: INACTIVE
[========= INACTIVE: Sparse sparks, all outputs off ==========]

STATE 1: BOOT UP
         |<------------------- Boot Up Sequence ------------------->|
         |  Sparks, Chases, Pincers, Beam, Servos (see details)     |

STATE 2: DEMONSTRATE
                |<------------------- DEMONSTRATE ------------------->|
                | Core, Beam, Servos, Sparks, NeoPixels, Pump/Filament |
                |                                                     |
                |  Pump/Filament:   [==== ON ====]                    |
                |  (1s)            (10.75s)                           |
                |                                                     |
                |  Beam/Neo:       [==== Effects & Color Changes ====]|
                |  Servos:         [==== Rigid → Random → Freeze ====]|
                |  Sparks:         [==== Flicker ====]                |

STATE 3: DEVICE FAILURE
                                             |<--- FAILURE ---->|
                                             |                  |
                                             | Pump/Filament:   [=== ON ===] (0–3s)
                                             |                  [= HOLD =]  (3–4s)
                                             |                  [= OFF =]   (4s+)
                                             | Beam/Neo:        [= Flash/Fade/Off =]
                                             | Servos:          [= Twitch → Limp =]

Legend:
- [====]  = Active effect
- |...|   = State duration
- Time axis is approximate and not to scale
```

---

## Mermaid Gantt Diagram (for compatible viewers)

```mermaid
gantt
title Orciny Device Animation Timeline
dateFormat  s
section INACTIVE
Inactive           :a1, 0, 1
section BOOT UP
Boot Up Sequence   :a2, 1, 27
section DEMONSTRATE
Demonstrate        :a3, 28, 10.75
Pump/Filament      :active, 29, 9.75
section FAILURE
Failure Sequence   :a4, 38.75, 4
Pump/Filament      :active, 38.75, 3
Filament Hold      :hold, 41.75, 1
Pump/Filament Off  :off, 42.75, 1
```

---

**See the main Animation Timeline document for detailed descriptions.**
