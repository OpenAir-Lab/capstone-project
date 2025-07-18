# ESP32-WROVER-E Info

### Strapping Pins
<img width="383" height="175" alt="DefualtStrappingPins" src="https://github.com/user-attachments/assets/c776c2bf-279a-46ea-8877-99aa5f066faa" />

All strapping pins have latches. At system reset, the latches sample the bit values of their respective strapping pins and store them until the chip is powered down or shut down. The states of latches cannot be changed in any other way. It makes the strapping pin values available during the entire chip operation, and the pins are freed up to be used as regular IO pins after reset.

GPIO0 and GPIO2 control the boot mode after the reset is released

<img width="394" height="204" alt="Screenshot 2025-07-18 at 16 11 45" src="https://github.com/user-attachments/assets/dc18030c-ebb9-4ffc-bb81-f2b76ff5eb9e" />

### Notes on Power Supply
* When a battery is used as the power supply for the ESP32 series of chips and modules, a supply voltage supervisor is recommended, so that a boot failure due to low voltage is avoided. Users are recommended to pull CHIP_PU low if the power supply for ESP32 is below 2.3 V.
* The operating voltage of ESP32 ranges from 2.3 V to 3.6 V. When using a single-power supply, the recommended voltage of the power supply is 3.3 V, and its recommended output current is 500 mA or more.
* PSRAM and flash both are powered by VDD_SDIO. If the chip has an in-package flash, the voltage of VDD_SDIO is determined by the operating voltage of the in-package flash. If the chip also connects to an external PSRAM, the operating voltage of external PSRAM must match that of the in-package flash. This also applies if the chip has an in-package PSRAM but also connects to an external flash.
* When VDD_SDIO 1.8 V is used as the power supply for external flash/PSRAM, a 2 kΩ grounding resistor should be added to VDD_SDIO.
* When the three digital power supplies are used to drive peripherals, e.g., 3.3 V flash, they should comply with the peripherals’ specifications.
