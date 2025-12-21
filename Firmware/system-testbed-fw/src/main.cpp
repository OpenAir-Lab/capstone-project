#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include <bq25622.h>
#include <BQ27427.h>

#include <cc1200.h>          // used in Transceiver Modulino 
#include <sky13330.h>        // used in RF Switch Modulino
#include <grf5604.h>         // used in Amplifier Modulino

#include <pcf8575.h>         // used in MCU, HMI, and RF Modulinos

#include <st7789.h>          // used in HMI Modulino
#include <adafruit_display_demo.h>

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

demonstration_t demo = HUMAN_MACHINE_INTERFACE;

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
#define CC1200_NRST  4    // Reset is not correct pin yet. 
#define CC1200_SCLK  18    //  SCK=05 -> 18
#define CC1200_MISO  19    // MISO=18 -> 19
#define CC1200_MOSI  23    // MOSI=19 -> 23
#define CC1200_SS    5     //   SS=23 -> 5
#define CC1200_GDIO0 32
#define CC1200_GDIO2 33
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

extern Adafruit_ST7789 tft;
st7789_config_t display_config;

extern const unsigned char PROGMEM image_guy_bits[];
extern const unsigned char PROGMEM image_battery_0_bits[];
extern const unsigned char PROGMEM image_battery_100_bits[];

extern keypad_t keypad;                       // State of the keypad.
extern encoder_t encoder;
extern EventGroupHandle_t keypad_event_group; // Event group handle for keypad events.
extern QueueHandle_t keypad_queue;            // Queue handle for keypad input.
extern QueueHandle_t encoder_queue;

int hmi_init() {
    // Texas Instruments PCF8575 16-bit Port Expander
    pcf_mcu_config.i2c = &Wire;   // I2C0
    pcf_mcu_config.pin_interrupt = 12;
    pcf_mcu_config.subsystem_name = "MCU";
    pcf_mcu_config.sensor_address = PCF8575_I2CADDR_DEFAULT+0;
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

    while (!(pcf8575_init(pcf_hmi, pcf_hmi_config) == 0)) {
        vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
    }
    pcf8575_writePort(pcf_hmi, 2, HIGH); // doubled up reset
    while (!(pcf8575_init(pcf_mcu, pcf_mcu_config) == 0)) {
        vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
    }
    pcf8575_writePort(pcf_mcu, 4, HIGH); // doubled up reset
    while (!(pcf8575_scan_init(pcf_hmi, pcf_hmi_config) == 0)) {
        vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
    }
    while (!(display_init(display_config) == 0)) {
        vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
    };
    return 0;
}

extern TaskHandle_t vDisplayRSSITaskHandle;
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

    while (!(pcf8575_init(pcf_radio, pcf_radio_config) == 0)) {
        vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
    }
    // Initialize RF Amplifiers in powered down state
    while (!(grf5604_init(uhf_grf5604) == 0)) {
        vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
    }
    while (!(grf5604_init(vhf_grf5604) == 0)) {
        vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
    }
    #ifdef DEBUG
    DEBUG.println("ENTER SWITCH INITIALIZER\n");
    #endif
    // Initialize SP4T Switch for UHF receive on Port 2
    while (!(sky13330_init(sky13330) == 0)) {
        vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
    }
    // Initialize RF Transceiver in UHF receive Analog FM mode
    // while (!(cc1200_init(cc1200) == 0)) {
    //     vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
    // }
    // xTaskCreate(vDisplayRSSITask, "displayRSSI", STACK_SIZE, NULL, 2, &vDisplayRSSITaskHandle);
    return 0;   
}

void demonstrate_mcu(void *parameter) {
    // Texas Instruments PCF8575 16-bit Port Expander
    pcf_mcu_config.i2c = &Wire;   // I2C0
    pcf_mcu_config.pin_interrupt = 12;
    pcf_mcu_config.subsystem_name = "MCU";
    pcf_mcu_config.sensor_address = PCF8575_I2CADDR_DEFAULT+0;
    while (!(pcf8575_init(pcf_mcu, pcf_mcu_config) == 0));
    for(;;) {
        // Yield the CPU to other tasks
        vTaskDelay(KEYPAD_TASK_DELAY_TIME);
    }
}

void demonstrate_usb_power_delivery(void *parameter) {
    // Requires having run the one time advertisement programming routine.
}
// create instance of the PMIC driver
BQ25622 pmic;
const unsigned int BATTERY_CAPACITY = 2500; // e.g. 850mAh battery

void demonstrate_battery_power_supply(void *parameter) {
    #ifdef DEBUG
    DEBUG.println("(I2C0 -----) Scanning for bq27427 Fuel Gauge on address 0x55...\n");
    #endif
    lipo.begin();
    lipo.setCapacity(BATTERY_CAPACITY);
    #ifdef DEBUG
    // tft.fillScreen(ST77XX_BLACK);
    DEBUG.printf("(I2C0 @0x55) Fuel Gauge Information:\n"
        "State-Of-Charge: %d%\n"
        "Battery Voltage: %d mV\n"
        "Average Current: %d mA\n"
        "Battery Capacity: %d / %d mAh\n"
        "Average Power Draw: %d mW\n"
        "State-Of-Health: %d%\n",
        lipo.soc(),
        lipo.voltage(),
        lipo.current(AVG),
        lipo.capacity(REMAIN),
        lipo.capacity(FULL),
        lipo.power(),
        lipo.soh()
    );
    #endif
    for (;;) {
        // Yield the CPU to other tasks
        vTaskDelay(KEYPAD_TASK_DELAY_TIME);
    }
}

