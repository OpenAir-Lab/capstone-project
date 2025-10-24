#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include <cc1200.h>

#include <adafruit_display_demo.h>

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>

// For the breakout board, you can use any 2 or 3 pins.
// These pins will also work for the 1.8" TFT shield.
#define TFT_CS        4
#define TFT_RST       -1 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        26

// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's microSD card.

//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


// OPTION 2 lets you interface the display using ANY TWO or THREE PINS,
// tradeoff being that performance is not as fast as hardware SPI above.
#define TFT_MOSI 13  // Data out
#define TFT_SCLK 14  // Clock out

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
volatile cc1200_config_t cc1200;

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

static const unsigned char PROGMEM image_battery_0_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xff,0xf0,0x80,0x00,0x08,0x82,0x02,0x08,0x81,0x04,0x0e,0x80,0x88,0x01,0x80,0x50,0x01,0x80,0x20,0x01,0x80,0x50,0x01,0x80,0x88,0x01,0x81,0x04,0x0e,0x82,0x02,0x08,0x80,0x00,0x08,0x7f,0xff,0xf0,0x00,0x00,0x00};

static const unsigned char PROGMEM image_battery_100_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xff,0xf0,0x80,0x00,0x08,0xaa,0xaa,0xa8,0xaa,0xaa,0xae,0xaa,0xaa,0xa1,0xaa,0xaa,0xa1,0xaa,0xaa,0xa1,0xaa,0xaa,0xa1,0xaa,0xaa,0xa1,0xaa,0xaa,0xae,0xaa,0xaa,0xa8,0x80,0x00,0x08,0x7f,0xff,0xf0,0x00,0x00,0x00};

static const unsigned char PROGMEM image_guy_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x08,0x40,0x00,0x00,0x00,0x00,0x04,0x40,0x00,0x00,0x00,0x00,0x04,0x40,0x00,0x00,0x00,0x00,0x06,0x40,0x00,0x00,0x00,0x00,0x02,0xc0,0x0f,0xc0,0x00,0x00,0x03,0xc1,0xfc,0x60,0x00,0x00,0x01,0xff,0x00,0xf0,0x00,0x00,0x07,0xc0,0x00,0xf8,0x00,0x00,0x3e,0x01,0xf0,0xfc,0x00,0x00,0xe0,0x2b,0x08,0xfe,0x00,0x01,0x9c,0xa2,0x45,0xff,0x00,0x01,0x62,0x04,0x65,0xff,0x80,0x03,0xc3,0x04,0xf5,0xff,0x00,0x02,0x99,0x05,0xe5,0xff,0x00,0x02,0x9d,0x05,0xeb,0xff,0x00,0x03,0x3d,0x05,0xcb,0xfe,0x00,0x05,0x3a,0x04,0x13,0xfe,0x00,0x05,0x32,0x02,0x23,0xfe,0x00,0x04,0x84,0x3d,0xff,0xfe,0x00,0x04,0xfb,0xd4,0x2f,0xfe,0x00,0x05,0x81,0x5c,0x3f,0xfe,0x00,0x06,0xc3,0xf0,0x17,0xfc,0x00,0x07,0xc0,0x3f,0xff,0xfc,0x00,0x04,0x3f,0xff,0xff,0xfc,0x00,0x07,0xff,0xff,0xff,0xfc,0x00,0x03,0xfe,0x7f,0x9f,0xfc,0x00,0x00,0x0e,0x7f,0x9f,0xfc,0x00,0x00,0x04,0xc1,0x90,0x00,0x00,0x00,0x04,0x80,0x90,0x00,0x00,0x00,0x04,0xc0,0x98,0x00,0x00,0x00,0x02,0x40,0x4c,0x00,0x00,0x00,0x37,0xe0,0x7e,0x00,0x00,0x00,0x7d,0xe0,0xdf,0x00,0x00,0x00,0xdf,0xf3,0xdf,0x00,0x00,0x00,0xff,0xf6,0xff,0x80,0x00,0x00,0xff,0xf7,0xfe,0x80,0x00,0x00,0xff,0xf7,0xf9,0x00,0x00,0x00,0x88,0x17,0xe6,0x00,0x00,0x00,0x38,0x04,0x18,0x00,0x00,0x00,0x00,0x03,0xc0,0x00,0x00};



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

void setup(void) {
    Serial.begin(115200);
    Serial.printf("Hello! ST77xx TFT Test\n");


    cc1200.pin_sck = 4; //Default 5, GPIO5 is a pull-up strapping pin.
    cc1200.spi = new SPIClass(VSPI);

    // // Use this initializer (uncomment) if using a 1.3" or 1.54" 240x240 TFT:
    // //tft.init(240, 240);           // Init ST7789 240x240

    // // OR use this initializer (uncomment) if using a 1.69" 280x240 TFT:
    // //tft.init(240, 280);           // Init ST7789 280x240

    // // OR use this initializer (uncomment) if using a 2.0" 320x240 TFT:
    // //tft.init(240, 320);           // Init ST7789 320x240

    // // OR use this initializer (uncomment) if using a 1.14" 240x135 TFT:
    // //tft.init(135, 240);           // Init ST7789 240x135
    
    // // OR use this initializer (uncomment) if using a 1.47" 172x320 TFT:
    // //tft.init(172, 320);           // Init ST7789 172x320

    // // OR use this initializer (uncomment) if using a 1.9" 170x320 TFT:
    tft.init(170, 320);           // Init ST7789 170x320

    // // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
    // // Note that speed allowable depends on chip and quality of wiring, if you go too fast, you
    // // may end up with a black screen some times, or all the time.
    tft.setSPISpeed(40000000); // HSPI Speed = 40 MHz
    Serial.printf("Initialized");
    uint16_t time = millis();
    tft.fillScreen(ST77XX_BLACK);


    // frequencyScreen();
    testdemoA();
    // demoAdafruitDriver();
    

    time = millis() - time;

    Serial.println(time, DEC);
    delay(500);

    
}

void loop() {
    
}