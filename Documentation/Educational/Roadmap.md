# OpenAir Lab Educational Roadmap  
### Hands-on Labs + Short Explainer Videos (For Expo & Curriculum)

This roadmap is designed for students with little or no RF or embedded experience. It gradually reveals how a handheld radio works from power, to MCU, to buses, to RF, audio, UI, and full system integration. All content references your actual project hardware and subsystem documentation.

---

# Section 0 — Orientation (Video Only)

### Video 0: “What Is OpenAir Lab?” (2–3 min)
Purpose: Provide context for expo judges.  
Contents:
- Why modular radios matter (accessible, hackable, open-source).  
- Overview of modulinos: MCU, RF, HMI, Audio, BMS, USB-PD.  
- Visual of the whole system (EYESPI + 4-pin I²C harness).  
- Explanation that a radio is sensors, signals, timing, interrupts, and power.

---

# Section 1 — The MCU Foundation (ESP32-WROVER-E)

### Lab 1: ESP32 Boot, Flashing, Strapping Pins
What students learn:
- How microcontrollers boot and why GPIO0/2/5/12/15 matter (strapping pins).  
- How to flash via UART, create “hello world,” and debug boot issues.

Activities:
- Wire ESP-PROG to WROVER modulino.  
- Observe boot logs.  
- Modify code and reflash.  
- Check PSRAM availability.

Video 1: “How ESP32 Starts Up and Why Boot Pins Matter” (3 min)

---

### Lab 2: Buses 101 — SPI, I²C, I²S on the Same Board
Students learn how different digital buses behave and why each subsystem uses a particular one.

Uses project pin map.

Activities:
- Scan the I²C bus.  
- Perform an SPI loopback test.  
- Capture an I²S audio sample.

Video 2: “What’s the Difference? SPI vs I²C vs I²S” (3–4 min)

---

# Section 2 — Power & Battery Subsystems

### Lab 3: USB-PD and BMS Deep Dive (STUSB4500 + BQ25620)
What students learn:
- USB-PD voltage negotiation.  
- Li-ion battery charging and system load sharing.  
- Reading battery percentage and alerts via I²C.

Activities:
- Plug into PD charger and observe negotiation.  
- Read fuel gauge via I²C.  
- Trigger PD_ALERT interrupt via the expander.

Video 3: “How USB-PD and a Li-Ion BMS Keep Your Radio Alive” (2–3 min)

---

# Section 3 — Human-Machine Interface Subsystem

### Lab 4: ST7789 Display with EYESPI (Graphics over SPI)
Students learn:
- How SPI displays work.  
- How to send commands and draw graphics.  
- How EYESPI simplifies modular interconnects.

Activities:
- Send SPI commands.  
- Draw shapes or pixels.  
- Display sensor values.  
- Test TFT_CS and TFT_DC behavior.

Video 4: “Driving a TFT Display Over SPI with EYESPI” (3 min)

---

### Lab 5: Keypad, Buttons, LEDs via the I²C Expander
Students learn:
- How to expand GPIO count with minimal MCU pins.  
- How interrupt lines from expanders work.

Activities:
- Configure expander ports.  
- Detect keypad presses.  
- Toggle LEDs through the expander.

Video 5: “Why We Use a GPIO Expander and How INT Lines Work” (3 min)

---

# Section 4 — Audio Subsystem (TX & RX)

### Lab 6: MEMS Microphone Capture Using I²S (ICS-43434)
Students learn:
- Digital microphone basics.  
- Time-domain and amplitude capture.

Activities:
- Capture 1-second audio.  
- Display waveform on TFT.  
- Discuss bit clock, word clock, and sample rates.

Video 6: “How Digital Microphones Work Using I²S” (2–3 min)

---

### Lab 7: Speaker Output Using MAX98357A
Students learn:
- How digital-to-audio paths work using I²S.  
- How to control amplifier enable pins.

Activities:
- Generate tones using I²S.  
- Play a PCM buffer.  
- Toggle AMP_SD via the expander.

Video 7: “Creating Sound with a Digital Amplifier” (2 min)

---

# Section 5 — RF Subsystem (CC1200)

### Lab 8: SPI Communication with CC1200
Students learn:
- How an RF transceiver is configured.  
- How SPI registers control modulation, frequency, and power.

Activities:
- Read status registers.  
- Modify operating frequency.  
- Log RSSI.  
- Handle GDO0/GDO2 interrupts.

Video 8: “Talking to a Real RF Transceiver Over SPI” (3–4 min)

---

### Lab 9: Build a Simple RX/TX Loop (No Audio Yet)
Students learn:
- Basic digital radio communication.  
- Packet transmission and reception.  
- Channel metrics and reliability.

Activities:
- Send and receive packets.  
- Visualize RSSI and packet success rate.  
- Display link state on TFT.

Video 9: “How Digital Radios Encode and Decode Data” (4 min)

---

# Section 6 — System Integration

### Lab 10: FreeRTOS Task Architecture
Students learn:
- How multitasking works on ESP32.  
- Why radios need real-time task separation.  
- Queues, semaphores, timers, and ISRs.

Activities:
- Create Display, Radio, Power, and Input tasks.  
- Implement message passing.  
- Trigger timers for display refresh and PTT.

Video 10: “How Tasks, Queues, and Interrupts Make a Radio Responsive” (3 min)

---

### Lab 11: Build the “Mini Radio OS”
Full integration of subsystems.

Features students implement:
- Frequency tuning via keypad.  
- Display updates.  
- Audio TX/RX path.  
- Battery indicator.  
- PTT → TX mode via interrupt.

Video 11: “Bringing It All Together — Your First Working Radio” (2–3 min)

---

# Section 7 — Stretch / Advanced Modules

### Advanced Lab A: Digital Squelch and RSSI Thresholding  
### Advanced Lab B: Add Recording Buffer Using PSRAM  
### Advanced Lab C: Implement Scan Mode (Frequency Hopping)  
### Advanced Lab D: Add a Soft Menu System on the TFT  
### Advanced Lab E: Create a PC GUI for Packet Visualization  

---

# Expo Deliverables

### One-Page Educational Map Poster
- Linear or circular pathway of labs.  
- Short blurbs and arrows showing dependency.

### QR Codes to Video Shorts
- Each QR links to a 60–120 second video.  
- Mounted on board or handout.

### Printable Lab Packets
Each with:
- One-paragraph intro.  
- Wiring diagram.  
- Key code snippet.  
- Expected output screenshot or log.  

