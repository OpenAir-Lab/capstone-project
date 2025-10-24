#include <cc1200.h>
// 
// Configure the CC120X
// 
extern cc1200_config_t cc1200;

int cc1200_init(cc1200_config_t &cc1200) {
    // Initialize SPI
    cc1200.spi->setFrequency(10000000); // 10 MHz
    cc1200.spi->begin(cc1200.pin_sck);
    // 100 ns delay between consecutive data bytes must be added
    // during burst write access to the configuration registers.
    return 0;
}
// Section 3.1.1 4-Wire Serial Configuration and Data Interface
// SPI interface transfers are MSB-first.
// CSn pin must be kept low during transfers on the SPI bus.
// Header byte with R/Wn bit, Burst access B bit, and 6-bit address.

// Table 3: SPI Access Types
int cc1200_single_register_access() {
    return 0;
}

int cc1200_burst_register_access() {
    return 0;
}


int cc1200_command_strobe_access(cc1200_command_strobe_t strobe) {
    // The chip status byte is returned on the MISO line 
    // when a command strobe is sent on the MOSI line.
    return 0;
}

// Section 3.1.2 Chip Status Byte
// Status byte is sent on the MISO pin each time a header byte is transmitted on the MOSI pin.

// `
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