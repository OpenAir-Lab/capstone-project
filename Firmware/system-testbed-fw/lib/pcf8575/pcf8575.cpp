#include <pcf8575.h>

#include <Adafruit_PCF8575.h>
Adafruit_PCF8575 pcf;

#define DEBUG_PCF8575 Serial

extern pcf8575_config_t pcf8575;

bool pcf8575_portMode(uint8_t port, uint8_t mode) {
  return pcf.pinMode(port, mode);
}

int pcf8575_readPort(uint8_t port) {
  return pcf.digitalRead((uint8_t)port);
}

int pcf8575_writePort(uint8_t port, uint8_t value) {
  return pcf.digitalWrite(port, value);
}

int pcf8575_init(pcf8575_config_t &configuration) {
  pcf8575 = configuration;
  pcf.begin(pcf8575.sensor_address, pcf8575.i2c);
  pinMode(pcf8575.pin_interrupt, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(pcf8575.pin_interrupt), portChanged, FALLING);
  return 0;
}
