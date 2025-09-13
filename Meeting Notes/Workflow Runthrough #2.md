# Team Workflow Runthrough #2 – Summary

## Purpose of Session
- Second workshop in the semester-long workflow training.
- Focus shifted from general tools (**GitHub, VS Code, KiCad basics**) to **advanced KiCad usage**.  
- Goal: build team familiarity with **symbols, footprints, and libraries** in KiCad while tying the work back to GitHub project management.

---

## Key Topics Covered

### 1. Component Libraries
- Two main types: **Symbol libraries** (schematic representation) and **Footprint libraries** (physical board layout).
- Custom project libraries allow the team to store parts not included in KiCad’s defaults.
- Example: Creating and editing an **Adafruit FPC connector** and an **RF power amplifier** symbol with defined properties (footprint, datasheet, description).

### 2. Symbol Editor
- Used more often than schematic editor for customizing or creating new parts.
- Benefits of custom symbols:
  - Properties like datasheet links and pin descriptions directly accessible.
  - Cleaner, standardized schematics across the team.
- Demonstration: creating a **24-bit port expander symbol** when only a 16-bit version was in the default libraries.

### 3. Footprint Editor
- Essential for ensuring correct package types (e.g., **QFN vs VQFN**).
- Process shown for:
  - Importing manufacturer-provided footprints.
  - Validating footprint dimensions against datasheets.
  - Associating footprints with symbols correctly within the project library.

### 4. Importing from External Sources
- Demonstrated downloading from Mouser/Component Engine.
- Emphasis: verify imported files (symbols, footprints, 3D models) because external sources may contain errors.
- Best practice: **import into the project library** (not local-only) so all team members can access parts across machines.

### 5. 3D Models
- How to add and link **STEP files** for visualization.
- Importance of correct directory setup for portability across team systems.

### 6. Practical Workflow Demonstration
- Step-by-step:  
  1. Identifying a missing part.  
  2. Finding datasheet.  
  3. Downloading ECAD model.  
  4. Importing symbol & footprint into KiCad.  
  5. Linking footprint.  
  6. Updating PCB.  
- Example conversion: swapping the **16-bit port expander** for a **24-bit version** in both schematic and PCB.

---

## Lessons Learned
- **Most time will be spent** in the symbol and footprint editors, not just schematic capture.
- **Consistency and portability**: importing into the shared project library avoids “it works on my computer” issues.
- **Validation is critical**: always compare footprints against datasheets before committing to a design.
- **Efficiency**: leverage existing ECAD models to save time but confirm accuracy.

---

## Next Steps
- Afternoon session (4 PM) will cover:
  - **PCB Editor** in detail.
  - **Calculator tool** for vias, regulators, resistor values, etc.
  - **Plugins** for KiCad to improve workflow.
- Continued practice with symbol and footprint creation to reinforce today’s demo.
