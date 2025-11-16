#include <cc1200.h>
#include <Arduino.h>
#include <pcf8575.h>
#include <st7789.h>
#include <bitset>

/*! \file cc1200.cpp
    \brief Device Driver for CC1200 using Espressif Arduino Core.
    
    Implements Texas Instruments CC1200 Radio Transceiver Interface.
*/

#define CC1200_DEBUG Serial

// 
// Configure the CC120X
// 
extern cc1200_config_t cc1200;
extern Adafruit_PCF8575 pcf_radio;
extern pcf8575_config_t pcf_radio_config;
extern Adafruit_ST7789 tft;
extern st7789_config_t display_config; 
/*
https://docs.arduino.cc/learn/communication/spi/#serial-peripheral-interface-spi
Mode	    Clock Polarity (CPOL)	Clock Phase (CPHA)	Output Edge	Data Capture
SPI_MODE0	0	                    0	                Falling	    Rising
SPI_MODE1	0	                    1	                Rising	    Falling
SPI_MODE2	1	                    0	                Rising	    Falling
SPI_MODE3	1	                    1	                Falling	    Rising
*/
#define CC1200_SPI_MODE SPI_MODE0 // per User Guide 3.1.1

int setup_interface() {
    // Set up VSPI interface pins
    pinMode(cc1200.pin_ss, OUTPUT);
    digitalWrite(cc1200.pin_ss, HIGH);
    pinMode(cc1200.pin_sck, OUTPUT);
    digitalWrite(cc1200.pin_ss, HIGH);
    pinMode(cc1200.pin_miso, INPUT_PULLUP); // used in Power-On
    pinMode(cc1200.pin_mosi, OUTPUT); // used in Power-On
    pcf8575_portMode(pcf_radio, cc1200.pin_nrst, OUTPUT); // used in Power-On
    // Configure SPIClass from Espressif HAL Arduino Core compat
    // cc1200.spi_frequency = 7700000; // // keep at 7.7 MHz per Martin B of TI E2E forums over 12 years ago 
    cc1200.spi_frequency = 1000000; // 1 MHz
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.printf("(VSPI) Beginning use of VSPI...SS=%d,SCK=%d, MISO=%d, MOSI=%d,\n",
            cc1200.pin_ss, cc1200.pin_sck, cc1200.pin_miso, cc1200.pin_mosi
        );
        CC1200_DEBUG.printf("(I2C)  CC1200 Reset signal on \n", pcf_radio_config.sensor_address, cc1200.pin_nrst);
    #endif
    digitalWrite(cc1200.pin_ss, HIGH); // use of SS is bitbanged?
    cc1200.spi->begin(cc1200.pin_sck, cc1200.pin_miso, cc1200.pin_mosi, cc1200.pin_ss);
    cc1200.spi->beginTransaction(
        SPISettings(cc1200.spi_frequency, MSBFIRST, CC1200_SPI_MODE)
    );
    return 0;
}

// Section 3.1.1 4-Wire Serial Configuration and Data Interface


// Table 3: SPI Access Types
int cc1200_single_register_access(bool readwrite_flag, cc1200_spi_access_t &register_access) {
    uint8_t chip_status = 0;
    digitalWrite(cc1200.pin_ss, LOW);
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.print("Wait for MISO to go low...\n");
    #endif
    while (digitalRead(cc1200.pin_miso)); // Wait for MISO to go low
    bool isCommandStrobe = register_access.configuration_address >= 0x30 
        && register_access.configuration_address <= 0x3D;
    if (isCommandStrobe) {
        uint8_t command = (readwrite_flag ? COMMAND_RW_FLAG : 0x00)
            | register_access.configuration_address;
        chip_status = cc1200.spi->transfer(command);
        #ifdef CC1200_DEBUG
        CC1200_DEBUG.printf("(VSPI) Transferred strobe 0x%2.2X (0x%2.2X)\n", command, chip_status);
        #endif
    } else {
        uint8_t address;
        // extended access requires two byte transfers
        if (register_access.extended_address != NOTUSED) {
            // first transfer as command (0x2F)
            uint8_t command = (readwrite_flag ? COMMAND_RW_FLAG : 0x00)
            | register_access.configuration_address;
            chip_status = cc1200.spi->transfer(command);
            #ifdef CC1200_DEBUG
            CC1200_DEBUG.printf("(VSPI) Transferred command 0x%2.2X (0x%2.2X)\n", command, chip_status);
            #endif
            // second transfer as extended address
            address = (readwrite_flag ? COMMAND_RW_FLAG : 0x00)
            | register_access.extended_address;
        // one byte transfer as configuration address
        } else {
            address = (readwrite_flag ? COMMAND_RW_FLAG : 0x00)
            | register_access.configuration_address;
        }
        chip_status = cc1200.spi->transfer(address);
        #ifdef CC1200_DEBUG
            CC1200_DEBUG.printf("(VSPI) Transferred address 0x%2.2X (0x%2.2X)\n", address, chip_status);
        #endif
        if (readwrite_flag == READ) {
            register_access.data = cc1200.spi->transfer(0x00);
        } else {
            chip_status = cc1200.spi->transfer(register_access.data);
        }
    }
    digitalWrite(cc1200.pin_ss, HIGH);
    return chip_status;
}

