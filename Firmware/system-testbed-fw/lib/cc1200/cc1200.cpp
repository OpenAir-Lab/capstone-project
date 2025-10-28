#include <cc1200.h>
#include <Arduino.h>

/*! \file cc1200.cpp
    \brief Device Driver for CC1200 using Espressif Arduino Core.
    
    Implements Texas Instruments CC1200 Radio Transceiver Interface.
*/

#define CC1200_DEBUG Serial

// 
// Configure the CC120X
// 
extern cc1200_config_t cc1200;

/*
https://docs.arduino.cc/learn/communication/spi/#serial-peripheral-interface-spi
Mode	    Clock Polarity (CPOL)	Clock Phase (CPHA)	Output Edge	Data Capture
SPI_MODE0	0	                    0	                Falling	    Rising
SPI_MODE1	0	                    1	                Rising	    Falling
SPI_MODE2	1	                    0	                Rising	    Falling
SPI_MODE3	1	                    1	                Falling	    Rising
*/
// #define CC1200_SPI_MODE SPI_MODE0 // per User Guide 3.1.1

int cc1200_init(cc1200_config_t &cc1200) {
    // Initialize SPI
    // https://e2e.ti.com/support/wireless-connectivity/other-wireless-group/other-wireless/f/other-wireless-technologies-forum/307966/cc1200-spi-clock-query
    cc1200.spi->setFrequency(cc1200.spi_frequency); // 7.7 MHz per Martin B of TI E2E forums over 12 years ago 
    cc1200.spi->begin(cc1200.pin_sck);
    // set up slave select pins as outputs as the Arduino API
    // doesn't handle automatically pulling SS low
    // https://docs.espressif.com/projects/arduino-esp32/en/latest/api/spi.html
    pinMode(cc1200.spi->pinSS(), OUTPUT);  // VSPI SS

    // 100 ns delay between consecutive data bytes must be added
    // during burst write access to the configuration registers.

    // To verify initialization, the read the part number to confirm CC1200.
    uint8_t status_byte = 0;
    cc1200_spi_access_t register_access;
    register_access.readwrite_flag = COMMAND_RW_FLAG; // write = 0, read = 1
    register_access.configuration_address = cc1200_configuration_register_space_t::EXTENDED_ADDRESS;
    register_access.extended_address = cc1200_extended_register_space_t::PARTNUMBER;
    status_byte = cc1200_single_register_access(READ, register_access);
    cc1200_partnumber_t partnumber = (cc1200_partnumber_t)register_access.data;
    register_access.configuration_address = cc1200_configuration_register_space_t::EXTENDED_ADDRESS;
    register_access.extended_address = cc1200_extended_register_space_t::PARTVERSION;
    status_byte = cc1200_single_register_access(READ, register_access);
    uint8_t partversion = register_access.data;

    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("Radio Transceiver Part Number: 0x%2.2X", partnumber);
    #endif 
    return 0;
}

// Section 3.1.1 4-Wire Serial Configuration and Data Interface

// Table 3: SPI Access Types
int cc1200_single_register_access(bool readwrite_flag, cc1200_spi_access_t register_access) {
    register_access.burst_flag = !COMMAND_BURST_FLAG;
    // Extended register space access
    // This access mode starts with a specific command byte (0x2F).
    // When the extended address byte is sent on the SI line, SO will return all zeros. 
    // The chip status byte is returned on the SO line when the command is
    // transmitted as well as when data are written to the extended address 
    uint8_t address = readwrite_flag
        || register_access.burst_flag
        || register_access.configuration_address;
    uint8_t command;
    uint8_t data = 0;
    if (register_access.configuration_address == cc1200_configuration_register_space_t::EXTENDED_ADDRESS) {
        cc1200.spi_frequency = 7700000; // 7.7 MHz for extended memory read access
        // The first byte is interpreted as the command byte.
        command = address;
        // The first byte following this command is interpreted as the extended address
        address = register_access.extended_address;
        // Exactly one data byte is expected after the extended address byte
        data = 0;
    }
        // SPI interface transfers are MSB-first.
    cc1200.spi->beginTransaction(SPISettings(cc1200.spi_frequency, MSBFIRST, SPI_MODE0));
    // SS pin must be kept low during transfers on the SPI bus.
    digitalWrite(cc1200.spi->pinSS(), LOW);  //pull SS slow to prep other end for transfer

    // Transfer command byte with R/Wn bit, Burst access B bit, and 6-bit address.
    uint8_t status_byte = cc1200.spi->transfer(command);
    if (register_access.extended_address != 0) {
        // Transfer address byte with 8-bit address.
        status_byte = cc1200.spi->transfer(register_access.extended_address);
        if (status_byte == 0x00) {
            #ifdef CC1200_DEBUG
            CC1200_DEBUG.printf("Extended address transferred: %s\n", register_access.extended_address);
            #endif
        }
    }
    // Transfer data byte.
    status_byte = cc1200.spi->transfer(data);
    digitalWrite(cc1200.spi->pinSS(), HIGH);  //pull ss high to signify end of data transfer
    cc1200.spi->endTransaction();

    cc1200.spi_frequency = 10000000; // return to 10 MHz from 7.7 MHz
    return 0;
}

