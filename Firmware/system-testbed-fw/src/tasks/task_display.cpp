#include "task_display.h"
#include <Arduino.h>
#include "core/system_state.h"
#include "ui/ui_display.h"

void displayTask(void *pvParameters) {
    ui_init();

    const TickType_t delay_interval = pdMS_TO_TICKS(150); // ~6-7 Hz

    while (1) {
        // Snapshot the state (all primitive types)
        uint32_t freq  = state.rx_freq_hz;
        int16_t  rssi  = state.rssi_dbm;
        uint8_t  batt  = state.battery_percent;
        bool     is_tx = state.is_tx;

        ui_update_frequency(freq);
        ui_update_rssi(rssi);
        ui_update_battery(batt);
        ui_update_tx_indicator(is_tx);

        vTaskDelay(delay_interval);
    }
}
