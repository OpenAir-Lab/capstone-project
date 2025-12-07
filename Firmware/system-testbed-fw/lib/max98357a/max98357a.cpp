#include <max98357a.h>

extern max98357a_config_t max98357a;

// https://www.analog.com/en/products/MAX98357A.html
int max98357a_init(max98357a_config_t &max98357a) {
    // I2SClass I2S is internally I2S0 controller.
    I2S.begin(I2S_PHILIPS_MODE, max98357a.sample_rate, max98357a.bits_per_sample);
    I2S.setSckPin(max98357a.pin_bclk); // continuous serial clock is bit clock
    I2S.setFsPin(max98357a.pin_lrclk); // frame sync is left-right clock
    I2S.setDataInPin(max98357a.pin_data_in);
    return 0;
}


#define BUFFER_SAMPLES 128 // indicating how many sample of buffer will be read and write during I2S cycle , 64-512 ( the bigger number the bigger latency, lower can be affected to CPU)

extern int16_t max98357a_data_buffer[BUFFER_SAMPLES];

int max98357a_playback(int16_t *max98357a_data_buffer) {
    I2S.setBufferSize(sizeof(max98357a_data_buffer));
    I2S.write(max98357a_data_buffer, sizeof(max98357a_data_buffer));
    return 0;
}