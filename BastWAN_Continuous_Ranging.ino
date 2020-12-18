#include "Seeed_vl53l0x.h"
#include "DHT12.h"
#include <Wire.h> //The DHT12 uses I2C comunication.
#include <SparkFunMLX90614.h> //Click here to get the library: http://librarymanager/All#Qwiic_IR_Thermometer by SparkFun

DHT12 dht12; //Preset scale CELSIUS and ID 0x5c.
Seeed_vl53l0x VL53L0X;
IRTherm therm; // Create an IRTherm object to interact with throughout

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

uint8_t hitCounts = 0;
void setup() {
  analogReference(AR_DEFAULT); //set the reference voltage 1.1V,the distinguishability can up to 1mV.
  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  SERIAL.begin(115200);
  SERIAL.flush();
  delay(3000);
  SERIAL.println("\n\nContinuous ranging...");
  pinMode(A0, OUTPUT);
  analogWrite(A0, 0);
  Status = VL53L0X.VL53L0X_common_init();
  if (VL53L0X_ERROR_NONE != Status) {
    SERIAL.println("start vl53l0x mesurement failed!");
    VL53L0X.print_pal_error(Status);
    while (1);
  }
  VL53L0X.VL53L0X_continuous_ranging_init();
  if (VL53L0X_ERROR_NONE != Status) {
    SERIAL.println("start vl53l0x mesurement failed!");
    VL53L0X.print_pal_error(Status);
    while (1);
  }
  Wire.begin(); //Join I2C bus
  SERIAL.print("IR thermometer ");
  if (therm.begin() == false) {
    // Initialize thermal IR sensor
    SERIAL.println("did not acknowledge! Freezing!");
    while (1);
  }
  SERIAL.println("acknowledged.");
  therm.setUnit(TEMP_C); // Set the library's units to Farenheit
  if (therm.readID()) {
    // Read from the ID registers
    // If the read succeeded, print the ID:
    Serial.println("ID: 0x" + String(therm.getIDH(), HEX) + String(therm.getIDL(), HEX));
  }
  Serial.println(String(therm.readEmissivity()));
}

void loop() {
  float ds;
  VL53L0X_RangingMeasurementData_t RangingMeasurementData;
  VL53L0X.PerformContinuousRangingMeasurement(&RangingMeasurementData);
  if (RangingMeasurementData.RangeMilliMeter >= 2000) {
    //SERIAL.println("out of range");
  } else {
    ds = RangingMeasurementData.RangeMilliMeter / 10.0;
    if (ds < 15.0) {
      hitCounts += 1;
      if (hitCounts == 10) {
        SERIAL.print("distance: ");
        SERIAL.print(ds);
        SERIAL.println(" cm");
        analogWrite(A0, 255);
        SERIAL.println("\nSwitching on!");
        SERIAL.print(" . T: "); SERIAL.print(dht12.readTemperature()); SERIAL.println(" C");
        SERIAL.print(" . H: "); SERIAL.print(dht12.readHumidity()); SERIAL.println("%");
        if (therm.read()) {
          // On success, read() will return 1, on fail 0.
          // Use the object() and ambient() functions to grab the object and ambient
          // temperatures.
          // They'll be floats, calculated out to the unit you set with setUnit().
          Serial.print("Object: " + String(therm.object(), 2));
          Serial.println("* C");
          Serial.print("Ambient: " + String(therm.ambient(), 2));
          Serial.println("* C");
        }
      }
    } else {
      // SERIAL.print("distance: ");
      // SERIAL.print(ds);
      // SERIAL.println(" cm");
      if (hitCounts > 0) {
        analogWrite(A0, 0);
        hitCounts = 0;
        SERIAL.println("\nSwitching off!");
      }
    }
  }
  delay(100);
}
