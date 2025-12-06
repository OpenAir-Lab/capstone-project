//*****************************************************************************
//! @file       cc112x_analog_fm_rx.c
//! @brief      This program sets up an easy link between two trxEB's with
//              CC112x EM's connected. 
//              The program can take any recomended register settings exported
//              from SmartRF Studio 7 without any modification with exeption 
//              from the assumtions decribed below.
//              
//              Notes:          The following asumptions must be fulfilled for 
//              the program to work:
//              
//              1. GPIO2 has to be set up with GPIO2_CFG = 0x06
//              PKT_SYNC_RXTX for correct interupt
//              2. Packet engine has to be set up with status bytes enabled 
//              PKT_CFG1.APPEND_STATUS = 1
//
//  Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/

/*****************************************************************************
* INCLUDES
*/
#include "msp430.h"
#include "lcd_dogm128_6.h"
#include "hal_spi_rf_trxeb.h"
#include "stdlib.h"
#include "bsp.h"
#include "bsp_key.h"
#include "io_pin_int.h"
#include "bsp_led.h"
#include "cc112x_spi.h"
#include "cc112x_analog_fm_reg_config.h"

/******************************************************************************
* CONSTANTS
*/ 

/******************************************************************************
* DEFINES
*/
#define PKTLEN              10
#define VARIABEL_LENGTH     1   //1 if variabel packet length is used, 0 if fixed packet length
#define ISR_ACTION_REQUIRED 1
#define ISR_IDLE            0
#define CRC16_POLY          0x8005
#define CRC_INIT            0xFFFF

/******************************************************************************
* LOCAL VARIABLES
*/
static uint32 packetCounter = 0;
static uint8 softTxDataSemaphore;
/******************************************************************************
* STATIC FUNCTIONS
*/
static void initMCU(void);
static void registerConfig(void);
static void manualCalibration(void);
static void runAnalogFmRX(void);
static void softTxClockISR(void);
static void updateLcd(void);
static uint16 calcCRC(uint8 crcData, uint16 crcReg);
static void softTxDataReceivePacket(void);
/******************************************************************************
* @fn          main
*
* @brief       Runs the main routine
*                
* @param       none
*
* @return      none
*/
void main(void){
  
  // Initialize MCU and peripherals
  initMCU();
  
  // Write radio registers
  registerConfig();
  
  // Update LCD
  updateLcd();
  
  // Enter runAnalogFmRX, never coming back
  runAnalogFmRX();
  
}
/******************************************************************************
* @fn          runAnalogFmRX
*
* @brief       none
*                
* @param       none
*
* @return      none
*/
static void runAnalogFmRX(void){
 
  uint8 writeByte;

  // Connect ISR function to GPIO2
  ioPinIntRegister(IO_PIN_PORT_1, 8, &softTxClockISR);
  // Interrupt on falling edge
  ioPinIntTypeSet(IO_PIN_PORT_1, 8, IO_PIN_FALLING_EDGE);
  
  //Instantiate tranceiver RF spi interface to SCLK ~ 4 MHz */
  //input clockDivider - SMCLK/clockDivider gives SCLK frequency
  trxRfSpiInterfaceInit(0x04);
  
  // Update LCD
  updateLcd();
  
  // Calibrate radio according to errata
  manualCalibration();
  
  // Set radio in RX
  trxSpiCmdStrobe(CC112X_SRX);
  
  // Enable analog FM
  writeByte = 0x01;
  cc112xSpiWriteReg(CC112X_CFM_DATA_CFG, &writeByte, 1);  
  
  // Set the signal CLKEN_SOFT as output on GPIO2
  writeByte = 0x1d;
  cc112xSpiWriteReg(CC112X_IOCFG2, &writeByte, 1);

  // PKT_FORMAT needs to be set to other then FIFO mode (serial or transparent mode)
  writeByte = 0x05;
  cc112xSpiWriteReg(CC112X_PKT_CFG2, &writeByte, 1);

  
  // Infinite loop
  while(TRUE){
      
    //Clear semaphore
    softTxDataSemaphore = ISR_IDLE;
    
    // Enable interrupt
  ioPinIntEnable(IO_PIN_PORT_1, 8);
    
    //Set radio in RX
    trxSpiCmdStrobe(CC112X_SRX);
    
    //Receive packet
    softTxDataReceivePacket();
    
    // Update LCD
    updateLcd();
    
    //Enter IDLE mode
    trxSpiCmdStrobe(CC112X_SIDLE);
    
    // Wait until the radio is in IDLE mode 
    while(((trxSpiCmdStrobe(CC112X_SNOP))& 0xF0) != (0x00));
  }
}



