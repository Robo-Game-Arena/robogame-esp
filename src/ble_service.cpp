#include <Arduino.h>
#include <NimBLEDevice.h>

#include "ble_service.h"
#include "command_handler.h"
#include "config.h"
#include "motors.h"

class RobotServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *server) override {
    Serial.println("Controller connected");
  }

  void onDisconnect(NimBLEServer *server) override {
    Serial.println("Controller disconnected, motors stopped");
    stopMotors();
    NimBLEDevice::startAdvertising();
  }
};

class RobotCommandCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *characteristic) override {
    std::string value = characteristic->getValue();

    if (value.empty()) {
      return;
    }

    handleCommands(value.c_str(), value.length());
  }
};

static RobotServerCallbacks serverCallbacks;
static RobotCommandCallbacks commandCallbacks;

static char deviceName[32] = {0};

const char *getBleDeviceName() {
  if (deviceName[0] == '\0') {
    snprintf(
        deviceName,
        sizeof(deviceName),
        "%s-%d",
        BLE_NAME_PREFIX,
        ROBOT_ID);
  }

  return deviceName;
}

void setupBleService() {
  NimBLEDevice::init(getBleDeviceName());

  NimBLEServer *server = NimBLEDevice::createServer();
  server->setCallbacks(&serverCallbacks);

  NimBLEService *service = server->createService(SERVICE_UUID);

  NimBLECharacteristic *commandCharacteristic = service->createCharacteristic(
      CHARACTERISTIC_UUID,
      NIMBLE_PROPERTY::READ |
      NIMBLE_PROPERTY::WRITE |
      NIMBLE_PROPERTY::WRITE_NR);

  commandCharacteristic->setCallbacks(&commandCallbacks);
  service->start();

  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->start();
}
