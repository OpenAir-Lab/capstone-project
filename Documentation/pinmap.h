#pragma once
#include <Arduino.h>

/*  
 * ---------------------------------------------------------
 *  OpenAir Lab — ESP32-WROVER-E Pin Map
 *  Structured namespace layout for subsystem drivers.
 *  
 *  SOURCE: OpenAir Lab – Pin Map (MCU vs Expander) – V1
 * ---------------------------------------------------------
 */

namespace pins {

    /* =====================
     *  SPI — CC1200 Radio
     * ===================== */
    namespace cc1200 {
        constexpr uint8_t SCK   = 18;   // VSPI_SCK — high-speed
        constexpr uint8_t MOSI  = 23;   // VSPI_MOSI
        constexpr uint8_t MISO  = 19;   // VSPI_MISO
        constexpr uint8_t CS    = 5;    // VSPI_CS — STRAP pin, pulled up externally
        constexpr uint8_t GDO0  = 32;   // interrupt (latency-critical)
        constexpr uint8_t GDO2  = 33;   // interrupt (latency-critical)
        // RESET moved to expander (slow-rate control)
    }


    /* =====================
     *  SPI — TFT ST7789 via EYESPI
     * ===================== */
    namespace tft {
        constexpr uint8_t SCK   = 14;   // HSPI_SCK
        constexpr uint8_t MOSI  = 13;   // HSPI_MOSI
        constexpr uint8_t CS    = 15;   // TFT_CS — pull-up required at boot
        constexpr uint8_t DC    = 4;    // Data/Command
        // RST, BL_EN handled by expander
        // TE (optional): GPIO36 if VSYNC needed
        constexpr uint8_t TE    = 36;   // optional tearing-effect input (unused = ignore)
    }


    /* =====================
     *  I²S — Audio System (ICS-43434 mic + MAX98357A amp)
     * ===================== */
    namespace audio {
        constexpr uint8_t BCLK  = 26;   // I²S bit clock
        constexpr uint8_t LRCLK = 25;   // I²S left/right clock
        constexpr uint8_t DOUT  = 27;   // ESP32 → MAX98357A
        constexpr uint8_t DIN   = 34;   // ICS-43434 mic → ESP (input-only GPIO)
        // AMP_SD, GAIN via I²C expander
    }


    /* =====================
     *  I²C — Shared Control Bus (PD, Fuel Gauge, GPIO Expander)
     * ===================== */
    namespace i2c {
        constexpr uint8_t SDA   = 21;
        constexpr uint8_t SCL   = 22;
    }


    /* =====================
     *  GPIO Expander Interrupt → MCU
     * ===================== */
    namespace expander {
        constexpr uint8_t INT   = 35;   // expander → MCU, GPIO35 input-only
    }


    /* =====================
     *  Power & System Functions
     * ===================== */
    namespace system_pins {
        // Boot/strap pins intentionally avoided except where needed
        // GPIO0, GPIO2, GPIO12 — DO NOT use for peripherals
        // GPIO5, GPIO15 — safe after boot if pulled up

        constexpr uint8_t BOOT_BUTTON  = 0;   // physical BOOT, if wired
        constexpr uint8_t EN_PIN       = -1;  // EN is not a GPIO; controlled via CH340/esp-prog circuitry
    }


    /* =====================
     *  Optional MCU-side Inputs
     * ===================== */
    namespace optional {
        constexpr uint8_t ROT_A = 16;  // if rotary encoder moved to MCU
        constexpr uint8_t ROT_B = 17;
    }

} // namespace pins

