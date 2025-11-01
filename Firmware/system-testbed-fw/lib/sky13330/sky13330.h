#include <stdint.h>
#include <stdio.h>

typedef struct {
    int8_t pin_band;
    int8_t pin_trx;
} sky13330_config_t;

typedef enum {
    RX_UHF,
    RX_VHF,
    TX_UHF,
    TX_VHF
} rfsw_port_selections_t;

void rfsw_switchTo(bool band_select, bool trx_select);

void rfsw_switchTo(rfsw_port_selections_t port_select);

int sky13330_init(sky13330_config_t &sky13330);

void demonstrate_radio_switch();