#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include <cc1200.h>          // used in Transceiver Modulino 
#include <sky13330.h>        // used in RF Switch Modulino
#include <grf5604.h>         // used in Amplifier Modulino

#include <pcf8575.h>         // used in MCU, HMI, and RF Modulinos

#include <st7789.h>          // used in HMI Modulino
#include <max98357a.h>
#include <ics43434.h>

#define STACK_SIZE 2048
// use doxygen formatting in block comments
// for auto-generated firmware documentation!

// NOTE: UART bridge uses UART0 over RX and TX pins.
// This is only enabled when UART0 is not reassigned! 
#define DEBUG Serial         // uncomment to enable print debugging.
typedef enum {
    MCU_ECHO_TEST,           // Verify ESP32 communication interfaces  
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
#define UHF_SHUTDOWN 13
#define UHF_ENABLE1  15
#define UHF_ENABLE2  14 // bands share enables
grf5604_config_t uhf_grf5604;
#define VHF_SHUTDOWN 12
#define VHF_ENABLE1  15
#define VHF_ENABLE2  14 // bands share enables
grf5604_config_t vhf_grf5604;

// Human-Machine Interface Configuration

/*! \def I2S0 Controller Pin Matrix
    \brief used with an instance of I2CClass class.
    I2S0 utilized by MAX98357A Audio Amplifier and ICS-43434 Microphone.
    Controller is put into full duplex mode, common serial data not used.
*/
#define I2S0_PIN_BCLK  27 // shared bit clock
#define I2S0_PIN_LRCLK 25 // shared left-right clock
#define I2S0_PIN_DOUT  33 // data output to amplifier 
#define I2S0_PIN_DIN   36 // data input from microphone

#define I2S0_SAMPLE_RATE    40000 // 40 kSps (40 kHz audio)
#define I2S0_WORD_SIZE      32 // 32-bit data words
max98357a_config_t max98357a;
ics43434_config_t ics43434;


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

TaskHandle_t vDisplayRSSITaskHandle = NULL;
void vDisplayRSSITask(void *pv) {
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // wait for timer
        // blocking calls here allowed
        bool high_resolution = false;
        cc1200_calculate_rssi(high_resolution);
        bool valid_RSSI = (cc1200.registers.RSSI0 & RSSI_VALID);
        if (valid_RSSI) {
            tft.fillRect(71, 45, 89, 16, ST77XX_BLACK);
            tft.fillRect(147, 30, 89, 16, ST77XX_BLACK);
            tft.setFont(&FreeMono9pt7b);
            tft.setCursor(148, 42); tft.printf("%d dBm", cc1200.rssi_offset);
            tft.setCursor(76, 58); tft.printf(high_resolution ? "%3.4f dBm" : "%3.0f dBm", cc1200.rssi);
        }
    }
}

int hmi_init() {
    // PCF8575 HMI Port Expander
    pcf_hmi_config.i2c = &Wire;   // I2C0
    pcf_hmi_config.pin_interrupt = 35;
    pcf_hmi_config.subsystem_name = "HMI";
    pcf_hmi_config.sensor_address = PCF8575_I2CADDR_DEFAULT+2;
    // ST7789 TFT Display Driver onboard Adafruit 1.9" 170x320 TFT Module
    display_config.pin_ss = TFT_SS;
    display_config.pin_dc = TFT_DC;
    display_config.pin_mosi = TFT_MOSI;
    display_config.pin_sclk = TFT_SCLK;
    display_config.pin_reset = TFT_RST;
    display_config.rotation = 3;

    while (!(pcf8575_init(pcf_hmi, pcf_hmi_config) == 0));
    while (!(display_init(display_config) == 0));
    return 0;
}

