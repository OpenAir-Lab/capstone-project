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

// @brief configuration of CC1200 sub 1-GHz radio transceiver
typedef struct {
    SPIClass *spi = NULL; // no default spi 
    int32_t spi_frequency = 7700000; // 7.7 MHz default despite 10 MHz configuration read
    int8_t pin_sck = 5;
} cc1200_config_t;

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
    SNOP     // no operation. May be used to get access to the chip status byte
} cc1200_command_strobe_t;

typedef enum {
    IOCFG3 = 0x00, IOCFG2, IOCFG1, IOCFG0,
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
    PARTNUMBER = 0x8F, // Part Number
    #define PARTNUM GENMASK(7, 0) 
    PARTVERSION = 0x90 // Part Revision
    #define PARTVER GENMASK(7, 0)
} cc1200_extended_register_space_t;

// ---------------------------
// Descriptive Register Values
// ---------------------------
typedef enum {
    CC1200=0x20,
    CC1201=0x21
} cc1200_partnumber_t;

// ---------------------------
// SPI Access Types
// ---------------------------

/*! \struct SPI Access Type cc1200_spi_access_t
    \brief Contains all data related to register access.
*/
typedef struct {
    // header byte contents
    uint8_t readwrite_flag;
    uint8_t burst_flag;
    uint8_t address; 
    cc1200_configuration_register_space_t configuration_address : 6; // 6-bit address
    // address byte contents
    cc1200_extended_register_space_t extended_address; // 8-bit address
    // data byte contents 
    uint8_t data = 0x00;
} cc1200_spi_access_t;

#define COMMAND_RW_FLAG BIT(7) 
#define COMMAND_BURST_FLAG BIT(6)

#define WRITE 0
#define READ 1

int cc1200_single_register_access(bool readwrite, cc1200_spi_access_t register_access);

int cc1200_init(cc1200_config_t &cc1200);