#include "system_state.h"
#include <Arduino.h>

SystemState_t state;   // Global shared state

void debug_print_state(const SystemState_t &s) {
    Serial.printf(
        "[STATE] freq=%lu Hz | TX=%d | PTT=%d | RSSI=%d dBm | Batt=%u%%\n",
        s.rx_freq_hz,
        s.is_tx,
        s.ptt_pressed,
        s.rssi_dbm,
        s.battery_percent
    );
}
