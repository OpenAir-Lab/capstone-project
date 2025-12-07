#include <cc1200.h>
#include <Arduino.h>
#include <pcf8575.h>
#include <st7789.h>
#include <algorithm>
#include <bitset>
#include <grf5604.h>
#include <sky13330.h>

/*! \file cc1200.cpp
    \brief Device Driver for CC1200 using Espressif Arduino Core.
    
    Implements Texas Instruments CC1200 Radio Transceiver Interface.
*/

#define CC1200_DEBUG Serial

extern Adafruit_ST7789 tft;
extern st7789_config_t display_config; 

extern Adafruit_PCF8575 pcf_radio;
extern pcf8575_config_t pcf_radio_config;

extern cc1200_config_t cc1200;
extern grf5604_config_t uhf_grf5604;
extern grf5604_config_t vhf_grf5604;
extern sky13330_config_t sky13330;

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
    "RX or RX_END"
    // transmit states
    "TX or TX_END"
    // fast TX ready
    "FSTXON",
    // frequency synthesizer calibration is running
    "CALIBRATE, BIAS_SETTLE_MC, MANCAL, STARTCAL, or ENDCAL",
    // PLL is setting
    "BIAS_SETTLE, REG_SETTLE, BWBOOST, FS_LOCK or IFADCON",
    "RXTX_SWITCH, TXRX_SWITCH, or IFADCON_TXRX"
    // RX FIFO has over/underflowed. Read out any useful data,
    // then flush the FIFO with an SFRX strobe
    "RX_FIFO_ERR",
    // TX FIFO has over/underflowed. 
    // Flush the FIFO with an SFTX strobe
    "TX_FIFO_ERR"
};

int update_status(uint8_t chip_status) {
    cc1200.chip_nrdy =  (uint8_t)((chip_status >> 7) & BIT7); // ready when low
    cc1200.main_state = (uint8_t)((chip_status >> 4) & GENMASK(6,4));
    return cc1200.main_state; 
}

const char* command_strobes[14] = {
    "SRES", "SFSTXON", "SXOFF", "SCAL",
    "SRX", "STX", "SIDLE", "SAFC", 
    "SWOR", "SPWD", "SFRX",
    "SFTX", "SWORRST", "SNOP"
};

