#include <SPI.h>

#include <stdint.h>
#include <stdio.h>

/*! \file cc1200.h
    \brief Device Driver for CC1200 using Espressif Arduino Core.
    
    Texas Instruments CC1200 Radio Transceiver.
*/

/** Number of bits in a long int. */
#define BITS_PER_LONG	(__CHAR_BIT__ * __SIZEOF_LONG__)
/**
 * @brief Create a contiguous bitmask starting at bit position @p l
 *        and ending at position @p h.
 */
#define GENMASK(h, l) \
	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define COMMAND_RW_FLAG BIT(7) 
#define COMMAND_BURST_FLAG BIT(6)

#define WRITE 0
#define READ 1
#define BURST 1

// 6.1 RX Channel Filter Bandwidth Configuration

// The following rule should be used when programming the RX filter BW:
// SYNC_CFG0.RX_CONFIG_LIMITATION = 0:
// The RX filter BW must be larger or equal to twice the symbol rate
// SYNC_CFG0.RX_CONFIG_LIMITATION = 1:
// The RX filter BW must be larger or equal to the symbol rate

/** Equation 10: RX Filter BW
 * f_xosc/(CHAN_BW.ADC_CIC_DECFACT*CHAN_BW.BB_CIC_DECFACT*2)
 */
    // Target is 25kHz for Wide FM or 12.5kHz for Narrow FM.
    // f_xosc = 40 MHz
    // CHAN_BW.BB_CIC_DECFACT starts at 0x02 = 24
    // CHAN_BW.ADC_CIC_DECFACT starts at 0x14 = 20
    // Default BW: BW = (40 MHz)/(2*(24)(20)) = 41.67 kHz
/**
 * 
 * ADC_CIC_DECFACT BB_CIC_DECFACT RX Filter BW Range [kHz]
            12         1 - 44            37.9 - 1666.7
            24         1 - 44            18.9 - 833.3
            48         1 - 44            9.5 - 416.7
*/

typedef enum {
    FACTOR12, FACTOR24, FACTOR48
} decimation_t;
#define WFM_DECFACT_UNDERSHOOT 17 // 24.51 kHz at FACTOR48
#define NFM_DECFACT_UNDERSHOOT 34 // 12.25 kHz at FACTOR48
#define WFM_DECFACT_OVERSHOOT 33  //  25.25 kHz at FACTOR24
#define NFM_DECFACT_OVERSHOOT 28  //  14.88 kHz at FACTOR48
typedef struct {
    uint16_t IOCFG3 = 0x08;
    uint16_t IOCFG2 = 0x09;
    uint16_t IOCFG1 = 0xB0; 
    uint16_t IOCFG0 = 0xB0;
    uint16_t SYNC3; uint16_t SYNC2; uint16_t SYNC1; uint16_t SYNC0;
    uint16_t SYNC_CFG1 = 0x0; uint16_t SYNC_CFG0;
    uint16_t DEVIATION_M;
    uint16_t MODCFG_DEV_E = 0x0;
    uint16_t DCFILT_CFG = 0x1C;
    uint16_t PREAMBLE_CFG1 = 0x14; uint16_t PREAMBLE_CFG0;
    uint16_t IQIC = 0xC4;
    uint16_t CHAN_BW = 0x28;
    #define ADC_CIC_DECFACT GENMASK(7,6) // BW into first digital low-IF mixer
    #define BB_CIC_DECFACT  GENMASK(5,0) // BW into second decimation filter
    uint16_t MDMCFG1 = 0x06; uint16_t MDMCFG0 = 0x0A;
    uint16_t SYMBOL_RATE2 = 0x43; uint16_t SYMBOL_RATE1 = 0xA9; uint16_t SYMBOL_RATE0 = 0x2A;
    uint16_t AGC_REF = 0x20;
    uint16_t AGC_CS_THR = 0x19;
    uint16_t AGC_GAIN_ADJUST;
    uint16_t AGC_CFG3; uint16_t AGC_CFG2;
    uint16_t AGC_CFG1 = 0xAF; uint16_t AGC_CFG0 = 0xCF;
    uint16_t FIFO_CFG = 0x00;
    uint16_t DEV_ADDR;
    uint16_t SETTLING_CFG;
    uint16_t FS_CFG = 0x12;
    uint16_t WOR_CFG1; uint16_t WOR_CFG0;
    uint16_t WOR_EVENT0_MSB; uint16_t WOR_EVENT0_LSB;
    uint16_t RXDCM_TIME;
    uint16_t PKT_CFG2 = 0x05; uint16_t PKT_CFG1 = 0x00; uint16_t PKT_CFG0 = 0x20;
    uint16_t RFEND_CFG1; uint16_t RFEND_CFG0;
    uint16_t PA_CFG1 = 0x78; uint16_t PA_CFG0 = 0x7C;
    uint16_t ASK_CFG;
    uint16_t PKT_LEN;
} cc1200_settings_t;

