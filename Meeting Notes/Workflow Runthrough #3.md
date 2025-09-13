# Team Workflow Runthrough #3 – Summary

## Purpose of Session
- Third workshop in the workflow series.  
- Focus: **PCB Editor in KiCad** and building a **Modulino board** template.  
- Goal: ensure everyone can import, edit, and customize PCB designs while reinforcing schematic-to-board workflows.

---

## Key Topics Covered

### 1. Plugins & Tools
- **IBOM (Interactive Bill of Materials)** plugin:  
  - Generates a live, clickable BOM once footprints are assigned.  
  - Helps automate part documentation early in the design.  
- **StepUp/Blender 3D export** plugin: not used now, but useful for final presentations.  
- Non-recommended: outdated “free router” plugin.

### 2. Schematic vs PCB Editor
- **Similarities:** both use a “sheet” concept.  
- **Differences:** schematics allow subsheets; PCBs are one-shot (all blocks import together).  
- Workflow: always **start from schematic → update PCB (F8)**, not the reverse.

### 3. Importing External Designs
- Demonstrated importing an **Arduino Modulino board** from Altium files.  
- Conversion steps:  
  1. Download raw Altium CAD files.  
  2. Import via “File → Import Non-KiCad Board.”  
  3. Auto-match layers → KiCad generates editable PCB + 3D model.  
- Limitation: imported parts are “ghost parts” (not native KiCad symbols/footprints). They break when syncing schematic ↔ PCB.

### 4. Making Modulino Boards Usable
- To customize Modulinos:  
  - Strip away unnecessary layers/tracks using **selection filters**.  
  - Preserve only what’s useful (board outline, zones, key graphics).  
  - Refresh PCB from schematic to remove ghost parts.  
- Resizing boards: must be done manually using **edge.cuts layer**.  
- Mounting holes:  
  - Created by selecting proper footprints (e.g., ISO-standard 3.2 mm).  
  - Must also be tied to schematic symbols to persist across updates.  
- Demonstrated workflow:  
  1. Add mounting hole symbols in schematic.  
  2. Assign footprints.  
  3. Update PCB → holes appear and align correctly.

### 5. Breadboard & Modularity Constraints
- Modulinos satisfy **breadboard compatibility** and **modularity**, which became critical design standards after the expo.  
- Headers and subsystem pinouts will be decided later, once requirements stabilize.

### 6. Integration with Connectors
- Example: **STEMMA QT (JST) connectors** added through schematic editor.  
- Process: choose symbol → assign matching footprint → sync with PCB.  
- Reinforces rule: all parts must exist in the schematic first.

### 7. Core Lessons
- **Schematics anchor the design**. PCB files are outputs, not the source of truth.  
- Importing external boards is a **kickstart**, not a substitute for proper schematic/footprint linkage.  
- **Edge.cuts layer is critical** for defining valid board outlines.  
- Collaboration requires native KiCad symbols/footprints to ensure portability across machines.

---

## Practical Outcomes
- Each participant created a **blank Modulino PCB** with:  
  - Correct mounting holes.  
  - Cleaned outline ready for subsystem-specific components.  
  - Knowledge of resizing and connector placement.

---

## Next Steps
- Continue Modulino recapture for subsystems (power, RF, HMI).  
- Explore **manual resizing techniques** and board outline best practices.  
- Prepare for Tuesday session to add headers, connectors, and subsystem layouts.  
- Use IBOM plugin to start building project-wide Bill of Materials.