// Table 3: SPI Access Types
uint8_t cc1200_register_access(bool readwrite_flag, bool burst_flag, cc1200_configuration_register_space_t configuration_address, uint8_t data) {
    digitalWrite(cc1200.pin_ss, LOW);
    while (digitalRead(cc1200.pin_miso)); // Wait for MISO to go low
    bool isCommandStrobe = (
           (uint8_t)configuration_address >= 0x30 
        && (uint8_t)configuration_address <= 0x3D
    );
    uint8_t address = (readwrite_flag ? COMMAND_RW_FLAG : !COMMAND_RW_FLAG);
    address |= configuration_address;
    update_status(cc1200.spi->transfer(address));
    if (readwrite_flag == READ) {
        data = cc1200.spi->transfer(0x00);
    } else {
        update_status(cc1200.spi->transfer(data));
    }
    switch (configuration_address) {
        case(IOCFG3): cc1200.registers.IOCFG3 = data; break;
        case(IOCFG2): cc1200.registers.IOCFG2 = data; break;
        case(IOCFG1): cc1200.registers.IOCFG1 = data; break;
        case(IOCFG0): cc1200.registers.IOCFG0 = data; break;
        case(SYNC3): cc1200.registers.SYNC3 = data; break;
        case(SYNC2): cc1200.registers.SYNC2 = data; break;
        case(SYNC1): cc1200.registers.SYNC1 = data; break;
        case(SYNC0): cc1200.registers.SYNC0 = data; break;
        case(SYNC_CFG1): cc1200.registers.SYNC_CFG1 = data; break; 
        case(SYNC_CFG0): cc1200.registers.SYNC_CFG0 = data; break;
        case(DEVIATION_M): cc1200.registers.DEVIATION_M = data; break;
        case(MODCFG_DEV_E): cc1200.registers.MODCFG_DEV_E = data; break;
        case(DCFILT_CFG): cc1200.registers.DCFILT_CFG = data; break;
        case(PREAMBLE_CFG1): cc1200.registers.PREAMBLE_CFG1 = data; break;
        case(PREAMBLE_CFG0): cc1200.registers.PREAMBLE_CFG0 = data; break;
        case(IQIC): cc1200.registers.IQIC = data; break;
        case(CHAN_BW): cc1200.registers.CHAN_BW = data; break;
        case(MDMCFG1): cc1200.registers.MDMCFG1 = data; break; 
        case(MDMCFG0): cc1200.registers.MDMCFG0 = data; break;
        case(SYMBOL_RATE2): cc1200.registers.SYMBOL_RATE2 = data; break;
        case(SYMBOL_RATE1): cc1200.registers.SYMBOL_RATE1 = data; break;
        case(SYMBOL_RATE0): cc1200.registers.SYMBOL_RATE0 = data; break;
        case(AGC_REF): cc1200.registers.AGC_REF = data; break;
        case(AGC_CS_THR): cc1200.registers.AGC_CS_THR = data; break;
        case(AGC_GAIN_ADJUST): cc1200.registers.AGC_GAIN_ADJUST = data; break;
        case(AGC_CFG3): cc1200.registers.AGC_CFG3 = data; break;
        case(AGC_CFG2): cc1200.registers.AGC_CFG2 = data; break;
        case(AGC_CFG1): cc1200.registers.AGC_CFG1 = data; break;
        case(AGC_CFG0): cc1200.registers.AGC_CFG0 = data; break;
        case(FIFO_CFG): cc1200.registers.FIFO_CFG = data; break;
        case(DEV_ADDR): cc1200.registers.DEV_ADDR = data; break;
        case(SETTLING_CFG): cc1200.registers.SETTLING_CFG = data; break;
        case(FS_CFG): cc1200.registers.FS_CFG = data; break;
        case(WOR_CFG1): cc1200.registers.WOR_CFG1 = data; break;
        case(WOR_CFG0): cc1200.registers.WOR_CFG0 = data; break;
        case(WOR_EVENT0_MSB): cc1200.registers.WOR_EVENT0_MSB = data; break;
        case(WOR_EVENT0_LSB): cc1200.registers.WOR_EVENT0_LSB = data; break;
        case(RXDCM_TIME): cc1200.registers.RXDCM_TIME = data; break;
        case(PKT_CFG2): cc1200.registers.PKT_CFG2 = data; break; 
        case(PKT_CFG1): cc1200.registers.PKT_CFG1 = data; break;
        case(PKT_CFG0): cc1200.registers.PKT_CFG0 = data; break;
        case(RFEND_CFG1): cc1200.registers.RFEND_CFG1 = data; break;
        case(RFEND_CFG0): cc1200.registers.RFEND_CFG0 = data; break;
        case(PA_CFG1): cc1200.registers.PA_CFG1 = data; break;
        case(PA_CFG0): cc1200.registers.PA_CFG0 = data; break;
        case(ASK_CFG): cc1200.registers.ASK_CFG = data; break;
        case(PKT_LEN): cc1200.registers.PKT_LEN = data; break;
    }

    #ifdef CC1200_DEBUG
    isCommandStrobe ? 
        CC1200_DEBUG.printf("(VSPI) %s data 0x%2.2X [%s]"
        " after accessing strobe %s\n",
        readwrite_flag ? "Read in" : "Wrote in", 
        data, main_states[cc1200.main_state], command_strobes[configuration_address-0x30]) :
        CC1200_DEBUG.printf("(VSPI) %s data 0x%2.2X [%s]"
        " after accessing address 0x%2.2X\n", 
        readwrite_flag ? "Read in" : "Wrote in", 
        data, main_states[cc1200.main_state], address);
    #endif
    digitalWrite(cc1200.pin_ss, HIGH);
    return data;
}
uint8_t cc1200_register_access(bool readwrite_flag, bool burst_flag, cc1200_extended_register_space_t extended_address, uint8_t data) {
    digitalWrite(cc1200.pin_ss, LOW);
    while (digitalRead(cc1200.pin_miso)); // Wait for MISO to go low
    uint8_t address = (readwrite_flag ? COMMAND_RW_FLAG : !COMMAND_RW_FLAG);
    // extended access requires two byte transfers
    // first transfer as command (0x2F)
    address |= cc1200_configuration_register_space_t::EXTENDED_ADDRESS;
    update_status(cc1200.spi->transfer(address));
    uint8_t command = address;
    // second transfer as extended address
    address = extended_address;
    // one byte transfer as configuration address
    update_status(cc1200.spi->transfer(address));
    if (readwrite_flag == READ) {
        data = cc1200.spi->transfer(0x00);
    } else {
        update_status(cc1200.spi->transfer(data));
    }
    // TODO: minimize boilerplate for access enum->struct associations
    switch(extended_address) {
        case (IF_MIX_CFG): cc1200.registers.IF_MIX_CFG = data; break;
        case (FREQOFF_CFG): cc1200.registers.FREQOFF_CFG = data; break;
        case (TOC_CFG): cc1200.registers.TOC_CFG = data; break;
        case (MARC_SPARE): cc1200.registers.MARC_SPARE = data; break;
        case (ECG_CFG): cc1200.registers.ECG_CFG = data; break;
        case (MDMCFG2): cc1200.registers.MDMCFG2 = data; break; 
        case (EXT_CTRL): cc1200.registers.EXT_CTRL = data; break;
        case (RCCAL_FINE): cc1200.registers.RCCAL_FINE = data; break;
        case (RCCAL_COARSE): cc1200.registers.RCCAL_COARSE = data; break;
        case (RCCAL_OFFSET): cc1200.registers.RCCAL_OFFSET = data; break;
        case (FREQOFF1): cc1200.registers.FREQOFF1 = data; break;
        case (FREQOFF0): cc1200.registers.FREQOFF0 = data; break;
        case (FREQ2): cc1200.registers.FREQ2 = data; break;
        case (FREQ1): cc1200.registers.FREQ1 = data; break;
        case (FREQ0): cc1200.registers.FREQ0 = data; break;
        case (IF_ADC2): cc1200.registers.IF_ADC2 = data; break;
        case (IF_ADC1): cc1200.registers.IF_ADC1 = data; break;
        case (IF_ADC0): cc1200.registers.IF_ADC0 = data; break;
        case (FS_DIG1): cc1200.registers.FS_DIG1 = data; break;
        case (FS_DIG0): cc1200.registers.FS_DIG0 = data; break;
        case (FS_CAL3): cc1200.registers.FS_CAL3 = data; break;
        case (FS_CAL2): cc1200.registers.FS_CAL2 = data; break;
        case (FS_CAL1): cc1200.registers.FS_CAL1 = data; break;
        case (FS_CAL0): cc1200.registers.FS_CAL0 = data; break;
        case (FS_CHP): cc1200.registers.FS_CHP = data; break;
        case (FS_DIVTWO): cc1200.registers.FS_DIVTWO = data; break;
        case (FS_DSM1): cc1200.registers.FS_DSM1 = data; break;
        case (FS_DSM0): cc1200.registers.FS_DSM0 = data; break;
        case (FS_DVC1): cc1200.registers.FS_DVC1 = data; break;
        case (FS_DVC0): cc1200.registers.FS_DVC0 = data; break;
        case (FS_LBI): cc1200.registers.FS_LBI = data; break;
        case (FS_PFD): cc1200.registers.FS_PFD = data; break;
        case (FS_PRE): cc1200.registers.FS_PRE = data; break;
        case (FS_REG_DIV_CML): cc1200.registers.FS_REG_DIV_CML = data; break;
        case (FS_SPARE): cc1200.registers.FS_SPARE = data; break;
        case (FS_VCO4): cc1200.registers.FS_VCO4 = data; break;
        case (FS_VCO3): cc1200.registers.FS_VCO3 = data; break;
        case (FS_VCO2): cc1200.registers.FS_VCO2 = data; break;
        case (FS_VCO1): cc1200.registers.FS_VCO1 = data; break;
        case (FS_VCO0): cc1200.registers.FS_VCO0 = data; break;
        case (GBIAS6): cc1200.registers.GBIAS6 = data; break;
        case (GBIAS5): cc1200.registers.GBIAS5 = data; break;
        case (GBIAS4): cc1200.registers.GBIAS4 = data; break;
        case (GBIAS3): cc1200.registers.GBIAS3 = data; break;
        case (GBIAS2): cc1200.registers.GBIAS2 = data; break;
        case (GBIAS1): cc1200.registers.GBIAS1 = data; break;
        case (GBIAS0): cc1200.registers.GBIAS0 = data; break;
        case (IFAMP): cc1200.registers.IFAMP = data; break;
        case (LNA): cc1200.registers.LNA = data; break;
        case (RXMIX): cc1200.registers.RXMIX = data; break;
        case (XOSC5): cc1200.registers.XOSC5 = data; break;
        case (XOSC4): cc1200.registers.XOSC4 = data; break;
        case (XOSC3): cc1200.registers.XOSC3 = data; break;
        case (XOSC2): cc1200.registers.XOSC2 = data; break;
        case (XOSC1): cc1200.registers.XOSC1 = data; break;
        case (XOSC0): cc1200.registers.XOSC0 = data; break;
        case (ANALOG_SPARE): cc1200.registers.ANALOG_SPARE = data; break;
        case (PA_CFG3): cc1200.registers.PA_CFG3 = data; break;
        case (WOR_TIME1): cc1200.registers.WOR_TIME1 = data; break;
        case (WOR_TIME0): cc1200.registers.WOR_TIME0 = data; break;
        case (WOR_CAPTURE1): cc1200.registers.WOR_CAPTURE1 = data; break;
        case (WOR_CAPTURE0): cc1200.registers.WOR_CAPTURE0 = data; break;
        case (BIST): cc1200.registers.BIST = data; break;
        case (DCFILTOFFSET_I1): cc1200.registers.DCFILTOFFSET_I1 = data; break;
        case (DCFILTOFFSET_I0): cc1200.registers.DCFILTOFFSET_I0 = data; break;
        case (DCFILTOFFSET_Q1): cc1200.registers.DCFILTOFFSET_Q1 = data; break;
        case (DCFILTOFFSET_Q0): cc1200.registers.DCFILTOFFSET_Q0 = data; break; 
        case (IQIE_I1): cc1200.registers.IQIE_I1 = data; break;
        case (IQIE_I0): cc1200.registers.IQIE_I0 = data; break;
        case (IQIE_Q1): cc1200.registers.IQIE_Q1 = data; break;
        case (IQIE_Q0): cc1200.registers.IQIE_Q0 = data; break; 
        case (RSSI1): cc1200.registers.RSSI1 = data; break;
        case (RSSI0): cc1200.registers.RSSI0 = data; break;
        case (MARCSTATE): cc1200.registers.MARCSTATE = data; break;
        case (LQI_VAL): cc1200.registers.LQI_VAL = data; break;
        case (PQT_SYNC_ERR): cc1200.registers.PQT_SYNC_ERR = data; break;
        case (DEM_STATUS): cc1200.registers.DEM_STATUS = data; break;
        case (FREQOFF_EST1): cc1200.registers.FREQOFF_EST1 = data; break;
        case (FREQOFF_EST0): cc1200.registers.FREQOFF_EST0 = data; break; 
        case (AGC_GAIN3): cc1200.registers.AGC_GAIN3 = data; break;
        case (AGC_GAIN2): cc1200.registers.AGC_GAIN2 = data; break;
        case (AGC_GAIN1): cc1200.registers.AGC_GAIN1 = data; break;
        case (AGC_GAIN0): cc1200.registers.AGC_GAIN0 = data; break;
        case (CFM_RX_DATA_OUT): cc1200.registers.CFM_RX_DATA_OUT = data; break;
        case (CFM_TX_DATA_IN): cc1200.registers.CFM_TX_DATA_IN = data; break;
        case (ASK_SOFT_RX_DATA): cc1200.registers.ASK_SOFT_RX_DATA = data; break;
        case (RNDGEN): cc1200.registers.RNDGEN = data; break;
        case (MAGN2): cc1200.registers.MAGN2 = data; break;
        case (MAGN1): cc1200.registers.MAGN1 = data; break;
        case (MAGN0): cc1200.registers.MAGN0 = data; break;
        case (ANG1): cc1200.registers.ANG1 = data; break;
        case (ANG0): cc1200.registers.ANG0 = data; break;
        case (CHFILT_I2): cc1200.registers.CHFILT_I2 = data; break;
        case (CHFILT_I1): cc1200.registers.CHFILT_I1 = data; break;
        case (CHFILT_I0): cc1200.registers.CHFILT_I0 = data; break;
        case (CHFILT_Q2): cc1200.registers.CHFILT_Q2 = data; break;
        case (CHFILT_Q1): cc1200.registers.CHFILT_Q1 = data; break;
        case (CHFILT_Q0): cc1200.registers.CHFILT_Q0 = data; break;
        case (GPIO_STATUS): cc1200.registers.GPIO_STATUS = data; break;
        case (FSCAL_CTRL): cc1200.registers.FSCAL_CTRL = data; break;
        case (PHASE_ADJUST): cc1200.registers.PHASE_ADJUST = data; break;
        case (PARTNUMBER): cc1200.registers.PARTNUMBER = data; break;
        case (PARTVERSION): cc1200.registers.PARTVERSION = data; break;
        case (SERIAL_STATUS): cc1200.registers.SERIAL_STATUS = data; break; 
        case (MODEM_STATUS1): cc1200.registers.MODEM_STATUS1 = data; break;
        case (MODEM_STATUS0): cc1200.registers.MODEM_STATUS0 = data; break;
        case (MARC_STATUS1): cc1200.registers.MARC_STATUS1 = data; break;
        case (MARC_STATUS0): cc1200.registers.MARC_STATUS0 = data; break;
        case (PA_IFAMP_TEST): cc1200.registers.PA_IFAMP_TEST = data; break;
        case (FSRF_TEST): cc1200.registers.FSRF_TEST = data; break;
        case (PRE_TEST): cc1200.registers.PRE_TEST = data; break;
        case (PRE_OVR): cc1200.registers.PRE_OVR = data; break;
        case (ADC_TEST): cc1200.registers.ADC_TEST = data; break;
        case (DVC_TEST): cc1200.registers.DVC_TEST = data; break;
        case (ATEST): cc1200.registers.ATEST = data; break;
        case (ATEST_LVDS): cc1200.registers.ATEST_LVDS = data; break;
        case (ATEST_MODE): cc1200.registers.ATEST_MODE = data; break;
        case (XOSC_TEST1): cc1200.registers.XOSC_TEST1 = data; break;
        case (XOSC_TEST0): cc1200.registers.XOSC_TEST0 = data; break;
        case (AES): cc1200.registers.AES = data; break;
        case (MDM_TEST): cc1200.registers.MDM_TEST = data; break;
        case (RXFIRST): cc1200.registers.RXFIRST = data; break;
        case (TXFIRST): cc1200.registers.TXFIRST = data; break;
        case (RXLAST): cc1200.registers.RXLAST = data; break;
        case (TXLAST): cc1200.registers.TXLAST = data; break; 
        case (NUM_TXBYTES): cc1200.registers.NUM_TXBYTES = data; break;
        case (NUM_RXBYTES): cc1200.registers.NUM_RXBYTES = data; break;
        case (FIFO_NUM_TXBYTES): cc1200.registers.FIFO_NUM_TXBYTES = data; break;
        case (FIFO_NUM_RXBYTES): cc1200.registers.FIFO_NUM_RXBYTES = data; break;
        case (RXFIFO_PRE_BUF): cc1200.registers.RXFIFO_PRE_BUF = data; break;
    }
    
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("(VSPI) %s data 0x%2.2X [%s]"
        " after accessing register 0x%2.2X%2.2X\n", 
        readwrite_flag ? "Read in" : "Wrote in", 
        data, main_states[cc1200.main_state],
        command, address
    );
    #endif
    digitalWrite(cc1200.pin_ss, HIGH);
    return data;
}