int radio_init() {
    pcf_radio_config.i2c = &Wire; // I2C0
    pcf_radio_config.pin_interrupt = 39;
    pcf_radio_config.subsystem_name = "Radio";
    pcf_radio_config.sensor_address = PCF8575_I2CADDR_DEFAULT+1;
    // attachInterrupt(digitalPinToInterrupt(pcf_mcu_config.pin_interrupt), inputISR, CHANGE);
    // Skyworks SKY13330-397LF SPDT RF Switch
    sky13330.pin_enable = RFSW_ENABLE;
    sky13330.pin_band = RFSW_BAND;
    sky13330.pin_trx = RFSW_TRX;
    // GuerrillaRF GRF5604 RF Amplifier
    uhf_grf5604.band = UHF;
    uhf_grf5604.pin_shutdown = UHF_SHUTDOWN;
    uhf_grf5604.pin_enable1 = UHF_ENABLE1;
    uhf_grf5604.pin_enable2 = UHF_ENABLE2;
    vhf_grf5604.band = VHF;
    vhf_grf5604.pin_shutdown = VHF_SHUTDOWN;
    vhf_grf5604.pin_enable1 = VHF_ENABLE1;
    vhf_grf5604.pin_enable2 = VHF_ENABLE2;
    // Texas Instruments CC1200 Sub 1-GHz Radio Transceiver
    cc1200.pin_ss    = CC1200_SS;    // Default
    cc1200.pin_sck   = CC1200_SCLK;  // Default
    cc1200.pin_miso  = CC1200_MISO;  // Default
    cc1200.pin_mosi  = CC1200_MOSI;  // Default
    cc1200.pin_nrst  = CC1200_NRST;  //
    cc1200.pin_gdio0 = CC1200_GDIO0; //
    cc1200.pin_gdio2 = CC1200_GDIO2; //
    cc1200.spi = new SPIClass(VSPI);
    // cc1200.spi_frequency = 100000;

    while (!(pcf8575_init(pcf_radio, pcf_radio_config) == 0));
    // Initialize RF Amplifiers in powered down state
    while (!(grf5604_init(uhf_grf5604) == 0));
    while (!(grf5604_init(vhf_grf5604) == 0));
    // Initialize SP4T Switch for UHF receive on Port 2
    while (!(sky13330_init(sky13330) == 0));
    // Initialize RF Transceiver in UHF receive Analog FM mode
    while (!(cc1200_init(cc1200) == 0));
    xTaskCreate(vDisplayRSSITask, "displayRSSI", STACK_SIZE, NULL, 2, &vDisplayRSSITaskHandle);
    return 0;   
}

void demonstrate_mcu(void *parameter) {
    // Texas Instruments PCF8575 16-bit Port Expander
    pcf_mcu_config.i2c = &Wire;   // I2C0
    pcf_mcu_config.pin_interrupt = 12;
    pcf_mcu_config.subsystem_name = "MCU";
    pcf_mcu_config.sensor_address = PCF8575_I2CADDR_DEFAULT+0;
    while (!(pcf8575_init(pcf_mcu, pcf_mcu_config) == 0));
}

void demonstrate_usb_power_delivery(void *parameter) {
    // Requires having run the one time advertisement programming routine.
}

void demonstrate_battery_power_supply(void *parameter) {

}

void demonstrate_human_machine_interface(void *parameter) {

}

void demonstrate_digital_audio_interface(void *parameter) {

}


