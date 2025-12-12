#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Fonts/TomThumb.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>

#define TFT_SS   15 // 15 -> 4
#define TFT_DC   4 // Display Command pin
#define TFT_MOSI 13 // ESP32 IOMUX Default
#define TFT_SCLK 14 // ESP32 IOMUX Default
#define TFT_RST  -1  // Not connected
typedef struct {
    bool initialized = false;
    // HSPI 
    int8_t pin_ss = TFT_SS; 
    int8_t pin_mosi = TFT_MOSI;
    int8_t pin_miso = -1;
    int8_t pin_sclk = TFT_SCLK;
    int8_t pin_dc = TFT_DC;  // must be set
    int8_t pin_reset = TFT_RST; // can be floating
    uint16_t width = 170;
    uint16_t height = 320;
    int8_t rotation = 3;
    uint32_t spi_speed = 40000000; // 40 MHz
} st7789_config_t;


int display_init(st7789_config_t &display_config);