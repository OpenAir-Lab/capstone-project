| Bus/Subsystem | Signal | ESP32 GPIO | Notes |
|--------|--------|--------|--------|
| VSPI —> CC1200 | SCLK | GPIO18 | reliable at high speeds |
| | MOSI | GPIO23 |  |
|  | MISO | GPIO19 |  |
|  | CS | IO5 |  | 
|  | GDO0(IRQ) | GPIO32 | interrupt |
|  | GDO2(IRQ) | GPIO33 | second interrupt line |
| RF Int | | GPIO39 |
|  | CS | IO5 |  | 
|  | GDO0(IRQ) | GPIO32 | interrupt |
|  | GDO2(IRQ) | GPIO33 | second interrupt line |
| RF Int | | GPIO39 |
| HSPI —> TFT Display | SCLK | GPIO14 |  | 
| | MOSI | GPIO13 |  |
| | MISO | -- | TFT doesn’t need MISO (for writing on SD card) leave NC |
| | CS | GPIO04 | separate CS from radio | 
| | DC | GPIO26 | data/command |
| | RST | -- | tie to 3V3 w/RC or to board’s reset |
| | HMI_INT | GPIO35 | interrupt for HMI |
| I2C —> | SDA | GPIO21 | shared bus - use 4.7-10 kΩ pull-up resistor | 
| | SCL | GPIO22 | |
| | Expander INT| IO12 | interrupt for expander |
| BMS Int|  |GPIO36 |  
| I2S —> Audio (full-duplex) | BCLK | GPIO27 | connect to Mic SCK + Amp BCLK, one clock for both devices |
| | LRCLK/WS | GPIO25 | connect to Mic WS + Amp LRCLK, shared LRCLK/WS  |
| | DOUT (ESP32—>Amp) | GPIO33 | connected to MAX98357A DIN, no MCLK needed | 
| | DIN (mic—>ESP32) | GPIO36 | connected to CS‑43434 SD, no MCLK needed |
| Audio Int | | IO2 |
| Audio Int | | IO2 |
| HMI encoder | A/B | -- | put both phases on expander, INT—>GPIO39 |
| Keypad | Rows/cols | -- | entire matrix on expander, INT—>GPIO39 | 
| Debug UART | TX0 / RX0 | GPIO1/GPIO3 | keep default for logs/CLI |

Avoid boot strapping pins (GPIO0/2/5/12/15). GPIO34–39 are input‑only. WROVER modules reserve GPIO16/17 internally for PSRAM

