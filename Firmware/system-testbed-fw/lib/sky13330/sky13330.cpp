#include <sky13330.h>
#include <Arduino.h>

#define RFSW_DEBUG Serial

extern sky13330_config_t sky13330;

const char* scatter_parameter[4] = {
        (char*)"S21", // RX_UHF
        (char*)"S31", // RX_VHF
        (char*)"S41", // TX_UHF
        (char*)"S51"  // TX_VHF 
};

void rfsw_switchTo(bool band_select, bool trx_select) {
    digitalWrite(sky13330.pin_band, band_select);
    digitalWrite(sky13330.pin_trx, trx_select);
    #ifdef DEBUG
    DEBUG.printf("Now switched to measurement %s\n", scatter_parameter[(int)((radio_band << 1) & radio_trx)]);
    #endif 
        // Port 1 (Common) is antenna
    /* band select 
        * pin 11 net SW5 -> RF2 (vhf select switch) BAND = HIGH
        * pin 5  net SW5 -> RF3 (uhf select switch) BAND = LOW
        * SW5 -> ANTENNA
        */

    if (band_select == LOW) { // UHF
        if (trx_select == LOW) {
                // S21
                /* uhf select
                * pin 5  net SW4 -> RF3 (RX_UHF) TRX = LOW
                * pin 11 net SW4 -> RF2 (TX_UHF) TRX = HIGH
                * SW4 -> BAND SELECT
                */
            } else {
                // S31
            }
    } else {
            /* vhf select
            * pin 5  net SW3 -> RF3 (RX_VHF) TRX = LOW
            * pin 11 net SW3 -> RF2 (TX_VHF) TRX = HIGH 
            * SW3 -> BAND SELECT
            */
            if (trx_select == LOW) {
                // S41
            } else {
                // S51
            }
    }
}

void rfsw_switchTo(rfsw_port_selections_t port_select) {
    rfsw_switchTo((port_select << 1 ), port_select);
    #ifdef RFSW_DEBUG
    RFSW_DEBUG.printf("Now switched to measurement %s\n", scatter_parameter[(int)(port_select)]);
    #endif 
        // Port 1 (Common) is antenna
    /* band select 
        * pin 11 net SW5 -> RF2 (vhf select switch) BAND = HIGH
        * pin 5  net SW5 -> RF3 (uhf select switch) BAND = LOW
        * SW5 -> ANTENNA
        */
}

int sky13330_init(sky13330_config_t &sky13330) {
    rfsw_switchTo(RX_UHF);
    return 0;
}

/*
- [ ] Can the RF subsystem receive a signal through one of the two receive ports of the 4-to-1 switch? If so, can the correct band be selected?
- [ ] For transmission, can the RF switch being driven by a VNA move the input signal from either Port 2, 3, 4, or 5 to Port 1 (antenna)? If so, can the input signal be measured through the antenna port, Port 1?
- [ ] Before introducing the Amplifier Modulino in line with the RF transceiver, first attempt to use the transceiver directly. If the switch behavior is consistent, only then re-run the tests with amplification. 
*/
// TODO: Utilize "press any key to continue" instead of time delay.
void demonstrate_radio_switch() {
    rfsw_switchTo(RX_UHF);
    delay(30000); // wait 30 seconds
    rfsw_switchTo(TX_UHF);
    delay(30000); // wait 30 seconds
    rfsw_switchTo(RX_VHF);
    delay(30000); // wait 30 seconds
    rfsw_switchTo(TX_VHF);
}