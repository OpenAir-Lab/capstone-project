#include <st7789.h>

extern Adafruit_ST7789 tft;
extern st7789_config_t display_config;

#define ST7789_DEBUG Serial

int display_init(st7789_config_t &display_config) {
    tft.init(display_config.width, display_config.height); // Initialize the ST7789 onboard module
    tft.setSPISpeed(display_config.spi_speed); // HSPI Speed = 40 MHz
    tft.fillScreen(ST77XX_BLACK);
    #ifdef ST7789_DEBUG
    ST7789_DEBUG.print("(HSPI) Initialized TFT Module and Blanked Screen\n");
    #endif
}