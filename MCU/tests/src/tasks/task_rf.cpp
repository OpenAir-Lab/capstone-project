#include "task_rf.h"
#include <Arduino.h>
#include "system_state.h"
#include <cc1200.h>
#include <sky13330.h>
#include <grf5604.h>

// These are defined in main.cpp
extern cc1200_config_t cc1200;
extern sky13330_config_t sky13330;
extern grf5604_config_t grf5604_vhf;  // or whatever you named your VHF PA

// TX timeout in ms (for demo)
#define TX_TIMEOUT_MS 10000

// TODO: map these helper functions to your existing driver API.
static void enter_tx_mode() {
    // Set RF switch to TX
    sky13330_set_tx(sky13330);

    // Enable PA (choose the correct band: VHF or UHF)
    grf5604_enable(grf5604_vhf);

    // Put CC1200 in TX mode at the current frequency
    cc1200_enter_tx(&cc1200, state.rx_freq_hz);   // TODO: match your function signature
}

static void enter_rx_mode() {
    // Set RF switch to RX
    sky13330_set_rx(sky13330);

    // Disable PA
    grf5604_disable(grf5604_vhf);

    // Put CC1200 in RX mode at the current frequency
    cc1200_enter_rx(&cc1200, state.rx_freq_hz);   // TODO: match your function signature
}

void rfTask(void *pvParameters) {
    uint32_t tx_start_ms = 0;

    enter_rx_mode();
    state.is_tx = false;

    const TickType_t poll = pdMS_TO_TICKS(30);

    while (1) {
        // ---- ENTER TX ----
        if (state.ptt_pressed && !state.is_tx) {
            enter_tx_mode();
            state.is_tx = true;
            tx_start_ms = millis();
        }

        // ---- STAY / EXIT TX ----
        if (state.is_tx) {
            bool timed_out = (millis() - tx_start_ms) > TX_TIMEOUT_MS;

            if (!state.ptt_pressed || timed_out) {
                enter_rx_mode();
                state.is_tx = false;
            }
        }

        // Optionally: read RSSI while in RX
        if (!state.is_tx) {
            // TODO: replace with your CC1200 RSSI read function
            // state.rssi_dbm = cc1200_read_rssi(&cc1200);
        }

        vTaskDelay(poll);
    }
}
