#### Bootloader Key Responsibilities:
(https://saludpcb.com/esp32-bootloader-guide/)

- Initializing the CPU, clocks, memory, and other hardware
- Loading the application (app.main) from flash
- Verifying firmware integrity 
- Supporting OTA

Bootloader is typically stored at flash address Ox1000, and directly executed by *on-chip ROM code* <-- **(Reset Vector)**



## Theory
(https://circuitlabs.net/understanding-esp32-boot-process/)

![WROVER-E Block Diagram](/capstone-project/MCU/Subsystem-Documentation/Images/WROVER-E-Block-Diagram.png)

The ESP32 boot process involves multiple stages, transitioning from internal, unchangeable code in ROM to configurable software loaded from external SPI Flash memory.

![Top Half of Flowchart Image](/capstone-project/MCU/Subsystem-Documentation/Images/Flowchart-top.png)

![Bottom Half of Flowchart Image](/capstone-project/MCU/Subsystem-Documentation/Images/Flowchart-bottom.png)

### 1. Reset on Hardware Initialization 
| Reset Trigger | Description |
|--------|--------|
| Power-on Reset (POR) | Occurs when power is initially applied to the ESP32 chip |
| External Reset | Triggered by asserting the CHIP_PU(Enable) pin low, then high |
| Watchdog Timer Reset | Initiated if HW or SW watchdog timer expires, indicating a potential system hang. |
| Software Reset | Triggered intentionally by application code (e.g. vis 'esp_restart()') |
| Deep Sleep Wake-up | Occurs when ESP32 wakes up from a low-power deep sleep state, which involves a reset cycle |

**Reset Vector** points into the chip's internal ROM which contains the 1st Stage Bootloader (immutable code burned into silicon during manufacturing)

### 2. ROM Bootloader (1st Stage Bootloader)
Responsible for:
- Basic hardware initialization
- Check strapping pins 
- Boot mode selection
- Load 2nd Stage Bootloader (Run Mode) --> if in Run Mode, attempts to load '`bootloader.bin`' from flash addr 0x1000
- Basic Security Checks --> if Secure Boot is enabled, performs initial verification of the 2nd stage bootloader.

>#### Strapping Pins
>---
>GPIO0 
>
>LOW (held during reset) = Download Mode for flashing new firmware. ROM Bootloader enters a state where it listens for commands over
UART0/USB Serial/JTAG. 
`esptool.py` uses this mode to write firmware (bootloader, partition table, application) to the flash memory.
>
>HIGH or Floating (during reset) = Run Mode (SPI Boot 2nd Stage Bootloader) - normal exe of the application firmware

### 3. SPI Bootloader (2nd Stage Bootloader)
ROM Bootloader loads the 2nd Stage Bootloader (`bootloader.bin`) from SPI Flash into internal SRAM

Bootloader _is_ part of project and is flashed along with our application

Performs tasks like:
- initializing additional peripherals 
    - including SPI Flash interface with correct settings like speed and mode read from the image header
- configuring memory mapping (MMU) and enabling Flash cache (XIP)
- reading the partition table from flash (usually at offset 0x8000)
- checking for OTA update instructions
- selecting which application partition to boot
- if Secure Boot enabled: perform intergrity checks (checksum/hash) or signature verification on application image
- loading app image segments (code and data)from flash into the appropriate RAM regions 
- jumping to application's entry point

 **Tip:**
>The source code for the 2nd Stage Bootloader is available within the ESP-IDF components:
>    - (`/components/bootloader`)
>    - (`bootloader_start.c`) can be insightful

### 4. Partition Table

![SPI Flash Layout](/capstone-project/MCU/Subsystem-Documentation/Images/SPI-Flash_Layout.png)

The 2nd Stage Bootloader reads this table (located at 0x8000 by default) to find the location and size of the application partition it needs to load. Partition tables are defined in simple CSV format within your ESP-IDF project (e.g., `partitions.csv`) and compiled into a binary format (`partition_table.bin`) during the build process.

### 5. Application Loading and Execution
>Once 2nd Stage Bootloader identifies valid applicaiton partition ('factory') it performs the following:

**Step 1: Verification**
- Integrity & Authenticity Check (SHA-256 hash)
    - if Secure Boot is enabled, verifies digital signature against trusted keys

**Step 2: Loading**
- Copy to RAM
    - 


