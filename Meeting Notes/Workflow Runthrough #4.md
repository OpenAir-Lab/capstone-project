# Team Workflow Runthrough #4 – Summary

## Purpose of Session
- Fourth workshop in the series.  
- Focus: **advanced PCB layout practices** in KiCad.  
- Goals:  
  - Teach proper **layout and routing techniques**.  
  - Emphasize **design rules, constraints, and manufacturability**.  
  - Begin preparing subsystem Modulinos for integration.

---

## Key Topics Covered

### 1. Design Rule Setup
- **Design Rules Editor** used to define:  
  - Trace widths.  
  - Via sizes.  
  - Clearance between nets.  
- Showed how to set **net classes** for power vs signal lines.  
- Example: thicker traces for high-current lines, standard widths for logic.

### 2. PCB Constraints
- Importance of **clearances**: prevents shorts and ensures manufacturability.  
- Discussed **annular ring requirements** for vias.  
- **Board house specs** must be referenced before finalizing values.

### 3. Layout Strategies
- **Top vs bottom layer usage**:  
  - One for signals, one for ground/power planes when possible.  
- **Ground pour**: explained why it reduces noise and provides shielding.  
- Demonstrated using **zones** in KiCad to create copper pours.

### 4. Routing Techniques
- Taught best practices for routing:  
  - 45° angles (avoid 90° corners).  
  - Shortest path for high-speed signals.  
  - Keep analog and digital traces separate.  
- Highlighted **differential pair routing** for high-frequency signals.  
- Showed how to use **highlight net** feature for tracking specific signals.

### 5. Placement Considerations
- Start with **connectors** and **critical components** (e.g., crystals, antennas).  
- Place **decoupling capacitors** close to IC power pins.  
- Maintain symmetry and logical flow for easier debugging.  
- Example: placing MCU near center, connectors on edges.

### 6. Modulino Subsystem Progress
- Teams began rough layouts for their assigned Modulinos (power, RF, HMI).  
- Guidance on **keeping consistent dimensions** for modularity.  
- Mounting hole alignment reinforced as mandatory.  
- Started applying **project-wide constraints** for consistency.

### 7. Common Pitfalls
- Don’t ignore DRC (Design Rule Check) warnings.  
- Avoid long, wandering traces that introduce noise.  
- Watch for unconnected pads after imports or updates.  
- Ensure polygons/zones are refilled after edits.

---

## Lessons Learned
- **Design rules drive manufacturability**: always configure before routing.  
- **Ground pours are critical** for stability and noise reduction.  
- **Placement is just as important as routing**: start with big-picture layout.  
- Modulinos must remain **standardized** so subsystems interconnect seamlessly.  

---

## Practical Outcomes
- Each participant:  
  - Configured **design rules** in KiCad.  
  - Placed preliminary components on their Modulino boards.  
  - Practiced routing basics with proper angles and clearances.  
  - Created at least one **ground pour**.

---

## Next Steps
- Continue refining Modulino layouts with proper net classes.  
- Apply consistent **mounting hole + connector standards**.  
- Tuesday session: focus on **multi-layer design** and **power distribution networks**.  
- Teams should bring updated Modulino PCBs for review.

