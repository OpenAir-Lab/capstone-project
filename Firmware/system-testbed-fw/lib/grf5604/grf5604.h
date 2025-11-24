#include <stdint.h>
#include <stdio.h>

typedef enum {
    UHF,
    VHF
} grf5604_band_t;
typedef struct {
    bool initialized = false;
    int8_t pin_shutdown;
    int8_t pin_enable1; // input stage
    int8_t pin_enable2; // output stage
    bool band = UHF;
} grf5604_config_t;

int grf5604_init(grf5604_config_t &grf5604);

void demonstrate_radio_amplifier();