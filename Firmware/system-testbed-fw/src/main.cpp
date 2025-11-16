#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include <cc1200.h>   // used in Transceiver Modulino 
#include <sky13330.h> // used in RF Switch Modulino
#include <grf5604.h>  // used in Amplifier Modulino

#include <pcf8575.h>  // used in MCU, HMI, and RF Modulinos

#include <st7789.h> // used in HMI Modulino

// use doxygen formatting in block comments
// for auto-generated firmware documentation!

// NOTE: UART bridge uses UART0 over RX and TX pins.
// This is only enabled when UART0 is not reassigned! 
#define DEBUG Serial // uncomment to enable print debugging.
typedef enum {
    MCU_ECHO_TEST,           // Verifies ESP32 communication interfaces  
    USB_POWER_DELIVERY,      // Advertise modern power delivery profiles 
    BATTERY_POWER_SUPPLY,    // Charge and customize battery power
    HUMAN_MACHINE_INTERFACE, // Navigate menus and accept user input
    DIGITAL_AUDIO_INTERFACE, // Source and sink digital audio
    RADIO_TRANSCEIVER,       // Traverse radio control states 
    RADIO_AMPLIFIER,         // Amplify UHF or VHF radio signals 
    RADIO_SWITCH,            // Scatter parameterize multiport Switch 
    EXPO_DEMO                // Comprehensive system integration tests
} demonstration_t;

demonstration_t demo = RADIO_AMPLIFIER;

#define PIN_SDA 21
#define PIN_SCL 22
// MCU Port Expander
Adafruit_PCF8575 pcf_mcu;
pcf8575_config_t pcf_mcu_config; // on 0x20+0
// HMI Port Expander
Adafruit_PCF8575 pcf_hmi;
pcf8575_config_t pcf_hmi_config; // on 0x20+1
// Radio Port Expander
Adafruit_PCF8575 pcf_radio;
pcf8575_config_t pcf_radio_config; // on 0x20+2

// Texas Instruments CC1200 Configuration
// VSPI normally attached to pins 5, 18, 19, and 23,
// but can be matrixed to any pins as shown below.
#define CC1200_NRST  3    // Reset is not correct pin yet. 
#define CC1200_SCLK  18    //  SCK=05 -> 18
#define CC1200_MISO  19    // MISO=18 -> 19
#define CC1200_MOSI  23    // MOSI=19 -> 23
#define CC1200_SS    5     //   SS=23 -> 5
#define CC1200_GDIO0 32
#define CC1200_GDIO2 33
// cc1200.pin_sck, cc1200.pin_miso, cc1200.pin_mosi, cc1200.pin_ss
cc1200_config_t cc1200;
#define RFSW_ENABLE  8
#define RFSW_BAND    9
#define RFSW_TRX     10
sky13330_config_t sky13330;
#define UHF_SHUTDOWN 11
#define UHF_ENABLE1  13
#define UHF_ENABLE2  UHF_ENABLE1 // stages share enables
grf5604_config_t uhf_grf5604;
#define VHF_SHUTDOWN 12
#define VHF_ENABLE1  14
#define VHF_ENABLE2  VHF_ENABLE1 // stages share enables
grf5604_config_t vhf_grf5604;

// Human-Machine Interface Configuration

/*! \def TFT Module Pin Matrix
    \brief used with an instance of Adafruit_ST7789 class.
*/
// For the breakout board, matrix to any 2 or 3 pins.
// HSPI normally attached to pins 12, 13, 14, and 15, 
// but can be matrixed to any pins as shown below.
// HSPI MISO not utilized, default 12
#define TFT_SS    4 // 15 -> 4
#define TFT_DC   26 // Display Command pin
#define TFT_MOSI 13 // ESP32 IOMUX Default
#define TFT_SCLK 14 // ESP32 IOMUX Default
#define TFT_RST  -1 // Not connected
st7789_config_t display_config;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_SS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