// @brief configuration of CC1200 sub 1-GHz radio transceiver
typedef struct {
    bool initialized = false;
    SPIClass *spi = NULL; // no default spi 
    uint32_t spi_frequency = 7700000; // 7.7 MHz default despite 10 MHz configuration read
    int8_t pin_nrst;
    int8_t pin_sck = 5;
    int8_t pin_mosi;
    int8_t pin_miso;
    int8_t pin_ss;
    int8_t pin_gdio0;
    int8_t pin_gdio2;
    uint8_t partnumber = -1;
    uint8_t partrevision = -1;
    bool chip_nrdy;
    uint8_t main_state;
    cc1200_settings_t radio_settings;
} cc1200_config_t;
typedef enum {
    // idle state
    MARC_IDLE           = 0b000,
    // receive states
    MARC_RX             = 0b001,
    MARC_RX_END         = 0b001,
    // transmit states
    MARC_TX             = 0b010,
    MARC_TX_END         = 0b010,
    // fast TX ready
    MARC_FSTXON         = 0b011,
    // frequency synthesizer calibration is running
    MARC_CALIBRATE      = 0b100,
    MARC_BIAS_SETTLE_MC = 0b100,
    MARC_MANCAL         = 0b100,
    MARC_STARTCAL       = 0b100,
    MARC_ENDCAL         = 0b100,
    // PLL is setting
    MARC_BIAS_SETTLE    = 0b101,
    MARC_REG_SETTLE     = 0b101,
    MARC_BWBOOST        = 0b101,
    MARC_FS_LOCK        = 0b101,
    MARC_IFADCON        = 0b101,
    MARC_RXTX_SWITCH    = 0b101,
    MARC_TXRX_SWITCH    = 0b101,
    MARC_IFADCON_TXRX   = 0b101,
    // RX FIFO has over/underflowed. Read out any 
    // useful data, then flush the FIFO with an
    MARC_RX_FIFO_ERR = 0b110,
    // SFRX strobe
    MARC_TX_FIFO_ERR = 0b111
} marc_state_t;

/*! \enum cc1200_command_strobe_t
    \brief CC1200 Command Strobes
    
    (0x30 ≤ SPI Address Space ≤ 0x3D)
*/
typedef enum {
    SRES = 0x30, // reset chip
    SFSTXON, // enable and calibrate FS if SETTLING_CFG.FS_AUTOCAL=1
    // SFSTXON wait FS for quick RX/TX when from RX & PKT_CFG2.CCA_MODE ≠ 0
    SXOFF,   // enter XOFF state when CSn is de-asserted
    SCAL,    // calibrate FS and turn it off
    // SRX/STX perform SFSTXON first from IDLE if SETTLING_CFG.FS_AUTOCAL=1 
    SRX,     // enable RX
    STX,     // enable TX from IDLE.
    // STX only to TX if clear channel when RX & PKT_CFG2.CCA_MODE ≠ 0 
    SIDLE,   // exit RX/TX, turn off FS and exit eWOR mode
    SAFC,    // Automatic Frequency Compensation
    SWOR,    // start eWOR (Section 9.6) if WOR_CFG0.RC_PD=0
    SPWD,    // enter SLEEP mode when CSn is de-asserted
    SFRX,    // flush the RX FIFO. Only issue SFRX in IDLE or RX_FIFO_ERR states    
    SFTX,    // flush the TX FIFO. Only issue SFTX in IDLE or TX_FIFO_ERR states
    SWORRST, // reset the eWOR timer to the Event1 value
    SNOP // no operation. May be used to get access to the chip status byte
} cc1200_command_strobe_t;

