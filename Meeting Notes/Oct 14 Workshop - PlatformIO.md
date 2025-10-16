# OpenAir Lab Capstone – Team Meeting Summary  
*(Internal Team Workshop & Working Session)*  

## Overview  
This meeting functioned as a **hands-on workshop** introducing **PlatformIO** as the unified development and testing environment for the ESP32-based handheld radio system. The team also finalized **short-term firmware milestones**, clarified **testing expectations**, and planned **GitHub project management updates** before the upcoming advisor meeting with Dr. Tacca.

---

## Key Technical Topics  

### PlatformIO Setup & Structure  
- All members confirmed use of **Visual Studio Code + PlatformIO** for embedded development.  
- The **`platformio.ini`** configuration defines:
  - Platform: `espressif32`
  - Framework: `Arduino` (built atop FreeRTOS)
  - Dependencies: Adafruit and other device-specific libraries.  
- **Project directory layout**:
  - `src/` – main source files (e.g., `main.cpp`)  
  - `lib/` – libraries per device (e.g., Adafruit display driver)  
  - `test/` – optional unit tests for future use  

### Library Integration  
- Libraries can be located via the **PlatformIO Registry** or **GitHub** by device part number.  
- Preference for **C++ libraries** (C acceptable); avoid other languages.  
- Members should select libraries that include **demo/example code**.  
- Demo code from **Adafruit** was used as an example for the **HMI display driver**, demonstrating setup, initialization, and drawing operations.  

### Coding Conventions  
- Each subsystem demo will use its own header and source pair for clarity.  
- `setup()` function initializes hardware; `loop()` remains empty under FreeRTOS.  
- Individual demos should be toggle-enabled via comments or conditional statements.  
- Team consensus: stick to simple, functional code first—refactoring and cleanup can come later.  

---

## Firmware Development Expectations  

- Each subsystem owner (MCU, BMS, PSU, Audio, RF) must:
  1. Identify demo code for their devices.  
  2. Implement a working test that proves communication (e.g., “read part number,” “draw text,” “negotiate PD profile”).  
  3. Integrate with the shared **testbed firmware** on ESP32.  
- **Goal:** unified, compilable demos for all modules by **Monday** (≈ 6 days from meeting).  
- Purpose: detect integration issues before PCB assemblies arrive (~10 days lead time).  

---

## Project Management & Deliverables  

### Immediate Action Items  
1. **GitHub Organization**
   - Update issues, tasks, and project board.  
   - Re-activate status tracking and due dates.  
   - Move reviewed items forward; create new subtasks for demos.  
2. **Firmware Branching**
   - Each member to clone the `system-testbed` repository and create their own **firmware branch** (e.g., `testbed-mcu-branch`).  
   - Use `platformio.ini` from system testbed; ensure compatibility across modules.  
3. **Wiki & Documentation**
   - Begin populating the **GitHub Wiki** and **README.md** with project overview, module descriptions, and demo procedures.  
   - Adrianne to post a **week-by-week roadmap** to the repo README.  
4. **Educational Outreach**
   - Zach created the team’s **YouTube channel**.  
   - Decide video structure, filming roles, and initial topics (charging test, radio communication, audio playback).  
   - Produce 2–3 example videos plus a roadmap for future educational content.  

---

## Coordination & Upcoming Milestones  

| Date | Milestone | Description |
|------|------------|-------------|
| **Mon (≈ 6 days)** | Firmware demo deadline | Each subsystem submits runnable demo & testable function. |
| **Thu** | Team meeting | Review slides and project documentation. |
| **Fri** | Advisor meeting (Dr. Tacca) | Present firmware progress, testing plan, and GitHub updates. |
| **Following week** | PCB assembly | Begin hardware integration testing using demo firmware. |

---

## Discussion Highlights  
- Emphasis on **collaborative accountability**: every member commits to tangible outputs before deadlines.  
- **Testing first, debugging second:** start from known-working demo code to isolate hardware or interface issues.  
- **Educational focus:** record demonstrations and develop materials to ensure reproducibility for future students.  
- **Long-term repository plan:** finalized GitHub layout, structured README, and cross-linked Wiki pages summarizing major report sections (Preliminary Design → Solution → Test Plan → Implementation → Instructions → Appendices).  

---

## Next Steps Summary  
- Complete and commit subsystem demo code.  
- Update GitHub project tasks and documentation.  
- Prepare slide content for advisor review.  
- Record or outline initial educational demos.  
- Continue using PlatformIO as the unified build environment.  

---

**End of Meeting Summary**
