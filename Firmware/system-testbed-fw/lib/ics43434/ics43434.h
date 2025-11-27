#include <I2S.h>

typedef struct {
    uint8_t pin_bclk; // continuous serial clock (SCK), bit clock (BCLK)
    uint8_t pin_lrclk; // word select (WS), left-right clock (LRCLK)
    uint8_t pin_data_out;
    uint16_t sample_rate;
    uint8_t bits_per_sample;
} ics43434_config_t;

int ics43434_init(ics43434_config_t &ics43434);