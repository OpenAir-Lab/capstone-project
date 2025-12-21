# **MCU Modulino – Board Bring-Up Guide**  
*(OpenAir Lab – ESP32-WROVER-E Subsystem)*  

This guide describes the **required procedure for first-power, validation, and subsystem bring-up** of the MCU Modulino.  
It applies to the ESP32-WROVER-E–based controller and its attached modulinos (RF, HMI, Audio, Power), as defined in the project overview and pin maps.  
---

# **1. Before You Begin**

## **1.1 Required Documents**
- **OpenAir Lab – Compiled Overview** (subsystems, interconnect map)  
- **MCU vs Expander Pin Map (V1)**  
- **ESP32-WROVER-E Datasheet**  
- **Adafruit 1.9" ST7789 TFT EYESPI Guide**

---

# **2. Safety & Power-Up Requirements**

## **2.1 Confirm Powering Requirements**
From the ESP32-WROVER-E datasheet:

- Operating voltage **3.0–3.6 V**, recommended 3.3 V.  
- Supply must be capable of **≥500 mA** instantaneous current.  
- With battery: Espressif recommends a **supply supervisor** and pulling **CHIP_PU low if VDD < 2.3 V**.

---

# **3. Pre-Power Hardware Validation (Unpowered Tests)**

## **3.1 Strap Pin Verification**
Verify strap pin behavior matches expected ESP32 boot-mode requirements:

| Strap Pin | Project Use | Required Reset Level | Notes |
|-----------|-------------|----------------------|-------|
| GPIO0 | Boot mode | HIGH for normal boot | Do not externally pull low. |
| GPIO2 | Boot mode | HIGH | Avoid heavy loads during reset. |
| GPIO15 (MTDO) | TFT_CS | **HIGH via 10 kΩ pull-up** | Required by project pin map. |
| GPIO5 | CC1200 CS | **HIGH via 10 kΩ pull-up** | Device must not pull low at reset. |
| GPIO12 (MTDI) | VDD_SDIO strap | Must not be biased | Avoid external circuitry. |

Confirm with continuity/resistance checks to 3.3 V.

## **3.2 Short-Circuit Scan**
Check:
- **3V3 ↔ GND**  
- SPI buses  
- SDA/SCL routing  
- Expander INT → GPIO35 (input-only)

## **3.3 EYESPI Connector Orientation**
Match PCB routing to Adafruit’s documented pinout:

- MOSI, SCK, TFT_CS, TFT_DC, RST, BL pins  
- Verify that **HSPI SCK → GPIO14**, **MOSI → GPIO13**, **TFT_CS → GPIO15**, **TFT_DC → GPIO4**.

---

# **4. First Power-Up (No Peripherals Attached)**

## **4.1 Apply Power**
- Provide stable 3.3 V to the appropriate power pin(s).  
- Do **not** connect TFT, RF, audio, or sensors yet.

## **4.2 Expected Behavior**
- ESP32 should boot into UART log mode at **115200 baud** on U0TXD.  
- If no output:  
  - Check EN/CHIP_PU = HIGH  
  - Check GPIO0 = HIGH  
  - Verify strap-level pull-ups

---

# **5. UART + Bootloader Bring-Up**

### Required connections (ESP-PROG → MCU):
| MCU Pin | ESP-PROG |
|---------|----------|
| GPIO0 | BOOT / IO0 |
| EN / CHIP_PU | RESET |
| U0TXD | RX |
| U0RXD | TX |
| GND | GND |

### Bootloader Flash Test
1. Hold **GPIO0 LOW**, pulse **EN LOW→HIGH**.  
2. Run:  
```
esptool.py --chip esp32 flash_id
```
3. Output should show flash ID.

If you see:
```
Invalid head of packet (0xFA
```
→ This indicates noise or improper boot-pin sampling.

---

# **6. Subsystem Bring-Up Sequence**

Bring subsystems up **one at a time**, in this order:

---

## **6.1 I²C Bus + GPIO Expander**

