#include "Seeed_vl53l0x.h"
#include "DHT12.h"
#include <Wire.h> //The DHT12 uses I2C comunication.

DHT12 dht12; //Preset scale CELSIUS and ID 0x5c.
Seeed_vl53l0x VL53L0X;

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

uint8_t hitCounts = 0;
void setup() {
  analogReference(AR_DEFAULT); //set the reference voltage 1.1V,the distinguishability can up to 1mV.
  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  Serial.begin(115200);
  Serial.flush();
  delay(3000);
  Serial.print("\n\nContinuous ranging...");
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
}

void loop() {
  float ds;
  VL53L0X_RangingMeasurementData_t RangingMeasurementData;
  VL53L0X.PerformContinuousRangingMeasurement(&RangingMeasurementData);
  if (RangingMeasurementData.RangeMilliMeter >= 2000) {
    SERIAL.println("out of range");
  } else {
    ds = RangingMeasurementData.RangeMilliMeter / 10.0;
    if (ds < 45.0) {
      hitCounts += 1;
      if (hitCounts == 10) {
        SERIAL.print("distance: ");
        SERIAL.print(ds);
        SERIAL.println(" cm");
        analogWrite(A0, 255);
        Serial.println("Switching on!");
        Serial.print(" . T: "); Serial.print(dht12.readTemperature()); Serial.println(" C");
        Serial.print(" . H: "); Serial.print(dht12.readHumidity()); Serial.println("%");
      }
    } else {
      SERIAL.print("distance: ");
      SERIAL.print(ds);
      SERIAL.println(" cm");
      if (hitCounts > 0) {
        analogWrite(A0, 0);
        hitCounts = 0;
        Serial.println("Switching off!");
      }
    }
  }
  delay(100);
}