// 100 ns delay between consecutive data bytes must be added
// during burst write access to the configuration registers.

int cc1200_command_strobe_access(cc1200_command_strobe_t command_strobe) {
    // header read write flag ignored, burst access not possible.
    // the chip status byte is returned on the MISO line 
    // when a command strobe is sent on the MOSI line.
    return cc1200_register_access(READ, SINGLE, (cc1200_configuration_register_space_t)command_strobe, 0x00);;
}

// -------------------------------------------------------------
//                SWRU346B 9.1.2 Manual Reset
// ------------------------------------------------------------- 
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

/** Calculate Local and Voltage Controlled Oscillator Programming
 * \brief implements SWRU346B 9.12 RF Programming.
 * \param targetFrequency target VCO frequency in Hertz
 * \param targetOffset target frequency offset in Hertz
 * 
 * 
 * Frequency offset intended to adjust for crystal intolerance
 * or fine adjustments of the RF programming.
 * Strobe SAFC automatic frequency compensation to accumulate offset estimation.
 * Equation 27: Radio Frequency
 */

const double band_resolution[8] = {
    38.1, // RF Band=820.0-960 MHz (33cm band)
    19.1, // RF Band=410.0-480 MHz (70cm U.S. ONLY band)
    19.1, // RF Band=410.0-480 MHz (70cm band)
    12.7, // RF Band=273.3-320 MHz (Government)
    9.5,  // RF Band=205.0-240 MHz (1.25m Digital Link)
    9.5,  // RF Band=205.0-240 MHz (1.25m band)
    7.6,  // RF Band=164.0-192 MHz (VHF Highband)
    6.4   // RF Band=136.7-160 MHz (2m band)
};

// Table 34: RF Band Selection Decoding
const uint8_t lo_factor[17] {
    0, 0, 4, 0, 8, 0, 12, 0, 16, 0, 20, 0, 24, 0, 0, 0, 0 // zeros are bands never decoded.
};