void setup(void) {
    // ESP32 Serial Monitor over USB->UART
    #ifdef DEBUG
    while(!DEBUG) {
        DEBUG.begin(115200); // Monitor has 115200 Baud rate.
    }
    #endif
    // ESP32 I2C0 Interface
    #ifdef DEBUG
    DEBUG.printf("(I2C0 -----) Beginning use of I2C0 Interface... "
        "[SDA=%d, SCL=%d]\n", PIN_SDA, PIN_SCL
    );
    #endif
    Wire.setPins(PIN_SDA, PIN_SCL);
    while(!Wire.begin());

    // ESP32 I2S0 Interface
    max98357a.pin_bclk = I2S0_PIN_BCLK;
    max98357a.pin_lrclk = I2S0_PIN_LRCLK;
    max98357a.pin_data_in = I2S0_PIN_DOUT;
    ics43434.pin_bclk = I2S0_PIN_BCLK;
    ics43434.pin_lrclk = I2S0_PIN_LRCLK;
    ics43434.pin_data_out = I2S0_PIN_DIN;
    ics43434.sample_rate = I2S0_SAMPLE_RATE;
    ics43434.bits_per_sample = I2S0_WORD_SIZE;
    #ifdef DEBUG
    DEBUG.printf("(I2S0 -----) Beginning use of I2S0 Interface... "
        "[BCLK=%d, LRCLK=%d, DOUT=%d, DIN=%d]\n", I2S0_PIN_BCLK, I2S0_PIN_LRCLK, max98357a.pin_data_in, ics43434.pin_data_out 
    );
    #endif
    I2S.begin(I2S_PHILIPS_MODE, I2S0_SAMPLE_RATE, I2S0_WORD_SIZE);
    I2S.setAllPins(I2S0_PIN_BCLK, I2S0_PIN_LRCLK, I2S_PIN_NO_CHANGE, max98357a.pin_data_in, ics43434.pin_data_out);
    
    switch(demo) {
        case (USB_POWER_DELIVERY):      // Advertise modern power delivery profiles
            #ifdef DEBUG
            DEBUG.printf("Selected Demo is of the USB Power Delivery Modulino!\n"
                "[X] USB PD Modulino requires Battery Modulino to be plugged in.\n"
                "[X] USB PD Modulino must first have PD profiles loaded onto STUSB4500.\n"
            );
            #endif
            xTaskCreate(demonstrate_usb_power_delivery,"MainTask",STACK_SIZE,NULL,1,NULL);
            break;
        case (BATTERY_POWER_SUPPLY):    // Charge and customize battery power
            #ifdef DEBUG
            DEBUG.printf("Selected Demo is of the Battery Modulino!\n"
                "[X] Battery Modulino requires HMI Modulino to be plugged in.\n"
                "[X] Battery Modulino requires Li-Ion Battery Pack to be plugged in.\n"
            );
            #endif
            xTaskCreate(demonstrate_battery_power_supply,"MainTask",STACK_SIZE,NULL,1,NULL);
            break;
        case (HUMAN_MACHINE_INTERFACE): // Navigate menus and accept user input
            #ifdef DEBUG
            DEBUG.printf("Selected Demo is of the Human-Machine Interface Modulino!\n"
                "[X] HMI Modulino requires Adafruit 1.9\" 170x320 TFT Module to be plugged in.\n"
            );
            #endif
            // Initialize HMI
            while (!(hmi_init() == 0));
            xTaskCreate(demonstrate_human_machine_interface,"MainTask",STACK_SIZE,NULL,1,NULL);
            break;
        case (DIGITAL_AUDIO_INTERFACE): // Source and sink digital audio
            #ifdef DEBUG
            DEBUG.printf("Selected Demo is of the Digital Audio Modulino!\n"
                "[X] Audio Modulino requires HMI Modulino to be plugged in.\n"
                "[X] Audio Modulino requires Mono Speaker to be plugged in.\n"
            );
            #endif
            xTaskCreate(demonstrate_digital_audio_interface,"MainTask",STACK_SIZE,NULL,1,NULL);
            break;
        case (RADIO_TRANSCEIVER):       // Traverse radio control states
            #ifdef DEBUG
            DEBUG.printf("Selected Demo is of the Dual-Band Transceiver Modulino!\n"
                "[X] Transceiver Modulino expects Amplifier Modulino to be plugged in.\n"
                "[X] Transceiver Modulino expects Switch Modulino to be plugged in.\n"
            );
            #endif
            // Initialize Radio Devices
            while (!(radio_init() == 0));
            // Initialize HMI Devices
            while (!(hmi_init() == 0));
            xTaskCreate(demonstrate_radio_transceiver,"MainTask",STACK_SIZE,NULL,1,NULL);
            break;
        case (RADIO_AMPLIFIER):         // Amplify UHF or VHF radio signals
            #ifdef DEBUG
            DEBUG.printf("Selected Demo is of the Dual-Band RF Amplifier Modulino!\n"
                "[X] Amplifier Modulino expects Transceiver Modulino to be plugged in.\n"
                "[X] Amplifier Modulino expects Switch Modulino to be plugged in.\n"
                "[X] Amplifier Modulino requires Dual-Band Antenna to be plugged in.\n"
            );
            #endif
            // Initialize Radio Devices
            while (!(radio_init() == 0));
            // Initialize HMI Devices
            while (!(hmi_init() == 0));
            xTaskCreate(demonstrate_radio_amplifier,"MainTask",STACK_SIZE,NULL,1,NULL);
            break;
        case (RADIO_SWITCH):            // Scatter parameterize multiport Switch
            #ifdef DEBUG
            DEBUG.printf("Selected Demo is of the SP4T RF Switch Modulino!\n"
                "[X] Switch Modulino requires HMI Modulino to be plugged in.\n"
                "[X] Switch Modulino requires Dual-Band Antenna to be plugged in.\n"
            );
            #endif
            // Initialize Radio Devices
            while (!(radio_init() == 0));
            // Initialize HMI Devices
            while (!(hmi_init() == 0));
            xTaskCreate(demonstrate_radio_switch,"MainTask",STACK_SIZE,NULL,1,NULL);
            break;
    }
}

void loop() {
    
}