/*
 * PROJECT: OPEN AIR LAB - BMS CONTROLLER FIRMWARE
 * SYSTEM:  MCU + BQ27427 FUEL GAUGE
 * ROLE:    Read Battery -> Send to HMI
 * AUTHOR: J
 *
 * DESCRIPTION:
 * This firmware runs on the dedicated BMS microcontroller. 
 * It performs two main tasks:
 * 1. Configures the TI BQ27427 Fuel Gauge chip (via I2C) to match our specific battery.
 * 2. Reads the State of Charge (SOC) and transmits it to the main HMI board via Serial.
 */

#include <Wire.h>
#include <BQ27427.h> // Library: "BQ27427" by Edrean Ernst

// --- CONFIGURATION ---
// I2C Pins for the MCU connecting to the Fuel Gauge
#define PIN_SDA 21       // Adjust if your specific MCU uses different default pins
#define PIN_SCL 22

// Battery Specifications (CRITICAL FOR ACCURACY)
// Capacity: 2500mAh (Polymer Li-Ion Model 785060)
#define BAT_CAPACITY 2500

// Chemistry ID:
// 0x1202 = Standard 4.2V Li-Ion (matches our battery)
// 0x3230 = 4.35V (Default - DO NOT USE, will show wrong %)
#define CHEM_ID_42V  0x1202 

// Create the fuel gauge object
BQ27427 fuelGauge;

void setup() {
  // 1. Debug Serial (PC Connection)
  Serial.begin(115200);
  
  // 2. Comms Serial (Connection to HMI Board)
  // If using a hardware UART port to talk to the HMI, initialize it here.
  // Connect TX of this board -> RX of HMI Board.
  // Serial2.begin(9600); 

  Serial.println("BMS: Initializing Fuel Gauge...");

  // 3. Initialize I2C Bus
  // Standard speed (100kHz) is usually fine for short onboard traces.
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000); 

  // 4. Connect to Chip
  // begin() checks if the device at address 0x55 responds.
  if (!fuelGauge.begin()) {
    Serial.println("BMS: Error - BQ27427 not found!");
    Serial.println("Check SDA/SCL wiring and pull-up resistors.");
    while(1); // Halt execution if hardware is missing
  }

  // 5. Configure Battery Profile (One-time setup on boot)
  // The chip defaults to a generic profile. We must tell it our battery specs.
  
  // Unlock the chip's configuration registers
  fuelGauge.enterConfig(true);
  
  // Set the Chemistry Profile to 4.2V so 100% actually equals 4.2V
  send_cmd(CHEM_ID_42V);       
  
  // Tell the algorithm the battery capacity so it can calculate remaining runtime
  fuelGauge.setCapacity(BAT_CAPACITY); 
  
  // Save changes and lock the registers (reseal)
  fuelGauge.exitConfig(true);
  
  Serial.println("BMS: Configuration Loaded. Starting Loop.");
}

void loop() {
  // A. Read SOC from Chip
  // Returns an integer from 0 to 100
  int soc = fuelGauge.soc();
  int volts = fuelGauge.voltage(); // Voltage in mV (Optional, for debug)

  // B. Sanity Check
  // Sometimes I2C glitches can return weird values (like 255 or -1).
  // We clamp the value to ensure the HMI display doesn't break.
  if (soc > 100) soc = 100;
  if (soc < 0) soc = 0;

  // C. Send to HMI
  // Format: "BAT:XX" followed by a newline.
  // The HMI looks for the "BAT:" prefix to know this is battery data.
  String dataPacket = "BAT:" + String(soc);
  
  // Send to USB (for debugging on PC)
  Serial.println(dataPacket);  
  
  // Send to HMI (Uncomment the line below if using a 2nd Serial port)
  // Serial2.println(dataPacket); 

  // Wait 2 seconds before the next update to save power/bandwidth
  delay(2000); 
}

/*
 * Helper Function: send_cmd
 * -------------------------
 * Directly writes a 16-bit command to the Control Register (0x00).
 * Used here specifically to set the Chemistry ID, as the standard library
 * might not have a dedicated function for changing chemistry profiles.
 */
void send_cmd(uint16_t cmd) {
  Wire.beginTransmission(0x55); // Address of BQ27427
  Wire.write(0x00);             // Target Register (Control)
  Wire.write(cmd & 0xFF);       // Low Byte
  Wire.write((cmd >> 8) & 0xFF);// High Byte
  Wire.endTransmission();
}