void cc1200_calculate_frequency_programming(double targetFrequency) {
    if (targetFrequency >= 420000000 && targetFrequency <= 450000000) {
        cc1200.band = BAND_70CM;
        cc1200.lo_divider = DIVIDE_BY_08;
    } else if (targetFrequency >= 144000000 && targetFrequency <= 148000000) {
        cc1200.band = BAND_2M;
        cc1200.lo_divider = DIVIDE_BY_24;
    } else {
        #ifdef CC1200_DEBUG
        CC1200_DEBUG.print("(VSPI) Cannot enter an unimplemented band!\n");
        #endif
        return;
    }
    cc1200.frequency_resolution = band_resolution[cc1200.band];
    // vco frequency programming

    // Equation 27: Radio Frequency [Hz]
    // targetFrequency [Hz] is f_{rf}
    // f_{vco} = f_{rf}*(band lo divider factor)
    uint8_t numeric_lo_divider = lo_factor[cc1200.lo_divider];
    double vco_frequency = targetFrequency*(double)numeric_lo_divider; // VCO should be targetFrequency*8 or *24
    // now that f_{vco} is known, the FREQ word and FREQOFF word must be found.

    // Frequency Offset Estimate as utilized Offset Programming
    cc1200_command_strobe_access(SAFC); // any compensation is independent of the selected RF band.
    cc1200.registers.FREQOFF_EST1 = cc1200_register_access(READ, SINGLE, FREQOFF_EST1, 0x00);
    cc1200.registers.FREQOFF_EST0 = cc1200_register_access(READ, SINGLE, FREQOFF_EST0, 0x00);

    uint16_t U_FREQOFF_EST = (cc1200.registers.FREQOFF_EST1 << 8) | cc1200.registers.FREQOFF_EST0;
    U_FREQOFF_EST &= MAX_14_BIT_VALUE; // mask
    // sign-extend FREQOFF MSB and LSB two's comp register values into one two's comp value
    int16_t FREQOFF_EST = (U_FREQOFF_EST & TWO_TO_THE_13) ? (int16_t)(U_FREQOFF_EST - TWO_TO_THE_14) : (int16_t)U_FREQOFF_EST;
    // FREQOFF 18-bit signed word is only computed for debugging purposes.
    int32_t FREQOFF = (int32_t)std::llround(cc1200.frequency_offset / CC1200_OSC_FREQ * TWO_TO_THE_18);
    FREQOFF = (int32_t)std::clamp(FREQOFF,
        (int32_t)-TWO_TO_THE_17, (int32_t)MAX_17_BIT_VALUE
    );
    FREQOFF &= MAX_18_BIT_VALUE;
    
    // Equation 28: VCO Frequency [Hz] word FREQ after estimating FREQOFF:
    double normalized_frequency_offset = (FREQOFF/TWO_TO_THE_18*CC1200_OSC_FREQ);
    double vcoOscillatorRatio = (vco_frequency - normalized_frequency_offset) / CC1200_OSC_FREQ;
    uint32_t FREQ = (uint32_t)std::clamp((long long)std::llround(vcoOscillatorRatio*TWO_TO_THE_16),
        0LL, (long long)MAX_24_BIT_VALUE
    );
    // Calculated Offset, VCO, and RF Frequencies
    cc1200.frequency_offset = CC1200_OSC_FREQ * ((double)FREQOFF_EST / TWO_TO_THE_18); // [Hz]
    cc1200.vco_frequency = CC1200_OSC_FREQ * ((double)FREQ/TWO_TO_THE_16 + (double)FREQOFF/TWO_TO_THE_18); // [Hz]
    cc1200.frequency = (double)vco_frequency / numeric_lo_divider; // [Hz]
    // Calculated register values
    cc1200.registers.FREQOFF1 = cc1200.registers.FREQOFF_EST1; // updated on SAFC
    cc1200.registers.FREQOFF0 = cc1200.registers.FREQOFF_EST0; // updated on SAFC
    cc1200.registers.FREQ2 = (uint8_t)(FREQ >> 16);  // [23:16]
    cc1200.registers.FREQ1 = (uint8_t)(FREQ >> 8);   // [15:8]
    cc1200.registers.FREQ0 = (uint8_t)(FREQ & 0xFF); // [7:0]
    // 
    cc1200.registers.FS_CFG &= ~FSD_BANDSELECT;
    cc1200.registers.FS_CFG |= (uint8_t)((cc1200.lo_divider << FSD_BANDSELECT_SHIFT ) & FSD_BANDSELECT);
    // enable Frequency Synthesizer Out of Lock detector on FSCAL_CTRL.LOCK
    cc1200.registers.FS_CFG &= ~FS_LOCK_EN;
    cc1200.registers.FS_CFG |= (0b1 << FS_LOCK_EN_SHIFT) & FS_LOCK_EN;
    cc1200_register_access(WRITE, SINGLE, FS_CFG, cc1200.registers.FS_CFG);

    cc1200_register_access(READ, SINGLE, FSCAL_CTRL, 0x00);
    (cc1200.registers.FSCAL_CTRL & LOCK) ? "FS is in lock" : "FS is Out of Lock";
    
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.printf("(VSPI) Programming %s band... [FSD_BANDSELECT=0x%1.1X]\n"
            "Target FREQ=%3.3f MHz, VCO=%3.3f MHz [FREQ=0x%3.3X]\n" // FREQ is 24-bit 
            "Estimated Frequency Offset = %3.3f MHz, [FREQOFF_EST=%3.3f MHz, FREQOFF=%3.3f MHz]\n" // FREQOFF is 16-bit
            "Calculated FREQ = %3.3f MHz\n",
            (cc1200.band == BAND_70CM) ? "UHF" : "VHF", cc1200.lo_divider,
            targetFrequency/1000000.0, cc1200.vco_frequency/1000000.0, FREQ,
            cc1200.frequency_offset/1000000.0, (double)FREQOFF_EST/1000000.0, (double)FREQOFF/1000000.0,
            cc1200.frequency/1000000.0
    );
    #endif
}

/** Calculate Symbol Rate Programming
 * \brief implements SWRU346B 5.4 Frequency Deviation Programming.
 * NOTE: SWRU346B shows units in Hz. Maximum DEV = ~1.2 MHz
 * 
 * CC1200_OSC_FREQ from given f_{xosc} = 40 MHz
 * CC1200_OSC_FREQ_LOG2 from log2(40*10^6) = 25.253496f
 */
uint8_t frequency_deviation_exponent; // 03 bit Exponent
uint8_t frequency_deviation_mantissa; // 08 bit Mantissa
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
            0.0, (double)std::numeric_limits<uint8_t>::max()
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
        "08-bit DEV_M=0x%2.2X\n"
        "03-bit DEV_E=0x%1.1X\n"
        "Calculated DEV=%2.2f Hz\n", 
        targetDeviation, 
        frequency_deviation_mantissa,
        frequency_deviation_exponent,
        cc1200.frequency_deviation
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
        "20-bit SRATE_M=0x%5.5X\n"
        "04-bit SRATE_E=0x%1.1X\n"
        "Calculated SRATE=%2.2f sps\n", 
        targetSampleRate, 
        symbol_rate_mantissa,
        symbol_rate_exponent,
        cc1200.symbol_rate
    );
    #endif
}

