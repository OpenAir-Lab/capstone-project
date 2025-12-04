# Lab 1 — ESP32 Boot, Flashing, and Strapping Pins  
OpenAir Lab – Educational Curriculum  
Difficulty: Beginner  
Time: 30–45 minutes  
Prerequisites: None  

---

# 1. Purpose of This Lab

Before interacting with displays, audio, or RF hardware, students must understand how the microcontroller boots and how code is loaded into it.  
This lab uses the ESP32-WROVER-E modulino and the ESP-PROG UART/JTAG debugger to teach:

- The ESP32’s boot process  
- The role of strapping pins  
- How to enter bootloader mode  
- How to flash firmware  
- How to read serial debug output  
- How to diagnose common flashing failures

This establishes the foundation for the rest of the curriculum.

---

# 2. Learning Outcomes

Students will be able to:

1. Describe the ESP32 bootloader process.  
2. Explain the function of GPIO0, GPIO2, GPIO5, GPIO12, and GPIO15 during reset.  
3. Enter bootloader mode using EN and BOOT buttons.  
4. Flash firmware using the ESP-PROG.  
5. Use a serial monitor to read debug output.  
6. Fix common upload errors.

---

# 3. Background Concepts

## 3.1 The ESP32 Boot Process
On reset or power-up, the ESP32:

1. Samples strapping pins  
2. Chooses a boot mode  
3. Executes code from flash, PSRAM, or ROM based on those pin states  

## 3.2 Strapping Pins (from datasheet)
These pins are sampled only during reset; afterward they act as normal GPIOs.

| Pin | Boot Role | Notes |
|-----|-----------|-------|
| GPIO0 | Selects UART bootloader mode | Hold low to program |
| GPIO2 | Boot configuration | Must not float |
| GPIO5 | SDIO/boot control | Typically pulled-up |
| GPIO12 | Flash voltage selection | Must match flash specs |
| GPIO15 | Boot/debug configuration | Usually pulled-down |

Incorrect values can cause failed boots, unreadable serial output, or flash errors.

## 3.3 EN and BOOT Buttons
- EN resets the ESP32.  
- BOOT pulls GPIO0 low to force UART bootloader mode.

To enter programming mode:  
Hold BOOT → tap EN → release BOOT after upload begins.

---

# 4. Required Hardware

- ESP32-WROVER-E modulino  
- ESP-PROG or equivalent UART/JTAG programmer  
- USB-C cable  
- Jumper wires  
- Development computer with PlatformIO or ESP-IDF installed  

---

# 5. Wiring Instructions

**ESP-PROG → ESP32 Connections**

| ESP-PROG Pin | ESP32-WROVER Pin | Function |
|--------------|------------------|----------|
| TXD0 | RX (Pin 18 in your mapping) | Serial data into ESP32 |
| RXD0 | TX (Pin 17) | Serial data out of ESP32 |
| GND | GND | Shared reference |
| 3V3 | 3V3 | Optional board power |

Ensure EN and BOOT buttons are populated on your board.

---

# 6. Step-by-Step Lab Procedure

## Step 1 — Connect the ESP-PROG
Plug in the ESP-PROG. Verify 3.3 V present on the board.

## Step 2 — Open Serial Monitor
Baud rate: 115200  
Tap EN. You should see messages similar to:
```
rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00
```

If nothing prints → connection issue  
If garbage prints → wrong baud or flash voltage mismatch  

## Step 3 — Enter Bootloader Mode
1. Hold BOOT  
2. Tap EN  
3. Release BOOT when upload begins  

PlatformIO should detect the bootloader.

## Step 4 — Upload Test Program

```cpp
#include <Arduino.h>

void setup() {
    pinMode(2, OUTPUT);   // Replace with a known-good GPIO
    Serial.begin(115200);
    Serial.println("ESP32 Boot Successful");
}

void loop() {
    digitalWrite(2, HIGH);
    delay(500);
    digitalWrite(2, LOW);
    delay(500);
}
```
## Step 5 — View Serial Output

Expected:
```
ESP32 Boot Successful
```
The LED on GPIO2 should blink.

# 7. Expected Results

A correct setup will:
- Print boot messages consistently
- Enter bootloader mode on command
- Accept firmware uploads
- Blink the selected GPIO
- Print expected serial messages

If not, students should know how to identify the cause.

# 8. Troubleshooting Guide

### Issue: “Failed to connect: Invalid head of packet”
Cause: Noise or incorrect boot mode
Fix:
- Check TX/RX wiring
- Enter bootloader manually
- Verify ground
- Shorter cables

### Issue: Random characters in serial monitor
Cause: Wrong baud or flash voltage setting
Fix:
- Use 115200
- Check GPIO12 pull resistor

### Issue: No output
Cause: Board not powered or EN pulled low
Fix:
- Verify 3.3 V
- Inspect EN pull-up
- Check USB cable

# 9. Optional Extensions

1. Change LED blink rate.
2. Print strapping pin values at startup.
3. Manually alter GPIO0 and observe different boot modes.
4. Trace reset and UART signals on a logic analyzer.

# 10. End-of-Lab Knowledge Check
1. What is a strapping pin?
2. When are strapping pins sampled?
3. How does the ESP32 enter UART bootloader mode?
4. Why is GPIO12 important?
5. Why must TX and RX be crossed between programmer and MCU?