| **Subsystem**                   | **Test Name**                       | **Expected Result**                              | **Pass / Fail** |
| ------------------------------- | ----------------------------------- | ------------------------------------------------ | --------------- |
| **Visual / Pre-Power-On**       | Inspect solder joints & orientation | No solder bridges, correct IC/polarity placement | ☐ / ☐           |
|                                 | Check decoupling capacitors         | 0.1 µF near every IC VCC pin                     | ☐ / ☐           |
|                                 | Verify pull-ups/downs on straps     | GPIO 0, 2, 5, 15 at correct boot levels          | ☐ / ☐           |
|                                 | Continuity / shorts check           | No short between 3V3 & GND                       | ☐ / ☐           |
|                                 | Connector orientation               | EYESPI / FPC / JST keyed & aligned               | ☐ / ☐           |
| **Power Subsystem**             | 5 V input current                   | < 100 mA idle; steady draw                       | ☐ / ☐           |
|                                 | 3.3 V rail regulation               | 3.28 – 3.35 V stable                             | ☐ / ☐           |
|                                 | BQ25620 charge cycle                | Status LEDs & I²C registers show charging        | ☐ / ☐           |
|                                 | STUSB4500 PD negotiation            | 5 → 9 → 12 V profiles accepted                   | ☐ / ☐           |
|                                 | Fuel Gauge I²C response             | Correct I²C ACK at address 0x55                  | ☐ / ☐           |
|                                 | Rail ripple test                    | < 50 mV p-p under load                           | ☐ / ☐           |
| **MCU Core**                    | Power-on boot log                   | ESP32 UART prints boot banner                    | ☐ / ☐           |
|                                 | Flash programming test              | Code uploads successfully via UART0              | ☐ / ☐           |
|                                 | BOOT / EN buttons                   | Proper reset and bootloader modes                | ☐ / ☐           |
|                                 | Deep sleep current                  | < 0.2 mA typical                                 | ☐ / ☐           |
|                                 | GPIO input/output                   | All test pins toggle/read correctly              | ☐ / ☐           |
|                                 | Strap behavior                      | GPIO 0 LOW → bootload mode confirmed             | ☐ / ☐           |
| **SPI Bus**                     | Loopback test                       | MOSI ↔ MISO data echo valid                      | ☐ / ☐           |
|                                 | CC1200 SPI comm                     | WHOAMI or status readback valid                  | ☐ / ☐           |
|                                 | TFT SPI comm                        | Screen initializes without artefacts             | ☐ / ☐           |
| **I²C Bus**                     | I²C scan                            | STUSB4500, BQ25620, Expander found               | ☐ / ☐           |
|                                 | Pull-up verification                | 2.2–4.7 kΩ confirmed on SDA/SCL                  | ☐ / ☐           |
|                                 | Expander INT line                   | GPIO 35 pulses on key press                      | ☐ / ☐           |
| **UART**                        | UART loopback                       | Sent bytes echo correctly                        | ☐ / ☐           |
|                                 | BMS serial link                     | UART Tx/Rx data visible in console               | ☐ / ☐           |
| **I²S Audio**                   | Clock signals scope check           | BCLK @ ~1.4 MHz, LRCLK @ 44.1 kHz                | ☐ / ☐           |
|                                 | MIC input capture                   | Stable waveform on GPIO 34                       | ☐ / ☐           |
|                                 | AMP output playback                 | Clear audio through speaker                      | ☐ / ☐           |
| **Display / HMI**               | ST7789 init                         | Full-screen color test OK                        | ☐ / ☐           |
|                                 | Backlight PWM                       | Adjustable brightness via duty cycle             | ☐ / ☐           |
|                                 | Keypad inputs                       | Correct key matrix scanning via expander         | ☐ / ☐           |
|                                 | LEDs / buttons                      | Toggle as commanded                              | ☐ / ☐           |
| **RF / CC1200**                 | SPI register read/write             | Config registers match expected                  | ☐ / ☐           |
|                                 | RSSI readback                       | Varies with input signal                         | ☐ / ☐           |
|                                 | TX loop test                        | Signal visible on spectrum analyzer              | ☐ / ☐           |
|                                 | GDO0/GDO2 IRQ                       | ISR triggers on packet events                    | ☐ / ☐           |
| **Power & Battery**             | Charge/discharge cycle              | Smooth transition between states                 | ☐ / ☐           |
|                                 | Fuel gauge readings                 | Voltage ± 5 % of multimeter                      | ☐ / ☐           |
|                                 | Battery-only runtime                | Meets target endurance (> 1 hr)                  | ☐ / ☐           |
| **System Integration**          | Sequential startup                  | No brown-out or reset loops                      | ☐ / ☐           |
|                                 | FreeRTOS tasks run                  | Display + RF + Audio tasks active                | ☐ / ☐           |
|                                 | Task interaction                    | Queues and semaphores respond correctly          | ☐ / ☐           |
|                                 | Interrupt contention                | No lost INT events on expander                   | ☐ / ☐           |
|                                 | Thermal check                       | < 60 °C under load                               | ☐ / ☐           |
| **Environmental / Reliability** | 12-hour runtime                     | Stable operation (no reset logs)                 | ☐ / ☐           |
|                                 | Battery endurance                   | Continuous operation until low-voltage cutoff    | ☐ / ☐           |
|                                 | USB PD hot-plug                     | No brown-out or reboot                           | ☐ / ☐           |
|                                 | EMI check                           | No visible spurs on I²S/SPI lines                | ☐ / ☐           |
|                                 | Mechanical robustness               | Survives mild vibration/drop                     | ☐ / ☐           |
