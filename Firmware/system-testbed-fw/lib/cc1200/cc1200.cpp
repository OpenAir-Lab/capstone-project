#include <cc1200.h>
#include <Arduino.h>
#include <pcf8575.h>
#include <st7789.h>
#include <algorithm>
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
    "RX","RX_END",
    // transmit states
    "TX","TX_END",
    // fast TX ready
    "FSTXON",
    // frequency synthesizer calibration is running
    "CALIBRATE","BIAS_SETTLE_MC","MANCAL","STARTCAL","ENDCAL",
    // PLL is setting
    "BIAS_SETTLE","REG_SETTLE","BWBOOST","FS_LOCK","IFADCON",
    "RXTX_SWITCH","TXRX_SWITCH","IFADCON_TXRX"
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
int cc1200_single_register_access(cc1200_configuration_register_space_t configuration_address, cc1200_spi_access_t &register_access) {
    digitalWrite(cc1200.pin_ss, LOW);
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.print("(VSPI) Wait for MISO to go low...\n");
    #endif
    while (digitalRead(cc1200.pin_miso)); // Wait for MISO to go low
    bool isCommandStrobe = (
           (uint8_t)configuration_address >= 0x30 
        && (uint8_t)configuration_address <= 0x3D
    );
    uint8_t address = (register_access.readwrite_flag ? COMMAND_RW_FLAG : !COMMAND_RW_FLAG);
    address |= configuration_address;
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

int cc1200_single_register_access(cc1200_extended_register_space_t extended_address, cc1200_spi_access_t &register_access) {
    digitalWrite(cc1200.pin_ss, LOW);
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.print("(VSPI) Wait for MISO to go low...\n");
    #endif
    while (digitalRead(cc1200.pin_miso)); // Wait for MISO to go low
    uint8_t address = (register_access.readwrite_flag ? COMMAND_RW_FLAG : !COMMAND_RW_FLAG);
    // extended access requires two byte transfers
    // first transfer as command (0x2F)
    address |= cc1200_configuration_register_space_t::EXTENDED_ADDRESS;
    update_status(cc1200.spi->transfer(address));
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("(VSPI) Transferred command 0x%2.2X [%s]\n", 
        address, main_states[cc1200.main_state]
    );
    #endif
    // second transfer as extended address
    address = extended_address;
    // one byte transfer as configuration address
    update_status(cc1200.spi->transfer(address));
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.printf("(VSPI) Transferred address 0x%2.2X [%s]\n",
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
int cc1200_burst_register_access(cc1200_configuration_register_space_t configuration_address, cc1200_spi_access_t &register_access) {
    register_access.burst_flag = BURST;
    cc1200_single_register_access(configuration_address, register_access);
    return 0;
}
int cc1200_burst_register_access(cc1200_extended_register_space_t extended_address, cc1200_spi_access_t &register_access) {
    register_access.burst_flag = BURST;
    cc1200_single_register_access(extended_address, register_access);
    return 0;
}

int cc1200_command_strobe_access(cc1200_command_strobe_t command_strobe) {
    // header read write flag ignored, burst access not possible.
    cc1200_spi_access_t strobe_access;
    // the chip status byte is returned on the MISO line 
    // when a command strobe is sent on the MOSI line.
    update_status(cc1200_single_register_access((cc1200_configuration_register_space_t)command_strobe, strobe_access));
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


// set up slave select pins as outputs as the Arduino API
    // doesn't handle automatically pulling SS low
    // https://docs.espressif.com/projects/arduino-esp32/en/latest/api/spi.html
    // pinMode(cc1200.spi->pinSS(), OUTPUT);  // VSPI SS
    // Initialize SPI
    // https://e2e.ti.com/support/wireless-connectivity/other-wireless-group/other-wireless/f/other-wireless-technologies-forum/307966/cc1200-spi-clock-query   
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
    // -------------------------------------------------------------
    // SWRU346B 3.1.1 4-wire Serial Configuration and Data Interface
    // -------------------------------------------------------------
    setup_interface();
    #ifdef CC1200_DEBUG
            CC1200_DEBUG.printf("(VSPI+I2C0 @0x%2.2X) Initialized CC1200 Interfaces!\n", pcf_radio_config.sensor_address);
    #endif
    // ---------------------------------------------------------
    //        SWRU346B 9.1 Power-On Start-Up Sequence 
    // ---------------------------------------------------------
    // Must be reset before entering the state diagram in Figure 2.
    // CHIP_RDYn on the SO pin after SS is pulled low.
    cc1200_reset(true);
    // once reset is completed, chip will be in the IDLE state.
    // To verify initialization, the read the part number to confirm CC1200.
    cc1200_spi_access_t config_access;
    config_access.readwrite_flag = READ;
    update_status(cc1200_single_register_access(PARTNUMBER, config_access));
    cc1200.partnumber = (cc1200_partnumber_t)config_access.data;
    update_status(cc1200_single_register_access(PARTVERSION, config_access));
    cc1200.partrevision = config_access.data;    
    config_access.readwrite_flag = WRITE;
    // ---------------------------------------------------------
    //  SWRU346B 3.4 General Purpose Input/Output Control Pins 
    // ---------------------------------------------------------
    // To control an external LNA, PA, or RX/TX switch in applications where the 
    // SLEEP state is used it is therefore recommended to map this signal to
    // GDO3 as this signal will be hardwired to 1(0) in the SLEEP state.
    cc1200.registers.IOCFG3 |= ((PA_PD << GPIOx_CFG_SHIFT) & GPIOx_CFG); // GPIO3 asserts PA_PD GPIO signal 
    config_access.data = cc1200.registers.IOCFG3;
    update_status(cc1200_single_register_access(IOCFG3, config_access));
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("(VSPI+I2C0 @0x%2.2X) Initialized Radio Transceiver!\n"
        "* CC1200 Initialization Results: [Part Number=0x%2.2X, %s] [Part Revision=0x%2.2X, %s]\n"
        "* MAin Radio Control 'MARC' Unit is %s: [State=0x%2.2X, %s]\n", pcf_radio_config.sensor_address,
        cc1200.partnumber, cc1200.partnumber == 0x20 ? "PASS" : "FAIL", 
        cc1200.partrevision, cc1200.partrevision == 0x11 ? "PASS" : "FAIL",
        main_states[cc1200.main_state], cc1200.main_state, main_states[cc1200.main_state] == "IDLE" ? "PASS" : "FAIL"
    );
    #endif
    if (cc1200.partnumber == 0x20 && cc1200.partrevision == 0x11) {
        cc1200.initialized = true;
    }
    return 0;
}

uint8_t frequency_deviation_exponent; // 03 bit Exponent
uint8_t frequency_deviation_mantissa; // 08 bit Mantissa

/** Calculate Symbol Rate Programming
 * \brief implements SWRU346B 5.4 Frequency Deviation Programming.
 * NOTE: SWRU346B shows units in Hz. Maximum DEV = ~1.2 MHz
 * 
 * CC1200_OSC_FREQ from given f_{xosc} = 40 MHz
 * CC1200_OSC_FREQ_LOG2 from log2(40*10^6) = 25.253496f
 */
void cc1200_calculate_frequency_deviation(double targetDeviation) {
    
    // SWRU346B Deviation Equation (DEV_E for given frequency deviation) 
    double oscillatorRatio = targetDeviation / CC1200_OSC_FREQ;
    frequency_deviation_exponent = (uint8_t)std::clamp(
        (int)std::floor((int)std::log2(oscillatorRatio*TWO_TO_THE_14)),
        0, (int)MAX_03_BIT_VALUE
    );
    if (frequency_deviation_exponent > 0) { // use DEV_E > 0 Equation
        frequency_deviation_mantissa = (uint8_t)std::clamp(
            std::round((oscillatorRatio*std::pow(2.0f, 22 - frequency_deviation_exponent)) - TWO_TO_THE_08), 
            0.0, (double)MAX_08_BIT_VALUE
        );
        cc1200.frequency_deviation = (CC1200_OSC_FREQ/TWO_TO_THE_22) 
            * (TWO_TO_THE_08 + frequency_deviation_mantissa)
            * std::pow(2.0f, (double)(frequency_deviation_exponent)); // [Hz]
    } else { // use DEV_E = 0 Equation
        frequency_deviation_mantissa = oscillatorRatio*TWO_TO_THE_21;
        cc1200.frequency_deviation = (CC1200_OSC_FREQ/TWO_TO_THE_21) 
            * (TWO_TO_THE_08 + frequency_deviation_mantissa); // [Hz]
    }
    cc1200.registers.DEVIATION_M = frequency_deviation_mantissa;
    cc1200.registers.MODCFG_DEV_E &= ~DEV_E;
    cc1200.registers.MODCFG_DEV_E |= (frequency_deviation_exponent & DEV_E);
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("Target DEV=%2.2f Hz\n"
        "03-bit DEV_E=0b%3.3B=0x%1.1X\n"
        "08-bit DEV_M=0b%3.3B=0x%2.2X\n"
        "Calculated DEV=%2.2f Hz\n", 
        targetDeviation, 
        frequency_deviation_exponent,
        frequency_deviation_mantissa,
        cc1200.symbol_rate
    );
    #endif
}

uint8_t symbol_rate_exponent;  // 04-bit Exponent
uint32_t symbol_rate_mantissa; // 20-bit Mantissa

/** Calculate Symbol Rate Programming
 * \brief implements SWRU346B 5.4 Symbol Rate Programming.
 * NOTE: SWRU346B shows units in ksps, sps actually configurable. 
 * 
 * CC1200_OSC_FREQ from given f_{xosc} = 40 MHz
 * CC1200_OSC_FREQ_LOG2 from log2(40*10^6) = 25.253496f
 */
void cc1200_calculate_symbol_rate(double targetSampleRate) {
    // SWRU346B Equation 8 (SRATE_E for given symbol rate)
    double oscillatorRatio = targetSampleRate / CC1200_OSC_FREQ;
    // 4-bits wide exponent, select clamped SRATE_E value.
    symbol_rate_exponent = (uint8_t)std::clamp((int)std::floor((int)std::log2(oscillatorRatio*TWO_TO_THE_19)), 0, (int)MAX_04_BIT_VALUE);
    // SWRU346B Equation 9 (SRATE_M for given symbol rate)
    if (symbol_rate_exponent >= 1) {
        // 20-bits wide mantissa. select clamped SRATE_M value.
        symbol_rate_mantissa = std::round((oscillatorRatio*std::pow(2.0f, 39 - symbol_rate_exponent)) - TWO_TO_THE_20); 
        // If SYMBOL_RATE_M is rounded to the nearest integer and becomes 2^20, one 
        // should increment SYMBOL_RATE_E and use SYMBOL_RATE_M = 0 instead.
        if (symbol_rate_mantissa == TWO_TO_THE_20) {
            if (symbol_rate_exponent < MAX_04_BIT_VALUE) {
                symbol_rate_exponent++;
                symbol_rate_mantissa = 0;
            } else {
                symbol_rate_mantissa = MAX_20_BIT_VALUE - 1;
            }
        }
        symbol_rate_mantissa = (uint32_t)std::clamp((double)symbol_rate_mantissa, 0.0, (double)MAX_20_BIT_VALUE);
        // SWRU346B Equation 6 (Symbol Rate when SRATE_E > 0)
        cc1200.symbol_rate = ((double)(symbol_rate_mantissa) + TWO_TO_THE_20) *
                pow(2.0f,(double)(symbol_rate_exponent))*CC1200_OSC_FREQ
                / TWO_TO_THE_39; // sps samples per second
    } else {
        // SWRU346B Equation 9 (SRATE_M for given symbol rate when SRATE_E = 0)
        symbol_rate_mantissa = oscillatorRatio*TWO_TO_THE_38;
        // SWRU346B Equation 7 (Symbol Rate when SRATE_E = 0)
        cc1200.symbol_rate = symbol_rate_mantissa*CC1200_OSC_FREQ/TWO_TO_THE_38; // sps samples per second
    }
    cc1200.registers.SYMBOL_RATE2 &= ~(SRATE_E | SRATE_M_19_16); // clear register fields
    cc1200.registers.SYMBOL_RATE2 |= ((symbol_rate_exponent << SRATE_E_SHIFT) & SRATE_E);
    cc1200.registers.SYMBOL_RATE2 |= (((symbol_rate_mantissa >> 16) << SRATE_M_19_16_SHIFT) & SRATE_M_19_16);
    cc1200.registers.SYMBOL_RATE1 = symbol_rate_mantissa >> 8; // [15:8]
    cc1200.registers.SYMBOL_RATE0 = symbol_rate_mantissa >> 0; // [7:0]
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("Target SRATE=%2.2f sps\n"
        "04-bit SRATE_E=0x%1.1X\n"
        "20-bit SRATE_M=0x%5.5X\n"
        "Calculated SRATE=%2.2f sps\n", 
        targetSampleRate, 
        symbol_rate_exponent,
        symbol_rate_mantissa,
        cc1200.symbol_rate
    );
    #endif
}

// ---------------------------------------------------------
// SWRU346B 5.2.4 Custom Frequency Modulation(CFM)/Analog FM
// ---------------------------------------------------------

void cc1200_enter_custom_frequency_modulation(double targetSampleRate, double targetDeviation) {
    cc1200_spi_access_t config_access;
    config_access.readwrite_flag = WRITE;

    // ---------------------------------------------------------
    // SWRU346B Frequency Deviation Configuration
    // ---------------------------------------------------------
    cc1200_calculate_frequency_deviation(targetDeviation);
    // Frequency Deviation Configuration
    config_access.data = cc1200.registers.DEVIATION_M;
    update_status(cc1200_single_register_access(DEVIATION_M, config_access));
    // Modulation Format and Frequency Deviation Configuration
    config_access.data = cc1200.registers.MODCFG_DEV_E;
    update_status(cc1200_single_register_access(MODCFG_DEV_E, config_access));
    // ---------------------------------------------------------
    //             SWRU346B 5.4 Symbol Rate Programming
    // ---------------------------------------------------------
    // The modulator writes values to the PLL at 16x the 
    // programmed symbol rate using the soft data clock.
    cc1200_calculate_symbol_rate(targetSampleRate);
    config_access.data = cc1200.registers.SYMBOL_RATE2;
    update_status(cc1200_single_register_access(SYMBOL_RATE2, config_access));
    config_access.data = cc1200.registers.SYMBOL_RATE1;
    update_status(cc1200_single_register_access(SYMBOL_RATE1, config_access));
    config_access.data = cc1200.registers.SYMBOL_RATE0;
    update_status(cc1200_single_register_access(SYMBOL_RATE0, config_access));
    // ---------------------------------------------------------
    //             SWRU346B 6 Receive Configuration
    // ---------------------------------------------------------
    // 6.1 RX Channel Filter Bandwidth
    bool widefm = true; // WFM and NFM configurations available
    // set CHAN_BW.ADC_CIC_DECFACT first decimation filter factor
    cc1200.registers.CHAN_BW &= ~(ADC_CIC_DECFACT | BB_CIC_DECFACT); // clear register fields
    cc1200.registers.CHAN_BW |= ((widefm ? FACTOR24 : FACTOR48) << ADC_CIC_DECFACT_SHIFT & ADC_CIC_DECFACT);
    // set CHAN_BW.BB_CIC_DECFACT second decimation filter factor
    cc1200.registers.CHAN_BW |= ((widefm ? WFM_DECFACT_OVERSHOOT : NFM_DECFACT_OVERSHOOT) & BB_CIC_DECFACT);
    config_access.data = cc1200.registers.CHAN_BW;
    update_status(cc1200_single_register_access(CHAN_BW, config_access));

    cc1200.registers.MDMCFG2 |= ((0b1 << CFM_DATA_EN_SHIFT) & CFM_DATA_EN); // CFM mode enabled (write frequency word directly)
    cc1200.registers.MDMCFG2 |= ((0x04 << UPSAMPLER_P_SHIFT) & UPSAMPLER_P); // variable TX upsampling factor default is P=16 0x04
    config_access.data = cc1200.registers.MDMCFG2; 
    update_status(cc1200_single_register_access(MDMCFG2, config_access));
    // disable Normal/FIFO Mode packet format configuration
    cc1200.registers.MDMCFG1 |= ((0b0 << FIFO_EN_SHIFT) & FIFO_EN); 
    config_access.data = cc1200.registers.MDMCFG1; 
    update_status(cc1200_single_register_access(MDMCFG1, config_access));
    // disable Transparent Mode packet format configuration
    cc1200.registers.MDMCFG0 |= ((0b0 << TRANSPARENT_MODE_EN_SHIFT) & TRANSPARENT_MODE_EN); 
    config_access.data = cc1200.registers.MDMCFG0; 
    update_status(cc1200_single_register_access(MDMCFG0, config_access));
    // select Synchronous serial mode packet format configuration
    cc1200.registers.PKT_CFG2 |= ((0b1 << CCA_MODE_SHIFT) & CCA_MODE); // indicate clear channel when RSSI is below threshold
    cc1200.registers.PKT_CFG2 |= ((0b01 << PKT_FORMAT_SHIFT) & PKT_FORMAT); // select Synchronous serial mode
    config_access.data = cc1200.registers.PKT_CFG2;
    update_status(cc1200_single_register_access(PKT_CFG2, config_access));

    // 
    /** SWRU346B Equation 3 f_{offset}
     * f_{offset} = f_{dev}*CFM_TX_DATA_IN/64 [Hz]
     * This equation is only valid when -64 ≤ CFM_TX_DATA_IN ≤ +64. 
     * CFM_TX_DATA_IN > 64 corresponds to +fdev while 
     * CFM_TX_DATA_IN < -64 gives a frequency of −fdev. 
     * CFM_TX_DATA_IN = -128 is the same as setting CFM_TX_DATA_IN = 0.
     * See Section 5.2.1 regarding frequency deviation.
     */
    // set EXT_CTRL.BURST_ADDR_INCR_EN = 0 to continuously access
    // the same register address without any SPI address overhead.
    cc1200.registers.EXT_CTRL |= ((0b0 << BURST_ADDR_INCR_EN_SHIFT) & BURST_ADDR_INCR_EN);
    config_access.data = cc1200.registers.EXT_CTRL;
    update_status(cc1200_single_register_access(EXT_CTRL, config_access));

    

    
    // ---------------------------------------------------------
    //  SWRU346B 3.4 General Purpose Input/Output Control Pins 
    // ---------------------------------------------------------
    // set IOCFGx.GPIOx_CFG=29=0x1D to use GPIO signal CLKEN_CFM data clock 
    // trigger to read the CFM_TX_DATA_OUT samples for demodulator soft data.
    // this GPIO signal runs at the same rate as the programmed symbol rate.
    cc1200.registers.IOCFG2 |= ((CLKEN_CFM << GPIOx_CFG_SHIFT) & GPIOx_CFG); // GPIO2 asserts CLKEN_CFM GPIO signal 
    config_access.data = cc1200.registers.IOCFG2;
    update_status(cc1200_single_register_access(IOCFG2, config_access));
    // set IOCFGx.GPIOx_CFG=30=0x1E to use GPIO signal CFM_TX_DATA_CLK data clock
    // interrupt to the MCU to synchronize the SPI data to the internal modulation rate.
    // this GPIO signal runs at 16x the programmed symbol rate.
    cc1200.registers.IOCFG0 |= ((CFM_TX_DATA_CLK << GPIOx_CFG_SHIFT) & GPIOx_CFG); // GPIO0 asserts CFM_TX_DATA_CLK GPIO signal 
    config_access.data = cc1200.registers.IOCFG0;
    update_status(cc1200_single_register_access(IOCFG0, config_access));
    // -- end of Custom FM configuration changes --
}

void IRAM_ATTR CLKEN_CFM_ISR() {

}

void IRAM_ATTR CFM_TX_DATA_CLK_ISR() {

}

void cc1200_exit_custom_frequency_modulation() {
    // Note that in TX mode, 3 dummy symbols should be written to the 
    // CFM_TX_DATA_IN register before strobing SIDLE 
    // in order for all symbols to be sent on the air before TX mode is ended.
    cc1200_spi_access_t config_access;
    config_access.readwrite_flag = WRITE;
    config_access.data = 0x00;
    update_status(cc1200_single_register_access(CFM_TX_DATA_IN, config_access));
    update_status(cc1200_single_register_access(CFM_TX_DATA_IN, config_access));
    update_status(cc1200_single_register_access(CFM_TX_DATA_IN, config_access));
    cc1200_command_strobe_access(SIDLE);
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
    

    uint8_t* cfm_data_buffer;
    // curiouselectron demo targets 40kHz sample rate.
    cc1200_enter_custom_frequency_modulation(40000.0, 5000.0);

    attachInterrupt(digitalPinToInterrupt(cc1200.pin_gdio0), CFM_TX_DATA_CLK_ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(cc1200.pin_gdio2), CLKEN_CFM_ISR, RISING);



    cc1200_command_strobe_access(SRX);
    

    



    cc1200_command_strobe_access(STX);
    

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