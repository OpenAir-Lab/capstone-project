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

int pcf8575_init(Adafruit_PCF8575 &pcf, pcf8575_config_t &configuration) {\
  if (configuration.initialized) {
      return 0;
  }
  pcf.begin(configuration.sensor_address, configuration.i2c);
  pinMode(configuration.pin_interrupt, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(configuration.pin_interrupt), portChanged, FALLING);
  #ifdef DEBUG_PCF8575
  DEBUG_PCF8575.println("\nStarting I2C scan...");
  #endif
  for (uint8_t addr = 1; addr < 127; addr++) { // Iterates through all addresses 1-126
      Wire.beginTransmission(addr);            // If address is found this should begin the exchange with said address
      if (Wire.endTransmission() == 0) {       // ends that exchange
          #ifdef DEBUG_PCF8575
          DEBUG_PCF8575.print("Found device at 0x");
          DEBUG_PCF8575.println(addr, HEX);
          #endif
          if (addr == configuration.sensor_address) {
              configuration.initialized = true;
          }
      }
  }
  #ifdef DEBUG_PCF8575
  if (!configuration.initialized) DEBUG_PCF8575.println("No devices found when scanned.");
  #endif
  return 0;
}