/*******************************************************************************
* @fn          softTxDataReceivePacket
*
* @brief       Receive the packet and fills it in the rxBuffer.
*
* @param       none
*
* @return      none
*/
static void softTxDataReceivePacket(void){
  uint8 rxBuffer[128] = {0};
  uint8 receivedDev;
  int8 deviation;
  uint8 lengthByte = PKTLEN;
  uint8 dataByte;
  uint8 bufferIndex = 0;
  uint8 crcBytes = 2;
    
  //read length byte
  if(VARIABEL_LENGTH){   
    lengthByte = 0;
    for (uint8 i = 0; i < 8; i++){
      
      while (!softTxDataSemaphore);
      
      softTxDataSemaphore = ISR_IDLE;
      cc112xSpiReadReg(CC112X_CFM_RX_DATA_OUT, &receivedDev, 1);
      deviation = (int8) receivedDev;
      
      lengthByte <<= 1;
      if(deviation< 0)
        lengthByte &= 0xFE;
      else
        lengthByte |= 0x01;
    }
    rxBuffer[bufferIndex++] = lengthByte;
  }
  
  //Read payload
  for(uint8 i = 0; i <lengthByte + crcBytes; i++){
    dataByte = 0;
    for(uint8 j = 0; j <8; j++){
      
      while(!softTxDataSemaphore);
      
      softTxDataSemaphore = ISR_IDLE;
      cc112xSpiReadReg(CC112X_CFM_RX_DATA_OUT, &receivedDev, 1);
      deviation = (int8) receivedDev;
      dataByte <<= 1;
      
      if(deviation < 0)
        dataByte &= 0xFE;
      else
        dataByte |= 0x01;
    }
    rxBuffer[bufferIndex++]= dataByte;
  }
    
  while(!softTxDataSemaphore);
  
  //Calculate CRC, and check if the received packet is correct
  uint16 checksum = CRC_INIT; // Init value for CRC calculation
  for (uint8 i = 0; i < lengthByte + crcBytes + VARIABEL_LENGTH; i++){
    checksum = calcCRC(rxBuffer[i], checksum);
  }
  
  if(checksum == 0){
    packetCounter++;
  }
}
/*******************************************************************************
* @fn          softTxClockISR
*
* @brief       none
*
* @param       none
*
* @return      none
*/
static void softTxClockISR(void){
 
  softTxDataSemaphore = ISR_ACTION_REQUIRED;
  
  // Clear isr flag
  ioPinIntClear(IO_PIN_PORT_1, 8);
  
}
/******************************************************************************
* @fn          calcCRC
*
* @brief       Calculates a checksum over 
*                
* @param       uint8 sync3, uint8 sync2, uint8 sync1, uint8 sync0
*              The sync word that is being searched for. 
*              sync3 is the most significant byte.
*
* @return      none
*/
static uint16 calcCRC(uint8 crcData, uint16 crcReg)
{
    for (uint8 i = 0; i < 8; i++){
    if (((crcReg & 0x8000) >> 8) ^ (crcData & 0x80))
      crcReg = (crcReg << 1) ^ CRC16_POLY;
    else
      crcReg = (crcReg << 1);
      crcData <<= 1;
  }
  return crcReg;
}

/******************************************************************************
 * @fn          initMCU
 *
 * @brief       Initialize MCU and board peripherals
 *                
 * @param       input, output parameters
 *
 * @return      describe return value, if any
 */
