// #include <Arduino.h>
// #include <driver/i2s.h> 
// #include <hal/i2s_hal.h>

#include <I2S.h>

typedef struct {
    uint8_t pin_bclk; // continuous serial clock (SCK), bit clock (BCLK)
    uint8_t pin_lrclk; // word select (WS), left-right clock (LRCLK)
    uint8_t pin_data_in;
    uint16_t sample_rate;
    uint8_t bits_per_sample;
} max98357a_config_t;

int max98357a_init(max98357a_config_t &max98357a);