Responsible for keypad, LEDs, user buttons, TFT_RST, backlight, CC1200_RESET, PD/FuelGauge alerts.

### Steps:
1. Connect SDA (GPIO21), SCL (GPIO22).  
2. Ensure 2.2–4.7 kΩ pull-ups installed.  
3. Run I²C scan; expect expander address to appear.  
4. Confirm expander **INT → GPIO35** toggles correctly.

⚠️ **INSERT: Expander model (TCA9539/MCP23017) + I²C address.**

---

## **6.2 TFT / Display (ST7789 via EYESPI)**

Pins (write-only SPI):
- SCK → GPIO14  
- MOSI → GPIO13  
- TFT_CS → GPIO15 (strap-sensitive; must be HIGH at reset)  
- TFT_DC → GPIO4  
- RST, BL via expander  

### Bring-Up:
1. Toggle CS/DC without display to verify logic levels.  
2. Attach display through EYESPI cable.  
3. Run minimal ST7789 initialization.  
4. Expect white flash → test pattern.

---

## **6.3 Audio (ICS-43434 MEMS Mic + MAX98357A Amp)**

Pin map:
- **I²S_BCLK → GPIO26**  
- **I²S_LRCLK → GPIO25**  
- **I²S_DOUT → GPIO27**  
- **I²S_DIN → GPIO34** (input-only)  

### Bring-Up:
1. Initialize I²S in Philips mode.  
2. Confirm mic presence via DMA reads.  
3. Generate sine wave to speaker through MAX98357A.

---

## **6.4 RF Subsystem (CC1200)**

Pin map:
- SCK18, MOSI23, MISO19, CS5  
- GDO0 → GPIO32  
- GDO2 → GPIO33  

### Bring-Up:
1. Ensure **CS remains HIGH at reset** (GPIO5 is a strap).  
2. Perform register read (e.g., PARTNUM).  
3. Validate interrupts on GDO lines.

---

## **6.5 Power Subsystem (STUSB4500 PD + BMS/Fuel Gauge)**

- I²C bus shared with expander  
- ALERT pins routed through expander inputs  

### Bring-Up:
1. Confirm 5 V / 3V3 rails.  
2. Read “Port Status” from PD controller.  
3. Query battery/fuel gauge status.

⚠️ **INSERT: I²C addresses + pin designations.**

---

# **7. Initial Firmware Bring-Up**

Recommended bring-up firmware sequence:

1. **Basic GPIO test**  
2. **I²C scan** (find expander, PD, fuel gauge)  
3. **SPI test** (send commands to TFT)  
4. **I²S loopback tests**  
5. **CC1200 identification**  
6. **Interrupt matrix validation** (simulate INT/GDO pulses)

Your architecture uses ISR-driven input handling, FreeRTOS tasks for display updates, PTT timing, etc.

---

# **8. Checklists**

## **8.1 Required Pass Conditions**
- [ ] MCU boots and prints ROM logs  
- [ ] UART flashing works  
- [ ] Strap pins confirm correct reset levels  
- [ ] I²C expander detected  
- [ ] Display initializes  
- [ ] Audio subsystem clocks active  
- [ ] CC1200 responds over SPI  
- [ ] Power subsystem reads valid data  

## **8.2 Common Failure Modes**
| Symptom | Likely Cause |
|---------|--------------|
| No boot log | GPIO15/GPIO5 not pulled up, EN low |
| Flashing fails | Noise on TX/RX, boot mode incorrect |
| TFT stays white | Wrong CS/DC pins, missing pull-up on GPIO15 |
| I²C bus hangs | Missing pull-ups, wrong expander address |
| RF not responding | CC1200_CS low during reset (breaks strap behavior) |

---

# **9. Insert These Missing Details Before Finalizing**
- Does MCU board generate its own 3V3?  
- Expander model + address  
- Presence + values of GPIO5 / GPIO15 pull-ups  
- Exact PD / Fuel Gauge wiring  
- Whether you want included firmware examples per subsystem  

---

# **End of Bring-Up Guide**
