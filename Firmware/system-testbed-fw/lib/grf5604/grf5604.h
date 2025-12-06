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

int grf5604_powerup(grf5604_config_t &grf5604);

int grf5604_powerdown(grf5604_config_t &grf5604);

void grf5604_drawPowerup(grf5604_config_t &grf5604);
void grf5604_drawPowerdown(grf5604_config_t &grf5604);
void grf5604_drawPorts(grf5604_config_t &grf5604);

int grf5604_init(grf5604_config_t &grf5604);

void demonstrate_radio_amplifier();