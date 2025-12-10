#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "AiEsp32RotaryEncoder.h" //encoder
#include "Arduino.h"  //for encoder

#include <pcf8575.h>         // used in MCU, HMI, and RF Modulinos

#include <st7789.h>          // used in HMI Modulino
#include <max98357a.h>
#include <ics43434.h>

#define PIN_SDA 21
#define PIN_SCL 22

#define TFT_SS    4 // 15 -> 4
#define TFT_DC   26 // Display Command pin
#define TFT_MOSI 13 // ESP32 IOMUX Default
#define TFT_SCLK 14 // ESP32 IOMUX Default
#define TFT_RST  -1 // Not connected
st7789_config_t display_config;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_SS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// HMI Port Expander
Adafruit_PCF8575 pcf_hmi;
pcf8575_config_t pcf_hmi_config; // on 0x20+1

int hmi_init() {
    // PCF8575 HMI Port Expander
    pcf_hmi_config.i2c = &Wire;   // I2C0
    pcf_hmi_config.pin_interrupt = 35;
    pcf_hmi_config.subsystem_name = "HMI";
    pcf_hmi_config.sensor_address = PCF8575_I2CADDR_DEFAULT+2;
    pcf_hmi.pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT);
    pcf_hmi.pinMode(ROTARY_ENCODER_A_PIN, INPUT);
    pcf_hmi.pinMode(ROTARY_ENCODER_B_PIN, INPUT);
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

//instead of changing here, rather change numbers above
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

//paramaters for button
unsigned long shortPressAfterMiliseconds = 50;   //how long short press shoud be. Do not set too low to avoid bouncing (false press events).
unsigned long longPressAfterMiliseconds = 1000;  //how long čong press shoud be.


void on_button_short_click() {
  Serial.print("button SHORT press ");
  Serial.print(millis());
  Serial.println(" milliseconds after restart");
}

void on_button_long_click() {
  Serial.print("button LONG press ");
  Serial.print(millis());
  Serial.println(" milliseconds after restart");
}


void handle_rotary_button() {
  static unsigned long lastTimeButtonDown = 0;
  static bool wasButtonDown = false;

  bool isEncoderButtonDown = rotaryEncoder.isEncoderButtonDown();
  //isEncoderButtonDown = !isEncoderButtonDown; //uncomment this line if your button is reversed

  if (isEncoderButtonDown) {
    Serial.print("+");  //REMOVE THIS LINE IF YOU DONT WANT TO SEE
    if (!wasButtonDown) {
      //start measuring
      lastTimeButtonDown = millis();
    }
    //else we wait since button is still down
    wasButtonDown = true;
    return;
  }

//button is up
  if (wasButtonDown) {
    Serial.println("");  //REMOVE THIS LINE IF YOU DONT WANT TO SEE
    //click happened, lets see if it was short click, long click or just too short
    if (millis() - lastTimeButtonDown >= longPressAfterMiliseconds) {
      on_button_long_click();
    } else if (millis() - lastTimeButtonDown >= shortPressAfterMiliseconds) {
      on_button_short_click();
    }
  }
  wasButtonDown = false;
}

void rotary_loop() {
  //dont print anything unless value changed
  if (rotaryEncoder.encoderChanged()) {
    Serial.print("Value: ");
    Serial.println(rotaryEncoder.readEncoder());
  }
  handle_rotary_button();
}

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

void setup() {
  Serial.begin(115200);
  hmi_init();
  //we must initialize rotary encoder
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  //set boundaries and if values should cycle or not
  //in this example we will set possible values between 0 and 1000;
  bool circleValues = false;
  rotaryEncoder.setBoundaries(0, 1000, circleValues);  //minValue, maxValue, circleValues true|false (when max go to min and vice versa)

  /*Rotary acceleration introduced 25.2.2021.
   * in case range to select is huge, for example - select a value between 0 and 1000 and we want 785
   * without accelerateion you need long time to get to that number
   * Using acceleration, faster you turn, faster will the value raise.
   * For fine tuning slow down.
   */
  //rotaryEncoder.disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it
  rotaryEncoder.setAcceleration(250);  //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration
}

void loop() {
  //in loop call your custom function which will process rotary encoder values
  rotary_loop();
  delay(50);  //or do whatever you need to do...
}



