# OpenAir Lab – Progress.md  
**Most up-to-date status of MCU subsystem development and overall integration work**  
*(Updated as of current working session — this file should be revised regularly as tasks are completed.)*

---

# Completed Work

## **1. MCU Modulino Hardware (ESP32-WROVER-E)**
- Designed and assembled **Version 1 MCU PCB**  
- Soldered:
  - ESP32-WROVER-E module  
  - Enable & Boot buttons  
  - All passive components  
  - Customized EYESPI connectors for: Power, HMI, Audio, RF  
  - 4-pin I²C header  
- Performed early bring-up on V1 board:
  - Verified power rails  
  - Verified EN/BOOT button behavior  
  - Verified board boots when powered via EYESPI breakout  
  - Confirmed UART enumeration with ESP-PROG  

---

## **2. Pin Mapping**
- Completed full MCU vs Expander pin map (V1)  
  - High-speed buses (SPI, I²S) mapped to MCU  
  - Interrupt-driven lines mapped appropriately  
  - Strap-pin constraints respected  
- Converted mapping into **structured firmware-ready `pinmap.h`**

---

## **3. Firmware Bring-Up & Testing**
### Early Functional Tests
- UART connectivity tested with ESP-PROG  
- I²C scanning and communication tests built  
- SPI loopback test implemented  
- Interrupt test stubs created  
- PSRAM troubleshooting started

### Firmware structure planning
- FreeRTOS task layout defined:  
  - Radio Task  
  - HMI Task  
  - Power Management Task  
  - Logging / Telemetry Task  
- Non-blocking programming patterns drafted  
- SPI + I²C driver initialization templates complete  
- Expanded `main.cpp` scaffolding for:
  - I²C scan  
  - SPI loopback  
  - Interrupt test mode  

---

## **4. Modulino Subsystem Work**
### **RF Modulino**
- Pinout finalized for CC1200 radio  
- SPI + GDO0/GDO2 interrupt lines established  
- Reset line placed on I²C expander  

### **HMI Modulino**
- EYESPI TFT (ST7789) connection validated  
- Pin mapping for CS, DC, BL_EN, RST completed  
- TE optional sync line mapped to input-only MCU pin  

### **Audio Modulino**
- I²S mapping for MEMS mic + MAX98357A finalized  
- Confirmed mic on GPIO34 (input-only)  
- Amp SD/GAIN moved to I²C expander  

### **Power Modulino**
- ✔ Charger IC changed from **BQ25620 → BQ25622**  
- STUSB4500 USB-PD controller integrated  
- Fuel gauge alert line moved to expander  
- Power-path rules updated  

---

## **5. Documentation**
- Project README draft started (template based)  
- Abstracts for Expo audience written  
- MCU concept overview and technical explanations documented  
- Pin map documentation converted to a compiled PDF  
- Full repository structure planned (hardware/firmware/docs/etc.)  

---

# In Progress

## **1. Bootloader flashing & UART troubleshooting**
- ESP-PROG UART interface validated, but  
  - Custom board still shows connection errors  
  - Troubleshooting underway using Espressif boot mode references  
- Next step: verify EN/BOOT timing, ensure clean wiring, test with slower baud rate

## **2. MCU Firmware Base**
- Integrating:  
  - I²S audio test  
  - CC1200 initialization routine  
  - Display bring-up (SPI)  
- Refactoring into modular drivers  

## **3. Expander Control Plane**
- Mapping of all control signals to pins on PCF8575  
- Interrupt consolidation logic (EXP_INT → GPIO35) under development  

---

# To-Do List (Remaining Work)

## **MCU Firmware**
- [ ] Implement CC1200 driver (init, RX/TX, packet handling)  
- [ ] Implement display driver (ST7789) via SPI  
- [ ] Integrate I²S playback + microphone capture test  
- [ ] Build full FreeRTOS task scheduling  
- [ ] Create command interface for frequency/volume control  
- [ ] Add battery + charger telemetry (BQ25622 + fuel gauge)  
- [ ] Add error logging system  

## **Hardware Testing**
- [ ] Validate all V1 PCB power rails under load  
- [ ] Confirm PSRAM availability on WROVER module  
- [ ] Test interrupts without external hardware (mock triggering)  
- [ ] Verify expander INT functionality  
- [ ] Measure SPI integrity at operating clock speeds  

## **Power Modulino**
- [ ] Validate BQ25622 register initialization  
- [ ] Test charging + USB-PD negotiation  
- [ ] Verify battery discharge curves and fuel gauge accuracy  

## **HMI Modulino**
- [ ] Test keypad matrix via expander  
- [ ] Validate rotary encoder timing on MCU pins  
- [ ] Implement backlight PWM control  

## **RF Modulino**
- [ ] Build RF board prototype  
- [ ] Test CC1200 SPI communication in isolation  
- [ ] Validate GDO0/GDO2 interrupt timing  

## **Documentation & Repo Structure**
- [ ] Finalize full README.md  
- [ ] Add CONTRIBUTING.md & CODE_OF_CONDUCT.md  
- [ ] Import all schematics, gerbers, renders into `/hardware/`  
- [ ] Add system architecture diagrams  
- [ ] Add testing & bring-up procedures  

---


This file represents the **current authoritative status** of the OpenAir Lab MCU work and will continue to evolve as new milestones are completed.

