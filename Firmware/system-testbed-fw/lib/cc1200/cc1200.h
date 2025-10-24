#include <SPI.h>

// VSPI matrix 
// SCK 5 -> 4
// MISO 18
// MOSI 19
// SS 23

/// @brief configuration of CC1200 sub 1-GHz radio transceiver
typedef struct {
    SPIClass *spi;
    int8_t pin_sck = 5;
} cc1200_config_t;

// @brief CC1200 Command Strobes (0x30 ≤ SPI Address Space ≤ 0x3D)
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

int cc1200_init(cc1200_config_t cc1200);
