#include <pcf8575.h>
#include <Arduino.h>

#define DEBUG_PCF8575 Serial

extern EventGroupHandle_t keypad_event_group;
extern QueueHandle_t keypad_queue;
extern QueueHandle_t encoder_queue;
static const int8_t quad_table[16] = {
  0, -1,  1,  0,
  1,  0,  0, -1,
 -1,  0,  0,  1,
  0,  1, -1,  0
};
// HMI Port Expander
extern Adafruit_PCF8575 pcf_hmi;
extern pcf8575_config_t pcf_hmi_config; // on 0x20+1

bool pcf8575_portMode(Adafruit_PCF8575 &pcf, uint8_t port, uint8_t mode) {
    return pcf.pinMode(port, mode);
}

int pcf8575_readPort(Adafruit_PCF8575 &pcf, uint8_t port) {
    return pcf.digitalRead((uint8_t)port);
}

int pcf8575_writePort(Adafruit_PCF8575 &pcf, uint8_t port, uint8_t value) {
    return pcf.digitalWrite(port, value);
}

int pcf8575_init(Adafruit_PCF8575 &pcf, pcf8575_config_t &configuration) {
    if (configuration.initialized) {
        return 0;
    }
    pcf.begin(configuration.sensor_address, configuration.i2c);
    pinMode(configuration.pin_interrupt, INPUT_PULLUP);
    const char* subsystem_name = configuration.subsystem_name.c_str();
    // attachInterrupt(digitalPinToInterrupt(configuration.pin_interrupt), portChanged, FALLING);
    #ifdef DEBUG_PCF8575
    DEBUG_PCF8575.printf("(I2C0 -----) Scanning for %s Port Expander on address 0x%2.2X...\n", subsystem_name, configuration.sensor_address);
    #endif
    for (uint8_t addr = 1; addr < 127; addr++) { // Iterates through all addresses 1-126
        Wire.beginTransmission(addr);            // If address is found this should begin the exchange with said address
        if (Wire.endTransmission() == 0) {       // ends that exchange
            if (addr == configuration.sensor_address) {
                #ifdef DEBUG_PCF8575
                DEBUG_PCF8575.printf("(I2C0 @0x%2.2X) Initialized %s Port Expander!\n", configuration.sensor_address, subsystem_name);
                #endif
                configuration.initialized = true;
            }
        }
    }
    #ifdef DEBUG_PCF8575
    if (!configuration.initialized) DEBUG_PCF8575.printf("(I2C0 -----) %s Port Expander was not found on address 0x%2.2X when scanned.\n", subsystem_name, configuration.sensor_address);
    #endif
    return 0;
}

/*! \def Keypad Matrix
    \brief used with an instance of Adafruit_PCF8575 class.

    Keypad is on the first port of the port expander on HMI.
    Scanning is acheived using domino logic.
*/
keypad_t keypad;                              // State of the keypad.
encoder_t encoder;
EventGroupHandle_t keypad_event_group = NULL; // Event group handle for keypad events.
QueueHandle_t keypad_queue = NULL;            // Queue handle for keypad input.
QueueHandle_t encoder_queue = NULL;