/*! ICONAGRAPHY BITMAPS 
    \brief is loaded into PROGMEM used by TFT display buffers.
    
    Recommended: use https://lopaka.app/sandbox to import bitmaps.
*/ 
// Gary
static const unsigned char PROGMEM image_guy_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x08,0x40,0x00,0x00,0x00,0x00,0x04,0x40,0x00,0x00,0x00,0x00,0x04,0x40,0x00,0x00,0x00,0x00,0x06,0x40,0x00,0x00,0x00,0x00,0x02,0xc0,0x0f,0xc0,0x00,0x00,0x03,0xc1,0xfc,0x60,0x00,0x00,0x01,0xff,0x00,0xf0,0x00,0x00,0x07,0xc0,0x00,0xf8,0x00,0x00,0x3e,0x01,0xf0,0xfc,0x00,0x00,0xe0,0x2b,0x08,0xfe,0x00,0x01,0x9c,0xa2,0x45,0xff,0x00,0x01,0x62,0x04,0x65,0xff,0x80,0x03,0xc3,0x04,0xf5,0xff,0x00,0x02,0x99,0x05,0xe5,0xff,0x00,0x02,0x9d,0x05,0xeb,0xff,0x00,0x03,0x3d,0x05,0xcb,0xfe,0x00,0x05,0x3a,0x04,0x13,0xfe,0x00,0x05,0x32,0x02,0x23,0xfe,0x00,0x04,0x84,0x3d,0xff,0xfe,0x00,0x04,0xfb,0xd4,0x2f,0xfe,0x00,0x05,0x81,0x5c,0x3f,0xfe,0x00,0x06,0xc3,0xf0,0x17,0xfc,0x00,0x07,0xc0,0x3f,0xff,0xfc,0x00,0x04,0x3f,0xff,0xff,0xfc,0x00,0x07,0xff,0xff,0xff,0xfc,0x00,0x03,0xfe,0x7f,0x9f,0xfc,0x00,0x00,0x0e,0x7f,0x9f,0xfc,0x00,0x00,0x04,0xc1,0x90,0x00,0x00,0x00,0x04,0x80,0x90,0x00,0x00,0x00,0x04,0xc0,0x98,0x00,0x00,0x00,0x02,0x40,0x4c,0x00,0x00,0x00,0x37,0xe0,0x7e,0x00,0x00,0x00,0x7d,0xe0,0xdf,0x00,0x00,0x00,0xdf,0xf3,0xdf,0x00,0x00,0x00,0xff,0xf6,0xff,0x80,0x00,0x00,0xff,0xf7,0xfe,0x80,0x00,0x00,0xff,0xf7,0xf9,0x00,0x00,0x00,0x88,0x17,0xe6,0x00,0x00,0x00,0x38,0x04,0x18,0x00,0x00,0x00,0x00,0x03,0xc0,0x00,0x00};
// Battery Indicators
static const unsigned char PROGMEM image_battery_0_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xff,0xf0,0x80,0x00,0x08,0x82,0x02,0x08,0x81,0x04,0x0e,0x80,0x88,0x01,0x80,0x50,0x01,0x80,0x20,0x01,0x80,0x50,0x01,0x80,0x88,0x01,0x81,0x04,0x0e,0x82,0x02,0x08,0x80,0x00,0x08,0x7f,0xff,0xf0,0x00,0x00,0x00};
static const unsigned char PROGMEM image_battery_100_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xff,0xf0,0x80,0x00,0x08,0xaa,0xaa,0xa8,0xaa,0xaa,0xae,0xaa,0xaa,0xa1,0xaa,0xaa,0xa1,0xaa,0xaa,0xa1,0xaa,0xaa,0xa1,0xaa,0xaa,0xa1,0xaa,0xaa,0xae,0xaa,0xaa,0xa8,0x80,0x00,0x08,0x7f,0xff,0xf0,0x00,0x00,0x00};

/*! \def Keypad Pin Matrix
    \brief used with an instance of Adafruit_PCF8575 class.

    Keypad is on the first port of the port expander on HMI.
    Scanning is acheived using domino logic.
*/

bool initialized;
int hmi_init() {
    // Adafruit 1.9" 170x320 TFT Module
    do {
        display_init(display_config);
        if (!display_config.initialized) {
            #ifdef DEBUG
            DEBUG.printf("(HSPI) Still init HMI Display...\n");
            #endif
            delay(500);
        }
    } while (!display_config.initialized);
    #ifdef DEBUG
    DEBUG.printf("(HSPI) Initialized HMI Display\n");
    #endif
    // TODO: Initialize HMI Port Expander.
    do {
        pcf8575_init(pcf_hmi, pcf_hmi_config);
        if (!pcf_hmi_config.initialized) {
            #ifdef DEBUG
            DEBUG.printf("(I2C) No HMI Port Expander Found...\n");
            #endif
            delay(500);
        }
    } while (!pcf_hmi_config.initialized);
    #ifdef DEBUG
    DEBUG.printf("(I2C) Initialized HMI Port Expander\n");
    #endif
    return 0;
}

int radio_init() {
    do {
        initialized = (pcf8575_init(pcf_radio, pcf_radio_config) == 0);
    } while (!pcf_radio_config.initialized);
    #ifdef DEBUG
    DEBUG.printf("(I2C) Initialized Radio Port Expander\n");
    #endif
    do {
        cc1200_init(cc1200);
    } while (!cc1200.initialized);
    #ifdef DEBUG
    DEBUG.printf("(VSPI) Initialized Radio Transceiver\n");
    #endif
    do {
        sky13330_init(sky13330);
    } while (!sky13330.initialized);
    #ifdef DEBUG
    DEBUG.printf("(I2C) Initialized RF Switch\n");
    #endif
    return 0;   
}

