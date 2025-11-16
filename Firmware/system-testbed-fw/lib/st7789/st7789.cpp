#include <st7789.h>

extern Adafruit_ST7789 tft;
extern st7789_config_t display_config;

#define ST7789_DEBUG Serial

int display_init(st7789_config_t &display_config) {
    if (display_config.initialized) {
      return 0;
    }
    #ifdef ST7789_DEBUG
    ST7789_DEBUG.printf("(VSPI) Beginning use of VSPI... "
        "SS=%d, SCK=%d, MISO =-1, MOSI=%d\n",
        display_config.pin_ss, display_config.pin_sclk, display_config.pin_mosi
    );
    #endif
    #ifdef ST7789_DEBUG
    ST7789_DEBUG.printf("(VSPI) Initializing ST7789... "
        "DC=%d, RESET=%d\n", display_config.pin_dc, display_config.pin_reset
    );
    #endif
    tft.init(display_config.width, display_config.height); // Initialize the ST7789 onboard module
    tft.setSPISpeed(display_config.spi_speed); // HSPI Speed = 40 MHz
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(1);
    #ifdef ST7789_DEBUG
        ST7789_DEBUG.print("(HSPI) Initialized TFT Module and Blanked Screen\n");
    #endif
    display_config.initialized = true;
    return 0;
}