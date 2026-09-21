/*
  MKE-S17 AGS10 Advanced Example
  
  This example demonstrates advanced features of the MakerEduVN MKE-S17 AGS10 sensor:
  - Reading the firmware version
  - Reading the raw resistance of the MOX sensor
  - Performing a factory zero-point calibration (resetting to factory defaults)
  
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
  
  Serial.println("\nMKE-S17 AGS10 Advanced Test");

  // Initialize I2C Bus. 
  // For ESP32 or custom pins, you can pass pins here: Wire.begin(SDA_PIN, SCL_PIN);
  Wire.begin();

  // Set I2C clock speed to 15kHz to avoid communication issues with AGS10
  Wire.setClock(15000);

  // Initialize the sensor
  myAGS10.begin(&Wire);

  // 1. Read Firmware Version
  Serial.print("Reading Firmware Version: ");
  int version = myAGS10.readVersion();
  Serial.println(version);
  
  // 2. Factory Zero-Point Calibration (Optional)
  // WARNING: This restores the factory 0 ppb calibration point. 
  // You normally DO NOT need to run this unless the sensor is giving abnormal readings.
  // Uncomment the lines below to run the calibration once during setup.
  /*
  Serial.println("Performing Factory Zero-Point Calibration...");
  myAGS10.calibrateFact();
  delay(100); // Give it a short moment to process
  Serial.println("Calibration command sent.");
  */
  
  Serial.println("Setup complete. Starting main loop...\n");
}

void loop() {
  // Check if sensor is ready (RDY bit = 0)
  // Note: The library has a built-in 1.5s anti-spam cache. 
  // Calling isReady() repeatedly is perfectly safe.
  if (myAGS10.isReady()) {
    
    // Read the TVOC value (ppb)
    uint32_t tvoc = myAGS10.readTVOC();
    Serial.print("TVOC: ");
    Serial.print(tvoc);
    Serial.println(" ppb");
    
    // Read the raw resistance of the MOX sensor
    // Unit: 0.1 kOhm. E.g., 100000 = 10,000 kOhm = 10 MOhm
    // This value decreases when the sensor detects VOC gases.
    uint32_t resist = myAGS10.readResist();
    Serial.print("Raw Resistance: ");
    Serial.print(resist);
    Serial.println(" * 0.1 kOhm");
    
    Serial.println("--------------------");
    
  } else {
    // If the sensor was just powered on, it needs 120s of pre-heating.
    Serial.println("Sensor warming up or busy, waiting...");
  }
  
  // Wait before polling again. 
  // The library prevents sending I2C reads faster than 1.5s anyway.
  delay(2000);
}
