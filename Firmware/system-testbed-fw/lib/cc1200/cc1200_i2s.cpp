#include <cc1200.h>
#include <driver/i2s.h>      // Espressif Arduino Core I2S driver

extern cc1200_config_t cc1200; 

const i2s_bits_per_sample_t BITS = I2S_BITS_PER_SAMPLE_32BIT;
const size_t FRAMES_PER_BUF = 128; // frames per buffer (stereo frames)
const size_t BYTES_PER_FRAME = 8; // 2 * 32-bit = 8 bytes
const size_t BUF_BYTES = FRAMES_PER_BUF * BYTES_PER_FRAME;


// The two CFM_TRX_DATA registers have the same format (two’s complement)
    // to simplify software control and data buffering in both TX and RX.
    
    
// Receive Custom FM
    
// 8-bit signed soft-decision symbol data, 
// either from normal receiver or transparent receiver. 
// Can be read using burst mode to do custom demodulation
double cc1200_demodulate_cfm_byte() {
    // CFM_RX_DATA_OUT is used to read the instantaneous frequency offset.
    if (cc1200.read_CFM_RX_DATA_OUT) {
        cc1200.registers.CFM_RX_DATA_OUT = cc1200_register_access(READ, BURST, CFM_RX_DATA_OUT, 0x00);
    }
    // f_{offset} = \frac{f_{dev}*CFM_RX_DATA_OUT.CFM_RX_DATA}{64}  
    double offset = cc1200.frequency_deviation*cc1200.registers.CFM_RX_DATA_OUT/64; 
    return offset;
}


//
// Transmit CFM
//

// 8-bit signed soft TX data input register for custom SW controlled modulation. 
// Can be accessed using burst mode to get arbitrary modulation
double cc1200_modulate_cfm_byte() {
    
    // CFM_TX_DATA_IN is used to write the carrier frequency offset.
    if (cc1200.write_CFM_TX_DATA_IN) {
        cc1200_register_access(WRITE, BURST, CFM_TX_DATA_IN, cc1200.registers.CFM_TX_DATA_IN);
    }
    /** 129 values between -f_{dev} and +f_{dev}
     * f_{offset} = \frac{f_{dev}*CFM_TX_DATA_IN.CFM_TX_DATA}{64}
     * consider linear upsampler UPSAMPLER_P
     */
    double offset = cc1200.frequency_deviation*cc1200.registers.CFM_TX_DATA_IN/64; 
    return offset;
}
