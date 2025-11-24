#include <grf5604.h>
#include <sky13330.h>
#include <pcf8575.h>
#include <st7789.h>
#include <Arduino.h>

#define RFAMP_DEBUG Serial

extern Adafruit_ST7789 tft;
extern st7789_config_t display_config; 

extern Adafruit_PCF8575 pcf_radio;
extern pcf8575_config_t pcf_radio_config;

extern grf5604_config_t uhf_grf5604;
extern grf5604_config_t vhf_grf5604;

static const unsigned char PROGMEM image_amplifier_bits[] = {0x00,0x0e,0x00,0x00,0x00,0x00,0x00,0x0f,0x80,0x00,0x00,0x00,0x00,0x0c,0xe0,0x00,0x00,0x00,0x00,0x0c,0x70,0x00,0x00,0x00,0x00,0x0c,0x1c,0x00,0x00,0x00,0x00,0x0c,0x07,0x00,0x00,0x00,0x00,0x0c,0x01,0x80,0x00,0x00,0x00,0x0c,0x00,0x10,0x00,0x00,0x00,0x0c,0x00,0x18,0x00,0x00,0x00,0x0c,0x00,0x0e,0x00,0x00,0x00,0x0c,0x00,0x03,0x80,0x00,0xff,0xfc,0x00,0x00,0xff,0xe0,0x00,0x0c,0x00,0x03,0x80,0x00,0x00,0x0c,0x00,0x07,0x00,0x00,0x00,0x0c,0x00,0x1c,0x00,0x00,0x00,0x0c,0x00,0x70,0x00,0x00,0x00,0x0c,0x01,0xc0,0x00,0x00,0x00,0x0c,0x07,0x00,0x00,0x00,0x00,0x0c,0x1c,0x00,0x00,0x00,0x00,0x0c,0x38,0x00,0x00,0x00,0x00,0x0c,0x20,0x00,0x00,0x00,0x00,0x0f,0x80,0x00,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,0x00};
// VSHDN ≥ 1.8 volts (logic HIGH) disables device. 
// VSHDN ≤ 0.8 volts (logic LOW) enables device.

// VEN1 and series resistor set ICCQ for the input stage.
// VEN1 ≤ 0.2 volts disables stage 1.

// VEN2 and series resistor set ICCQ for the output stage.
// VEN2 ≤ 0.2 volts disables stage 2.

// AN006 Power-up Sequence:
// 1) Ensure that the device under test (DUT) is properly terminated with the correct source/load impedances and ground
// connections. In the case of an evaluation board or production test environment, this typically means that the board input
// and output ports are connected to a vector network analyzer (VNA) or other measurement equipment prior to power-up.

// 2) Apply main power supply V DD/VCC. As noted in item 1, all required ground connections should be made prior to the
// application of any voltage to the device.

// 3) Apply enable voltages at a time ≥ the time when VDD /VCC is applied. The key point here is that VENABLE should not occur
// prior to the application of the drain/collector voltages of the amplifier.

// 4) Apply RF input power. Note: Turning the device on (steps 2 and 3) with RF already applied is known as “hot switching”
// and is a frequent cause of amplifier damage. During device power-up, the device impedances and gain undergo a transition
// through a range of values prior to settling. During this time, a device which is stable under steady-state conditions may
// become unstable. Applying RF power can sometimes lead to destructive oscillations.

int grf5604_powerup(grf5604_config_t &grf5604) {
    #ifdef RFAMP_DEBUG
    RFAMP_DEBUG.printf("(I2C0 @0x%2.2X) %s Radio Amplifier powering up...\n",
        pcf_radio_config.sensor_address, grf5604.band ? "VHF" : "UHF"
    );
    #endif
    // TODO: stop transmitting, is there a lock?
    pcf8575_writePort(pcf_radio, grf5604.pin_enable1, LOW);
    pcf8575_writePort(pcf_radio, grf5604.pin_enable2, LOW);
    pcf8575_writePort(pcf_radio, grf5604.pin_shutdown, LOW);
    return 0;
};