int cc1200_burst_register_access(cc1200_spi_access_t register_access) {
    register_access.burst_flag = COMMAND_BURST_FLAG;
    return 0;
}

int cc1200_command_strobe_access(cc1200_command_strobe_t command_strobe) {
    // header read write flag ignored, burst access not possible.
    cc1200_spi_access_t strobe_access;
    strobe_access.address = command_strobe;
    // the chip status byte is returned on the MISO line 
    // when a command strobe is sent on the MOSI line.
    uint8_t chip_status = cc1200_single_register_access(WRITE, strobe_access);
    return chip_status;
}

// Section 3.1.2 Chip Status Byte
// Status byte is sent on the MISO pin each time a header byte is transmitted on the MOSI pin.

// 
// Program CC120X into different modes (RX, TX, SLEEP, IDLE, etc)
//

// RXOFF_MODE=01 or TXOFF_MODE=01 means return to SFSTXON.

// Entered on 
// SRX                      from FSSTXON
// SRX/WOR                  from FSCAL, 
// SRX     or TXOFF_MODE=11 from STX
int cc1200_receive_mode() {
    cc1200_command_strobe_access(SRX);
    return 0;
}
// Left on 
//            RXOFF_MODE=00 to FSCAL,
// SFSTXON or RXOFF_MODE=01 to SFSTXON,
// STX     or RXOFF_MODE=10 to STX,
// Data Buffer Error        to RX_FIFO_ERROR

// Entered on STX or RXOFF_MODE=10, 
int cc1200_transmit_mode() {
    cc1200_command_strobe_access(STX);
    return 0;
}
// Left on
//            TXOFF_MODE=00 to FSCAL,
//            TXOFF_MODE=01 to FSSTXON,
// SRX     or TXOFF_MODE=11 to SRX,
// Data Buffer Error        to TX_FIFO_ERROR 

// Entered on
// SIDLE                    
//                          from FSCAL
// SFRX                     from RX_FIFO_ERROR
// SFTX                     from TX_FIFO_ERROR
int cc1200_idle_mode() {
    cc1200_command_strobe_access(SNOP);
    return 0;
}
// Left on
// SFSTXON/SRX/STX/WOR      to SFSTXON
// SCAL                     to FSCAL
// SPWD or WOR              to SLEEP

// Entered on
// SPWD or WOR              from IDLE
int cc1200_sleep_mode() {
    cc1200_command_strobe_access(SPWD);
    return 0;
}
// Left on CSn=0
//
// Read and write buffered data (RX FIFO and TX FIFO)
//


int cc1200_standard_FIFO_access() {
    return 0;
}

int cc1200_receive_FIFO_byte() {
    // See Section 3.2.3
    return 0;
}

int cc1200_transmit_FIFO_byte() {
    return 0;
}
// SERIAL_STATUS.SPI_DIRECT_ACCESS_CFG
// Configures which memory to access when using direct memory access
int cc1200_direct_FIFO_access() {
    // SERIAL_STATUS.SPI_DIRECT_ACCESS_CFG = 0
    return 0;
}

// FEC Workspace or AES Command Workspace (128 bytes free area).
// (PKT_CFG1.FEC_EN = 1) (free space otherwise)
int cc1200_direct_FEC_access() {
    // SERIAL_STATUS.SPI_DIRECT_ACCESS_CFG = 1
    // (Address < 0x80)
    return 0;
}

int cc1200_direct_AES_access() {
    // SERIAL_STATUS.SPI_DIRECT_ACCESS_CFG = 1
    
    // (0x80 < Address < 0xFF)
    return 0;
}
//
// Read status information
//