void frequencyScreen() {

    tft.fillScreen(0x0);
    tft.setRotation(1);

    tft.setTextColor(0xC0E5);
    tft.setTextWrap(false);
    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(104, 60);
    tft.print("145.430 MHz");

    tft.setCursor(104, 75);
    tft.print("-0.6 MHz");

    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(11, 75);
    tft.print("Offset");

    tft.setCursor(10, 59);
    tft.print("Receive");

    tft.setCursor(105, 109);
    tft.print("FM Simplex");

    tft.setCursor(105, 39);
    tft.print("FM Repeater");

    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(105, 129);
    tft.print("145.430 MHz");

    tft.setCursor(105, 144);
    tft.print("144.830 MHz");

    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(11, 143);
    tft.print("Transmit");

    tft.setCursor(11, 128);
    tft.print("Receive");

    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(104, 90);
    tft.print("110.0 Hz");

    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(11, 90);
    tft.print("PL Tone");

    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(105, 159);
    tft.print("110.0 Hz");

    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(12, 159);
    tft.print("PL Tone");


}

void testdemoA() {
    tft.fillScreen(0x0);
    tft.setRotation(1);

    tft.setTextColor(0xC0E5);
    tft.setTextWrap(false);
    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(104, 60);
    tft.print("145.430 MHz");

    tft.setCursor(104, 75);
    tft.print("-0.6 MHz");

    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(11, 75);
    tft.print("Offset");

    tft.setCursor(10, 59);
    tft.print("Receive");

    tft.setFont(&FreeSerifBoldItalic12pt7b);
    tft.setCursor(105, 39);
    tft.print("FM Repeater");

    tft.setTextColor(0x8E09);
    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(105, 109);
    tft.print("FM Half Duplex");

    tft.setTextColor(0xC0E5);
    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(105, 129);
    tft.print("145.430 MHz");

    tft.setCursor(105, 144);
    tft.print("144.830 MHz");

    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(11, 143);
    tft.print("Transmit");

    tft.setCursor(11, 128);
    tft.print("Receive");

    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(104, 90);
    tft.print("110.0 Hz");

    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(11, 90);
    tft.print("PL Tone");

    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(105, 159);
    tft.print("110.0 Hz");

    tft.setFont(&FreeMonoBold9pt7b);
    tft.setCursor(12, 159);
    tft.print("PL Tone");

    tft.setTextColor(0xFFFF);
    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(44, 19);
    tft.print("0%");

    tft.drawBitmap(47, 24, image_battery_0_bits, 24, 16, 0xFFFF);

    tft.drawBitmap(11, 6, image_battery_100_bits, 24, 16, 0xFFFF);

    tft.drawBitmap(278, -2, image_guy_bits, 41, 43, 0xFFFF);
}

void demonstrate_mcu() {
    do {
        initialized = (pcf8575_init(pcf_mcu, pcf_mcu_config) == 0);
    } while (!pcf_mcu_config.initialized);
    #ifdef DEBUG
    DEBUG.printf("(I2C) Initialized MCU Port Expander\n");
    #endif
}

void demonstrate_usb_power_delivery() {

}

void demonstrate_battery_power_supply() {

}

void demonstrate_human_machine_interface() {
    // Initialize HMI
    do {
        initialized = (hmi_init() == 0);
    } while (!initialized);
}

void demonstrate_digital_audio_interface() {

}

void demonstrate_expo() {
    demonstrate_usb_power_delivery();
    demonstrate_battery_power_supply();
    demonstrate_human_machine_interface();
    demonstrate_digital_audio_interface();
    demonstrate_radio_transceiver();
    demonstrate_radio_amplifier();
    demonstrate_radio_switch();
}

