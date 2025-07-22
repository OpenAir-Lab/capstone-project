| Feature                      | **RP2350**                                   | **ESP32**                                             |
| ---------------------------- | -------------------------------------------- | ----------------------------------------------------- |
| **Core Architecture**        | Dual-core ARM Cortex-M33                     | Single or Dual-core Xtensa LX6                        |
| **Clock Speed**              | Up to 133 MHz                                | Up to 240 MHz                                         |
| **RAM**                      | 512 KB SRAM                                  | 520 KB SRAM + 16 KB RTC SRAM                          |
| **ROM**                      | 128 KB                                       | 448 KB                                                |
| **Flash**                    | External via QSPI                            | External QSPI (some variants have 2MB–4MB in-package) |
| **Wi-Fi / Bluetooth**        | None                                         | 802.11b/g/n, BT v4.2 + BLE                          |
| **GPIO Pins**                | 48 GPIOs across 2 banks                      | 34 GPIOs (not all available on all models)            |
| **Analog I/O**               | Integrated ADC, no DAC                       | 12-bit ADC (18 ch), 2× 8-bit DAC, 10 touch sensors    |
| **Hardware Crypto**          | AES, SHA, RSA, ECC                           | AES, SHA-2, RSA, ECC, RNG                             |
| **Power Modes**              | Not optimized for ultra-low power            | Deep sleep (\~10 µA), modem/light/hibernation modes   |
| **USB**                      | USB 2.0 device and host (via GPIO mux)       | Requires external PHY (not native)                    |
| **Dev Ecosystem**            | Raspberry Pi Pico SDK (C, Rust, MicroPython) | ESP-IDF, Arduino, PlatformIO                          |
| **Radio Control Capability** | Needs external RF interface                  | Built-in radio stack (for BT/Wi-Fi control)           |
| **Multicore + Coprocessors** | Dual Cortex-M33 + GPIOC/DCP/RCP/FPU          | Optional ULP coprocessor for sensor tasks             |

### So it looks like the RP2350 is not optimized for low power nor does it come with bluetooth/wifi capabilities. It seems as though the ESP-32 will be our best pick for microcontroller

## ESP32 Advantages
- Integrated Wireless: Built-in Wi-Fi and Bluetooth simplify HMI and over-the-air firmware upgrades, which are valuable for educational kits and remote debugging.
- Peripheral Breadth: Rich analog and digital I/O (touch sensing, ADC/DAC) supports display control, analog input from sensors, or audio signal generation.
- Low Power + Connectivity: ESP32’s sleep modes are highly refined and well-documented, helping conserve power in battery-operated designs.
- Software Stack: Arduino, ESP-IDF, and MicroPython support make it ideal for educational settings and rapid development.


## ESP32-WROOM vs ESP32-WROVER Comparison Table


| Feature                     | **ESP32-WROOM-32**                      | **ESP32-WROVER**                             |
| --------------------------- | --------------------------------------- | -------------------------------------------- |
| **Flash Memory**            | 4MB (typical)                           | 4MB, **8MB, or 16MB** (depends on variant)   |
| **PSRAM (external RAM)**    | None                                    |  **2MB or 8MB** external pseudo-static RAM  |
| **Package Size**            | 18×25.5 mm                              | 18×31.4 mm (longer due to PSRAM & shielding) |
| **Antenna Options**         | PCB trace or u.FL                       | PCB trace or u.FL                            |
| **CPU**                     | Xtensa LX6 dual-core, 240 MHz           | Same                                         |
| **SRAM (on-chip)**          | 520 KB + 16 KB RTC                      | Same                                         |
| **GPIO Pins Available**     | \~25 usable GPIOs                       | \~22–24 usable GPIOs (due to PSRAM sharing)  |
| **ADC, DAC, PWM, SPI, I2C** |  Yes                                    |  Yes                                        |
| **PSRAM Impact**            | Not present                             | Shares some internal resources and GPIOs     |
| **Cost**                    | \~\$4–\$6 (module), \~\$8–\$12 (DevKit) | \~\$6–\$9 (module), \~\$12–\$18 (DevKit)     |
| **Use Cases**               | General-purpose, minimal UI             | Rich GUI, large buffers, ML/audio, OTA       |
| **Example Dev Boards**      | ESP32-DevKitC-32, Wemos D1 Mini ESP32   | ESP32-WROVER DevKit, TTGO T-Display, ESP-EYE |

[This article](https://innovationyourself.com/esp32-wroom-vs-esp32-wrover/) also gives some insight into WROOM vs WROVER. Mainly:
### Key Differences:

#### 1.Memory Capacity:
- ESP32 WROOM: Standard flash memory.
- ESP32 WROVER: Additional PSRAM for increased memory capacity.

#### 2.Best Suited For:
- ESP32 WROOM: Compact and space-constrained projects.
- ESP32 WROVER: Projects demanding higher memory and processing power.
  
#### 3. Use Cases:
- ESP32 WROOM: Wearables, IoT devices, compact gadgets.
- ESP32 WROVER: Advanced IoT applications, multimedia projects, high-performance gadgets.
