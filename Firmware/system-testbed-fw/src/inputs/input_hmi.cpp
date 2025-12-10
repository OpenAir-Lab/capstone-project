#include "input_hmi.h"
#include <Arduino.h>
#include <pcf8575.h>

// Defined in main.cpp
extern Adafruit_PCF8575 pcf_hmi;

// Simple state for encoder decoding
static bool last_enc_a = false;
static bool last_enc_b = false;
static bool encoder_initialized = false;

static void init_encoder_state_once() {
    if (!encoder_initialized) {
        last_enc_a = pcf_hmi.digitalRead(HMI_ENC_A_PIN);
        last_enc_b = pcf_hmi.digitalRead(HMI_ENC_B_PIN);
        encoder_initialized = true;
    }
}

int8_t hmi_read_encoder_step() {
    init_encoder_state_once();

    bool a = pcf_hmi.digitalRead(HMI_ENC_A_PIN);
    bool b = pcf_hmi.digitalRead(HMI_ENC_B_PIN);

    int8_t step = 0;

    // Very simple quadrature decode: detect edges on A
    if (a != last_enc_a) {
        // A changed; direction depends on B
        if (a == b) {
            step = +1;
        } else {
            step = -1;
        }
    }

    last_enc_a = a;
    last_enc_b = b;

    return step;
}

bool hmi_read_ptt() {
    // NOTE: depends on whether PTT is active-low or active-high.
    // If hardware pulls the pin low when pressed, invert the read here.
    bool raw = pcf_hmi.digitalRead(HMI_PTT_PIN);

    // TODO: adjust this based on hardware:
    // return !raw;   // if active-low
    return raw;       // if active-high
}
