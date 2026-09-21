/*
  MKE-S17 AGS10 Set I2C Address Example
  
  This example demonstrates how to change the I2C address of the MKE-S17 AGS10 sensor.
  This is useful if you want to connect MULTIPLE AGS10 sensors to the same microcontroller.
  
  WARNING: 
  - The default factory I2C address is 0x1A.
  - You MUST only have ONE sensor connected to the I2C bus when running this script!
    This script includes a safety scan to prevent accidentally reprogramming multiple sensors.
  - The new address is saved to the sensor's permanent memory.
  
  Hardware connections (Only connect ONE sensor at a time):
  - VCC to 3.3V/5V
  - GND to GND
  - SDA to SDA (A4 on Uno, D21 on ESP32)
  - SCL to SCL (A5 on Uno, D22 on ESP32)
*/

#include <Wire.h>
#include <AGS10.h>

AGS10 myAGS10;

// Set your desired new address here (Valid I2C addresses are 0x08 to 0x77).
// Note: To revert a sensor back to the factory default address, simply set this to 0x1A.
uint8_t NEW_ADDRESS = 0x1B; 

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("\n===========================================");
  Serial.println("MKE-S17 AGS10 I2C Address Modification Tool");
  Serial.println("===========================================\n");

  // Initialize I2C Bus. 
  Wire.begin();

  // Set I2C clock speed to 15kHz (Datasheet requirement)
  Wire.setClock(15000);

  Serial.println("Scanning I2C bus for connected devices...");
  byte error, address;
  int nDevices = 0;
  uint8_t CURRENT_ADDRESS = 0x00;
  
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print(" -> Found device at address: 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      CURRENT_ADDRESS = address;
      nDevices++;
    }
  }
  
  Serial.println();

  // Safety Checks
  if (nDevices == 0) {
    Serial.println("ERROR: No I2C devices found! Please check your wiring.");
    while(1) delay(10); // Halt execution
  } 
  else if (nDevices > 1) {
    Serial.println("ERROR: Multiple I2C devices found!");
    Serial.println("You MUST connect ONLY ONE sensor to the I2C bus when changing the address.");
    Serial.println("Please disconnect other devices and restart.");
    while(1) delay(10); // Halt execution
  }
  
  if (CURRENT_ADDRESS == NEW_ADDRESS) {
    Serial.println("SUCCESS: The sensor is already set to your NEW_ADDRESS.");
    Serial.println("Nothing to do! You can start using your sensor.");
    while(1) delay(10); // Halt execution
  }

  // If we passed all checks, proceed with changing the address
  // Initialize the sensor with its automatically detected CURRENT address
  myAGS10.begin(&Wire, CURRENT_ADDRESS);
  
  Serial.println("--------------------------------------------------");
  Serial.print("Attempting to change sensor address from 0x");
  Serial.print(CURRENT_ADDRESS, HEX);
  Serial.print(" to 0x");
  Serial.println(NEW_ADDRESS, HEX);
  Serial.println("Sending command in 5 seconds...");
  Serial.println("--------------------------------------------------");
  
  delay(5000);
  
  // Send the command to change the address
  myAGS10.setAddress(NEW_ADDRESS);
  
  Serial.println("\nCommand sent!");
  Serial.println("Testing the new address...");
  delay(200);
  
  // Test if the sensor responds at the new address
  // (The library automatically updates its internal address pointer after setAddress)
  int version = myAGS10.readVersion();
  
  if (version > 0) {
    Serial.println("SUCCESS! Sensor responded at the new address.");
    Serial.print("Firmware Version: ");
    Serial.println(version);
    Serial.print("\nFrom now on, initialize your sensor like this:\n");
    Serial.print("myAGS10.begin(&Wire, 0x");
    Serial.print(NEW_ADDRESS, HEX);
    Serial.println(");");
  } else {
    Serial.println("FAILED. Sensor did not respond at the new address.");
    Serial.println("Please check your wiring and restart the board.");
  }
}

void loop() {
  // Do nothing
}