static void initMCU(void){
  
  // Init clocks and I/O 
  bspInit(BSP_SYS_CLK_25MHZ);
  
  // Init leds 
  bspLedInit(); 

  // Init Buttons
  bspKeyInit(BSP_KEY_MODE_POLL);
  
  // Initialize SPI interface to LCD (shared with SPI flash)
  bspIoSpiInit(BSP_FLASH_LCD_SPI, BSP_FLASH_LCD_SPI_SPD);  
  
  // Init LCD
  lcdInit();

  // Instantiate tranceiver RF spi interface to SCLK ~ 4 MHz */
  //input clockDivider - SMCLK/clockDivider gives SCLK frequency
  trxRfSpiInterfaceInit(0x02);

  // Enable global interrupt
  _BIS_SR(GIE);
}
/*******************************************************************************
* @fn          registerConfig
*
* @brief       Write register settings as given by SmartRF Studio found in
*              cc112x_analog_fm_reg_config.h
*
* @param       none
*
* @return      none
*/
static void registerConfig(void){
  
  uint8 writeByte;
  
  // Reset radio
  trxSpiCmdStrobe(CC112X_SRES);
  
  // Write registers to radio
  for(uint16 i = 0; i < (sizeof  preferredSettings/sizeof(registerSetting_t)); i++) {
    writeByte =  preferredSettings[i].data;
    cc112xSpiWriteReg( preferredSettings[i].addr, &writeByte, 1);
  }
}
/******************************************************************************
* @fn          updateLcd
*
* @brief       updates LCD buffer and sends bufer to LCD module.
*                
* @param       none
*
* @return      none
*/
static void updateLcd(void){
  
      // Update LDC buffer and send to screen.
      lcdBufferClear(0);
      lcdBufferPrintString(0, "    Analog FM Test    ", 0, eLcdPage0);
      lcdBufferSetHLine(0, 0, LCD_COLS-1, 7); 
      lcdBufferPrintString(0, "Sent packets:", 0, eLcdPage3);
      lcdBufferPrintInt(0, packetCounter, 70, eLcdPage4);
      lcdBufferPrintString(0, "      Packet TX        " , 0, eLcdPage7);
      lcdBufferSetHLine(0, 0, LCD_COLS-1, 55);
      lcdBufferInvertPage(0, 0, LCD_COLS, eLcdPage7);
      lcdSendBuffer(0);
}
/******************************************************************************
* @fn          manualCalibration
*
* @brief       calibrates radio according to CC112x errata
*                
* @param       none
*
* @return      none
*/
#define VCDAC_START_OFFSET 2
#define FS_VCO2_INDEX 0
#define FS_VCO4_INDEX 1
#define FS_CHP_INDEX 2
static void manualCalibration(void){
  
  uint8 original_fs_cal2;
  uint8 calResults_for_vcdac_start_high[3];
  uint8 calResults_for_vcdac_start_mid[3];
  uint8 marcstate;
  uint8 writeByte;
  
  // 1) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
  writeByte = 0x00;
  cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);
  
  // 2) Start with high VCDAC (original VCDAC_START + 2):
  cc112xSpiReadReg(CC112X_FS_CAL2, &original_fs_cal2, 1);
  writeByte = original_fs_cal2 + VCDAC_START_OFFSET;
  cc112xSpiWriteReg(CC112X_FS_CAL2, &writeByte, 1);
  
  // 3) Calibrate and wait for calibration to be done (radio back in IDLE state)
  trxSpiCmdStrobe(CC112X_SCAL);
  
  do 
  {
    cc112xSpiReadReg(CC112X_MARCSTATE, &marcstate, 1);
  } while (marcstate != 0x41);
  
  // 4) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with high VCDAC_START value
  cc112xSpiReadReg(CC112X_FS_VCO2, &calResults_for_vcdac_start_high[FS_VCO2_INDEX], 1);
  cc112xSpiReadReg(CC112X_FS_VCO4, &calResults_for_vcdac_start_high[FS_VCO4_INDEX], 1);
  cc112xSpiReadReg(CC112X_FS_CHP, &calResults_for_vcdac_start_high[FS_CHP_INDEX], 1);
  
  // 5) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
  writeByte = 0x00;
  cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);
  
  // 6) Continue with mid VCDAC (original VCDAC_START):
  writeByte = original_fs_cal2;
  cc112xSpiWriteReg(CC112X_FS_CAL2, &writeByte, 1);
  
  // 7) Calibrate and wait for calibration to be done (radio back in IDLE state)
  trxSpiCmdStrobe(CC112X_SCAL);
  
  do 
  {
    cc112xSpiReadReg(CC112X_MARCSTATE, &marcstate, 1);
  } while (marcstate != 0x41);
  
  // 8) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with mid VCDAC_START value
  cc112xSpiReadReg(CC112X_FS_VCO2, &calResults_for_vcdac_start_mid[FS_VCO2_INDEX], 1);
  cc112xSpiReadReg(CC112X_FS_VCO4, &calResults_for_vcdac_start_mid[FS_VCO4_INDEX], 1);
  cc112xSpiReadReg(CC112X_FS_CHP, &calResults_for_vcdac_start_mid[FS_CHP_INDEX], 1);
  
  // 9) Write back highest FS_VCO2 and corresponding FS_VCO and FS_CHP result
  if (calResults_for_vcdac_start_high[FS_VCO2_INDEX] > calResults_for_vcdac_start_mid[FS_VCO2_INDEX]) 
  {
    writeByte = calResults_for_vcdac_start_high[FS_VCO2_INDEX];
    cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);
    writeByte = calResults_for_vcdac_start_high[FS_VCO4_INDEX];
    cc112xSpiWriteReg(CC112X_FS_VCO4, &writeByte, 1);
    writeByte = calResults_for_vcdac_start_high[FS_CHP_INDEX];
    cc112xSpiWriteReg(CC112X_FS_CHP, &writeByte, 1);
  }
  else 
  {
    writeByte = calResults_for_vcdac_start_mid[FS_VCO2_INDEX];
    cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);
    writeByte = calResults_for_vcdac_start_mid[FS_VCO4_INDEX];
    cc112xSpiWriteReg(CC112X_FS_VCO4, &writeByte, 1);
    writeByte = calResults_for_vcdac_start_mid[FS_CHP_INDEX];
    cc112xSpiWriteReg(CC112X_FS_CHP, &writeByte, 1);
  }
}