// Keypad scan logic (blocking delay)
uint32_t pcf8575_keypad_scan(void) {
    uint32_t key = 0;
    uint8_t row; uint8_t col;
    for (row = 0; row < KEYPAD_NUM_ROWS; row++) {
        pcf8575_writePort(pcf_hmi, PIN_KEYPAD_ROW1+row, LOW);
        vTaskDelay(KEYPAD_GPIO_STABILIZATION_TIME);
        for (col = 0; col < KEYPAD_NUM_COLS; col++) {
            if (pcf8575_readPort(pcf_hmi, PIN_KEYPAD_COL1+col) == LOW) {
                key |= KEY_PRESSED << ((row * KEYPAD_NUM_COLS) + col);
            }
        }
        pcf8575_writePort(pcf_hmi, PIN_KEYPAD_ROW1+row, HIGH); // HIGH is HiZ
    }
    return key;
}
// Keypad FreeRTOS dummy timer
static void vKeypad_timer_callback(TimerHandle_t keypadTimer) {

}
// Keypad FreeRTOS task
void vKeypad_read(void *parameter) {
    // keypad software debouncing
    keypad.timer_debounce = xTimerCreate("keypadDebounce", KEYPAD_DEBOUNCE_TIME, pdFALSE, NULL, vKeypad_timer_callback);
    // keypad event synchronization
    keypad_event_group = xEventGroupCreate();
    xEventGroupClearBits(keypad_event_group, KEY_EVENT_BITMASK);
    // keypad queue for shared state
    keypad_queue = xQueueCreate(KEYPAD_QUEUE_SIZE, sizeof(uint32_t));
    // configASSERT(keypad_queue != NULL);
    // encoder_queue = xQueueCreate(KEYPAD_QUEUE_SIZE, sizeof(enc_event_t));
    // configASSERT(encoder_queue != NULL);

    uint8_t prev_ab = 0;
    uint8_t prev_sw = 1; // pull-up compatible

    for (;;) {
        keypad.keys_pressed = pcf8575_keypad_scan();
        if (keypad.keys_pressed != 0) {
            // reset debounce if a new key was pressed
            if (keypad.keys_pressed != keypad.keys_down) {
                xTimerReset(keypad.timer_debounce, portMAX_DELAY);
            }
            // add key to currently pressed keys
            keypad.keys_down |= keypad.keys_pressed;
        } else {
            // no keys are still pressed
            if (keypad.keys_down != KEY_NONE && !xTimerIsTimerActive(keypad.timer_debounce)) {
                // Broadcast the released key
                xEventGroupSetBits(keypad_event_group, (keypad.keys_down & KEY_EVENT_BITMASK));
                // Queue the released key 
                xQueueSend(keypad_queue, &keypad.keys_down, 0);
            }
            // Reset keys_down state
            keypad.keys_down = KEY_NONE;
        }

        // uint8_t a_clock = pcf8575_readPort(pcf_hmi, PIN_ENCODER_A_CLK);
        // uint8_t b_detent = pcf8575_readPort(pcf_hmi, PIN_ENCODER_B_DT);
        // uint8_t sw = pcf8575_readPort(pcf_hmi, PIN_ENCODER_SW);
        // uint8_t ab = (a_clock << 1) | b_detent;
        // // Quadrature decode
        // uint8_t idx = (prev_ab << 2) | ab;
        // int8_t delta = quad_table[idx];
        // if (delta != 0) {
        //     enc_event_t ev = { ENC_EVT_ROTATE, delta };
        //     xQueueSend(encoder_queue, &ev, 0);
        // }
        // prev_ab = ab;
        // // Button changes
        // if (sw != prev_sw) {
        //     enc_event_t ev;
        //     ev.type = ENC_EVT_BUTTON;
        //     ev.value = (sw == 0 ? 1 : 0);  // active low assumed
        //     xQueueSend(encoder_queue, &ev, 0);
        //     prev_sw = sw;
        // }
        // vTaskDelay(pdMS_TO_TICKS(1));  // 1 kHz scan
        // #ifdef DEBUG_PCF8575
        // UBaseType_t highWater = uxTaskGetStackHighWaterMark(NULL);
        // DEBUG_PCF8575.printf("keypadTask stack high water: %u words\n", (unsigned)highWater);
        // #endif
        // Yield the CPU to other tasks
        vTaskDelay(KEYPAD_TASK_DELAY_TIME);
    }
}
#define KEYPAD_STACK_SIZE 2048
int pcf8575_scan_init(Adafruit_PCF8575 &pcf, pcf8575_config_t &configuration) {
    // PCF8575 is an open drain driver.
    pcf8575_portMode(pcf, PIN_KEYPAD_ROW1, OUTPUT); 
    pcf8575_portMode(pcf, PIN_KEYPAD_ROW2, OUTPUT);
    pcf8575_portMode(pcf, PIN_KEYPAD_ROW3, OUTPUT);
    pcf8575_portMode(pcf, PIN_KEYPAD_ROW4, OUTPUT);
    pcf8575_portMode(pcf, PIN_ENCODER_A_CLK, OUTPUT);
    // PCF8575 does NOT have internal pullups!
    pcf8575_portMode(pcf, PIN_KEYPAD_COL1, INPUT); 
    pcf8575_portMode(pcf, PIN_KEYPAD_COL2, INPUT);
    pcf8575_portMode(pcf, PIN_KEYPAD_COL3, INPUT);
    pcf8575_portMode(pcf, PIN_KEYPAD_COL4, INPUT);
    pcf8575_portMode(pcf, PIN_ENCODER_B_DT, INPUT);
    pcf8575_portMode(pcf, PIN_ENCODER_SW, INPUT);
    keypad.keys_pressed = 0;
    keypad.keys_down = 0;
    xTaskCreate(vKeypad_read, "keypadTask", KEYPAD_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 3), NULL);
    return 0;
}