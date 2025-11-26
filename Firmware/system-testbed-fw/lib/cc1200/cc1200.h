#include <SPI.h>

#include <stdint.h>
#include <stdio.h>

/*! \file cc1200.h
    \brief Device Driver for CC1200 using Espressif Arduino Core.
    
    Texas Instruments CC1200 Radio Transceiver.
*/
#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif
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

// Sample Rate Definitions
#define MAX_04_BIT_VALUE (1U << 4) - 1
#define TWO_TO_THE_20 (1ULL << 20)
#define MAX_20_BIT_VALUE TWO_TO_THE_20 - 1
#define TWO_TO_THE_28 (1ULL << 28)
#define TWO_TO_THE_38 (1ULL << 38)
#define TWO_TO_THE_39 (1ULL << 39)
#define CC1200_OSC_FREQ 40000000UL
#define CC1200_OSC_FREQ_LOG2 25.253496f
typedef struct {
    uint8_t IOCFG3 = 0x08;
    uint8_t IOCFG2 = 0x09;
    uint8_t IOCFG1 = 0xB0; 
    uint8_t IOCFG0 = 0xB0;
    uint8_t SYNC3; uint8_t SYNC2; uint8_t SYNC1; uint8_t SYNC0;
    uint8_t SYNC_CFG1 = 0x0; uint8_t SYNC_CFG0;
    uint8_t DEVIATION_M;
    uint8_t MODCFG_DEV_E = 0x0;
    uint8_t DCFILT_CFG = 0x1C;
    uint8_t PREAMBLE_CFG1 = 0x14; uint8_t PREAMBLE_CFG0;
    uint8_t IQIC = 0xC4;
    uint8_t CHAN_BW = 0x28;
    uint8_t MDMCFG1 = 0x06; uint8_t MDMCFG0 = 0x0A;
    uint8_t SYMBOL_RATE2 = 0x43; uint8_t SYMBOL_RATE1 = 0xA9; uint8_t SYMBOL_RATE0 = 0x2A;
    uint8_t AGC_REF = 0x20;
    uint8_t AGC_CS_THR = 0x19;
    uint8_t AGC_GAIN_ADJUST;
    uint8_t AGC_CFG3; uint8_t AGC_CFG2;
    uint8_t AGC_CFG1 = 0xAF; uint8_t AGC_CFG0 = 0xCF;
    uint8_t FIFO_CFG = 0x00;
    uint8_t DEV_ADDR;
    uint8_t SETTLING_CFG;
    uint8_t FS_CFG = 0x12;
    uint8_t WOR_CFG1; uint8_t WOR_CFG0;
    uint8_t WOR_EVENT0_MSB; uint8_t WOR_EVENT0_LSB;
    uint8_t RXDCM_TIME;
    uint8_t PKT_CFG2 = 0x05; uint8_t PKT_CFG1 = 0x00; uint8_t PKT_CFG0 = 0x20;
    uint8_t RFEND_CFG1; uint8_t RFEND_CFG0;
    uint8_t PA_CFG1 = 0x78; uint8_t PA_CFG0 = 0x7C;
    uint8_t ASK_CFG;
    uint8_t PKT_LEN;
    // uint8_t EXTENDED_ADDRESS;
    uint8_t IF_MIX_CFG;
    uint8_t FREQOFF_CFG;
    uint8_t TOC_CFG;
    uint8_t MARC_SPARE;
    uint8_t ECG_CFG;
    uint8_t MDMCFG2;
    uint8_t EXT_CTRL;
    uint8_t RCCAL_FINE;
    uint8_t RCCAL_COARSE;
    uint8_t RCCAL_OFFSET;
    uint8_t FREQOFF1; uint8_t FREQOFF0;
    uint8_t FREQ2; uint8_t FREQ1; uint8_t FREQ0;
    uint8_t IF_ADC2; uint8_t IF_ADC1; uint8_t IF_ADC0;
    uint8_t FS_DIG1; uint8_t FS_DIG0;
    uint8_t FS_CAL3; uint8_t FS_CAL2; uint8_t FS_CAL1; uint8_t FS_CAL0;
    uint8_t FS_CHP;
    uint8_t FS_DIVTWO;
    uint8_t FS_DSM1;
    uint8_t FS_DSM0;
    uint8_t FS_DVC1; uint8_t FS_DVC0;
    uint8_t FS_LBI;
    uint8_t FS_PFD;
    uint8_t FS_PRE;
    uint8_t FS_REG_DIV_CML;
    uint8_t FS_SPARE;
    uint8_t FS_VCO4; uint8_t FS_VCO3; uint8_t FS_VCO2; uint8_t FS_VCO1; uint8_t FS_VCO0;
    uint8_t GBIAS6; uint8_t GBIAS5; uint8_t GBIAS4; uint8_t GBIAS3; uint8_t GBIAS2; uint8_t GBIAS1; uint8_t GBIAS0;
    uint8_t IFAMP;
    uint8_t LNA;
    uint8_t RXMIX;
    uint8_t XOSC5; uint8_t XOSC4; uint8_t XOSC3; uint8_t XOSC2; uint8_t XOSC1; uint8_t XOSC0;
    uint8_t ANALOG_SPARE;
    uint8_t PA_CFG3;
    uint8_t WOR_TIME1; uint8_t WOR_TIME0;
    uint8_t WOR_CAPTURE1; uint8_t WOR_CAPTURE0;
    uint8_t BIST;
    uint8_t DCFILTOFFSET_I1; uint8_t DCFILTOFFSET_I0;
    uint8_t DCFILTOFFSET_Q1; uint8_t DCFILTOFFSET_Q0; 
    uint8_t IQIE_I1; uint8_t IQIE_I0;
    uint8_t IQIE_Q1; uint8_t IQIE_Q0; 
    uint8_t RSSI1; uint8_t RSSI0;
    uint8_t MARCSTATE;
    uint8_t LQI_VAL;
    uint8_t PQT_SYNC_ERR;
    uint8_t DEM_STATUS;
    uint8_t FREQOFF_EST1; uint8_t FREQOFF_EST0; 
    uint8_t AGC_GAIN3; uint8_t AGC_GAIN2; uint8_t AGC_GAIN1; uint8_t AGC_GAIN0;
    uint8_t CFM_RX_DATA_OUT;
    uint8_t CFM_TX_DATA_IN;
    uint8_t ASK_SOFT_RX_DATA;
    uint8_t RNDGEN;
    uint8_t MAGN2; uint8_t MAGN1; uint8_t MAGN0;
    uint8_t ANG1; uint8_t ANG0;
    uint8_t CHFILT_I2; uint8_t CHFILT_I1; uint8_t CHFILT_I0; uint8_t CHFILT_Q2; uint8_t CHFILT_Q1; uint8_t CHFILT_Q0;
    uint8_t GPIO_STATUS;
    uint8_t FSCAL_CTRL;
    uint8_t PHASE_ADJUST;
    uint8_t PARTNUMBER; // Part Number
    uint8_t PARTVERSION; // Part Revision
    uint8_t SERIAL_STATUS; 
    uint8_t MODEM_STATUS1; uint8_t MODEM_STATUS0;
    uint8_t MARC_STATUS1; uint8_t MARC_STATUS0;
    uint8_t PA_IFAMP_TEST; uint8_t FSRF_TEST; uint8_t PRE_TEST;
    uint8_t PRE_OVR;
    uint8_t ADC_TEST; uint8_t  DVC_TEST;
    uint8_t ATEST; uint8_t ATEST_LVDS; uint8_t ATEST_MODE;
    uint8_t XOSC_TEST1; uint8_t XOSC_TEST0;
    uint8_t AES; uint8_t MDM_TEST;
    uint8_t RXFIRST; uint8_t TXFIRST; uint8_t RXLAST; uint8_t TXLAST;
    uint8_t NUM_TXBYTES; uint8_t NUM_RXBYTES; uint8_t FIFO_NUM_TXBYTES; uint8_t FIFO_NUM_RXBYTES; uint8_t RXFIFO_PRE_BUF;

} cc1200_registers_t;





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
    cc1200_registers_t registers;
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
#define PA_PD           0x19 // Control External PA (active low)
#define CLKEN_CFM       0x1D // Data clock for demodulator soft data
#define CFM_TX_DATA_CLK 0x1E // Data clock for modulator soft data
#define HIGHZ           0x30 // Default GPIO1_CFG 
#define EXT_OSC_EN      0x3C // Default GPIO0_CFG 

