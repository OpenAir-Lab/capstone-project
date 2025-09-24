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


    cc1200.sck = 4; //Default 5, GPIO5 is a pull-up strapping pin.
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
    demoAdafruitDriver();

    time = millis() - time;

    Serial.println(time, DEC);
    delay(500);

    
}

void loop() {
    
}