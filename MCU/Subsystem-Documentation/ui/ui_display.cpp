#include "ui_display.h"
#include <Arduino.h>
#include <st7789.h>
#include "core/system_state.h"

// These are defined in main.cpp
extern Adafruit_ST7789 tft;
extern st7789_config_t display_config;

static void ui_draw_static_layout() {
    tft.fillScreen(ST77XX_BLACK);   // color definitions from Adafruit_ST7789 / ST77XX

    tft.setTextWrap(false);

    // Title
    tft.setCursor(10, 10);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("OpenAir Radio");

    // Labels (RX/TX/Freq/RSSI/Batt)
    tft.setTextSize(1);

    tft.setCursor(10, 40);
    tft.print("FREQ:");

    tft.setCursor(10, 70);
    tft.print("RSSI:");

    tft.setCursor(10, 100);
    tft.print("STATE:");

    tft.setCursor(10, 130);
    tft.print("BATT:");
}

// Simple helpers that only rewrite the value fields.
// You may want to tune the x/y coordinates to match your layout.

void ui_init() {
    // If you want a specific rotation, set it here.
    // (You might want rotation 1 or 3; choose based on your existing tests.)
    tft.setRotation(1);
    ui_draw_static_layout();
}

void ui_update_frequency(uint32_t freq_hz) {
    // Clear old value area (e.g., rectangle to the right of "FREQ:")
    tft.fillRect(70, 40, 140, 16, ST77XX_BLACK);
    tft.setCursor(70, 40);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_GREEN);

    // Display in MHz with 3 decimal places, e.g., 146.520
    float freq_mhz = freq_hz / 1000000.0f;
    tft.print(freq_mhz, 3);
    tft.print(" MHz");
}

void ui_update_rssi(int16_t rssi_dbm) {
    tft.fillRect(70, 70, 80, 16, ST77XX_BLACK);
    tft.setCursor(70, 70);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_CYAN);

    tft.print(rssi_dbm);
    tft.print(" dBm");
}

void ui_update_battery(uint8_t batt_percent) {
    tft.fillRect(70, 130, 80, 16, ST77XX_BLACK);
    tft.setCursor(70, 130);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW);

    tft.print((int)batt_percent);
    tft.print("%");
}

void ui_update_tx_indicator(bool is_tx) {
    tft.fillRect(70, 100, 80, 16, ST77XX_BLACK);
    tft.setCursor(70, 100);
    tft.setTextSize(1);

    if (is_tx) {
        tft.setTextColor(ST77XX_RED);
        tft.print("TX");
    } else {
        tft.setTextColor(ST77XX_GREEN);
        tft.print("RX");
    }
}
