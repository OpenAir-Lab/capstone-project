
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "HardwareSerial.h"
#include "Wire.h"
#include <Adafruit_PCF8575.h>

#define ROTARY_ENCODER_A_PIN 6
#define ROTARY_ENCODER_B_PIN 7
#define ROTARY_ENCODER_BUTTON_PIN 5

#define ROTARY_ENCODER_VCC_PIN -1 
#define ROTARY_ENCODER_STEPS 4

typedef struct {
  bool initialized = false;
  TwoWire *i2c;
  uint8_t sensor_address = PCF8575_I2CADDR_DEFAULT;
  uint8_t pin_interrupt;
  std::string subsystem_name = "N/A";
} pcf8575_config_t;

bool pcf8575_portMode(Adafruit_PCF8575 &pcf, uint8_t port, uint8_t mode);

int pcf8575_readPort(Adafruit_PCF8575 &pcf, uint8_t port);

int pcf8575_writePort(Adafruit_PCF8575 &pcf, uint8_t port, uint8_t value);

int pcf8575_init(Adafruit_PCF8575 &pcf, pcf8575_config_t &configuration);