#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "HardwareSerial.h"
#include "Wire.h"
#include <Adafruit_PCF8575.h>

// Keypad implementation is FreeRTOS friendly
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
// type define of structure holding the state of the keypad.
#define KEY_EVENT_BITMASK 0x00FFFFFFU // FreeRTOS 24-bit EventGroup limit
// Configuration for the matrix keypad
#define KEYPAD_GPIO_STABILIZATION_TIME (1UL / portTICK_PERIOD_MS)  // Delay for GPIO pin stabilization
#define KEYPAD_TASK_DELAY_TIME         (5UL / portTICK_PERIOD_MS)  // Delay for keypad tasks
#define KEYPAD_DEBOUNCE_TIME           (50UL / portTICK_PERIOD_MS) // Time to stabilize key after being pressed
#define KEYPAD_QUEUE_SIZE              10                          // Size of keypad queue
#define KEY_PRESSED                    HIGH
#define KEY_NONE                       LOW
#define KEY_LONG                          0x10000
// TODO: key mapping 
/**
 *     C1  C2  C3  C4
 * R1: 1   2   3   ←LEFT
 * R2: 4   5   6   →RIGHT
 * R3: 7   8   9   ↑UP
 * R4: *   0   #   ↓DOWN
 */


// T9 keypad (cols 1–3)
#define KEY_1         (1U << 0)
#define KEY_2         (1U << 1)
#define KEY_3         (1U << 2)
#define KEY_4         (1U << 4)  
#define KEY_5         (1U << 5)  
#define KEY_6         (1U << 6)  
#define KEY_7         (1U << 8)  
#define KEY_8         (1U << 9)  
#define KEY_9         (1U << 10) 
#define KEY_STAR      (1U << 12)
#define KEY_0         (1U << 13)
#define KEY_POUND     (1U << 14)

#define KEY_UP        (1U << 7)
#define KEY_DOWN      (1U << 11) // not working
#define KEY_LEFT      (1U << 15)
#define KEY_RIGHT     (1U << 3)

typedef struct {
    uint8_t col_count;            // Number of columns in the keypad matrix
    uint8_t row_count;            // Number of rows in the keypad matrix
    uint32_t keys_pressed;        // Bitmask representing the currently pressed keys
    uint32_t keys_down;           // Bitmask representing the keys that were just pressed
    TimerHandle_t timer_debounce; // Timer handle for key debounce functionality
} keypad_t;

// Assign Port 0 on HMI Port Expander for Encoder
#define PIN_ENCODER_A_CLK 5
#define PIN_ENCODER_B_DT 6
#define PIN_ENCODER_SW 7
// Assign Port 1 on HMI Port Expander for Keypad
#define KEYPAD_NUM_ROWS 4
#define KEYPAD_NUM_COLS 4
#define PIN_KEYPAD_ROW1 8
#define PIN_KEYPAD_ROW2 9
#define PIN_KEYPAD_ROW3 10
#define PIN_KEYPAD_ROW4 11
#define PIN_KEYPAD_COL1 12
#define PIN_KEYPAD_COL2 13
#define PIN_KEYPAD_COL3 14
#define PIN_KEYPAD_COL4 15
typedef struct {
	bool initialized = false;
	TwoWire *i2c;
	uint8_t sensor_address = PCF8575_I2CADDR_DEFAULT;
	uint8_t pin_interrupt;
	std::string subsystem_name = "N/A";
} pcf8575_config_t;

bool pcf8575_portMode(Adafruit_PCF8575 &pcf, uint8_t port, uint8_t mode);

int pcf8575_readPort(Adafruit_PCF8575 &pcf, uint8_t port);

int pcf8575_writePort(Adafruit_PCF8575 &pcf, uint8_t port, uint8_t value);

int pcf8575_init(Adafruit_PCF8575 &pcf, pcf8575_config_t &configuration);

int pcf8575_scan_init(Adafruit_PCF8575 &pcf, pcf8575_config_t &configuration);

// Keypad scan logic
uint32_t pcf8575_keypad_scan(void);
// Keypad FreeRTOS task
void vKeypad_read(void *parameter);

// Quadrature encoding logic
typedef enum {
    ENC_EVT_ROTATE,
    ENC_EVT_BUTTON
} enc_event_type_t;

typedef struct {
    enc_event_type_t type;
    int32_t value;  // +1, -1, press/release
} enc_event_t;

typedef struct {
    uint8_t pin_a;
    uint8_t pin_b;
    uint8_t pin_sw;
    uint8_t prev_ab;
    uint8_t prev_sw;
} encoder_t;