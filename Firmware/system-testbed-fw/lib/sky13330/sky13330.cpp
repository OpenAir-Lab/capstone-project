#include <sky13330.h>
#include <Arduino.h>
#include <st7735.h>

extern Adafruit_ST7789 tft;

#define RFSW_DEBUG Serial

extern sky13330_config_t sky13330;

const char* scatter_parameter[4] = {
    (char*)"S21", // RX_UHF // band = 0, trx = 0
    (char*)"S31", // TX_UHF // band = 0, trx = 1
    (char*)"S41", // RX_VHF // band = 1, trx = 0
    (char*)"S51"  // TX_VHF // band = 1, trx = 1
};

void rfsw_switchTo(bool band_select, bool trx_select) {
    digitalWrite(sky13330.pin_band, band_select);
    digitalWrite(sky13330.pin_trx, trx_select);
    int8_t port_select = (band_select << 1) + (trx_select);
    #ifdef RFSW_DEBUG
    RFSW_DEBUG.printf("Now switched to P%d band=%d trx=%d for measurement %s\n", 2+port_select, band_select, trx_select, scatter_parameter[port_select]);
    #endif
}

void rfsw_drawPort(rfsw_port_selections_t port_select) {
    int portColor;
    if (port_select == sky13330.port_active) {
        tft.setFont(&FreeMonoBold9pt7b);
        portColor = 0x250D;
    } else {
        tft.setFont(&FreeMono9pt7b);
        portColor = 0x73AF;
    }
    switch(port_select) {
        case(RX_UHF):
            tft.fillRoundRect(183, 50, 25, 25, 3, portColor);
            tft.setCursor(185, 68);
            break;
        case(TX_UHF):
            tft.fillRoundRect(115, 50, 25, 25, 3, portColor);
            tft.setCursor(117, 68);
            break;
        case(RX_VHF):
            tft.fillRoundRect(183, 95, 25, 25, 3, portColor);
            tft.setCursor(185, 112);
            break;
        case(TX_VHF):
            tft.fillRoundRect(115, 95, 25, 25, 3, portColor);
            tft.setCursor(117, 112);
            break;
    }
    tft.printf("P%d",(2 + port_select));
}

void rfsw_switchTo(rfsw_port_selections_t port_select) {
    rfsw_switchTo((BIT1 & port_select),(BIT0 & port_select));
    sky13330.port_active = port_select;

    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(150, 90);
    tft.print("P1");

    rfsw_drawPort(RX_UHF);
    rfsw_drawPort(TX_UHF);
    rfsw_drawPort(RX_VHF);
    rfsw_drawPort(TX_VHF);
   
    tft.fillRect(257, 11, 48, 19, ST77XX_BLACK);
    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(23, 26);
    tft.printf("Switched to measuring %s\n", scatter_parameter[port_select]);
}

int sky13330_init(sky13330_config_t &sky13330) {
    digitalWrite(sky13330.pin_enable, HIGH);
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
    #ifdef RFSW_DEBUG
    RFSW_DEBUG.printf("Entering Radio Switch Demonstration!\n");
    #endif
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(1);

    tft.setTextWrap(false);
    tft.setTextColor(0xFFFF);
    tft.setCursor(10, 20);

    tft.fillRoundRect(126, 40, 69, 90, 10, 0x61B0);
    tft.fillRoundRect(148, 73, 25, 25, 3, 0xE521); // COM
    tft.fillRoundRect(115, 50, 25, 25, 3, 0xE0C4); // P2
    tft.fillRoundRect(115, 95, 25, 25, 3, 0xE0C4); // P3
    tft.fillRoundRect(183, 50, 25, 25, 3, 0xE0C4); // P4
    tft.fillRoundRect(183, 95, 25, 25, 3, 0xE0C4); // P5

    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(24, 128);
    tft.print("VHF TX");

    tft.setCursor(24, 55);
    tft.print("UHF TX");

    tft.setCursor(232, 128);
    tft.print("VHF RX");

    tft.setCursor(232, 55);
    tft.print("UHF RX");

    tft.setFont(&FreeMonoBoldOblique9pt7b);
    tft.setCursor(105, 155);
    tft.print("Switch DUT");
    
    rfsw_switchTo(RX_UHF);
    delay(15000); // wait 15 seconds
    rfsw_switchTo(TX_UHF);
    delay(15000); // wait 15 seconds
    rfsw_switchTo(RX_VHF);
    delay(15000); // wait 15 seconds
    rfsw_switchTo(TX_VHF);
    delay(15000); // wait 15 seconds
}