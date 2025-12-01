#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t rx_freq_hz;       // current receive frequency
    bool     is_tx;            // true while TX is active
    bool     ptt_pressed;      // from GPIO or PCF8575
    int16_t  rssi_dbm;         // CC1200 RSSI
    uint8_t  battery_percent;  // BMS reading
} SystemState;

extern SystemState state;

// Debug function for printing state to Serial
void debug_print_state(const SystemState &s);
