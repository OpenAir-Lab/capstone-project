#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include <cc1200.h>
#include <pcf8575.h>

#include <adafruit_display_demo.h>

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>

// NOTE: UART bridge uses UART0 over RX and TX pins.
// This is only enabled when UART0 is not reassigned! 
#define DEBUG Serial // uncomment to enable print debugging.

// Human-Machine Interface Configuration

/*! \def TFT Module Pin Matrix
    \brief used with an instance of Adafruit_ST7789 class.
*/
// For the breakout board, matrix to any 2 or 3 pins.
#define TFT_RST -1 // ESP32 Reset pin is -1
#define TFT_DC  26 // Display Command pin
// HSPI normally attached to pins 12, 13, 14, and 15, 
// but can be matrixed to any pins as shown below.
#define TFT_SCLK 14  // 12 -> 14
// HSPI MISO not utilized, default 13
#define TFT_MOSI 13  // 14 -> 13
#define TFT_SS   4   // 15 -> 4
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

// Radio Configuration

// Texas Instruments CC1200 Configuration 
// VSPI normally attached to pins 5, 18, 19, and 23,
// but can be matrixed to any pins as shown below.
// #define CC1200_NRST PORT03 // Reset -> Port 3 of MCU Port Expander
#define CC1200_NRST 33    // Reset -> 33
#define CC1200_SCLK  4    // SCK 5 -> 4
#define CC1200_MISO 18    // Default
#define CC1200_MOSI 19    // Default
#define CC1200_SS   23    // Default
cc1200_config_t cc1200;

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

void demoAdafruitDriver() {
    // large block of text
    tft.fillScreen(ST77XX_BLACK);
    char* sample_text = (char*)"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ";
    testdrawtext(sample_text, ST77XX_WHITE);
    delay(1000);

    // tft print function!
    tftPrintTest();
    delay(4000);

    // a single pixel
    tft.drawPixel(tft.width()/2, tft.height()/2, ST77XX_GREEN);
    delay(500);

    // line draw test
    testlines(ST77XX_YELLOW);
    delay(500);

    // optimized lines
    testfastlines(ST77XX_RED, ST77XX_BLUE);
    delay(500);

    testdrawrects(ST77XX_GREEN);
    delay(500);

    testfillrects(ST77XX_YELLOW, ST77XX_MAGENTA);
    delay(500);

    tft.fillScreen(ST77XX_BLACK);
    testfillcircles(10, ST77XX_BLUE);
    testdrawcircles(10, ST77XX_WHITE);
    delay(500);

    testroundrects();
    delay(500);

    testtriangles();
    delay(500);

    // mediabuttons();
    // delay(500);

    Serial.println("done");
    delay(1000);
}
typedef enum {
    USB_POWER_DELIVERY,      // Advertise modern power delivery profiles 
    BATTERY_POWER_SUPPLY,    // Charge and customize battery power
    HUMAN_MACHINE_INTERFACE, // Navigate menus and accept user input  
    DIGITAL_AUDIO_INTERFACE, // Source and sink digital audio
    RADIO_TRANSCEIVER,       // Traverse radio control states 
    RADIO_AMPLIFIER,         // Amplify UHF or VHF radio signals 
    RADIO_SWITCH,            // Scatter parameterize multiport Switch 
    EXPO_DEMO                // Comprehensive system integration tests
} demonstration_t;

demonstration_t demo = RADIO_TRANSCEIVER;

void demonstrate_usb_power_delivery() {

}
void demonstrate_battery_power_supply() {

}
void demonstrate_human_machine_interface() {
    tft.fillScreen(ST77XX_BLACK);
    // frequencyScreen();
    testdemoA();
    // demoAdafruitDriver();
}
void demonstrate_digital_audio_interface() {

}
void demonstrate_radio_transceiver() {

}
void demonstrate_radio_amplifier() {

}
void demonstrate_radio_switch() {

}

void setup(void) {
    // ESP32 Serial Monitor
    #ifdef DEBUG
    DEBUG.begin(115200); // Monitor has 115200 Baud rate.
    #endif

    // Texas Instruments CC1200 Sub 1-GHz Radio Transceiver
    cc1200.pin_nrst = CC1200_NRST; 
    cc1200.pin_sck = CC1200_SCLK; //Default 5, GPIO5 is a pull-up strapping pin.
    cc1200.pin_miso = CC1200_MISO;
    cc1200.pin_mosi = CC1200_MOSI;
    cc1200.spi = new SPIClass(VSPI);
    cc1200_init(cc1200);
    delay(500);
    #ifdef DEBUG
    DEBUG.printf("Initialized Radio Transceiver over VSPI\n");
    #endif
    // Adafruit 1.9" 170x320 TFT Module
    tft.init(170, 320); // Initialize the ST7789 onboard module
    tft.setSPISpeed(40000000); // HSPI Speed = 40 MHz
    tft.fillScreen(ST77XX_BLACK);
    #ifdef DEBUG
    DEBUG.printf("Initialized TFT Module over HSPI\n");
    #endif
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
            demonstrate_radio_transceiver();
            break;
        case (RADIO_AMPLIFIER):         // Amplify UHF or VHF radio signals
            demonstrate_radio_amplifier();
            break;
        case (RADIO_SWITCH):            // Scatter parameterize multiport Switch
            demonstrate_radio_switch();
            break;
    }
    time_to_completion = millis() - time_to_completion;
    #ifdef DEBUG
    DEBUG.printf("Time to demonstration completion: %d ms\n", time_to_completion, DEC);
    #endif
    delay(500);
}

void loop() {
    
}