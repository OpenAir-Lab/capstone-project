#pragma once
#include <stdint.h>
#include <stdbool.h>

// Initialize the UI (assumes display_init() was already called)
void ui_init();

// These will be called by displayTask to update only the dynamic parts
void ui_update_frequency(uint32_t freq_hz);
void ui_update_rssi(int16_t rssi_dbm);
void ui_update_battery(uint8_t batt_percent);
void ui_update_tx_indicator(bool is_tx);
