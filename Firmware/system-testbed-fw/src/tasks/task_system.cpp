#include "task_system.h"
#include <Arduino.h>
#include "core/system_state.h"
#include "inputs/input_hmi.h"

void systemTask(void *pvParameters) {
    const TickType_t interval = pdMS_TO_TICKS(50);  // 20 Hz poll

    while (1) {
        // ----- Encoder -----
        int8_t step = hmi_read_encoder_step();
        if (step != 0) {
            // Example: 12.5 kHz step
            state.rx_freq_hz += (int32_t)step * 12500;
        }

        // ----- PTT -----
        bool ptt = hmi_read_ptt();
        state.ptt_pressed = ptt;

        vTaskDelay(interval);
    }
}
