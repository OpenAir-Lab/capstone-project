#pragma once
#include <stdint.h>
#include <stdbool.h>

// =====================
// System State Structure
// =====================
// The commented values are the only ones we need for upcoming demo
typedef struct {
    double rx_freq_hz;        // current receive frequency
    double tx_freq_hz;
    bool     is_tx;             // current receive frequency
    bool     ptt_pressed;       // input from PTT GPIO/expander
    int8_t   volume;
    int8_t   squelch;
    int8_t   rssi_dbm;          // CC1200-reported RSSI
    uint8_t  battery_percent;   // BMS value
    bool     charging;
    // + any flags we REALLY need for demo
}SystemState_t;

extern SystemState_t state;

// Debug function for printing state to Serial
void debug_print_state(const SystemState_t &s);
