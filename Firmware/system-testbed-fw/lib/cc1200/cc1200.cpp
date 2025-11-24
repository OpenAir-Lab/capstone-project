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

extern Adafruit_ST7789 tft;
extern st7789_config_t display_config; 
// 
// Configure the CC120X
// 
extern cc1200_config_t cc1200;
extern Adafruit_PCF8575 pcf_radio;
extern pcf8575_config_t pcf_radio_config;
// Section 3.1.1 4-Wire Serial Configuration and Data Interface
/*
https://docs.arduino.cc/learn/communication/spi/#serial-peripheral-interface-spi
Mode	    Clock Polarity (CPOL)	Clock Phase (CPHA)	Output Edge	Data Capture
SPI_MODE0	0	                    0	                Falling	    Rising
SPI_MODE1	0	                    1	                Rising	    Falling
SPI_MODE2	1	                    0	                Rising	    Falling
SPI_MODE3	1	                    1	                Falling	    Rising
*/
#define CC1200_SPI_MODE SPI_MODE0 // per User Guide 3.1.1

// Section 3.1.2 Chip Status Byte
// Status byte is sent on the MISO pin each time a header byte is transmitted on the MOSI pin.
const char* main_states[21] = {
    // idle state
    "IDLE",
    // receive states
    "RX",
    "RX_END",
    // transmit states
    "TX",
    "TX_END",
    // fast TX ready
    "FSTXON",
    // frequency synthesizer calibration is running
    "CALIBRATE" ,
    "BIAS_SETTLE_MC",
    "MANCAL",
    "STARTCAL",
    "ENDCAL",
    // PLL is setting
    "BIAS_SETTLE",
    "REG_SETTLE",
    "BWBOOST",
    "FS_LOCK",
    "IFADCON",
    "RXTX_SWITCH"
    "TXRX_SWITCH"
    "IFADCON_TXRX"
    // RX FIFO has over/underflowed. Read out any useful data,
    // then flush the FIFO with an SFRX strobe
    "RX_FIFO_ERR",
    // TX FIFO has over/underflowed. 
    // Flush the FIFO with an SFTX strobe
    "TX_FIFO_ERR"
};

int update_status(uint8_t chip_status) {
    cc1200.chip_nrdy =  (uint8_t)(chip_status & BIT7); // ready when low
    cc1200.main_state = (uint8_t)(chip_status & GENMASK(6,4));
    return cc1200.main_state; 
}

// Table 3: SPI Access Types
int cc1200_single_register_access(cc1200_spi_access_t &register_access) {
    digitalWrite(cc1200.pin_ss, LOW);
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.print("(VSPI) Wait for MISO to go low...\n");
    #endif
    while (digitalRead(cc1200.pin_miso)); // Wait for MISO to go low
    bool isCommandStrobe = (
           (uint8_t)register_access.configuration_address >= 0x30 
        && (uint8_t)register_access.configuration_address <= 0x3D
    );
    uint8_t address = (register_access.readwrite_flag ? COMMAND_RW_FLAG : !COMMAND_RW_FLAG);
    address |= register_access.configuration_address;
    // extended access requires two byte transfers
    if (register_access.extended_address != NOTUSED) {
        // first transfer as command (0x2F)
        update_status(cc1200.spi->transfer(address));
        #ifdef CC1200_DEBUG
        CC1200_DEBUG.printf("(VSPI) Transferred command 0x%2.2X [%s]\n", 
            address, main_states[cc1200.main_state]
        );
        #endif
        // second transfer as extended address
        address = register_access.extended_address;
    // one byte transfer as configuration address
    }
    update_status(cc1200.spi->transfer(address));
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.printf("(VSPI) Transferred %s 0x%2.2X [%s]\n", isCommandStrobe ? "strobe" : "address",
            address, main_states[cc1200.main_state]
        );
    #endif
    if (register_access.readwrite_flag == READ) {
        register_access.data = cc1200.spi->transfer(0x00);
    } else {
        update_status(cc1200.spi->transfer(register_access.data));
    }
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("(VSPI) %s data 0x%2.2X [%s]\n", register_access.readwrite_flag ? "Read in" : "Wrote in", 
        register_access.data, main_states[cc1200.main_state]
    );
    #endif
    digitalWrite(cc1200.pin_ss, HIGH);
    return cc1200.main_state;
}

// 100 ns delay between consecutive data bytes must be added
// during burst write access to the configuration registers.
int cc1200_burst_register_access(cc1200_spi_access_t register_access) {
    register_access.burst_flag = BURST;
    cc1200_single_register_access(register_access);
    return 0;
}

