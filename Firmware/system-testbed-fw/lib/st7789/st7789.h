#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>

typedef struct {
    uint8_t width = 170;
    uint8_t height = 320;
    uint8_t spi_speed = 40000000; // 40 MHz
} st7789_config_t;

int display_init(st7789_config_t &display_config);