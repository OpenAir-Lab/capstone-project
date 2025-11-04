#include <pcf8575.h>
#include <Arduino.h>

#define DEBUG_PCF8575 Serial

bool pcf8575_portMode(Adafruit_PCF8575 &pcf, uint8_t port, uint8_t mode) {
  return pcf.pinMode(port, mode);
}

int pcf8575_readPort(Adafruit_PCF8575 &pcf, uint8_t port) {
  return pcf.digitalRead((uint8_t)port);
}

int pcf8575_writePort(Adafruit_PCF8575 &pcf, uint8_t port, uint8_t value) {
  return pcf.digitalWrite(port, value);
}

int pcf8575_init(Adafruit_PCF8575 &pcf, pcf8575_config_t &configuration) {
  pcf.begin(configuration.sensor_address, configuration.i2c);
  pinMode(configuration.pin_interrupt, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(configuration.pin_interrupt), portChanged, FALLING);
  return 0;
}
