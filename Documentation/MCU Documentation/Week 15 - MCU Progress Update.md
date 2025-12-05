# MCU Subsystem – Progress Report  
**Most Updated Version**

This document summarizes the current state of the MCU subsystem firmware for the OpenAir Lab project. It reflects the most up-to-date progress on architecture, firmware organization, subsystem integration, and demo readiness. All tasks are written with checkboxes to track completion.

---

## ✔ Current Status Overview

The last several days have focused on restructuring the firmware into a clean FreeRTOS-based architecture, integrating HMI inputs, display rendering, RF state handling, and preparing for the full-system vertical slice required for the expo demo.

Below is the structured progress as of now.

---

## ✔ Completed Work

### ### 1. FreeRTOS Architecture Migration
- [x] Identified blocking behavior in `setup()`  
- [x] Designed a proper task-based architecture  
- [x] Added `systemTask` (inputs → update state)  
- [x] Added `displayTask` (state → ST7789 UI)  
- [x] Added `rfTask` (PTT/TX/RX switching + timeout)  
- [x] Established data flow: Inputs → State → RF + UI  

### ### 2. SystemState Implementation
- [x] Created `SystemState` struct with all required fields (freq, TX, RSSI, PTT, battery)  
- [x] Added `system_state.h` and `system_state.cpp`  
- [x] Implemented `debug_print_state()` for live serial inspection  
- [x] Integrated state initialization into `setup()`  

### ### 3. Repo-Ready Firmware Folder Structure
- [x] Created modular directory structure:

```
src/

    main.cpp

    core/

        system_state.h

        system_state.cpp

    ui/

        ui_display.h

        ui_display.cpp

    inputs/

        input_hmi.h

        input_hmi.cpp

    tasks/

        task_system.h / task_system.cpp

        task_display.h / task_display.cpp

        task_rf.h / task_rf.cpp
```


- [x] Ensured it supports both PlatformIO + Arduino core  
- [x] Ensured all includes map cleanly  

### ### 4. ST7789 Display Layer
- [x] Built initial UI layout drawing functions  
- [x] Added dynamic UI updater functions:
  - `ui_update_frequency()`  
  - `ui_update_rssi()`  
  - `ui_update_battery()`  
  - `ui_update_tx_indicator()`  
- [x] Added `ui_init()` and static UI layout  
- [x] Designed non-flickering refresh loop in `task_display`  

### ### 5. PCF8575 Input Layer (HMI)
- [x] Added encoder decoding (A/B quadrature)  
- [x] Added PTT input function  
- [x] Wrapped input logic into `input_hmi.*`  
- [x] Integrated into `task_system` with frequency stepping  
- [x] Added TODO markers so no hardware assumptions are made incorrectly  

### ### 6. RF Task Logic
- [x] Drafted RF state machine:
  - TX entry  
  - TX timeout  
  - RX re-entry  
- [x] Added safe PA + RF switch sequencing placeholders  
- [x] Added CC1200 RX/TX call placeholders  
- [x] Integrated PTT decision logic  
- [x] Integrated RSSI polling placeholder  

### ### 7. Demo Contract + 12-Day Integration Plan
- [x] Wrote complete expo demo contract  
- [x] Established minimal vertical slice behavior  
- [x] Wrote 12-day integration schedule  
- [x] Froze scope to features required for expo demo only  

### ### 8. ESP32-WROVER-E Bootloader Understanding
- [x] Verified that ESP32 requires a bootloader at `0x1000`  
- [x] Confirmed PlatformIO auto-manages bootloader flashing  
- [x] Documented required hardware wiring on custom PCB  
- [x] Added esptool + USB-UART references to project memory  

---

# 🛠 TODO – What’s Left to Complete

These items must be completed to finish the vertical-slice demo and full MCU subsystem integration.

### ## A. Hardware Pin Mapping
- [ ] Confirm actual PCF8575 HMI pin assignments:
  - [ ] Encoder A  
  - [ ] Encoder B  
  - [ ] PTT pin  
  - [ ] Button polarity (active-high vs active-low)  
- [ ] Confirm RF control pin assignments map to driver API  
- [ ] Confirm CC1200 pins (MOSI, MISO, SCLK, CS, GDO0, GDO2) match firmware  

---

### ## B. RF Driver Integration
- [ ] Replace placeholder calls in `enter_tx_mode()` with real driver functions  
- [ ] Replace placeholder calls in `enter_rx_mode()` with real driver functions  
- [ ] Implement CC1200 RSSI read function  
- [ ] Confirm PA enable/disable calls for GRF5604  
- [ ] Confirm TRX routing with SKY13330  

---

### ## C. Display Improvements
- [ ] Adjust ST7789 rotation to match final PCB orientation  
- [ ] Tune the UI layout coordinates (cursor positions, text size, spacing)  
- [ ] Add a banner or splash screen if desired for expo  

---

### ## D. Power / BMS Integration
- [ ] Implement battery polling via BMS modulino  
- [ ] Add battery updates inside `task_system` or a new `task_power`  
- [ ] Verify I2C address and pullups for BMS board  
- [ ] Update `state.battery_percent` from real data  

---

### ## E. Input Polishing
- [ ] Add encoder debouncing if jitter appears  
- [ ] Add frequency bounds (min/max frequencies)  
- [ ] Possibly add button press/hold logic for future menu navigation  

---

### ## F. System Integration Tests
- [ ] Verify: power on → UI loads consistently  
- [ ] Verify: encoder changes frequency smoothly  
- [ ] Verify: RSSI updates live in RX  
- [ ] Verify: PTT → TX transition  
- [ ] Verify: TX timeout behavior  
- [ ] Verify: return to RX is stable  
- [ ] Verify: BMS reporting updates  
- [ ] Verify: entire system runs for 2 hours without reset  

---

### ## G. Expo-Ready Polish
- [ ] Create a stable “demo mode” screen  
- [ ] Finalize the demo sequence script  
- [ ] Run 5 full demo rehearsals  
- [ ] Prepare documentation and quick reference card  

---

# ✔ Final Notes

This `progress.md` reflects **the latest and most accurate status** of the MCU subsystem.  
It consolidates all architectural changes, implementation progress, and remaining work required to reach a fully functional handheld radio demo for expo.

Updates should be appended here as each task is completed.

---
