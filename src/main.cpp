#include <Arduino.h>

#include "arm.h"
#include "ble_service.h"
#include "command_handler.h"
#include "config.h"
#include "motors.h"

void setup() {
  Serial.begin(115200);
  delay(2000);

  setupMotors();
  setupArm();
  setupBleService();

  Serial.print("Advertising as ");
  Serial.println(getBleDeviceName());
}

void loop() {
  updateArm();
  updateDriveTimeout();
  delay(20);
}
