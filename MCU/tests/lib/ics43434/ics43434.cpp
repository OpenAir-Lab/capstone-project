#include <ics43434.h>

extern ics43434_config_t ics43434;

int ics43434_init(ics43434_config_t &ics43434) {
    // I2SClass I2S is internally I2S0 controller.
    I2S.begin(I2S_PHILIPS_MODE, ics43434.sample_rate, ics43434.bits_per_sample);
    I2S.setSckPin(ics43434.pin_bclk); // continuous serial clock is bit clock
    I2S.setFsPin(ics43434.pin_lrclk); // frame sync is left-right clock
    I2S.setDataOutPin(ics43434.pin_data_out);
    return 0;
}