// AN006 Power-Down Sequence:
// 1) Remove RF input power.
// 2) Bring VENABLE voltages to ground potential.
// 3) Bring VDD/VCC to ground potential at a time ≥ to the time when the VENABLE voltages are brought to ground.
int grf5604_powerdown(grf5604_config_t &grf5604) {
    #ifdef RFAMP_DEBUG
    RFAMP_DEBUG.printf("(I2C0 @0x%2.2X) %s Radio Amplifier powering down...\n", 
        pcf_radio_config.sensor_address, grf5604.band ? "VHF" : "UHF"
    );
    #endif
    tft.fillRect(6, 7, 307, 17, ST77XX_BLACK); // 'blanks out' message selection
    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(6, 19); tft.printf("Turning Off %s Amplifier...\n", 
        (grf5604.band == VHF) ? "VHF" : "UHF"
    );
    tft.fillRect(9,   (grf5604.band == UHF) ? 56 : 102, 75, 11, ST77XX_BLACK); 
    tft.fillRect(238, (grf5604.band == UHF) ? 56 : 102, 75, 11, ST77XX_BLACK);
    // TODO: stop transmitting, is there a lock?
    pcf8575_writePort(pcf_radio, grf5604.pin_enable1, HIGH);
    pcf8575_writePort(pcf_radio, grf5604.pin_enable2, HIGH);
    pcf8575_writePort(pcf_radio, grf5604.pin_shutdown, HIGH);
    return 0;
};
/* GRF5604 Amplifier Initialization
 * \brief power down amplifier and ready control GPIO.
 */
int grf5604_init(grf5604_config_t &grf5604) {
    if (grf5604.initialized) {
        return 0;
    }
    #ifdef RFAMP_DEBUG
    RFAMP_DEBUG.printf(grf5604.band ?                              \
        "(I2C0 @0x%2.2X) Beginning use of VHF Radio Amplifier... " \
        "[SD=%d, EN3=%d, EN4=%d on %s IOMUX]\n" :    \
        "(I2C0 @0x%2.2X) Beginning use of UHF Radio Amplifier... " \
        "[SD=%d, EN1=%d, EN2=%d on %s IOMUX]\n", pcf_radio_config.sensor_address,
        grf5604.pin_shutdown, grf5604.pin_enable1, grf5604.pin_enable2, pcf_radio_config.subsystem_name.c_str()
    );
    #endif
    if (digitalPinCanOutput(grf5604.pin_shutdown) && digitalPinCanOutput(grf5604.pin_enable1) && digitalPinCanOutput(grf5604.pin_enable2)) {
        pcf8575_portMode(pcf_radio, grf5604.pin_shutdown, OUTPUT);
        pcf8575_portMode(pcf_radio, grf5604.pin_enable1, OUTPUT);
        pcf8575_portMode(pcf_radio, grf5604.pin_enable2, OUTPUT);
        grf5604_powerdown(grf5604);
        grf5604.initialized == true;
        #ifdef RFAMP_DEBUG
        RFAMP_DEBUG.printf("(I2C0 @0x%2.2X) Initialized %s Radio Amplifier!\n", pcf_radio_config.sensor_address, grf5604.band ? "VHF" : "UHF");
        #endif
    } else {
        #ifdef RFAMP_DEBUG
        RFAMP_DEBUG.printf("(I2C0 @0x%2.2X) %s Radio Amplifier may not yet be initialized...\n", pcf_radio_config.sensor_address, grf5604.band ? "VHF" : "UHF");
        #endif
        grf5604.initialized == false;
    }
    return grf5604.initialized;
}

void grf5604_drawPowerup(grf5604_config_t &grf5604) {
    tft.fillRect(6, 7, 307, 17, ST77XX_BLACK); // 'blanks out' message selection
    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(23, 19); tft.printf("Enabling %s Amplifier...\n", 
        (grf5604.band == VHF) ? "VHF" : "UHF"
    );
}

void grf5604_drawPowerdown(grf5604_config_t &grf5604) {
    tft.fillRect(6, 7, 307, 17, ST77XX_BLACK); // 'blanks out' message selection
    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(6, 19); tft.printf("Turning Off %s Amplifier...\n",
        (grf5604.band == VHF) ? "VHF" : "UHF"
    );
    tft.fillRect(9,   (grf5604.band == UHF) ? 56 : 100, 75, 13, ST77XX_BLACK); 
    tft.fillRect(238, (grf5604.band == UHF) ? 56 : 100, 75, 13, ST77XX_BLACK);
}