int cc1200_command_strobe_access(cc1200_command_strobe_t command_strobe) {
    // header read write flag ignored, burst access not possible.
    cc1200_spi_access_t strobe_access;
    strobe_access.configuration_address = (cc1200_configuration_register_space_t)command_strobe;
    // the chip status byte is returned on the MISO line 
    // when a command strobe is sent on the MOSI line.
    update_status(cc1200_single_register_access(strobe_access));
    return cc1200.main_state;
}

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
    if (hard_reset) {
        pcf8575_writePort(pcf_radio, cc1200.pin_nrst, LOW); // active low reset signal
        delay(10);
        pcf8575_writePort(pcf_radio, cc1200.pin_nrst, HIGH); // active low reset signal
        // delay(1000);
    } else {
        update_status(cc1200_command_strobe_access(SRES));
    }
    // If the chip has had sufficient time for the 
    // crystal oscillator to stabilize after the power-on-reset: 
    // the SO pin will go low immediately after taking CSn low. 
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.print("(VSPI) Chip power and crystal have not yet stabilized...\n");
    #endif
    do {
        update_status(cc1200_command_strobe_access(SNOP));
        // If CSn is taken low before reset is completed:
        // the SO pin will first go high, indicating that the crystal 
        // oscillator is not stabilized, before going low
        if (cc1200.chip_nrdy == HIGH) {
            #ifdef CC1200_DEBUG
            CC1200_DEBUG.print("(VSPI) Insufficient time for Crystal Oscillator to stabilize!\n");
            #endif
        }
        // delayMicroseconds(100);
    } while (cc1200.chip_nrdy == HIGH);
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.print("(VSPI) Sufficient time for Crystal Oscillator to stabilize!\n");
    #endif
    return 0;
}

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
    cc1200.spi_frequency = 7700000; // // keep at 7.7 MHz per Martin B of TI E2E forums over 12 years ago 
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("(VSPI) Beginning use of VSPI Interface... "
        "[%d MHz, SS=%d,SCK=%d, MISO=%d, MOSI=%d]\n",
        cc1200.spi_frequency, cc1200.pin_ss, cc1200.pin_sck, cc1200.pin_miso, cc1200.pin_mosi
    );
    CC1200_DEBUG.printf("(VSPI ----------) Beginning use of CC1200... "
        "[GDIO0=%d, GDIO1=MISO, GDIO2=%d, GPIO3=-1]\n", 
        cc1200.pin_gdio0, cc1200.pin_gdio2
    );
    CC1200_DEBUG.printf("(---- I2C0 @0x%2.2X) Beginning use of CC1200... [RESET=%d on %s IOMUX]\n", 
        pcf_radio_config.sensor_address, cc1200.pin_nrst, pcf_radio_config.subsystem_name.c_str()
    );
    #endif
    digitalWrite(cc1200.pin_ss, HIGH); // use of SS is bitbanged?
    cc1200.spi->begin(cc1200.pin_sck, cc1200.pin_miso, cc1200.pin_mosi, cc1200.pin_ss);
    cc1200.spi->beginTransaction(
        SPISettings(cc1200.spi_frequency, MSBFIRST, CC1200_SPI_MODE)
    );
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
            CC1200_DEBUG.printf("(VSPI+I2C0 @0x%2.2X) Initialized CC1200 Interfaces!\n", pcf_radio_config.sensor_address);
    #endif
    // 9.1 Power-On Start-Up Sequence
    // Must be reset before entering the state diagram in Figure 2.
    // CHIP_RDYn on the SO pin after SS is pulled low.
    cc1200_reset(true);
    // once reset is completed, chip will be in the IDLE state.
    // To verify initialization, the read the part number to confirm CC1200.
    cc1200_spi_access_t register_access;
    register_access.readwrite_flag = READ;
    register_access.extended_address = cc1200_extended_register_space_t::PARTNUMBER;
    update_status(cc1200_single_register_access(register_access));
    cc1200.partnumber = (cc1200_partnumber_t)register_access.data;
    register_access.extended_address = cc1200_extended_register_space_t::PARTVERSION;
    update_status(cc1200_single_register_access(register_access));
    cc1200.partrevision = register_access.data;
    cc1200.initialized = true;
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("(VSPI+I2C0 @0x%2.2X) Initialized Radio Transceiver!\n"
        "* CC1200 Initialization Results: [Part Number=0x%2.2X, %s] [Part Revision=0x%2.2X, %s]\n"
        "* MAin Radio Control 'MARC' Unit is %s: [State=0x%2.2X, %s]\n", pcf_radio_config.sensor_address,
        cc1200.partnumber, cc1200.partnumber == 0x20 ? "PASS" : "FAIL", 
        cc1200.partrevision, cc1200.partrevision == 0x11 ? "PASS" : "FAIL",
        main_states[cc1200.main_state], cc1200.main_state, main_states[cc1200.main_state] == "IDLE" ? "PASS" : "FAIL"
    );
    #endif
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
    tft.setFont();
    tft.setCursor(0, 0);
    tft.printf("MARC State: %s\n"
        "Current Transceiver Configuration:\nPart Number/Revision = 0x%2.2X/0x%2.2X\n", 
        main_states[cc1200.main_state],
        cc1200.partnumber, cc1200.partrevision
    );
    cc1200_spi_access_t config_access;
    config_access.readwrite_flag = WRITE;
    // 3.4 General Purpose Input/Output Control Pins 
    // TODO: use GPIO3 as external PA control SD? Is impossible with EMK.
    // config_access.configuration_address = cc1200_configuration_register_space_t::IOCFG3;
    // config_access.data = 0x00;
    // update_status(cc1200_single_register_access(config_access));


    // ---------------------------------------------------------
    //             SWRU346B 6 Receive Configuration
    // ---------------------------------------------------------
    // 6.1 RX Channel Filter Bandwidth
    bool widefm = true; // WFM and NFM configurations available
    config_access.configuration_address = cc1200_configuration_register_space_t::CHAN_BW;
    // set CHAN_BW.ADC_CIC_DECFACT first decimation filter factor
    config_access.data |= (widefm ? FACTOR24 : FACTOR48) << 6;
    // set CHAN_BW.BB_CIC_DECFACT second decimation filter factor
    config_access.data |= (widefm ? WFM_DECFACT_OVERSHOOT : NFM_DECFACT_OVERSHOOT);
    update_status(cc1200_single_register_access(config_access));
    

    // ---------------------------------------------------------
    // SWRU346B 5.2.4 Custom Frequency Modulation(CFM)/Analog FM
    // ---------------------------------------------------------

    // The modulator writes values to the PLL at 16x the 
    // programmed symbol rate using the soft data clock. 40kHz
    config_access.extended_address = cc1200_extended_register_space_t::NOTUSED;


    config_access.configuration_address = cc1200_configuration_register_space_t::SYMBOL_RATE2;
    config_access.data = 0x00;
    update_status(cc1200_single_register_access(config_access));

    // set IOCFGx.GPIOx_CFG = 29 to use GPIO signal CLKEN_CFM as a data clock 
    // trigger to read the CFM_TX_DATA_OUT samples for demodulator soft data.
    // this GPIO signal runs at the same rate as the programmed symbol rate.
    config_access.configuration_address = cc1200_configuration_register_space_t::IOCFG2;
    config_access.data = CFM_TX_DATA_CLK;
    update_status(cc1200_single_register_access(config_access));
    
    config_access.configuration_address = cc1200_configuration_register_space_t::EXTENDED_ADDRESS;
    config_access.extended_address = cc1200_extended_register_space_t::MDMCFG2;
    config_access.data = CFM_DATA_EN; // CFM mode enabled (write frequency word directly)
    update_status(cc1200_single_register_access(config_access));
    // set EXT_CTRL.BURST_ADDR_INCR_EN = 0 to continuously access
    // the same register address without any SPI address overhead.
    config_access.extended_address = cc1200_extended_register_space_t::EXT_CTRL;
    config_access.data = ~BURST_ADDR_INCR_EN;
    update_status(cc1200_single_register_access(config_access));

    

    // The two CFM_TX_DATA registers have the same format (two’s complement)
    // to simplify software control and data buffering in both TX and RX.
    
    // CFM_TX_DATA_IN is used to set the carrier frequency offset.
    config_access.configuration_address = cc1200_configuration_register_space_t::EXTENDED_ADDRESS;
    config_access.extended_address = cc1200_extended_register_space_t::CFM_TX_DATA_IN;
    config_access.data = 0x00;
    update_status(cc1200_single_register_access(config_access));
    
    // CFM_RX_DATA_OUT is used to read the instantaneous frequency offset.
    config_access.configuration_address = cc1200_configuration_register_space_t::EXTENDED_ADDRESS;
    config_access.extended_address = cc1200_extended_register_space_t::CFM_RX_DATA_OUT;
    update_status(cc1200_single_register_access(config_access));
    uint8_t offset = config_access.data; 
    /** 129 values between -f_{dev} and +f_{dev}
     * f_{offset} = \frac{f_{dev}*CFM_TX_DATA_IN}{64}
     * consider linear upsampler UPSAMPLER_P
     */

    // cc1200_command_strobe_access(STX);
    // Note that in TX mode, 3 dummy symbols should be written to the 
    // CFM_TX_DATA_IN register before strobing SIDLE 
    // in order for all symbols to be sent on the air before TX mode is ended.
    config_access.configuration_address = cc1200_configuration_register_space_t::EXTENDED_ADDRESS;
    config_access.extended_address = cc1200_extended_register_space_t::CFM_TX_DATA_IN;
    config_access.data = 0x00;
    update_status(cc1200_single_register_access(config_access));
    update_status(cc1200_single_register_access(config_access));
    update_status(cc1200_single_register_access(config_access));
    cc1200_command_strobe_access(SIDLE);

    // display radio settings
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
}