void cc1200_TER_SmartRF_export(void) {
    // these are superseded by cc1200_init() and cc1200_enter_custom_frequency_modulation()
    // cc1200_register_access(WRITE, SINGLE, IOCFG3, 0x08); // SERIAL_CLK
    // cc1200_register_access(WRITE, SINGLE, IOCFG2, 0x09); // SERIAL_RX
    // cc1200_register_access(WRITE, SINGLE, IOCFG1, 0xB0); // HighZ
    // cc1200_register_access(WRITE, SINGLE, IOCFG0, 0xB0); // HighZ
    
    cc1200_register_access(WRITE, SINGLE, SYNC_CFG1, 0x08); // SYNC_MODE=000 No sync word, 01010 Sync word threshold = 8 (strict)  

    // cc1200.registers.MODCFG_DEV_E     = 0x03; // default exponent (reset)
    cc1200_register_access(WRITE, SINGLE, DCFILT_CFG, 0x1C); // filtering enabled, 64 samples, default bandwidth
    cc1200_register_access(WRITE, SINGLE, PREAMBLE_CFG1, 0x14); // Preamble word = 0xAA, minimum preamble size = 3 bits
    cc1200_register_access(WRITE, SINGLE, IQIC, 0xC4); // image compensation enabled, coefficient enabled, 8 samples, THR > 256
    // cc1200.registers.CHAN_BW          = 0x28; // ADC_CIC_DECFACT=0b00=48 (reset), BB_CIC_DECFACT=0b101000=40 (reset = 20) 
    // cc1200.registers.MDMCFG1          = 0x06; // DVGA = reserved (reset), both channels (I/Q) (reset), bypass fifo 
    // cc1200.registers.MDMCFG0          = 0x0A; // VITERBI detection disabled, should reflect reserved value change.

    // these are superseded by cc1200_calculate_symbol_rate()
    // cc1200.registers.SYMBOL_RATE2     = 0x43; // SRATE_E=0b0100 (reset), SRATE_M_19_16=0b0011 (reset) 
    // cc1200.registers.SYMBOL_RATE1     = 0xA9; // SRATE_M_15_8=0xA9 (reset)
    // cc1200.registers.SYMBOL_RATE0     = 0x2A; // SRATE_M_7_0=0x2A (reset)
    // 
    cc1200_register_access(WRITE, SINGLE, AGC_REF, 0x20); // AGC_REFERENCE=0x20=10log10(RX_FILTER_BW)-92-(RSSI Offset)
    cc1200_register_access(WRITE, SINGLE, AGC_CS_THR, 0x19); // AGC_CS_TH=25 dB (1dB resolution in two's comp)
    cc1200_register_access(WRITE, SINGLE, AGC_CFG1, 0xAF); // AGC_CFG1_NOT_USED=1 RSSI_STEP_THR=0=3 dB sync search, 10 dB packet reception, AGC_WIN_SIZE=0b101=256 samples, AGC_SETTLE_WAIT=0b111=127 samples
    cc1200_register_access(WRITE, SINGLE, AGC_CFG0, 0xCF); // 11 00 RSSI_VALID_CNT=11=update after 9 input samples, 11
    // these are superseded by cc1200_enter_custom_frequency_modulation()
    // cc1200.registers.FIFO_CFG         = 0x00; // CRC_AUTOFLUSH=false
    // these are superseded by cc1200_calculate_frequency_programming()
    // cc1200.registers.FS_CFG           = 0x12; // 000 FS_LOCK_EN=0b1=Out of lock detector enabled, FSD_BANDSELECT=0b0010=(LO divider = 4)

    // these are superseded by cc1200_enter_custom_frequency_modulation()
    // cc1200.registers.PKT_CFG2         = 0x05; // 0 0 0 001 PKT_FORMAT=01 Synchronous serial mode
    // cc1200.registers.PKT_CFG1         = 0x00; // 0 0 0 00 CRC_CFG=00=disabled for TX and RX, APPEND_STATUS=0 status byte not appended 
    // cc1200.registers.PKT_CFG0         = 0x20; // 0 LENGTH_CONFIG=01=Variable packet length mode, 000 0 0

    cc1200_register_access(WRITE, SINGLE, PA_CFG1, 0x78); // 0 1 PA_POWER_RAMP=111000=(56+1)/2 - 18 = 10.5 dBm
    cc1200_register_access(WRITE, SINGLE, PA_CFG0, 0x7C); // FIRST_IPL=111=7/16 SECOND_IPL=111=15/16 RAMP_SHAPE=00 3/8 symbol ramp time and 1/32 symbol ASK/OOK shape length (legal UPSAMPLER_P values: 100b, 101b, and 110b)
    cc1200_register_access(WRITE, SINGLE, IF_MIX_CFG, 0x04); // 000 CMIX_CFG=001 f_{if} = -f_{xosc}/(CHAN_BW.ADC_CIC_DECFACT*4) [kHz] 00
    cc1200_register_access(WRITE, SINGLE, FREQOFF_CFG, 0x22); // 00 1 00 0 FOC_KI_FACTOR=MDMCFG0.TRANSPARENT_MODE_EN|10 = 1/64
    
    // these are superseded by cc1200_calculate_frequency_programming()
    // cc1200.registers.FREQ2            = 0xD8; // FREQ_23_16=0xD8 
    // cc1200.registers.FREQ1            = 0x80; // FREQ_15_8=0x80

    cc1200_register_access(WRITE, SINGLE, IF_ADC0, 0x04); // 00 IF_ADC0_RESERVED5_0=000100

    cc1200_register_access(WRITE, SINGLE, FS_DIG0, 0x5F); // FS_DIG0_RESERVED7_4=0101, RX_LPF_BW=11 500 kHz, TX_LPF_BW=11 500 kHz
    // cc1200.registers.FS_CAL2          = 0x20; // reset
    cc1200_register_access(WRITE, SINGLE, FS_CAL0, 0x0F); // 0000 LOCK_CFG=11 infinite average, FS_CAL0_RESERVED1_0=11
    cc1200_register_access(WRITE, SINGLE, FS_CHP, 0x16); // 00 FS_CHP_RESERVED5_0=010110
    // cc1200.registers.FS_DIVTWO        = 0x01; // reset
    cc1200_register_access(WRITE, SINGLE, FS_DSM1, 0x0B); // FS_DSM1_NOT_USED=00001 FS_DSM1_RESERVED2_0=011
    cc1200_register_access(WRITE, SINGLE, FS_DSM0, 0x30); // FS_DSM0_RESERVED7_0=0x30 
    // cc1200.registers.FS_DVC1          = 0xFF; // reset
    // cc1200.registers.FS_DVC0          = 0x1F; // reset
    // cc1200.registers.FS_PFD           = 0x51; // reset
    cc1200_register_access(WRITE, SINGLE, FS_PRE, 0x1F); // 0 FS_PRE_RESERVED6_0=0011111
    cc1200_register_access(WRITE, SINGLE, FS_REG_DIV_CML, 0x1D); // 000 FS_REG_DIV_CML_RESERVED4_0=11101
    // cc1200.registers.FS_VCO0          = 0x81; // reset
    cc1200_register_access(WRITE, SINGLE, XOSC3, 0xC7); // XOSC3_RESERVED7_0=0xC7;
    // XOSC1_NOT_USED=0b00001, XOSC1_RESERVED2=0b1,
    // XOSC_BUF_SEL=0b1 Low phase noise differential buffer (low power buffer still used for digital clock)
    // XOSC_STABLE=0b1 XOSC is stable (has finished settling)
    cc1200.registers.XOSC1 &= ~(XOSC1_RESERVED2 | XOSC_BUF_SEL);
    cc1200.registers.XOSC1 |= ((0b1 << XOSC1_RESERVED2_SHIFT) & XOSC1_RESERVED2); 
    cc1200.registers.XOSC1 |= ((0b1 << XOSC_BUF_SEL_SHIFT) & XOSC_BUF_SEL); 
    cc1200_register_access(WRITE, SINGLE, XOSC1, cc1200.registers.XOSC1);

    // cc1200.registers.XOSC1            = 0x0F; 
    // cc1200.registers.RSSI1            = 0x80; // read-only RSSI_11_4=0x80 

    // cc1200.registers.MARCSTATE        = 0x41; // 0 10 00001 (read-only)
    // cc1200.registers.PQT_SYNC_ERR     = 0xFF; // reset
    // cc1200.registers.AGC_GAIN2        = 0xD1; // reset
    // cc1200.registers.AGC_GAIN0        = 0x3F; // reset
    // cc1200.registers.ASK_SOFT_RX_DATA = 0x30; // reset
    // cc1200.registers.RNDGEN           = 0x7F; // reset

    // cc1200.registers.CHFILT_I2        = 0x08; // read-only, 000010 0 CHFILT_STARTUP_VALID=0 

    // cc1200.registers.FSCAL_CTRL       = 0x01; // reset
    // cc1200.registers.PARTNUMBER       = 0x58; // read-only
    // cc1200.registers.PARTVERSION      = 0x10; // read-only
    // cc1200.registers.MODEM_STATUS1    = 0x10; // read-only 
    // cc1200.registers.DVC_TEST         = 0x0B; // reset
    // cc1200.registers.FIFO_NUM_TXBYTES = 0x0F; // reset
    cc1200_register_access(WRITE, SINGLE, XOSC_TEST1, 0x0C); // XOSC_TEST1_RESERVED7_0=0x0C
}


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
        "[%3.3f MHz, SS=%d,SCK=%d, MISO=%d, MOSI=%d]\n",
        (double)cc1200.spi_frequency/1000000.0, cc1200.pin_ss, cc1200.pin_sck, cc1200.pin_miso, cc1200.pin_mosi
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
    #ifdef CC1200_DEBUG
            CC1200_DEBUG.printf("(VSPI+I2C0 @0x%2.2X) Initialized CC1200 Interfaces!\n", pcf_radio_config.sensor_address);
    #endif
    return 0;
}

