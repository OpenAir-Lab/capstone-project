#pragma once
#include <stdint.h>
#include <stdbool.h>

// These depend on your actual PCF8575 pin mapping.
// TODO: fill these in based on your schematic / modulino design.
#define HMI_PTT_PIN      0   // PTT on port 0
#define HMI_ENC_A_PIN    1   // TODO: encoder A pin
#define HMI_ENC_B_PIN    2   // TODO: encoder B pin

// Returns +1, -1, or 0 depending on encoder movement since last call.
int8_t hmi_read_encoder_step();

// Returns true if PTT is currently pressed.
bool hmi_read_ptt();
