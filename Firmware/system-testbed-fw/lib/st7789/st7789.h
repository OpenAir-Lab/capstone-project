#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>

typedef struct {
    bool initialized = false;
    // HSPI 
    int8_t pin_ss = 15; 
    int8_t pin_mosi = 26;
    int8_t pin_miso = 13;
    int8_t pin_sclk = 14;
    int8_t pin_dc = -1;  // must be set
    int8_t pin_reset = -1; // can be floating
    uint16_t width = 170;
    uint16_t height = 320;
    uint32_t spi_speed = 40000000; // 40 MHz
} st7789_config_t;

int display_init(st7789_config_t &display_config);