void cc1200_manual_calibration() {
    uint8_t FS_VCO_HIGH;
    uint8_t FS_VCO4_HIGH;
    uint8_t FS_CHP_HIGH;
    uint8_t FS_VCO_MID;
    uint8_t FS_VCO4_MID;
    uint8_t FS_CHP_MID;
    // 1) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    cc1200_register_access(WRITE, SINGLE, FS_VCO2, 0x00);
    // 2) Start with high VCDAC (original VCDAC_START + 2):
    cc1200.registers.FS_CAL2 = cc1200_register_access(READ, SINGLE, FS_CAL2, 0x00);
    cc1200_register_access(WRITE, SINGLE, FS_VCO2, cc1200.registers.FS_CAL2 + 2);
    // 3) Calibrate and wait for calibration to be done (radio back in IDLE state)
    cc1200_command_strobe_access(SCAL);
    do {
        cc1200_command_strobe_access(SNOP);
    }
    while (cc1200.main_state != MARC_IDLE);
    // 4) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with high VCDAC_START value
    FS_VCO_HIGH = cc1200_register_access(READ, SINGLE, FS_VCO2, 0x00);
    FS_VCO4_HIGH = cc1200_register_access(READ, SINGLE, FS_VCO4, 0x00);
    FS_CHP_HIGH = cc1200_register_access(READ, SINGLE, FS_CHP, 0x00);
    // 5) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    cc1200_register_access(WRITE, SINGLE, FS_VCO2, 0x00);
    // 6) Continue with mid VCDAC (original VCDAC_START):
    FS_VCO_MID = cc1200_register_access(READ, SINGLE, FS_CAL2, cc1200.registers.FS_CAL2);
    // 7) Calibrate and wait for calibration to be done (radio back in IDLE state)
    cc1200_command_strobe_access(SCAL);
    do {
        cc1200_command_strobe_access(SNOP);
    }
    while (cc1200.main_state != MARC_IDLE);
    // 8) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with mid VCDAC_START value
    FS_VCO_MID = cc1200_register_access(READ, SINGLE, FS_VCO2, 0x00);
    FS_VCO4_MID = cc1200_register_access(READ, SINGLE, FS_VCO4, 0x00);
    FS_CHP_MID = cc1200_register_access(READ, SINGLE, FS_CHP, 0x00);
    // 9) Write back highest FS_VCO2 and corresponding FS_VCO and FS_CHP result
    bool useHighestVDAC = FS_VCO_HIGH > FS_VCO_MID;
    cc1200.registers.FS_VCO2 = cc1200_register_access(READ, SINGLE, FS_VCO2,
        useHighestVDAC ? FS_VCO_HIGH : FS_VCO_MID
    );
    cc1200.registers.FS_VCO4 = cc1200_register_access(READ, SINGLE, FS_VCO4,
        useHighestVDAC ? FS_VCO4_HIGH : FS_VCO4_MID
    );
    cc1200.registers.FS_CHP = cc1200_register_access(READ, SINGLE, FS_CHP,
        useHighestVDAC ? FS_CHP_HIGH : FS_CHP_MID
    );
}

