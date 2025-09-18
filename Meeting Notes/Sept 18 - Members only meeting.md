# Team Slide Review – Meeting Summary  
**Date:** September 18, 2025  

## Discussion Summary
- **Agenda & Slides:**  
  - Team will refine slides for the advisor meeting.  
  - PCB design remains the key in-progress task.  
  - Timeline may be too compressed; adjustments discussed.  

- **Power Subsystem & BMS:**  
  - USB PD modulino to be tested independently before merging into BMS.  
  - Clarification on BMS boost converter behavior and current delivery.  
  - Need strategy for handling current draw greater than one pin can support.  

- **Microcontroller Subsystem:**  
  - Schematic captured and PCB layout ongoing.  
  - Repeated retracing required.  
  - Stemma QT connector footprint/model still missing.  

- **RF Subsystem:**  
  - RF board size reduced and reviewed informally without issues.  
  - Power amplifier requires high current; baseline will be 3.3 V @ 2 A.  
  - Optional 5 V @ 2 A supply may be added later if feasible.  

- **Workflow & Timeline:**  
  - GitHub collaboration issues noted; team avoiding commits to prevent conflicts.  
  - Kanban board updated to reflect realistic progress.  
  - Advisor suggested slowing pace, though team aims to stay ambitious.  

- **Audio Subsystem:**  
  - Microphone and speaker schematic captured; PCB layout ongoing.  
  - Traces being revised; need to check I²S vs SPI overlap.  

---

## Action Items
- **Adrianne:**  
  - Continue MCU PCB refinement.  

- **Eric:**  
  - Update RF slides and add jumper for PA supply.  
  - Present adjusted timeline at advisor meeting.  
  - Provide Stemma QT footprint/model.
  - Adopt feature-branch workflow for GitHub to reduce conflicts?

- **Jairo:**  
  - Finalize USB PD and BMS placement in KiCad.  
  - Meet with Zack to confirm PD integration details.  

- **Zack:**  
  - Support Jairo with PD integration.  
  - Clarify safety/load switch design.  

- **Dominic:**  
  - Finalize audio subsystem schematic and PCB layout.  

- **All Team Members:**  
  - Hold GitHub commits until workflow improves.  
  - Continue weekly slide reviews.  
