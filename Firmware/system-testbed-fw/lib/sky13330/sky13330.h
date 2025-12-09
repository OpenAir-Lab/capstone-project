#include <stdint.h>
#include <stdio.h>

typedef enum {
    RX_UHF,
    TX_UHF,
    RX_VHF,
    TX_VHF
} rfsw_port_selections_t;
typedef struct {
    bool initialized = false;
    int8_t pin_band;
    int8_t pin_trx;
    int8_t pin_enable;
    rfsw_port_selections_t port_active;
} sky13330_config_t;


void rfsw_switchTo(bool band_select, bool trx_select);

void rfsw_switchTo(rfsw_port_selections_t port_select);

int sky13330_init(sky13330_config_t &sky13330);

void demonstrate_radio_switch(void *parameter);