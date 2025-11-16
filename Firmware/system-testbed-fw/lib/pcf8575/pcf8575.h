
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "HardwareSerial.h"
#include "Wire.h"
#include <Adafruit_PCF8575.h>

typedef struct {
  bool initialized = false;
  TwoWire *i2c;
  uint8_t sensor_address = PCF8575_I2CADDR_DEFAULT; // known sensor address
  uint8_t pin_interrupt;
  // Select one of eight PCF8575 sensor address pairs
  // uint8_t sensor_read_address = 0x41;  // 0x41 -> 0x4F
  // uint8_t sensor_write_address = 0x40; // 0x40 -> 0x4E
} pcf8575_config_t;

bool pcf8575_portMode(Adafruit_PCF8575 &pcf, uint8_t port, uint8_t mode);

int pcf8575_readPort(Adafruit_PCF8575 &pcf, uint8_t port);

int pcf8575_writePort(Adafruit_PCF8575 &pcf, uint8_t port, uint8_t value);

int pcf8575_init(Adafruit_PCF8575 &pcf, pcf8575_config_t &configuration);