// GPIO Output Pin Mappings
#define PKT_SYNC_RXTX   0x06 // Default GPIO3_CFG 
#define PKT_CRC_OK      0x07 // Default GPIO2_CFG 
#define CFM_TX_DATA_CLK 0x1E
#define HIGHZ           0x30 // Default GPIO1_CFG 
#define EXT_OSC_EN      0x3C // Default GPIO0_CFG 

typedef enum {
    IOCFG3 = 0x00, IOCFG2, IOCFG1, IOCFG0,
    #define GPIOx_ATRAN BIT7 // Analog transfer enable
    #define GPIOx_INV   BIT6 // Invert output enable
    #define GPIO3_CFG   GENMASK(5,0) // Default PKT_SYNC_RXTX Output
    #define GPIO2_CFG   GENMASK(5,0) // Default PKT_CRC_OK Output
    #define GPIO1_CFG   GENMASK(5,0) // Default HIGHZ as MISO Output 
    #define GPIO0_CFG   GENMASK(5,0) // Default EXT_OSC_EN Output
    SYNC3, SYNC2, SYNC1, SYNC0,
    SYNC_CFG1, SYNC_CFG0,
    DEVIATION_M,
    MODCFG_DEV_E,
    DCFILT_CFG,
    PREAMBLE_CFG1, PREAMBLE_CFG0,
    IQIC,
    CHAN_BW,
    MDMCFG1, MDMCFG0,
    SYMBOL_RATE2, SYMBOL_RATE1, SYMBOL_RATE0,
    AGC_REF,
    AGC_CS_THR,
    AGC_GAIN_ADJUST,
    AGC_CFG3, AGC_CFG2, AGC_CFG1, AGC_CFG0,
    FIFO_CFG,
    DEV_ADDR,
    SETTLING_CFG,
    FS_CFG,
    WOR_CFG1, WOR_CFG0,
    WOR_EVENT0_MSB, WOR_EVENT0_LSB,
    RXDCM_TIME,
    PKT_CFG2, PKT_CFG1, PKT_CFG0,
    RFEND_CFG1, RFEND_CFG0,
    PA_CFG1, PA_CFG0,
    ASK_CFG,
    PKT_LEN,
    EXTENDED_ADDRESS
} cc1200_configuration_register_space_t;

