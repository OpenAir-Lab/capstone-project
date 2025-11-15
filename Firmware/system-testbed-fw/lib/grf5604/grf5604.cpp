#include <grf5604.h>
#include <pcf8575.h>
#include <st7735.h>
#include <Arduino.h>

#define RFAMP_DEBUG Serial

extern Adafruit_ST7789 tft;

extern grf5604_config_t uhf_grf5604;
extern grf5604_config_t vhf_grf5604;

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
    // TODO: stop transmitting, is there a lock?
    digitalWrite(grf5604.pin_enable1, HIGH);
    digitalWrite(grf5604.pin_enable2, HIGH);
    digitalWrite(grf5604.pin_shutdown, LOW);
    return 0;
};
// AN006 Power-Down Sequence:
// 1) Remove RF input power.
// 2) Bring VENABLE voltages to ground potential.
// 3) Bring VDD/VCC to ground potential at a time ≥ to the time when the VENABLE voltages are brought to ground.
int grf5604_powerdown(grf5604_config_t &grf5604) {
    // TODO: stop transmitting, is there a lock?
    digitalWrite(grf5604.pin_enable1, LOW);
    digitalWrite(grf5604.pin_enable2, LOW);
    digitalWrite(grf5604.pin_shutdown, HIGH);
    return 0;
};
/* GRF5604 Amplifier Initialization
 * \brief power down amplifier and ready control GPIO.
 */
int grf5604_init(grf5604_config_t &grf5604) {
    // if (!digitalPinCanOutput(grf5604.pin_shutdown)) {
    //     throw std::runtime_error("Error: Shutdown Pin " + String(grf5604.pin_shutdown) + " must be a digital pin that can output.");
    // }
    // if (!digitalPinCanOutput(grf5604.pin_enable1)) {
    //     throw std::runtime_error("Error: Enable1 Pin " + String(grf5604.pin_enable1) + " must be a digital pin that can output.");
    // }
    // if (digitalPinCanOutput(grf5604.pin_enable2)) {
    //     throw std::runtime_error("Error: Enable2 Pin " + String(grf5604.pin_enable2) + " must be a digital pin that can output.");
    // }
    pinMode(grf5604.pin_shutdown, OUTPUT);
    pinMode(grf5604.pin_enable1, OUTPUT);
    pinMode(grf5604.pin_enable2, OUTPUT);
    return grf5604_powerdown(grf5604);
}


void grf5604_drawPorts(grf5604_config_t &grf5604) {
    int portColor;
    tft.setFont((grf5604.band == UHF) ? &FreeMonoBold9pt7b : &FreeMono9pt7b);
    portColor = (grf5604.band == UHF) ? 0x250D : 0x73AF;
    tft.fillRoundRect(90, 50, 25, 25, 3, portColor);
    tft.setCursor(92, 68);
    tft.printf("P1");
    tft.fillRoundRect(205, 50, 25, 25, 3, portColor);
    tft.setCursor(207, 68);
    tft.printf("P2");
    tft.setFont((grf5604.band == VHF) ? &FreeMonoBold9pt7b : &FreeMono9pt7b);
    portColor = (grf5604.band == VHF) ? 0x250D : 0x73AF;
    tft.fillRoundRect(90, 95, 25, 25, 3, portColor);
    tft.setCursor(92, 112);
    tft.printf("P3");
    tft.fillRoundRect(205, 95, 25, 25, 3, portColor);
    tft.setCursor(207, 112);
    tft.printf("P4");
}

void demonstrate_radio_amplifier() {
    #ifdef RFAMP_DEBUG
    RFAMP_DEBUG.printf("Entering Radio Amplifier Demonstration!\n");
    #endif
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(1);

    tft.setTextWrap(false);
    tft.setTextColor(0xFFFF);
    // tft.setCursor(10, 20);

    tft.fillRoundRect(103, 40, 115, 90, 10, 0x61B0);
    tft.fillRoundRect(90, 50, 25, 25, 3, 0xE0C4); // P2
    tft.fillRoundRect(90, 95, 25, 25, 3, 0xE0C4); // P3
    tft.fillRoundRect(205, 50, 25, 25, 3, 0xE0C4); // P4
    tft.fillRoundRect(205, 95, 25, 25, 3, 0xE0C4); // P5

    #ifdef RFAMP_DEBUG
    RFAMP_DEBUG.printf("(I2C0 0x2?) UHF Radio Amplifier powering up...\n");
    #endif
    grf5604_drawPorts(uhf_grf5604);
    grf5604_powerup(uhf_grf5604);
    delay(15000); // wait 15 seconds
    #ifdef RFAMP_DEBUG
    RFAMP_DEBUG.printf("(I2C0 0x2?) UHF Radio Amplifier powering down...\n");
    #endif
    grf5604_powerdown(uhf_grf5604);
    delay(15000); // wait 15 seconds
    #ifdef RFAMP_DEBUG
    RFAMP_DEBUG.printf("(I2C0 0x2?) VHF Radio Amplifier powering up...\n");
    #endif
    grf5604_drawPorts(vhf_grf5604);
    grf5604_powerup(vhf_grf5604);
    delay(15000); // wait 15 seconds
    #ifdef RFAMP_DEBUG
    RFAMP_DEBUG.printf("(I2C0 0x2?) VHF Radio Amplifier powering down...\n");
    #endif
    grf5604_powerdown(vhf_grf5604);
    delay(15000); // wait 15 seconds
}