typedef enum {
    IOCFG3 = 0x00, IOCFG2, IOCFG1, IOCFG0,
    #define GPIOx_ATRAN_SHIFT 7
    #define GPIOx_ATRAN       BIT(7) // Analog transfer enable
    #define GPIOx_INV_SHIFT   6
    #define GPIOx_INV         BIT(6) // Invert output enable
    #define GPIOx_CFG_SHIFT   0
    #define GPIOx_CFG         GENMASK(5,0)
    SYNC3, SYNC2, SYNC1, SYNC0,
    SYNC_CFG1, SYNC_CFG0,
    DEVIATION_M,
    MODCFG_DEV_E,
    DCFILT_CFG,
    PREAMBLE_CFG1, PREAMBLE_CFG0,
    IQIC,
    CHAN_BW,
    #define ADC_CIC_DECFACT_SHIFT 6
    #define ADC_CIC_DECFACT       GENMASK(7,6) // BW into first digital low-IF mixer
    #define BB_CIC_DECFACT_SHIFT  0  
    #define BB_CIC_DECFACT        GENMASK(5,0) // BW into second decimation filter
    MDMCFG1, 
    #define FIFO_EN_SHIFT             6
    #define FIFO_EN                   BIT(6)
    MDMCFG0,
    #define TRANSPARENT_MODE_EN_SHIFT 6
    #define TRANSPARENT_MODE_EN       BIT(6)
    SYMBOL_RATE2,
    #define SRATE_E_SHIFT        4
    #define SRATE_E              GENMASK(7,4) // reset 0x04
    #define SRATE_M_19_16_SHIFT  0
    #define SRATE_M_19_16        GENMASK(3,0) // reset 0x03 
    SYMBOL_RATE1, SYMBOL_RATE0,
    #define SRATE_M_15_8         GENMASK(7,0) // reset 0xA9
    #define SRATE_M_7_0          GENMASK(7,0) // reset 0x2A
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
    PKT_CFG2, 
    #define CCA_MODE_SHIFT 2
    #define CCA_MODE GENMASK(4,2)
    #define PKT_FORMAT_SHIFT 0
    #define PKT_FORMAT GENMASK(1,0)
    PKT_CFG1, PKT_CFG0,
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
    #define UPSAMPLER_P_SHIFT         1
    #define UPSAMPLER_P               GENMASK(3,1) // reset 0x04, default P=16
    #define CFM_DATA_EN_SHIFT         0
    #define CFM_DATA_EN               BIT(0)
    EXT_CTRL,
    #define PIN_CTRL_EN_SHIFT         2
    #define PIN_CTRL_EN               BIT(2)
    #define EXT_40K_CLOCK_EN_SHIFT    1
    #define EXT_40K_CLOCK_EN          BIT(1)
    #define BURST_ADDR_INCR_EN_SHIFT  0
    #define BURST_ADDR_INCR_EN        BIT(0)
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
    // cc1200_configuration_register_space_t configuration_address = EXTENDED_ADDRESS; // 6-bit address
    // // address byte contents
    // cc1200_extended_register_space_t extended_address = NOTUSED; // 8-bit address
    // data byte contents 
    uint8_t data = 0x00;
} cc1200_spi_access_t;


int cc1200_single_register_access(cc1200_configuration_register_space_t configuration_address, cc1200_spi_access_t &register_access);
int cc1200_single_register_access(cc1200_extended_register_space_t extended_address, cc1200_spi_access_t &register_access);

int cc1200_burst_register_access(cc1200_configuration_register_space_t configuration_address, cc1200_spi_access_t &register_access);
int cc1200_burst_register_access(cc1200_extended_register_space_t extended_address, cc1200_spi_access_t &register_access);

int cc1200_init(cc1200_config_t &cc1200);
/**
 * To verify if the RF Modulino can communicate over baseband (SPI) to the MCU Modulino:
 * [ ] Can the transceiver complete a SPI transaction requesting for device information?
 * [ ] Can the transceiver be asked to go to sleep and then be woken up?
 * [ ] How about entering SPI transparent transaction mode? Can the GPIO pins be set up for Custom Frequency Modulation?
 */
void demonstrate_radio_transceiver();