/*
 * Priority 1 (PDO3): 12V @ 3.0A
 * Priority 2 (PDO2): 9V @ 3.0A
 * Priority 3 (PDO1): 5V @ 3.0A
 * * Run this sketch ONE TIME to program the board.
 */

// Include the required libraries
#include <Wire.h>
#include <SparkFun_STUSB4500.h>

// Create an instance of the STUSB4500 library
STUSB4500 usb;

bool UPLOAD_USB_ADVERTISEMENTS = true; // Set to true to upload advertisement profiles

void upload_usb_advertisements() {
    /* Set Number of Power Data Objects (PDO) to 3 */
    usb.setPdoNumber(3);
    
    /* PDO3 (Highest Priority Request)
      - Voltage: 12.0V
      - Current: 3.0A
    */
    usb.setVoltage(3, 12.0);
    usb.setCurrent(3, 3.0);

    /* PDO2 (Second Priority Request)
      - Voltage: 9.0V
      - Current: 3.0A
    */
    usb.setVoltage(2, 9.0);
    usb.setCurrent(2, 3.0);

    /* PDO1 (Lowest Priority / Default)
      - Voltage: Fixed at 5V by the chip
      - Current: 3.0A
    */
    usb.setCurrent(1, 3.0);

    /* Write and save settings to STUSB4500's permanent memory */
    usb.write();
    
    Serial.println("Done writing settings!");
    Serial.println("Reading back settings to verify...\n");

    /* Read the NVM settings to verify the new settings are correct */
    usb.read();

    Serial.println("New Parameters:\n");
    /* Read the Power Data Objects (PDO) highest priority */
    Serial.print("PDO Number: ");
    Serial.println(usb.getPdoNumber());
    
    /* Read settings for PDO1 */
    Serial.println();
    Serial.print("Voltage1 (V): ");
    Serial.println(usb.getVoltage(1));
    Serial.print("Current1 (A): ");
    Serial.println(usb.getCurrent(1));
    Serial.print("Lower Voltage Tolerance1 (%): ");
    Serial.println(usb.getLowerVoltageLimit(1));
    Serial.print("Upper Voltage Tolerance1 (%): ");
    Serial.println(usb.getUpperVoltageLimit(1));
    Serial.println();
    
    /* Read settings for PDO2 */
    Serial.print("Voltage2 (V): ");
    Serial.println(usb.getVoltage(2));
    Serial.print("Current2 (A): ");
    Serial.println(usb.getCurrent(2));
    Serial.print("Lower Voltage Tolerance2 (%): ");
    Serial.println(usb.getLowerVoltageLimit(2));
    Serial.print("Upper Voltage Tolerance2 (%): ");
    Serial.println(usb.getUpperVoltageLimit(2));
    Serial.println();

    /* Read settings for PDO3 */
    Serial.print("Voltage3 (V): ");
    Serial.println(usb.getVoltage(3));
    Serial.print("Current3 (A): ");
    Serial.println(usb.getCurrent(3));
    Serial.print("Lower Voltage Tolerance3 (%): ");
    Serial.println(usb.getLowerVoltageLimit(3));
    Serial.print("Upper Voltage Tolerance3 (%): ");
    Serial.println(usb.getUpperVoltageLimit(3));
    Serial.println();

    Serial.println("--- All done! ---");
  }



void setup() 
{
  Serial.begin(115200);
  Wire.begin(); //Join I2C bus
  
  delay(500);
  if (UPLOAD_USB_ADVERTISEMENTS) { 
      // Connect to the chip
      if(!usb.begin())
      {
        Serial.println("Cannot connect to STUSB4500.");
        Serial.println("Is the board connected? Is the device ID correct?");
        while(1); // Stop here if it can't connect
      }
      Serial.println("Connected to STUSB4500!");
      Serial.println("Writing your custom power profiles...");
      upload_usb_advertisements();
      delay(100);

  }
  

}

void loop()
{
  // Nothing to do here. This sketch only runs once in setup().
}