// 100 ns delay between consecutive data bytes must be added
// during burst write access to the configuration registers.
int cc1200_burst_register_access(cc1200_spi_access_t register_access) {
    register_access.burst_flag = COMMAND_BURST_FLAG;
    return 0;
}

int cc1200_command_strobe_access(cc1200_command_strobe_t command_strobe) {
    // header read write flag ignored, burst access not possible.
    cc1200_spi_access_t strobe_access;
    strobe_access.configuration_address = (cc1200_configuration_register_space_t)command_strobe;
    // the chip status byte is returned on the MISO line 
    // when a command strobe is sent on the MOSI line.
    uint8_t chip_status = cc1200_single_register_access(READ, strobe_access);
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

// 9.1.2 Manual Reset
// by issuing a manual reset, all internal registers are set
// to their default values and the radio will enter IDLE state.
int cc1200_reset(bool hard_reset) {
    uint8_t chip_status;
    if (hard_reset) {
        pcf8575_writePort(pcf_radio, cc1200.pin_nrst, LOW); // active low reset signal
        delay(10);
        pcf8575_writePort(pcf_radio, cc1200.pin_nrst, HIGH); // active low reset signal
        // delay(1000);
    } else {
        chip_status = cc1200_command_strobe_access(SRES);
    }
    // If the chip has had sufficient time for the 
    // crystal oscillator to stabilize after the power-on-reset: 
    // the SO pin will go low immediately after taking CSn low. 
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.print("(VSPI) Chip power and crystal have not yet stabilized...\n");
    #endif
    bool chip_nrdy;
    uint8_t chip_state;
    do {
        chip_status = cc1200_command_strobe_access(SNOP);
        chip_nrdy = (chip_status >> 7); // ready when low
        chip_state = ((chip_status >> 4) & 0x7);//  
        // If CSn is taken low before reset is completed:
        // the SO pin will first go high, indicating that the crystal 
        // oscillator is not stabilized, before going low
        if (chip_nrdy == HIGH) {
            #ifdef CC1200_DEBUG
            CC1200_DEBUG.print("Insufficient time for Crystal Oscillator to stabilize!\n");
            #endif
        }
        // delayMicroseconds(100);
    } while (chip_nrdy == HIGH);
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.print("Sufficient time for Crystal Oscillator to stabilize!\n");
    #endif
    return 0;
}

int cc1200_init(cc1200_config_t &cc1200) {
    if (cc1200.initialized) {
      return 0;
    }
    // set up slave select pins as outputs as the Arduino API
    // doesn't handle automatically pulling SS low
    // https://docs.espressif.com/projects/arduino-esp32/en/latest/api/spi.html
    // pinMode(cc1200.spi->pinSS(), OUTPUT);  // VSPI SS
    // Initialize SPI
    // https://e2e.ti.com/support/wireless-connectivity/other-wireless-group/other-wireless/f/other-wireless-technologies-forum/307966/cc1200-spi-clock-query    
    setup_interface();
    #ifdef CC1200_DEBUG
            CC1200_DEBUG.print("(VSPI) CC1200 SPI has begun...\n");
    #endif
    // 9.1 Power-On Start-Up Sequence
    // Must be reset before entering the state diagram in Figure 2.
    // CHIP_RDYn on the SO pin after SS is pulled low.
    cc1200_reset(true);
    // once reset is completed, chip will be in the IDLE state.
    // To verify initialization, the read the part number to confirm CC1200.
    uint8_t status_byte = 0;
    cc1200_spi_access_t register_access;
    register_access.configuration_address = cc1200_configuration_register_space_t::EXTENDED_ADDRESS;
    register_access.extended_address = cc1200_extended_register_space_t::PARTNUMBER;
    status_byte = cc1200_single_register_access(READ, register_access);
    cc1200.partnumber = (cc1200_partnumber_t)register_access.data;
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("Radio Transceiver Part Number: 0x%2.2X, expects 0x20\n", cc1200.partnumber);
    #endif 
    register_access.configuration_address = cc1200_configuration_register_space_t::EXTENDED_ADDRESS;
    register_access.extended_address = cc1200_extended_register_space_t::PARTVERSION;
    status_byte = cc1200_single_register_access(READ, register_access);
    cc1200.partrevision = register_access.data;
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("Radio Transceiver Part Revision: 0x%2.2X, expects 0x11\n", cc1200.partrevision);
    #endif
    cc1200.initialized = true;
    return 0;
}
// TER de TI E2E Forums: 
// Use of transparent mode requires oversampling of the output.
// Naїve upsampling: Can also try repeating a sample 3 times for 8kHz signal  
// TODO: Try sample rate of 24?
// Avoid using a edge based demodulation due to jitter/ spikes.
void demonstrate_radio_transceiver() {
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("Entering Radio Transceiver Demonstration!\n");
    #endif
    // Display register-level transceiver configuration for SmartRF interplay
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(1);
    tft.setTextWrap(false);
    tft.setTextColor(0xFFFF);
    tft.printf("Current Transceiver Configuration:\nPart Number/Revision = 0x%2.2X/0x%2.2X\n", 
        cc1200.partnumber, cc1200.partrevision
    );
        // IOCFG3 = 0x08, IOCFG2 = 0x09, IOCFG1 = 0xB0, IOCFG0 0xB0,
        // // SYNC3, SYNC2, SYNC1, SYNC0,
        // SYNC_CFG1 = 0x08, // SYNC_CFG0,
        // // DEVIATION_M,
        // MODCFG_DEV_E = 0x03,
        // DCFILT_CFG = 0x1C,
        // PREAMBLE_CFG1 = 0x14, // PREAMBLE_CFG0,
        // IQIC = 0xC4,
        // CHAN_BW = 0x28,
        // MDMCFG1 = 0x06, MDMCFG0 = 0x0A,
        // SYMBOL_RATE2 = 0x43, SYMBOL_RATE1 = 0xA9, SYMBOL_RATE0 = 0x2A,
        // AGC_REF = 0x20,
        // AGC_CS_THR = 0x19,
        // // AGC_GAIN_ADJUST,
        // // AGC_CFG3, AGC_CFG2, 
        // AGC_CFG1 = 0xAF, AGC_CFG0 = 0xCF,
        // FIFO_CFG = 0x00,
        // // DEV_ADDR,
        // // SETTLING_CFG,
        // FS_CFG = 0x12,
        // // WOR_CFG1, WOR_CFG0,
        // // WOR_EVENT0_MSB, WOR_EVENT0_LSB,
        // // RXDCM_TIME,
        // PKT_CFG2 = 0x05, PKT_CFG1 = 0x00, PKT_CFG0 = 0x20,
        // // RFEND_CFG1, RFEND_CFG0,
        // PA_CFG1 = 0x78, PA_CFG0 = 0x7C,
        // // ASK_CFG,
        // // PKT_LEN,
    tft.printf("IO Pin GPIO3/2/1/0=0x%2.2X %2.2X %2.2X %2.2X\n"
        "Sync Word [31:24] [23:16] [15:8] [7:0] SYNCHH_LL=0x00 00 00 00\n"
        "SYNC_CFG1=0x%2.2X\n"
        "Modulation Format and Frequency Deviation MODCFG_DEV_E=0x%2.2X\n"
        "Digital DC Removal DCFILT_CFG=0x%2.2X\n"
        "Preamble PREAMBLE_CFG1=0x%2.2X\n"
        "Digital Image Channel Compensation IQIC=0x%2.2X\n"
        "Channel Filter CHAN_BW=0x%2.2X\n"
        "General Modem Parameter MDMCFG1=0x%2.2X MDMCFG0=0x%2.2X\n"
        "Symbol Rate Exponent and Mantissa [19:16] [15:8] [7:0]\n"
        "DRATE2=0x%2.2X DRATE1=0x%2.2X DRATE0=0x%2.2X\n"
        "Automatic Gain Control Settings\n"
        "AGC_REF=0x%2.2X AGC_CS_THR=0x%2.2X AGC_CFG1=0x%2.2X AGC_CFG0=0x%2.2X\n"
        "FIFO FIFO_CFG=0x%2.2X\n"
        "Frequency Synthesizer FS_CFG=0x%2.2X\n"
        "Packet PKT_CFG2=0x%2.2X PKT_CFG1=0x%2.2X PKT_CFG0=0x%2.2X\n"
        "Power Amplifier PA_CFG2=0x%2.2X PA_CFG0=0x%2.2X\n",
        0x08,0x09,0xB0,0xB0,0x08,0x03,0x1C,0x14,0xC4,0x28,0x06,0x0A,0x43,0xA9,0x2A,0x20,0x19,0xAF,0xCF,0x00,0x12,0x05,0x00,0x20,0x78,0x7C    
    );
    // ((SYNC_CFG1 & GENMASK(7,5)) == 0x00) ? "No Sync Word" : "Sync Word" // no sync word in analog FM
    //         // sync threshold not yet understood
            
    // tft.printf("Soft Decision sync word threshold = 0x%2.2X\n", (SYNC_CFG1 & GENMASK(4,0)));
}