void grf5604_drawPorts(grf5604_config_t &grf5604) {
    tft.fillRect(6, 7, 306, 17, ST77XX_BLACK); // 'blanks out' message selection
    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(6, 19); tft.printf("Amplifying %s Transmissions\n",
        (grf5604.band == VHF) ? "VHF" : "UHF"
    );
     // 'blanks out' applied RF power selection
    tft.fillRect(9,   (grf5604.band == UHF) ? 102 : 57, 75, 11, ST77XX_BLACK); 
    tft.fillRect(238, (grf5604.band == UHF) ? 102 : 57, 75, 11, ST77XX_BLACK);
    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(7, (grf5604.band == UHF) ? 67 : 111);   tft.print("+16 dBm");
    tft.setCursor(237, (grf5604.band == UHF) ? 67 : 111); tft.print("+37 dBm");
    int portColor;
    tft.setFont((grf5604.band == UHF) ? &FreeMonoBold9pt7b : &FreeMono9pt7b);
    portColor = (grf5604.band == UHF) ? 0x250D : 0x73AF;
    tft.fillRoundRect(90, 50, 25, 25, 3, portColor);
    tft.setCursor(92, 68);   tft.printf("P1");
    tft.fillRoundRect(205, 50, 25, 25, 3, portColor);
    tft.setCursor(207, 68);  tft.printf("P2");
    tft.setFont((grf5604.band == VHF) ? &FreeMonoBold9pt7b : &FreeMono9pt7b);
    portColor = (grf5604.band == VHF) ? 0x250D : 0x73AF;
    tft.fillRoundRect(90, 95, 25, 25, 3, portColor);
    tft.setCursor(92, 112);  tft.printf("P3");
    tft.fillRoundRect(205, 95, 25, 25, 3, portColor);
    tft.setCursor(207, 112); tft.printf("P4");
}

void demonstrate_radio_amplifier() {
    #ifdef RFAMP_DEBUG
    RFAMP_DEBUG.printf("Entering Radio Amplifier Demonstration!\n");
    #endif
    // demonstration visuals
    display_init(display_config);
    tft.fillRoundRect(103, 40, 115, 90, 10, 0x61B0);
    tft.drawBitmap(138, 51, image_amplifier_bits, 43, 23, 0xFFFF);
    tft.drawLine(137, 62, 115, 62, 0xFFFF);
    tft.drawLine(204, 62, 181, 62, 0xFFFF);
    tft.drawBitmap(138, 96, image_amplifier_bits, 43, 23, 0xFFFF);
    tft.drawLine(138, 107, 116, 107, 0xFFFF);
    tft.drawLine(205, 107, 182, 107, 0xFFFF);
    tft.fillRoundRect(90, 50, 25, 25, 3, 0xE0C4); // P2
    tft.fillRoundRect(90, 95, 25, 25, 3, 0xE0C4); // P3
    tft.fillRoundRect(205, 50, 25, 25, 3, 0xE0C4); // P4
    tft.fillRoundRect(205, 95, 25, 25, 3, 0xE0C4); // P5
    tft.setTextWrap(false);
    tft.setTextColor(0xFFFF);
    tft.setFont(&FreeMonoBoldOblique9pt7b);
    tft.setCursor(72, 157);
    tft.print("RF Amplifier DUT");
    tft.setFont(&FreeMonoOblique9pt7b);
    tft.setCursor(6, 49);    tft.print("CC1200");
    tft.setCursor(236, 48);  tft.print("GRF5604");
    tft.setFont();
    tft.setCursor(134, 42);  tft.print("High Band");
    tft.setCursor(137, 121); tft.print("Low Band");
    tft.setCursor(119, 82);  tft.print("Range Extender");
    tft.setCursor(8, 87);    tft.print("Internal PA");
    tft.setCursor(247, 87);  tft.print("External PA");
    tft.setCursor(9, 76);    tft.print("Applied Power");
    tft.setCursor(241, 76);  tft.print("Output Power");
    // demonstration logic
    // utilizing UHF amplifier requires TX_H port
    grf5604_drawPorts(uhf_grf5604);
    rfsw_switchTo(TX_UHF); // S31 TX_UHF
    grf5604_powerup(uhf_grf5604);
    grf5604_drawPowerup(uhf_grf5604);
    delay(15000); // wait 15 seconds
    grf5604_powerdown(uhf_grf5604);
    grf5604_drawPowerdown(uhf_grf5604);
    delay(15000); // wait 15 seconds
    // utilizing VHF amplifier requires TX_L port
    // TODO: Investigate why drawPorts has to be done before switchTo()
    grf5604_drawPorts(vhf_grf5604);
    rfsw_switchTo(TX_VHF); // S51 TX_VHF
    grf5604_powerup(vhf_grf5604);
    grf5604_drawPowerup(vhf_grf5604);
    delay(15000); // wait 15 seconds
    grf5604_powerdown(vhf_grf5604);
    grf5604_drawPowerdown(vhf_grf5604);
    delay(15000); // wait 15 seconds
}