int cc1200_init(cc1200_config_t &cc1200) {
    if (cc1200.initialized) {
      return 0;
    }
    // -------------------------------------------------------------
    // SWRU346B 3.1.1 4-wire Serial Configuration and Data Interface
    // -------------------------------------------------------------
    setup_interface();
    // ---------------------------------------------------------
    //        SWRU346B 9.1 Power-On Start-Up Sequence 
    // ---------------------------------------------------------
    // Must be reset before entering the state diagram in Figure 2.
    // CHIP_RDYn on the SO pin after SS is pulled low.
    cc1200_reset(true);
    // once reset is completed, chip will be in the IDLE state.
    // To verify initialization, the read the part number to confirm CC1200.
    cc1200.partnumber = (cc1200_partnumber_t)cc1200_register_access(READ, SINGLE, PARTNUMBER, 0x00);
    cc1200.partrevision = cc1200_register_access(READ, SINGLE, PARTVERSION, 0x00);    

    // SFSTXON strobe enables and calibrates FS if SETTLING_CFG.FS_AUTOCAL=1
    cc1200.registers.SETTLING_CFG &= ~FS_AUTOCAL;
    cc1200.registers.SETTLING_CFG |= ((0b1 << FS_AUTOCAL_SHIFT) & FS_AUTOCAL); 
    cc1200_register_access(WRITE, SINGLE, SETTLING_CFG, cc1200.registers.SETTLING_CFG);

    update_status(cc1200_command_strobe_access(SCAL));
    // ---------------------------------------------------------
    //  SWRU346B 3.4 General Purpose Input/Output Control Pins 
    // ---------------------------------------------------------
    // To control an external LNA, PA, or RX/TX switch in applications where the 
    // SLEEP state is used it is therefore recommended to map this signal to
    // GDO3 as this signal will be hardwired to 1(0) in the SLEEP state.
    cc1200.registers.IOCFG3 &= ~GPIOx_CFG;
    cc1200.registers.IOCFG3 |= ((PA_PD << GPIOx_CFG_SHIFT) & GPIOx_CFG); // GPIO3 asserts PA_PD GPIO signal 
    cc1200_register_access(WRITE, SINGLE, IOCFG3, cc1200.registers.IOCFG3);
    #ifdef CC1200_DEBUG
    CC1200_DEBUG.printf("(VSPI+I2C0 @0x%2.2X) Initialized Radio Transceiver!\n"
        "* CC1200 Initialization Results: [Part Number=0x%2.2X, %s] [Part Revision=0x%2.2X, %s]\n"
        "* MAin Radio Control 'MARC' Unit is %s: [State=0x%1.1X, %s]\n", pcf_radio_config.sensor_address,
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

// ---------------------------------------------------------
// SWRU346B 5.2.4 Custom Frequency Modulation(CFM)/Analog FM
// ---------------------------------------------------------

void cc1200_enter_custom_frequency_modulation() {
    // ---------------------------------------------------------
    // SWRU346B Frequency Deviation Configuration
    // ---------------------------------------------------------
    cc1200_calculate_frequency_deviation(cc1200.frequency_deviation);
    // Frequency Deviation Configuration
    cc1200_register_access(WRITE, SINGLE, DEVIATION_M, cc1200.registers.DEVIATION_M);
    // Modulation Format and Frequency Deviation Configuration
    cc1200_register_access(WRITE, SINGLE, MODCFG_DEV_E, cc1200.registers.MODCFG_DEV_E);
    // ---------------------------------------------------------
    //             SWRU346B 5.4 Symbol Rate Programming
    // ---------------------------------------------------------
    // The modulator writes values to the PLL at 16x the 
    // programmed symbol rate using the soft data clock.
    cc1200_calculate_symbol_rate(cc1200.symbol_rate);
    cc1200_register_access(WRITE, SINGLE, SYMBOL_RATE2, cc1200.registers.SYMBOL_RATE2);
    cc1200_register_access(WRITE, SINGLE, SYMBOL_RATE1, cc1200.registers.SYMBOL_RATE1);
    cc1200_register_access(WRITE, SINGLE, SYMBOL_RATE0, cc1200.registers.SYMBOL_RATE0);
    // ---------------------------------------------------------
    //             SWRU346B 6 Receive Configuration
    // ---------------------------------------------------------
    // 6.1 RX Channel Filter Bandwidth
    bool widefm = true; // WFM (25.25 kHz) and NFM (14.88 kHz) configurations available
    // set CHAN_BW.ADC_CIC_DECFACT first decimation filter factor
    cc1200.registers.CHAN_BW &= ~(ADC_CIC_DECFACT | BB_CIC_DECFACT); // clear register fields
    cc1200.registers.CHAN_BW |= ((widefm ? FACTOR24 : FACTOR48) << ADC_CIC_DECFACT_SHIFT & ADC_CIC_DECFACT);
    // set CHAN_BW.BB_CIC_DECFACT second decimation filter factor
    cc1200.registers.CHAN_BW |= ((widefm ? WFM_DECFACT_OVERSHOOT : NFM_DECFACT_OVERSHOOT) & BB_CIC_DECFACT);
    cc1200_register_access(WRITE, SINGLE, CHAN_BW, cc1200.registers.CHAN_BW);

    cc1200.registers.FIFO_CFG &= ~CRC_AUTOFLUSH;
    cc1200.registers.FIFO_CFG |= (0b0 << CRC_AUTOFLUSH_SHIFT) & CRC_AUTOFLUSH;
    cc1200_register_access(WRITE, SINGLE, FIFO_CFG, cc1200.registers.FIFO_CFG);

    // ---------------------------------------------------------
    //         General Modem Parameter Configuration
    // ---------------------------------------------------------
    // MDMCFG2: enable CFM mode and set variable TX upsampling factor P=16.
    cc1200.registers.MDMCFG2 &= ~(CFM_DATA_EN | UPSAMPLER_P); 
    cc1200.registers.MDMCFG2 |= ((0b1 << CFM_DATA_EN_SHIFT) & CFM_DATA_EN);
    cc1200.registers.MDMCFG2 |= ((P16 << UPSAMPLER_P_SHIFT) & UPSAMPLER_P);
    cc1200_register_access(WRITE, SINGLE, MDMCFG2, cc1200.registers.MDMCFG2);
    // MDMCFG1: disable Normal/FIFO Mode packet format configuration.
    // NOTE: this means modem data goes directly to/from the serial pin(s)
    cc1200.registers.MDMCFG1 &= ~FIFO_EN;
    cc1200.registers.MDMCFG1 |= ((0b0 << FIFO_EN_SHIFT) & FIFO_EN);  
    cc1200_register_access(WRITE, SINGLE, MDMCFG1, cc1200.registers.MDMCFG1);
    // MDMCFG0: disable Transparent & Extended data filters and Viterbi detection.
    // NOTE: having the data filters enabled may improve sensitivity.
    cc1200.registers.MDMCFG0 &= ~TRANSPARENT_MODE_EN;
    cc1200.registers.MDMCFG0 |= ((0b0 << TRANSPARENT_MODE_EN_SHIFT) & TRANSPARENT_MODE_EN); 
    // DATA_FILTER_EN when MDMCFG0.TRANSPARENT_MODE_EN=0: (true)
    // iff WFM (25250 Hz)/(24000 Hz) > 10 or in NFM (14880 Hz)/(24000 Hz) > 10 (false)
    // and Timing offset correction limit offset < 0.2% [TOC_CFG.TOC_LIMIT=0] (reset)
    cc1200.registers.MDMCFG0 &= ~DATA_FILTER_EN;
    cc1200.registers.MDMCFG0 |= ((0b0 << DATA_FILTER_EN_SHIFT) & DATA_FILTER_EN); // Extended data filter disabled
    // NOTE: having Viterbi detection also improves sensitivity.
    cc1200.registers.MDMCFG0 &= ~VITERBI;
    cc1200.registers.MDMCFG0 |= ((0b0 << VITERBI_SHIFT) & VITERBI); // Viterbi detection disabled
    cc1200.registers.MDMCFG0 |= ((0b10) & MDMCFG0_RESERVED1_0); // use values from SmartRF Studio (reset = 0x01) 
    cc1200_register_access(WRITE, SINGLE, MDMCFG0, cc1200.registers.MDMCFG0);
    // SYNC_CFG1: disable sync word
    cc1200.registers.SYNC_CFG1 &= ~(SYNC_MODE_SHIFT);
    cc1200.registers.SYNC_CFG1 |= ((0b000 << SYNC_MODE_SHIFT) & SYNC_MODE);
    cc1200_register_access(WRITE, SINGLE, SYNC_CFG1, cc1200.registers.SYNC_CFG1);

    // select Synchronous serial mode packet format configuration
    // NOTE: synchronous serial mode makes use of the WaveMatch detector, which 
    // means the performance will be similar to the performance in FIFO mode.
    cc1200.registers.PKT_CFG2 &= ~(CCA_MODE | PKT_FORMAT);
    cc1200.registers.PKT_CFG2 |= ((0b1 << CCA_MODE_SHIFT) & CCA_MODE); // indicate clear channel when RSSI is below threshold
    cc1200.registers.PKT_CFG2 |= ((0b01 << PKT_FORMAT_SHIFT) & PKT_FORMAT); // select Synchronous serial mode
    cc1200_register_access(WRITE, SINGLE, PKT_CFG2, cc1200.registers.PKT_CFG2);

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
    cc1200.registers.EXT_CTRL &= ~BURST_ADDR_INCR_EN;
    cc1200.registers.EXT_CTRL |= ((0b0 << BURST_ADDR_INCR_EN_SHIFT) & BURST_ADDR_INCR_EN);
    cc1200_register_access(WRITE, SINGLE, EXT_CTRL, cc1200.registers.EXT_CTRL);

    // ---------------------------------------------------------
    //  SWRU346B 3.4 General Purpose Input/Output Control Pins 
    // ---------------------------------------------------------
    // set IOCFGx.GPIOx_CFG=29=0x1D to use GPIO signal CLKEN_CFM data clock 
    // trigger to read the CFM_TX_DATA_OUT samples for demodulator soft data.
    // this GPIO signal runs at the same rate as the programmed symbol rate.
    cc1200.registers.IOCFG2 &= ~GPIOx_CFG; // clear previous GPIO config
    cc1200.registers.IOCFG2 |= ((CLKEN_CFM << GPIOx_CFG_SHIFT) & GPIOx_CFG); // GPIO2 asserts CLKEN_CFM GPIO signal 
    cc1200_register_access(WRITE, SINGLE, IOCFG2, cc1200.registers.IOCFG2);
    // set IOCFGx.GPIOx_CFG=30=0x1E to use GPIO signal CFM_TX_DATA_CLK data clock
    // interrupt to the MCU to synchronize the SPI data to the internal modulation rate.
    // this GPIO signal runs at 16x the programmed symbol rate.
    cc1200.registers.IOCFG0 &= ~GPIOx_CFG; // clear previous GPIO config
    cc1200.registers.IOCFG0 |= ((CFM_TX_DATA_CLK << GPIOx_CFG_SHIFT) & GPIOx_CFG); // GPIO0 asserts CFM_TX_DATA_CLK GPIO signal 
    cc1200_register_access(WRITE, SINGLE, IOCFG0, cc1200.registers.IOCFG0);
    // -- end of one-time Custom FM configuration changes --
}

void IRAM_ATTR CLKEN_CFM_ISR() {
    cc1200.read_CFM_RX_DATA_OUT = true;
}

void IRAM_ATTR CFM_TX_DATA_CLK_ISR() {
    cc1200.write_CFM_TX_DATA_IN = true;
}

void cc1200_exit_custom_frequency_modulation() {
    // Note that in TX mode, 3 dummy symbols should be written to the 
    // CFM_TX_DATA_IN register before strobing SIDLE 
    // in order for all symbols to be sent on the air before TX mode is ended.
    cc1200_register_access(WRITE, SINGLE, CFM_TX_DATA_IN, 0x00);
    cc1200_register_access(WRITE, SINGLE, CFM_TX_DATA_IN, 0x00);
    cc1200_register_access(WRITE, SINGLE, CFM_TX_DATA_IN, 0x00);
    cc1200_command_strobe_access(SIDLE);
}

// 
// Program CC120X into different modes (RX, TX, SLEEP, IDLE, etc)
//
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

// RXOFF_MODE=01 or TXOFF_MODE=01 means return to SFSTXON.

// Entered on 
// SRX                      from FSTXON
// SRX/WOR                  from FSCAL, 
// SRX     or TXOFF_MODE=11 from STX
double currentFrequency;
radio_frequency_bands_t currentBand;

int cc1200_receive_mode(double targetFrequency) {
    if (currentFrequency != targetFrequency ) {
        cc1200_idle_mode();
        // update_status(cc1200_command_strobe_access(SFSTXON));
        cc1200_calculate_frequency_programming(targetFrequency); // 446.000 MHz
        cc1200_register_access(WRITE, SINGLE, FREQOFF1, cc1200.registers.FREQOFF1);
        cc1200_register_access(WRITE, SINGLE, FREQOFF0, cc1200.registers.FREQOFF0);
        cc1200_register_access(WRITE, SINGLE, FREQ2, cc1200.registers.FREQ2);
        cc1200_register_access(WRITE, SINGLE, FREQ1, cc1200.registers.FREQ1);
        cc1200_register_access(WRITE, SINGLE, FREQ0, cc1200.registers.FREQ0);
        //
        if (currentBand != cc1200.band) {
            rfsw_switchTo(((cc1200.band == BAND_70CM) ? UHF : VHF), LOW);
            currentBand = cc1200.band;
        }
        currentFrequency = targetFrequency;
    }
    update_status(cc1200_command_strobe_access(SRX));

    // update_status(cc1200_command_strobe_access(SNOP));
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.printf("(VSPI) Entering Receive Mode! [%s]\n", main_states[cc1200.main_state]);
    #endif
    return 0;
}
// Left on 
//            RXOFF_MODE=00 to FSCAL,
// SFSTXON or RXOFF_MODE=01 to SFSTXON,
// STX     or RXOFF_MODE=10 to STX,
// Data Buffer Error        to RX_FIFO_ERROR

// Entered on STX or RXOFF_MODE=10, 
int cc1200_transmit_mode(double targetFrequency) {
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.printf("Beginning Radio Transmit Mode...\n");
    #endif
    if (currentFrequency != targetFrequency ) {
        cc1200_idle_mode();
        // cc1200_command_strobe_access(SFSTXON);
        cc1200_calculate_frequency_programming(targetFrequency); // 446.000 MHz
        cc1200_register_access(WRITE, SINGLE, FREQOFF1, cc1200.registers.FREQOFF1);
        cc1200_register_access(WRITE, SINGLE, FREQOFF0, cc1200.registers.FREQOFF0);
        cc1200_register_access(WRITE, SINGLE, FREQ2, cc1200.registers.FREQ2);
        cc1200_register_access(WRITE, SINGLE, FREQ1, cc1200.registers.FREQ1);
        cc1200_register_access(WRITE, SINGLE, FREQ0, cc1200.registers.FREQ0);
        if (currentBand != cc1200.band) {
            rfsw_switchTo(((cc1200.band == BAND_70CM) ? UHF : VHF), HIGH);
            currentBand = cc1200.band;
        }
        currentFrequency = targetFrequency;
    }
    cc1200_command_strobe_access(STX);
    // TODO: correctly select RF switch port
    #ifdef CC1200_DEBUG
        CC1200_DEBUG.printf("(VSPI) Entering Transmit Mode... [%s]\n", main_states[cc1200.main_state]);
    #endif
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
    display_init(display_config);
    tft.printf("MARC State: %s\n"
        "Current Transceiver Configuration:\nPart Number/Revision = 0x%2.2X/0x%2.2X\n", 
        main_states[cc1200.main_state],
        cc1200.partnumber, cc1200.partrevision
    );

    uint8_t* cfm_data_buffer;
    attachInterrupt(digitalPinToInterrupt(cc1200.pin_gdio0), CFM_TX_DATA_CLK_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(cc1200.pin_gdio2), CLKEN_CFM_ISR, FALLING);

    cc1200.symbol_rate = 8000; // curiouselectron demo targets 40kHz sample rate. Default is 8kHz.
    
    cc1200_enter_custom_frequency_modulation();
    // // update RF switch port to reflect current RF band and tentative active state  
    marc_state_t active_state = MARC_RX; 
    if (active_state == MARC_RX) { // RX
        cc1200_receive_mode(446000000); // frequency programming is adjusted on change.
    } else if (active_state == MARC_TX) { // TX
        cc1200_transmit_mode(446000000);
    }
    vTaskDelay(10000/portTICK_PERIOD_MS);
    cc1200_exit_custom_frequency_modulation();

    // cc1200_command_strobe_access(STX);
    
    tft.setFont();
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