typedef enum {
    IF_MIX_CFG = 0x00,
    FREQOFF_CFG,
    TOC_CFG,
    MARC_SPARE,
    ECG_CFG,
    MDMCFG2,
    #define CFM_DATA_EN        BIT0
    EXT_CTRL,
    #define PIN_CTRL_EN        BIT2
    #define EXT_40K_CLOCK_EN   BIT1
    #define BURST_ADDR_INCR_EN BIT0
    RCCAL_FINE,
    RCCAL_COARSE,
    RCCAL_OFFSET,
    FREQOFF1, FREQOFF0,
    FREQ2, FREQ1, FREQ0,
    IF_ADC2, IF_ADC1, IF_ADC0,
    FS_DIG1, FS_DIG0,
    FS_CAL3, FS_CAL2, FS_CAL1, FS_CAL0,
    FS_CHP,
    FS_DIVTWO,
    FS_DSM1,
    FS_DSM0,
    FS_DVC1, FS_DVC0,
    FS_LBI,
    FS_PFD,
    FS_PRE,
    FS_REG_DIV_CML,
    FS_SPARE,
    FS_VCO4, FS_VCO3, FS_VCO2, FS_VCO1, FS_VCO0,
    GBIAS6, GBIAS5, GBIAS4, GBIAS3, GBIAS2, GBIAS1, GBIAS0,
    IFAMP,
    LNA,
    RXMIX,
    XOSC5, XOSC4, XOSC3, XOSC2, XOSC1, XOSC0,
    ANALOG_SPARE,
    PA_CFG3 = 0x39,
    // 0x3A - 0x3E Not Used
    // 0x3F - 0x40 Reserved
    // 0x41 - 0x63 Not Used
    WOR_TIME1 = 0x64, WOR_TIME0,
    WOR_CAPTURE1, WOR_CAPTURE0,
    BIST,
    DCFILTOFFSET_I1, DCFILTOFFSET_I0,
    DCFILTOFFSET_Q1, DCFILTOFFSET_Q0, 
    IQIE_I1, IQIE_I0,
    IQIE_Q1, IQIE_Q0, 
    RSSI1, RSSI0,
    MARCSTATE,
    LQI_VAL,
    PQT_SYNC_ERR,
    DEM_STATUS,
    FREQOFF_EST1, FREQOFF_EST0, 
    AGC_GAIN3, AGC_GAIN2, AGC_GAIN1, AGC_GAIN0,
    CFM_RX_DATA_OUT,
    CFM_TX_DATA_IN,
    ASK_SOFT_RX_DATA,
    RNDGEN,
    MAGN2, MAGN1, MAGN0,
    ANG1, ANG0,
    CHFILT_I2, CHFILT_I1, CHFILT_I0, CHFILT_Q2, CHFILT_Q1, CHFILT_Q0,
    GPIO_STATUS,
    FSCAL_CTRL,
    PHASE_ADJUST,
    PARTNUMBER = 0x8F, // Part Number
    #define PARTNUM GENMASK(7, 0) 
    PARTVERSION = 0x90, // Part Revision
    #define PARTVER GENMASK(7, 0)
    SERIAL_STATUS, 
    MODEM_STATUS1, MODEM_STATUS0,
    MARC_STATUS1, MARC_STATUS0,
    PA_IFAMP_TEST, FSRF_TEST, PRE_TEST,
    PRE_OVR,
    ADC_TEST, DVC_TEST,
    ATEST, ATEST_LVDS, ATEST_MODE,
    XOSC_TEST1, XOSC_TEST0,
    AES, MDM_TEST = 0xA2,
    // 0xA3 - 0xD1 Not Used
    RXFIRST = 0xD2, TXFIRST, RXLAST, TXLAST, 
    NUM_TXBYTES, NUM_RXBYTES, FIFO_NUM_TXBYTES, FIFO_NUM_RXBYTES, RXFIFO_PRE_BUF,
    // 0xDB - 0xDF Not Used
    NOTUSED = 0xDB
} cc1200_extended_register_space_t;






// ---------------------------
// Descriptive Register Values
// ---------------------------
typedef enum {
    // See TI swru346b
    CC1200 = 0x20, // selected Chip ID
    CC1201 = 0x21, // economy version without narrow bandwidth
    // See TI swru295e
    CC1121 = 0x40,
    CC1120 = 0x48, // utilized in Skyworks reference designs
    CC1125 = 0x58,
    CC1175 = 0x5A  // based on CC1120
} cc1200_partnumber_t;

// ---------------------------
// SPI Access Types
// ---------------------------

/*! \struct SPI Access Type cc1200_spi_access_t
    \brief Contains all data related to register access.
*/
typedef struct {
    // header byte contents
    uint8_t readwrite_flag = READ;
    uint8_t burst_flag = false;
    uint8_t header; 
    cc1200_configuration_register_space_t configuration_address = EXTENDED_ADDRESS; // 6-bit address
    // address byte contents
    cc1200_extended_register_space_t extended_address = NOTUSED; // 8-bit address
    // data byte contents 
    uint8_t data = 0x00;
} cc1200_spi_access_t;


int cc1200_single_register_access(cc1200_spi_access_t &register_access);

int cc1200_burst_register_access(cc1200_spi_access_t register_access);

int cc1200_init(cc1200_config_t &cc1200);
/**
 * To verify if the RF Modulino can communicate over baseband (SPI) to the MCU Modulino:
 * [ ] Can the transceiver complete a SPI transaction requesting for device information?
 * [ ] Can the transceiver be asked to go to sleep and then be woken up?
 * [ ] How about entering SPI transparent transaction mode? Can the GPIO pins be set up for Custom Frequency Modulation?
 */
void demonstrate_radio_transceiver();