### **Week 11 (Nov 2 – Nov 8\) — Firmware Foundation & Prep**

**Goal:** Establish firmware structure, set up tests, and start board assembly.

**Tasks & Steps**

1. **Firmware Development**  
   * Create new PlatformIO or ESP-IDF project workspace.  
   * Confirm all libraries / drivers (ST7789, CC1200, MAX98357A, BQ25620, STUSB4500) are installed.  
   * Write simple compile-test firmware (e.g., blink \+ serial print) to confirm toolchain.  
   * Stub out task headers for FreeRTOS (e.g., `display_task.cpp`, `radio_task.cpp`, `audio_task.cpp`).  
2. **Test Setup**  
   * Prepare bench: power supply (3.3 V \+ 5 V), multimeter, logic analyzer, USB UART cable.  
   * Verify continuity between MCU pins and connectors on each bare PCB.  
   * Flash “hello world” to check USB programming path and serial output.  
3. **RTOS Task Structure / Prioritization**  
   * Draft initial task hierarchy: Radio \> Display \> Audio \> Control.  
   * Define priorities, stack sizes, and inter-task queues in a header file.  
   * Document this in `wiki/firmware_structure.md`.  
4. **Outline YouTube Video Structure**  
   * Decide on scenes: intro, soldering montage, testing demo, results.  
   * Assign team roles for filming segments.  
5. **Wiki / Documentation**  
   * Create folders (`/firmware`, `/hardware`, `/testing`).  
   * Start PCB bring-up guide.  
6. **Begin Board Assembly**  
   * Sort components by subsystem.  
   * Hand-solder passives and simple ICs.  
   * Record any issues for BOM revision.

**Deliverables:** Firmware skeleton \+ toolchain verified \+ RTOS layout draft \+ partial board assembly.

---

### **Week 12 (Nov 9 – Nov 15\) — Hardware Integration & Debugging (Demo Week ?)**

**Goal:** Assemble and power-on all boards; begin software integration.

**Tasks & Steps**

1. **Assemble Boards**  
   * Finish soldering critical ICs (ESP32, CC1200, BMS, STUSB4500).  
   * Connect all modulinos via EYESPI and I²C harnesses.  
   * Inspect solder joints under microscope; rework as needed.  
2. **Software Integration**  
   * Merge subsystem firmware modules into main project.  
   * Initialize I²C bus → scan and log addresses for each device.  
   * Verify SPI bus (TFT and CC1200) responds to read commands.  
3. **Debugging**  
   * Use serial logs to trace initialization sequence.  
   * Verify power consumption on each rail at boot.  
   * Add error LED blink codes for fault diagnosis.  
4. **Full System Tests**  
   * Power on with battery and USB PD.  
   * Confirm MCU communicates with expander, display, and RF.  
   * Record video of first boot for demo archive.

**Deliverables:** All subsystems assembled \+ successful first boot \+ baseline logs.

---

### **Week 13 (Nov 16 – Nov 22\) — Full System Debug & Documentation**

**Goal:** Run integrated tests, refine code, begin V2 PCB planning.

**Tasks & Steps**

1. **Full System Tests**  
   * Verify interrupt lines trigger (Expander INT, RF GDOs).  
   * Test audio I²S loopback or mic→speaker path.  
   * Confirm display refresh timing and frame buffer update.  
2. **Debugging & Optimization**  
   * Analyze task timing (jitter, stack usage) with FreeRTOS trace.  
   * Fix SPI timing issues or bus contention if present.  
   * Adjust pin pull-ups and debounce for inputs.  
3. **Record / Edit YouTube Video**  
   * Capture integration and testing footage.  
   * Overlay system diagrams and commentary.  
4. **Update Wiki / Documentation**  
   * Add debug logs and schematic updates.  
   * Upload pin-map tables and scope screenshots.  
5. **Begin PCB Redesign**  
   * Note assembly issues for next rev (pad sizes, trace routing).  
   * Create KiCad revision branch and mark changes.

**Deliverables:** Working prototype with verified data paths \+ draft V2 change list.

---

### **Week 14 (Nov 23 – Nov 29\) — Firmware Features & V2 Design**

**Goal:** Refine functionality and prepare next-rev PCB order.

**Tasks & Steps**

1. **Develop Firmware Features**  
   * Add menu UI for frequency/volume control.  
   * Implement PTT task with TX timeout.  
   * Add battery status display and charging indicator.  
2. **Troubleshoot Existing Issues**  
   * Re-test power management and charging.  
   * Fix display refresh bugs or audio distortion.  
   * Validate RF link budget (loopback range test).  
3. **PCB Redesign and Order**  
   * Finalize schematic changes (BOM cleanup, trace widths, connector labels).  
   * Run ERC/DRC → generate Gerbers.  
   * Submit V2 order (rush fabrication before Week 15).

**Deliverables:** Feature-complete firmware \+ V2 design files submitted.

---

### **Week 15 (Nov 30 – Dec 6\) — Validation & Educational Material**

**Goal:** Polish the demo, document the system, prepare for Expo.

**Tasks & Steps**

1. **Finalize Troubleshooting**  
   * Run system endurance tests (battery drain, PTT timeout, display stress).  
   * Measure noise floor and signal quality on RF and audio.  
2. **Build Tutorials & Educational Lab Materials**  
   * Write student-facing lab guides for each modulino.  
   * Create demo code snippets and diagrams for GitHub wiki.  
   * Record voice-over walkthrough of system architecture.  
3. **Assemble V2 Boards**  
   * If V2 PCBs arrive, hand-assemble and spot-test power only.  
4. **Finish Final Report**  
   * Summarize design choices, test data, and educational impact.  
   * Add cost analysis and future work section.

**Deliverables:** Stable demo \+ documentation package \+ final report draft.

---

### **Week 16 (Dec 7 – Dec 13\) — Expo Week / Final Presentation**

**Goal:** Finalize all deliverables and showcase the working prototype.

**Tasks & Steps**

1. **Finishing Touches**  
   * Polish enclosure and label modulinos.  
   * Prepare poster slides and demo script.  
   * Conduct rehearsal run (boot, demo, shutdown sequence).  
   * Backup firmware binaries and videos.  
   * Verify charging and runtime for Expo presentation.  
2. **Deliverables**  
   * Live demo unit ready for Expo booth.  
   * GitHub repo and wiki finalized (public release).  
   * Final report submitted.

   ---