void demo_display(void *parameter) {
    for (;;) {
        demoAdafruitDriver();
        vTaskDelay(KEYPAD_TASK_DELAY_TIME);
    }
}

void demonstrate_human_machine_interface(void *parameter) {
    const EventBits_t waitMask = KEY_EVENT_BITMASK; // listen for any keypad bits
    uint32_t keyMask;

    tft.printf("Selected Demo is of the Human-Machine Interface Modulino!\n"
        "[X] HMI Modulino requires Adafruit 1.9\" 170x320 TFT Module to be plugged in.\n"
    );
    // enc_event_t ev;
    for(;;) {
        EventBits_t bits = xEventGroupWaitBits(keypad_event_group, waitMask, pdTRUE, pdFALSE, portMAX_DELAY);
        if (bits != 0) {
            #ifdef DEBUG
            DEBUG.printf("(I2C0 @0x%2.2X) Key Mask Changed... \n", pcf_hmi_config.sensor_address);
            #endif
            if (xQueueReceive(keypad_queue, &keyMask, KEYPAD_TASK_DELAY_TIME) == pdTRUE) {
                #ifdef DEBUG
                DEBUG.printf("(I2C0 @0x%2.2X) Key Mask = 0x%4.4X\n", pcf_hmi_config.sensor_address, keyMask);
                #endif
                switch(keyMask) {
                    case(KEY_1): DEBUG.printf("KEY_1\n"); break;
                    case(KEY_2): DEBUG.printf("KEY_2\n"); break;
                    case(KEY_3): DEBUG.printf("KEY_3\n"); break;
                    case(KEY_4): DEBUG.printf("KEY_4\n"); break;
                    case(KEY_5): DEBUG.printf("KEY_5\n"); break;
                    case(KEY_6): DEBUG.printf("KEY_6\n"); break;
                    case(KEY_7): DEBUG.printf("KEY_7\n"); break;
                    case(KEY_8): DEBUG.printf("KEY_8\n"); break;
                    case(KEY_9): DEBUG.printf("KEY_9\n"); break;
                    case(KEY_STAR): DEBUG.printf("KEY_STAR\n"); break;
                    case(KEY_0): DEBUG.printf("KEY_0\n"); break;
                    case(KEY_POUND): DEBUG.printf("KEY_POUND\n"); break;
                    case(KEY_RIGHT): DEBUG.printf("KEY_RIGHT\n"); break;
                    case(KEY_UP): DEBUG.printf("KEY_UP\n"); break;
                    case(KEY_DOWN): DEBUG.printf("KEY_DOWN\n"); break;
                    case(KEY_LEFT): DEBUG.printf("KEY_LEFT\n"); break;
                    default: 
                        // multiple keys pressed
                        break;
                }
            }
        }   
        // if (xQueueReceive(encoder_queue, &ev, portMAX_DELAY)) {

        //     if (ev.type == ENC_EVT_ROTATE) {
        //         DEBUG.printf("ENC delta: %d\n", ev.value);
        //     }

        //     if (ev.type == ENC_EVT_BUTTON) {
        //         if (ev.value == 1) DEBUG.println("ENC SW pressed");
        //         else DEBUG.println("ENC SW released");
        //     }
        // }
        // Yield the CPU to other tasks
        vTaskDelay(KEYPAD_TASK_DELAY_TIME);
    }
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
            while (!(hmi_init() == 0)) {
                vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
            };
            while (!(bq25662_init() == 0)) {
                vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
            };
            xTaskCreate(demonstrate_battery_power_supply,"MainTask",STACK_SIZE,NULL,1,NULL);
            break;
        case (HUMAN_MACHINE_INTERFACE): // Navigate menus and accept user input
            #ifdef DEBUG
            DEBUG.printf("Selected Demo is of the Human-Machine Interface Modulino!\n"
                "[X] HMI Modulino requires Adafruit 1.9\" 170x320 TFT Module to be plugged in.\n"
            );
            #endif
            // Initialize HMI
            while (!(hmi_init() == 0)) {
                vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
            };
            // xTaskCreate(demo_display,"adafruitTask",STACK_SIZE,NULL,1,NULL);
            xTaskCreate(demonstrate_human_machine_interface,"MainTask",STACK_SIZE,NULL,(tskIDLE_PRIORITY + 3),NULL);
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
            // Initialize HMI Devices
            while (!(hmi_init() == 0)) {
                vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
            };
            // Initialize Radio Devices
            while (!(radio_init() == 0)) {
                vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
            }
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
            // Initialize HMI Devices
            while (!(hmi_init() == 0)) {
                vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
            }
            // Initialize Radio Devices
            while (!(radio_init() == 0)) {
                vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
            }
            xTaskCreate(demonstrate_radio_amplifier,"MainTask",STACK_SIZE,NULL,1,NULL);
            break;
        case (RADIO_SWITCH):            // Scatter parameterize multiport Switch
            #ifdef DEBUG
            DEBUG.printf("Selected Demo is of the SP4T RF Switch Modulino!\n"
                "[X] Switch Modulino requires HMI Modulino to be plugged in.\n"
                "[X] Switch Modulino requires Dual-Band Antenna to be plugged in.\n"
            );
            #endif
            // Initialize HMI Devices
            while (!(hmi_init() == 0)) {
                vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
            }
            // Initialize Radio Devices
            while (!(radio_init() == 0)) {
                vTaskDelay(50/portTICK_PERIOD_MS); // must block keypad tasks
            }
            xTaskCreate(demonstrate_radio_switch,"MainTask",STACK_SIZE,NULL,1,NULL);
            break;
    }
}

void loop() {
    
}