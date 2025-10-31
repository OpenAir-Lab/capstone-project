| Bus/Subsystem | Signal | ESP32 GPIO | Notes |
|--------|--------|--------|--------|
| VSPI —> CC1200 | SCLK | GPIO18 | reliable at high speeds |
| | MOSI | GPIO23 |  |
|  | MISO | GPIO19 |  |
|  | CS | GPIO32 |  | 
|  | GDO0(IRQ) | GPIO34 | interrupt |
|  | GDO2(IRQ) | GPIO35 | second interrupt line |
| HSPI —> TFT Display | SCLK | GPIO14 |  | 
| | MOSI | GPIO13 |  |
| | MISO | -- | TFT doesn’t need MISO (for writing on SD card) leave NC |
| | CS | GPIO04 | separate CS from radio | 
| | DC | GPIO26 | data/command |
| | RST | -- | tie to 3V3 w/RC or to board’s reset |
| I2C —> BQ25620 + I/O Expander + STUSB4500| SDA | GPIO21 | shared bus - use 4.7-10 kΩ pull-up resistor | 
| | SCL | GPIO22 | |
| | Expander INT| GPIO39 | wakes HMI task on change |
| | ~STUSB4500 ALERT~ |~GPIO38~ DNE | open‑drain, pull‑up to 3V3, PD event interrupt | 
| I2S —> Audio (full-duplex) | BCLK | GPIO27 | connect to Mic SCK + Amp BCLK, one clock for both devices |
| | LRCLK/WS | GPIO25 | connect to Mic WS + Amp LRCLK, shared LRCLK/WS  |
| | DOUT (ESP32—>Amp) | GPIO33 | connected to MAX98357A DIN, no MCLK needed | 
| | DIN (mic—>ESP32) | GPIO36 | connected to CS‑43434 SD, no MCLK needed |
| HMI encoder | A/B | -- | put both phases on expander, INT—>GPIO39 |
| Keypad | Rows/cols | -- | entire matrix on expander, INT—>GPIO39 | 
| Debug UART | TX0 / RX0 | GPIO1/GPIO3 | keep default for logs/CLI |

Avoid boot strapping pins (GPIO0/2/5/12/15). GPIO34–39 are input‑only. WROVER modules reserve GPIO16/17 internally for PSRAM

