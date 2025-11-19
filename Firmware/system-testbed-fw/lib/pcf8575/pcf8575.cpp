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
    if (configuration.initialized) {
        return 0;
    }
    pcf.begin(configuration.sensor_address, configuration.i2c);
    pinMode(configuration.pin_interrupt, INPUT_PULLUP);
    const char* subsystem_name = configuration.subsystem_name.c_str();
    // attachInterrupt(digitalPinToInterrupt(configuration.pin_interrupt), portChanged, FALLING);
    #ifdef DEBUG_PCF8575
    DEBUG_PCF8575.printf("(I2C0 -----) Scanning for %s Port Expander on address 0x%2.2X...\n", subsystem_name, configuration.sensor_address);
    #endif
    for (uint8_t addr = 1; addr < 127; addr++) { // Iterates through all addresses 1-126
        Wire.beginTransmission(addr);            // If address is found this should begin the exchange with said address
        if (Wire.endTransmission() == 0) {       // ends that exchange
            if (addr == configuration.sensor_address) {
                #ifdef DEBUG_PCF8575
                DEBUG_PCF8575.printf("(I2C0 @0x%2.2X) Initialized %s Port Expander!\n", configuration.sensor_address, subsystem_name);
                #endif
                configuration.initialized = true;
            }
        }
    }
    #ifdef DEBUG_PCF8575
    if (!configuration.initialized) DEBUG_PCF8575.printf("(I2C0 -----) %s Port Expander was not found on address 0x%2.2X when scanned.", configuration.subsystem_name, configuration.sensor_address);
    #endif
    return 0;
}
