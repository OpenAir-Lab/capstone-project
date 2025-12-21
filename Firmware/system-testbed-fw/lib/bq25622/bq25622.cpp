/*
MIT License

Copyright (c) 2024 keith06388

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <bq25622.h>

// PERSONAL NOTES: .CPP USED ALONG WITH MAIN?
// ===================== I2C read/write =====================

// Read a single byte (8-bits) from a register
void BQ25622::_read(bq25622_reg_t reg, uint8_t *val) {
    _i2c->beginTransmission(_i2c_addr);
    _i2c->write(reg);
    _i2c->endTransmission();

    _i2c->requestFrom(_i2c_addr, 1);

    if (_i2c->available())
    {
      *val = _i2c->read();
    }
}

// Read 2 bytes (16-bits) from a register
void BQ25622::_read2(bq25622_reg_t reg, uint16_t *val) {
    _i2c->beginTransmission(_i2c_addr);
    _i2c->write(reg);
    _i2c->endTransmission();

    _i2c->requestFrom(_i2c_addr,2);

    if (_i2c->available() >= 2) {
        uint8_t lsb = _i2c->read();
        uint8_t msb = _i2c->read();
        *val = (msb << 8) | lsb; // Combine bytes, assuming little-endian order
    }
}

// Write a single byte to a register
void BQ25622::_write(bq25622_reg_t reg, uint8_t *val) {
    _i2c->beginTransmission(_i2c_addr);
    _i2c->write(reg);
    _i2c->write(*val);
    _i2c->endTransmission();
}

// Write 2 bytes to a register
void BQ25622::_write2(bq25622_reg_t reg, uint16_t *val) {
	uint16_t value = *val;
	_i2c->beginTransmission(_i2c_addr);
    _i2c->write(reg);
    _i2c->write(value & 0xFF);
	_i2c->write((value >> 8) & 0xFF);
    _i2c->endTransmission();
}

// ===================== Initialization =====================

// Initialize the I2C interface
void BQ25622::begin(TwoWire *theWire){
    _i2c = theWire;
    _i2c->begin();
}

// Check if device is connected on I2C bus
bool BQ25622::isConnected(){
    _i2c->beginTransmission(_i2c_addr);
    uint8_t error = _i2c->endTransmission();
    if(error == 0) return true;
    else return false;
}

// reset BQ25622 device
void BQ25622::reset(){
    BQ25622::setREG_RST(1);
}

// ===================== Charge Current (ICHG) =====================
// REG02

// getter for the charge current register value
ichg_reg_t BQ25622::getICHG_reg(){
    ichg_reg_t temp_reg;
	_read2(Charge_Current_Limit_LSB, (uint16_t*)&temp_reg);
    return temp_reg;
}

// setter for the charge current (mA)
bq25622_error_t BQ25622::setICHG(int value) {
    ichg_reg_t temp_reg;
    if(value < 80 || value > 3040){
        return BQ_RANGE_ERR;
    }
    uint8_t data = value / 80;
	_read2(Charge_Current_Limit_LSB, (uint16_t*)&temp_reg);
    temp_reg.ichg = data;
    _write2(Charge_Current_Limit_LSB, (uint16_t*)&temp_reg); // Write the low byte
    return BQ_OK;
}

// getter for the charge current (mA)
uint16_t BQ25622::getICHG(){
    ichg_reg_t temp_reg = BQ25622::getICHG_reg();
    uint16_t data = temp_reg.ichg * 80;
    return data;
}

// ===================== Charge Voltage (VREG) =====================
// REG04

// getter for charge voltage register 
vreg_reg_t BQ25622::getVREG_reg(){
    vreg_reg_t temp_reg;
	_read2(Charge_Voltage_Limit_LSB, (uint16_t*)&temp_reg);
    return temp_reg;
}

// setter for the charge voltage value
bq25622_error_t BQ25622::setVREG(int value){
    vreg_reg_t temp_reg;
    if(value < 3500 || value > 4800){
        return BQ_RANGE_ERR;
    }
    uint16_t data = value / 10;
	_read2(Charge_Voltage_Limit_LSB, (uint16_t*)&temp_reg);
    temp_reg.vreg = data;
    _write2(Charge_Voltage_Limit_LSB, (uint16_t*)&temp_reg); // Write the low byte
    return BQ_OK;
}

// getter for the charge voltage value
uint16_t BQ25622::getVREG(){
    vreg_reg_t temp_reg = BQ25622::getVREG_reg();
    uint16_t data = temp_reg.vreg * 10;
    return data;
}

// ===================== Input Current Limit (IINDPM) =====================
// REG06

// PERSONAL NOTE: WHY DO WE HAVE GETTERS FOR REGISTERS

// getter for input current limtit register
iindpm_reg_t BQ25622::getIINDPM_reg(){
    iindpm_reg_t temp_reg;
	_read2(Input_Current_Limit_LSB, (uint16_t*)&temp_reg);
    return temp_reg;
}

// setter for input current limit value (mA)
bq25622_error_t BQ25622::setIINDPM(int value){
    iindpm_reg_t temp_reg;
    if(value < 100 || value > 3200){
        return BQ_RANGE_ERR;
    }
    uint16_t data = value / 20;
	_read2(Input_Current_Limit_LSB, (uint16_t*)&temp_reg);
    temp_reg.iindpm = data;
    _write2(Input_Current_Limit_LSB, (uint16_t*)&temp_reg); // Write the low byte
    return BQ_OK;
}

// getter for input current limit value
uint16_t BQ25622::getIINDPM(){
    iindpm_reg_t temp_reg = BQ25622::getIINDPM_reg();
    uint16_t data = (temp_reg.iindpm * 20);
    return data;
}

// ===================== Input Voltage Limit (VINDPM) =====================
// REG08

// getter for Vin limit register
vindpm_reg_t BQ25622::getVINDPM_reg(){
    vindpm_reg_t temp_reg;
	_read2(Input_Voltage_Limit_LSB, (uint16_t*)&temp_reg);
    return temp_reg;
}

// setter for Vin limit val (mV)
bq25622_error_t BQ25622::setVINDPM(int value){
    vindpm_reg_t temp_reg;
    if(value < 3800 || value > 16800){
        return BQ_RANGE_ERR;
    }
    uint16_t data = value / 40;
	_read2(Input_Voltage_Limit_LSB, (uint16_t*)&temp_reg);
    temp_reg.vindpm = data;
    _write2(Input_Voltage_Limit_LSB, (uint16_t*)&temp_reg); // Write the low byte
    return BQ_OK;
}

// getter for Vin limit val
uint16_t BQ25622::getVINDPM(){
    vindpm_reg_t temp_reg = BQ25622::getVINDPM_reg();
    uint16_t data = (temp_reg.vindpm * 40);
    return data;
}

// ===================== System Minimum Voltage (VSYSMIN) =====================
// REG0E

// getter for register
vsysmin_reg_t BQ25622::getVSYSMIN_reg(){
    vsysmin_reg_t temp_reg;
    _read2(Minimal_System_Voltage_LSB, (uint16_t*)&temp_reg);
    return temp_reg;
}

// setter for min voltage value
bq25622_error_t BQ25622::setVSYSMIN(int value){
    vsysmin_reg_t temp_reg;
    if(value < 2560 || value > 3840){
        return BQ_RANGE_ERR;
    }
    uint16_t data = value / 80;
    _read2(Minimal_System_Voltage_LSB, (uint16_t*)&temp_reg);
    temp_reg.vsysmin = data;
    _write2(Minimal_System_Voltage_LSB, (uint16_t*)&temp_reg); // Write the low byte
    return BQ_OK;
}

// getter for min voltage value
uint16_t BQ25622::getVSYSMIN(){
    vsysmin_reg_t temp_reg = BQ25622::getVSYSMIN_reg();
    uint16_t data = (temp_reg.vsysmin * 80);
    return data;
}

// ===================== Precharge Current (IPRECHG) =====================
// REG10

// getter for register
ipre_reg_t BQ25622::getIPRE_reg(){
    ipre_reg_t temp_reg;
	_read2(Precharge_Control_LSB, (uint16_t*)&temp_reg);
    return temp_reg;
}

// setter for Precharge Current value
bq25622_error_t BQ25622::setIPRECHG(int value){
    ipre_reg_t temp_reg;
    if(value < 20 || value > 620){
        return BQ_RANGE_ERR;
    }
    uint8_t data = value / 20;
	_read2(Precharge_Control_LSB, (uint16_t*)&temp_reg);
    temp_reg.iprechg = data;
    _write2(Precharge_Control_LSB, (uint16_t*)&temp_reg); // Write the low byte
    return BQ_OK;
}

// getter for Precharge Current value
uint16_t BQ25622::getIPRECHG(){
    ipre_reg_t temp_reg = BQ25622::getIPRE_reg();
    uint16_t data = (temp_reg.iprechg * 20);
    return data;
}

// ===================== Termination Current (ITERM) =====================
// REG12

// getter for register
iterm_reg_t BQ25622::getITERM_reg(){
    iterm_reg_t temp_reg;
	_read2(Termination_Control_LSB, (uint16_t*)&temp_reg);
    return temp_reg;
}

// setter for I-term value
bq25622_error_t BQ25622::setITERM(int value){
    iterm_reg_t temp_reg;
    if(value < 10 || value > 620){
        return BQ_RANGE_ERR;
    }
    uint16_t data = value / 10;
	_read2(Termination_Control_LSB, (uint16_t*)&temp_reg);
    temp_reg.iterm = data;
    _write2(Termination_Control_LSB, (uint16_t*)&temp_reg); // Write the low byte
    return BQ_OK;
}

// getter for I-term value
uint16_t BQ25622::getITERM(){
    iterm_reg_t temp_reg = BQ25622::getITERM_reg();
    uint16_t data = (temp_reg.iterm * 10);
    return data;
}

// ===================== Charge Timer Control (CHG_TIMER)=====================
// REG13

// keep defaults

// getter for register
chg_timer_0_reg_t BQ25622::getCHG_TIMER_reg(){
    chg_timer_0_reg_t temp_reg;
    _read(Charge_Timer_Control, (uint8_t*)&temp_reg);
    return temp_reg;
}

void BQ25622::setTMR2X_EN(bool value){
    chg_timer_0_reg_t temp_reg;
    _read(Charge_Timer_Control, (uint8_t*)&temp_reg);
    temp_reg.tmr2x_en = value;
    _write(Charge_Timer_Control, (uint8_t*)&temp_reg);
}

void BQ25622::setEN_SAFETY_TMR(bool value){
    chg_timer_0_reg_t temp_reg;
    _read(Charge_Timer_Control, (uint8_t*)&temp_reg);
    temp_reg.en_safety_tmrs = value;
    _write(Charge_Timer_Control, (uint8_t*)&temp_reg);
}

void BQ25622::setPRECHG_TMR(bool value){
    chg_timer_0_reg_t temp_reg;
    _read(Charge_Timer_Control, (uint8_t*)&temp_reg);
    temp_reg.prechg_tmr = value;
    _write(Charge_Timer_Control, (uint8_t*)&temp_reg);
}

void BQ25622::setCHG_TIMER(bool value){
    chg_timer_0_reg_t temp_reg;
    _read(Charge_Timer_Control, (uint8_t*)&temp_reg);
    temp_reg.chg_timer = value;
    _write(Charge_Timer_Control, (uint8_t*)&temp_reg);
}

void BQ25622::setEN_AUTO_INDET(bool value){
    chg_timer_0_reg_t temp_reg;
    _read(Charger_Control_4, (uint8_t*)&temp_reg);
    temp_reg.en_auto_indet = value;
    _write(Charger_Control_4, (uint8_t*)&temp_reg);
}

void BQ25622::setFORCE_INDET(bool value){
    chg_timer_0_reg_t temp_reg;
    _read(Charger_Control_4, (uint8_t*)&temp_reg);
    temp_reg.force_indet = value;
    _write(Charger_Control_4, (uint8_t*)&temp_reg);
}

void BQ25622::setEN_DCP_BIAS(bool value){
    chg_timer_0_reg_t temp_reg;
    _read(Charger_Control_4, (uint8_t*)&temp_reg);
    temp_reg.en_dcp_bias = value;
    _write(Charger_Control_4, (uint8_t*)&temp_reg);
}

// ===================== Charge Control 0 =====================
// REG14

// ** Leave all as defaults (no need to call in main) **

// getter for register
ctrl0_reg_t BQ25622::getCTRL0_reg(){
    ctrl0_reg_t temp_reg;
    _read(Charge_Control_0, (uint8_t*)&temp_reg);
    return temp_reg;
}

void BQ25622::setQ1_FULLON(bool value){    
    ctrl0_reg_t temp_reg;
    _read(Charge_Control_0, (uint8_t*)&temp_reg);
    temp_reg.q1_fullon = value;
    _write(Charge_Control_0, (uint8_t*)&temp_reg);
}

void BQ25622::setQ4_FULLON(bool value){    
    ctrl0_reg_t temp_reg;
    _read(Charge_Control_0, (uint8_t*)&temp_reg);
    temp_reg.q4_fullon = value;
    _write(Charge_Control_0, (uint8_t*)&temp_reg);
}

bq25622_error_t BQ25622::setTOPOFF_TMR(int value){
    switch (value){
        case 0:
            value = 0b00;
            break;
        case 17:
            value = 0b01;
            break;
        case 35:
            value = 0b10;
            break;
        case 52:
            value = 0b11;
            break;
        default:
            return BQ_RANGE_ERR;
    }
    ctrl0_reg_t temp_reg;
    _read(Charge_Control_0, (uint8_t*)&temp_reg);
    temp_reg.topoff_tmr = value;
    _write(Charge_Control_0, (uint8_t*)&temp_reg);
    return BQ_OK;
}

// ===================== Charge Control 1 =====================
// REG16
// (the following encompass independent bit-fields that make up the 16-bit control register)

// ** leave all as defaults **

// getter for register
ctrl1_reg_t BQ25622::getCTRL1_reg(){
    ctrl1_reg_t temp_reg;
    _read(Charger_Control_1, (uint8_t*)&temp_reg);
    return temp_reg;
}

void BQ25622::setEN_CHG(bool value){
    ctrl1_reg_t temp_reg;
    _read(Charger_Control_1, (uint8_t*)&temp_reg);
    temp_reg.en_chg = value;
    _write(Charger_Control_1, (uint8_t*)&temp_reg);
}

bq25622_error_t BQ25622::setWATCHDOG(int value){
    switch(value) {
        case 0:
            value = 0b00;
            break;
        case 50:
            value = 0b01;
            break;
        case 100:
            value = 0b10;
            break;
        case 200:
            value = 0b11;
            break;
        default:
            return BQ_RANGE_ERR;
    }
    ctrl1_reg_t temp_reg;
    _read(Charger_Control_1, (uint8_t*)&temp_reg);
    temp_reg.watchdog = value;
    _write(Charger_Control_1, (uint8_t*)&temp_reg);
    return BQ_OK;
}

// ===================== Charge Control 2 =====================
// REG17

// getter for register
ctrl2_reg_t BQ25622::getCTRL2_reg(){
    ctrl2_reg_t temp_reg;
    _read(Charger_Control_2, (uint8_t*)&temp_reg);
    return temp_reg;
}

bq25622_error_t BQ25622::setVBUS_OVP(int value){
    ctrl2_reg_t temp_reg;
    switch(value) {
        case 6: 
            value = 0b0; //6.3 V
            break;
        case 18:
            value = 0b1; // 18.5 V
            break;
        default:
            return BQ_RANGE_ERR;
    }
    _read(Charger_Control_2, (uint8_t*)&temp_reg);
    temp_reg.vbus_ovp = value;
    _write(Charger_Control_2, (uint8_t*)&temp_reg);
    return BQ_OK; 
}

void BQ25622::setCONV_STRN(int value){
    //function encodes the integer to the corresponding bits of the byte (for normal mode 1 = 01b)
    ctrl2_reg_t temp_reg;
    _read(Charger_Control_2, (uint8_t*)&temp_reg);
    temp_reg.set_conv_strn = value;
    _write(Charger_Control_2, (uint8_t*)&temp_reg);
}

void BQ25622::setCONV_FREQ(int value){ // keep default
    ctrl2_reg_t temp_reg;
    _read(Charger_Control_2, (uint8_t*)&temp_reg);
    temp_reg.set_conv_freq = value;
    _write(Charger_Control_2, (uint8_t*)&temp_reg);
}

bq25622_error_t BQ25622::setTREG(int value) {
    switch (value)
    {
        case 60:
            value = 0b0;
            break;
        case 120:
            value = 0b1;
            break; 
        default:
            return BQ_RANGE_ERR;
    }
    ctrl2_reg_t temp_reg;
    _read(Charger_Control_2, (uint8_t*)&temp_reg);
    temp_reg.treg = value;
    _write(Charger_Control_2, (uint8_t*)&temp_reg);
    return BQ_OK;
}

void BQ25622::setREG_RST(bool value){ // keep default
    ctrl2_reg_t temp_reg;
    _read(Charger_Control_2, (uint8_t*)&temp_reg);
    temp_reg.reg_rst = value;
    _write(Charger_Control_2, (uint8_t*)&temp_reg);
}

// ===================== Charge Control 4 =====================
// REG19

// getter for register
ctrl4_reg_t BQ25622::getCTRL4_reg(){
    ctrl4_reg_t temp_reg;
    _read(Charger_Control_4, (uint8_t*)&temp_reg);
    return temp_reg;
}
void BQ25622::setEN_EXTLIM(bool value){        // EN_EXTLIM = 1 -> enable ILIM pin (When using resistor)
                                                    // EN_EXTLIM = 0 -> disable ILIM pin and use register to set ILIM (not using resistor)
    ctrl4_reg_t temp_reg;
    _read(Charger_Control_4, (uint8_t*)&temp_reg);
    temp_reg.en_extlim = value;
    _write(Charger_Control_4, (uint8_t*)&temp_reg);
}

// ===================== NTC Control =====================
// REG1A

// getter for register
ntc_reg_t BQ25622::getNTC_reg(){
    ntc_reg_t temp_reg;
    _read(NTC_Control_0, (uint8_t*)&temp_reg);
    return temp_reg;
}
void BQ25622::setTS_IGNORE(bool value){ // kept default
    ntc_reg_t temp_reg;
    _read(NTC_Control_0, (uint8_t*)&temp_reg);
    temp_reg.ts_ignore = value;
    _write(NTC_Control_0, (uint8_t*)&temp_reg);
}

// ===================== Charge Status 0 =====================
// REG1D

// This is a read-only status register

// getter for register
chrg_status_0_reg_t BQ25622::getCHRG_STATUS_0_reg(){
    chrg_status_0_reg_t temp_reg;
    _read(Charger_Status_0, (uint8_t*)&temp_reg);
    return temp_reg;
}

bool BQ25622::getWD_STAT(){
    chrg_status_0_reg_t temp_reg;
    _read(Charger_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.wd_stat;
}

bool BQ25622::getSAFETY_TMR_STAT(){
    chrg_status_0_reg_t temp_reg;
    _read(Charger_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.safety_tmr_stat;
}

bool BQ25622::getVINDPM_STAT(){
    chrg_status_0_reg_t temp_reg;
    _read(Charger_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.vindpm_stat;
}

bool BQ25622::getIINDPM_STAT(){
    chrg_status_0_reg_t temp_reg;
    _read(Charger_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.iindpm_stat;
}

bool BQ25622::getVSYS_STAT(){
    chrg_status_0_reg_t temp_reg;
    _read(Charger_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.vsys_stat;
}

bool BQ25622::getTREG_STAT(){
    chrg_status_0_reg_t temp_reg;
    _read(Charger_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.treg_stat;
}

bool BQ25622::getADC_DONE_STAT(){
    chrg_status_0_reg_t temp_reg;
    _read(Charger_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.adc_done_stat;
}

// ===================== Charge Status 1 =====================
// REG1E

// read-only status register

// getter for register
chrg_status_1_reg_t BQ25622::getCHRG_STATUS_1_reg(){
    chrg_status_1_reg_t temp_reg;
    _read(Charger_Status_1, (uint8_t*)&temp_reg);
    return temp_reg;
}
uint8_t BQ25622::getVBUS_STAT(){
    chrg_status_1_reg_t temp_reg;
    _read(Charger_Status_1, (uint8_t*)&temp_reg); 
    return temp_reg.vbus_stat;
}
uint8_t BQ25622::getCHG_STAT(){
    chrg_status_1_reg_t temp_reg;
    _read(Charger_Status_1, (uint8_t*)&temp_reg); // read the byte and store it in temp_reg
    return temp_reg.chg_stat;
}

// ===================== Fault Status 0 =====================
// REG1F

// read-only status register

// getter for register
fault_status_0_reg_t BQ25622::getFAULT_STATUS_0_reg(){
    fault_status_0_reg_t temp_reg;
    _read(Fault_Status_0, (uint8_t*)&temp_reg);
    return temp_reg;
}

bool BQ25622::getTS_STAT(){
    fault_status_0_reg_t temp_reg;
    _read(Fault_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.ts_stat;
}

bool BQ25622::getTSHUT_FAULT(){
    fault_status_0_reg_t temp_reg;
    _read(Fault_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.tshut_fault;
}

bool BQ25622::getOTG_FAULT(){
    fault_status_0_reg_t temp_reg;
    _read(Fault_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.otg_fault;
}

bool BQ25622::getSYS_FAULT(){
    fault_status_0_reg_t temp_reg;
    _read(Fault_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.sys_fault;
}

bool BQ25622::getBAT_FAULT(){
    fault_status_0_reg_t temp_reg;
    _read(Fault_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.bat_fault;
}

bool BQ25622::getVBUS_FAULT(){
    fault_status_0_reg_t temp_reg;
    _read(Fault_Status_0, (uint8_t*)&temp_reg);
    return temp_reg.vbus_fault;
}

// ===================== ADC Control =====================
// REG26 (table pg: 61 in datasheet)

// keep all default

// getter for register
adc_ctrl_reg_t BQ25622::getADC_CTRL_reg(){
    adc_ctrl_reg_t temp_reg;
    _read(ADC_Control, (uint8_t*)&temp_reg);
    return temp_reg;
}

void BQ25622::setCONV_START(bool value){
    adc_ctrl_reg_t temp_reg;
    _read(ADC_Control, (uint8_t*)&temp_reg);
    temp_reg.adc_en = value;
    _write(ADC_Control, (uint8_t*)&temp_reg);
}

void BQ25622::setCONV_RATE(bool value){
    adc_ctrl_reg_t temp_reg;
    _read(ADC_Control, (uint8_t*)&temp_reg);
    temp_reg.adc_rate = value;
    _write(ADC_Control, (uint8_t*)&temp_reg);
}

bq25622_error_t BQ25622::setADC_SAMPLE(int value){
    switch(value) {
        case 12:
            value = 0b00;
            break;
        case 11:
            value = 0b01;
            break;
        case 10:
            value = 0b10;
            break;
        case 9:
            value = 0b11;
            break;
        default:
            return BQ_RANGE_ERR;
    }
    adc_ctrl_reg_t temp_reg;
    uint16_t data = value;
    _read(ADC_Control, (uint8_t*)&temp_reg);
    temp_reg.adc_sample = data;
    _write(ADC_Control, (uint8_t*)&temp_reg);
    return BQ_OK; 
}

void BQ25622::setADC_AVG(bool value){
    adc_ctrl_reg_t temp_reg;
    _read(ADC_Control, (uint8_t*)&temp_reg);
    temp_reg.adc_avg = value;
    _write(ADC_Control, (uint8_t*)&temp_reg);
}

void BQ25622::setADC_AVG_INIT(bool value){
    adc_ctrl_reg_t temp_reg;
    _read(ADC_Control, (uint8_t*)&temp_reg);
    temp_reg.adc_avg_init = value;
    _write(ADC_Control, (uint8_t*)&temp_reg);
}

// ===================== ADC Function Disable =====================
// REG27 

// Keep all defaults (all enabled at default)

// getter for register
adc_dis_reg_t BQ25622::getADC_DIS_reg(){
    adc_dis_reg_t temp_reg;
    _read(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
    return temp_reg;
}

void BQ25622::setVPMID_ADC_DIS(bool value){
    adc_dis_reg_t temp_reg;
    _read(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
    temp_reg.vpmid_adc_dis = value;
    _write(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
}

void BQ25622::setTDIE_ADC_DIS(bool value){
    adc_dis_reg_t temp_reg;
    _read(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
    temp_reg.tdie_adc_dis = value;
    _write(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
}

void BQ25622::setTS_ADC_DIS(bool value){
    adc_dis_reg_t temp_reg;
    _read(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
    temp_reg.ts_adc_dis = value;
    _write(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
}

void BQ25622::setVSYS_ADC_DIS(bool value){
    adc_dis_reg_t temp_reg;
    _read(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
    temp_reg.vsys_adc_dis = value;
    _write(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
}

void BQ25622::setVBAT_ADC_DIS(bool value){
    adc_dis_reg_t temp_reg;
    _read(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
    temp_reg.vbat_adc_dis = value;
    _write(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
}

void BQ25622::setVBUS_ADC_DIS(bool value){
    adc_dis_reg_t temp_reg;
    _read(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
    temp_reg.vbus_adc_dis = value;
    _write(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
}

void BQ25622::setIBAT_ADC_DIS(bool value){
    adc_dis_reg_t temp_reg;
    _read(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
    temp_reg.ibat_adc_dis = value;
    _write(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
}

void BQ25622::setIBUS_ADC_DIS(bool value){
    adc_dis_reg_t temp_reg;
    _read(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
    temp_reg.ibus_adc_dis = value;
    _write(ADC_Function_Disable_0, (uint8_t*)&temp_reg);
}


// ===================== ADC Readings =====================
    // These readings are enabled from the ADC function (can be disabled above). 

// ===================== IBAT Reading =====================
// REG2A

// getter for register
ibat_reg_t BQ25622::getIBAT_reg(){
    ibat_reg_t temp_reg;
	_read2(IBAT_ADC_LSB, (uint16_t*)&temp_reg);
    return temp_reg;
}

uint16_t BQ25622::getIBAT(){
    ibat_reg_t temp_reg = BQ25622::getIBAT_reg();
    uint16_t data = temp_reg.ibat * 4.0;
    return data;
}

// ===================== VBUS Reading =====================
// REG2C

// get register
vbusv_reg_t BQ25622::getVBUSV_reg(){
    vbusv_reg_t temp_reg;
	_read2(VBUS_ADC_LSB, (uint16_t*)&temp_reg);
    return temp_reg;
}

uint16_t BQ25622::getVBUSV(){
    vbusv_reg_t temp_reg = BQ25622::getVBUSV_reg();
    uint16_t data = (temp_reg.vbusv * 3.97);
    return data;
}

// ===================== VBAT Reading =====================
// REG30

// get register
batv_reg_t BQ25622::getBATV_reg(){
    batv_reg_t temp_reg;
	_read2(VBAT_ADC_LSB, (uint16_t*)&temp_reg);
    return temp_reg;
}

uint16_t BQ25622::getBATV(){
    batv_reg_t temp_reg = BQ25622::getBATV_reg();
    uint16_t data = (temp_reg.batv * 1.99);
    return data;
}

const char* charge_statuses[4] = {
    "Not Charging",
    "Pre-Charging",
    "Taper Charge",
    "Active Charging"
};

extern BQ25622 pmic;

#define DEBUG_BQ25662 Serial
#define BMS_STACK_SIZE 2048

void vBMSTask(void *parameter) {
    //print Control 0 register status bits
    bool contrl0 = true;
    //print Fault Status register info
    bool fltStat0 = true;
    // ====== Charge Status 1 Register ======
    for (;;) {
        // ====== See if Battery is charging ======
        uint8_t chgStat = pmic.getCHG_STAT();

        #ifdef DEBUG_BQ25662
        DEBUG_BQ25662.printf("Charge Status: %s", charge_statuses[chgStat]);
        #endif
        // ====== Charge Status 0 Register ======
        if(contrl0) {
            // Watchdog Timer Status
            DEBUG_BQ25662.printf("WD Status: %s\n"
                "Safety timer stat %s\n"
                "VINDPM stat: %s\n"
                "IINDPM stat: %s\n"
                "VSYS stat: %s\n"
                "TREG stat: %s\n", 
                pmic.getWD_STAT() ? "WD expired" : "Normal",
                pmic.getSAFETY_TMR_STAT() ? "Timer Expired" : "Normal",
                pmic.getVINDPM_STAT() ? "In VINDPM regulation" : "Normal",
                pmic.getIINDPM_STAT() ? "In IINDPM regulation" : "Normal",
                pmic.getVSYS_STAT() ? "In VSYSMIN regulation (BAT<VSYSMIN)" : "Not in VSYSMIN regulation (BAT>VSYSMIN)",
                pmic.getTREG_STAT() ? "Device in thermal regulation" : "Normal"
            );
        }

        // ====== Fault Status 0 Register ======
        if(fltStat0) {
            // The TS temperature zone.
            //insert
            DEBUG_BQ25662.printf("IC temp shutdown status: %s\n"
                "VSYS stat: %s\n"
                "Batt fault stat: %s\n"
                "VBUS fault stat: %s\n",
                pmic.getTSHUT_FAULT() ? "Device in thermal shutdown protection" : "Normal",
                pmic.getSYS_FAULT() ? "SYS in SYS short circuit or over voltage" : "Normal",
                pmic.getBAT_FAULT() ? "Device in battery over current protection or battery overvoltage protection" : "Normal",
                pmic.getVBUS_FAULT() ? "Device not switching due to over voltage protection or sleep comparator" : "Normal"
            );
        }
        vTaskDelay(10000/portTICK_PERIOD_MS); // must block keypad tasks
    }
}

int bq25662_init() {
    #ifdef DEBUG_BQ25662
    DEBUG_BQ25662.printf("(I2C0 -----) Scanning for bq25662 BMS on address 0x6B...\n");
    #endif
    bool initialized = false;
    pmic.begin(&Wire);
    while (!initialized) {
        Wire.beginTransmission(0x6B);
        if (Wire.endTransmission() == 0) {       // ends that exchange
            #ifdef DEBUG_BQ25662
            DEBUG_BQ25662.printf("(I2C0 @0x6B) Initialized BMS!\n");
            #endif
            initialized = true;
            // if(!pmic.isConnected()) {
            //     #ifdef DEBUG_BQ25662
            //     DEBUG_BQ25662.println("(I2C0 -----) Found bq27427 BMS on address 0x6B!\n");
            //     #endif
            // }
        }
    }
    
    // ============== #3 Configuration ================

    // ==== Charge Current ====
    pmic.setICHG(500);      // Set charge current for 500mA

    // ==== Charge Voltage ====
    pmic.setVREG(4200);     // Set charge cut-off voltage for battery (see batt datasheet)

    // ==== Input Current Limit ====
    pmic.setIINDPM(2000);    // mA 

    // ==== Input Voltage Limit ====
    pmic.setVINDPM(4600);   // 4600mV = 4.6V
        /*       
        Purpose: In case of voltage drop from VBUS (USB for example going lower than 5V) 
        the chip will reduce charge current and input current to ensure USB stays "alive" (preventing shutdown)
        */

    // ==== System Min Voltage ====
    pmic.setVSYSMIN(2800);  // Set min system voltage (2.8V) if batt reaches this point system ensures it does not drop any further

    // ==== Precharge Current ====
    pmic.setIPRECHG(50);    // Set pre-charge current (typical 10% of charge current)

    // ==== Termination Current ====
    pmic.setITERM(40);      // Set chrg termination curr (when voltage is nearly full reduce chrg curr)

    // ==== Charge Timer Control ====
        // left all defaults

    // ==== Charge Control 0 ====
        // left all default

    // ==== Charge Control 1 ====
    pmic.setEN_CHG(true);   // Enable charging (also on by default)

    // ==== Charge Control 2 ====
    pmic.setVBUS_OVP(6);     // Set VBUS overvoltage protection to 6.3 V
    pmic.setCONV_STRN(1); // Normal 
    pmic.setTREG(60);        // Set Thermal Regulation (protects against chip overheating) 60 C

    // Note: not using OTG so skipping CC #3
        // defaults == disabled

    // ==== Charge Control 4 ====
    pmic.setEN_EXTLIM(1);   // Using resistor for setting charge current limit

    // ==== NTC Control ====
        // left all default

    // ==== ADC Control ====
        // left all default 

    // ==== ADC Function Disable ====
        // left all default (all enabled)
    xTaskCreate(vBMSTask, "BMSTask", BMS_STACK_SIZE*2, NULL, (tskIDLE_PRIORITY + 3), NULL);
    return 0;
}
