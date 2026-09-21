/*
  MKE-S17 AGS10 Basic Example
  
  This example demonstrates how to read TVOC (Total Volatile Organic Compounds)
  from the MakerEduVN MKE-S17 AGS10 I2C sensor.
  
  Hardware connections:
  - VCC to 3.3V/5V
  - GND to GND
  - SDA to SDA (A4 on Uno, D21 on ESP32)
  - SCL to SCL (A5 on Uno, D22 on ESP32)
  
  IMPORTANT: The AGS10 often requires a slow I2C clock speed (<= 15kHz).
  This example sets the I2C clock to 15kHz for stability.
*/

#include <Wire.h>
#include <AGS10.h>

AGS10 myAGS10;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("MKE-S17 AGS10 TVOC Sensor Test");

  // Initialize I2C Bus. 
  // For ESP32 or custom pins, you can pass pins here: Wire.begin(SDA_PIN, SCL_PIN);
  Wire.begin();

  // Set I2C clock speed to 15kHz to avoid communication issues with AGS10
  Wire.setClock(15000);

  // Initialize the sensor (uses the default Wire instance by default)
  myAGS10.begin(&Wire);
}

void loop() {
  // The sensor sets a "busy" flag when it is not ready to provide a new reading.
  // We can use isReady() to safely poll the sensor without getting corrupt data.
  if (myAGS10.isReady()) {
    // Read the TVOC value. Range is 0 to 99999 ppb.
    uint32_t tvoc = myAGS10.readTVOC();
    
    Serial.print("TVOC: ");
    Serial.print(tvoc);
    Serial.println(" ppb");
    Serial.println("--------------------");
    
  } else {
    Serial.println("Sensor warming up or busy, waiting...");
  }
  
  // Wait before polling again
  delay(1500);
}