void setup(void) {
    // ESP32 Serial Monitor over USB->UART
    #ifdef DEBUG
    while(!DEBUG) {
        DEBUG.begin(115200); // Monitor has 115200 Baud rate.
    }
    #endif
    // ESP32 I2C0 Interface
    Wire.setPins(PIN_SDA, PIN_SCL);
    Wire.begin();
    // ST7789 TFT Display Driver onboard Adafruit 1.9" TFT Module
    display_config.pin_ss = TFT_SS;
    display_config.pin_dc = TFT_DC;
    display_config.pin_mosi = TFT_MOSI;
    display_config.pin_sclk = TFT_SCLK;
    display_config.pin_reset = TFT_RST;
    // Texas Instruments PCF8575 16-bit Port Expander
    pcf_mcu_config.i2c = &Wire;   // I2C0
    pcf_hmi_config.i2c = &Wire;   // I2C0
    pcf_radio_config.i2c = &Wire; // I2C0
    pcf_mcu_config.sensor_address = PCF8575_I2CADDR_DEFAULT+0;
    pcf_hmi_config.sensor_address = PCF8575_I2CADDR_DEFAULT+1;
    // TODO: Use proper addressing for radio expander
    // pcf_radio_config.sensor_address = PCF8575_I2CADDR_DEFAULT+2;
    pcf_hmi_config.sensor_address = pcf_radio_config.sensor_address;
    pcf_mcu_config.pin_interrupt = 12;
    pcf_hmi_config.pin_interrupt = 35;
    pcf_radio_config.pin_interrupt = 39;
    // attachInterrupt(digitalPinToInterrupt(pcf_mcu_config.pin_interrupt), inputISR, CHANGE);
    // Skyworks SKY13330-397LF SPDT RF Switch
    sky13330.pin_enable = RFSW_ENABLE;
    sky13330.pin_band = RFSW_BAND;
    sky13330.pin_trx = RFSW_TRX;
    // GuerrillaRF GRF5604 RF Amplifier
    uhf_grf5604.band = UHF;
    vhf_grf5604.band = VHF;
    // Texas Instruments CC1200 Sub 1-GHz Radio Transceiver
    cc1200.pin_ss    = CC1200_SS;    // Default
    cc1200.pin_sck   = CC1200_SCLK;  // Default
    cc1200.pin_miso  = CC1200_MISO;  // Default
    cc1200.pin_mosi  = CC1200_MOSI;  // Default
    cc1200.pin_nrst  = CC1200_NRST;  //
    cc1200.pin_gdio0 = CC1200_GDIO0; //
    cc1200.pin_gdio2 = CC1200_GDIO2; //
    cc1200.spi = new SPIClass(VSPI);
    cc1200.spi_frequency = 100000;
    // measure time to demo completion
    uint16_t time_to_completion = millis();
    switch(demo) {
        case (USB_POWER_DELIVERY):      // Advertise modern power delivery profiles
            demonstrate_usb_power_delivery();
            break;
        case (BATTERY_POWER_SUPPLY):    // Charge and customize battery power
            demonstrate_battery_power_supply();
            break;
        case (HUMAN_MACHINE_INTERFACE): // Navigate menus and accept user input
            demonstrate_human_machine_interface();
            break;
        case (DIGITAL_AUDIO_INTERFACE): // Source and sink digital audio
            demonstrate_digital_audio_interface();
            break;
        case (RADIO_TRANSCEIVER):       // Traverse radio control states
            // Initialize HMI Devices
            do {
                initialized = (hmi_init() == 0);
            } while (!initialized);
            // Initialize Radio Devices
            do {
                initialized = (radio_init() == 0);
            } while (!initialized);
            demonstrate_radio_transceiver();
            break;
        case (RADIO_AMPLIFIER):         // Amplify UHF or VHF radio signals
            // Initialize HMI Devices
            do {
                initialized = (hmi_init() == 0);
            } while (!initialized);
            // Initialize Radio Devices
            do {
                initialized = (radio_init() == 0);
            } while (!initialized);
            // Initialize Amplifiers in powered down state
            do {
                initialized = (grf5604_init(uhf_grf5604) == 0);
            } while (!uhf_grf5604.initialized);
            do {
                initialized = (grf5604_init(vhf_grf5604) == 0);
            } while (!vhf_grf5604.initialized);
            demonstrate_radio_amplifier();
            break;
        case (RADIO_SWITCH):            // Scatter parameterize multiport Switch
            // Initialize HMI Devices
            do {
                initialized = (hmi_init() == 0);
            } while (!initialized);
            // Initialize Radio Devices
            do {
                initialized = (pcf8575_init(pcf_radio, pcf_radio_config) == 0);
            } while (!pcf_radio_config.initialized);
            #ifdef DEBUG
            DEBUG.printf("(I2C) Initialized Radio Port Expander\n");
            #endif
            do {
                sky13330_init(sky13330);
            } while (!sky13330.initialized);
            #ifdef DEBUG
            DEBUG.printf("(I2C) Initialized RF Switch\n");
            #endif
            demonstrate_radio_switch();
            break;
        case (EXPO_DEMO):
            demonstrate_expo();
            break;
    }
    time_to_completion = millis() - time_to_completion;
    #ifdef DEBUG
    DEBUG.printf("Time to demonstration completion: %d ms\n", time_to_completion, DEC);
    #endif
}

void loop() {
    
}