#include <stdint.h>
#include <stdio.h>

typedef struct {
    int8_t pin_shutdown;
    int8_t pin_enable1; // input stage
    int8_t pin_enable2; // output stage
} grf5604_config_t;

int grf5604_init(grf5604_config_t &grf5604);