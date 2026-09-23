#include <Arduino.h>
#include <Bluepad32.h>
#include <btstack.h>

#include "att_profile.h"
#include "ble_service.h"
#include "command_handler.h"
#include "config.h"

#define ROBOT_COMMAND_VALUE_HANDLE \
  ATT_CHARACTERISTIC_BEB5483E_36E1_4688_B7F5_EA07361B26A8_01_VALUE_HANDLE

static TaskHandle_t bleSetupTaskHandle;
static btstack_context_callback_registration_t btstackMainThreadCallback;

static char deviceName[32] = {0};
static uint8_t scanResponseData[32] = {0};
static uint8_t scanResponseLength = 0;

static const uint8_t advertisingData[] = {
    0x02, 0x01, 0x06,
    0x11, 0x07,
    0x4B, 0x91, 0x31, 0xC3, 0xC9, 0xC5, 0xCC, 0x8F,
    0x9E, 0x45, 0xB5, 0x1F, 0x01, 0xC2, 0xAF, 0x4F,
};

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

static void buildScanResponseData() {
  const char *name = getBleDeviceName();
  uint8_t nameLength = (uint8_t)strlen(name);

  scanResponseData[0] = nameLength + 1;
  scanResponseData[1] = 0x09;
  memcpy(&scanResponseData[2], name, nameLength);

  scanResponseLength = nameLength + 2;
}

static uint16_t attReadCallback(
    hci_con_handle_t connectionHandle,
    uint16_t attHandle,
    uint16_t offset,
    uint8_t *buffer,
    uint16_t bufferSize) {
  UNUSED(connectionHandle);
  UNUSED(attHandle);
  UNUSED(offset);
  UNUSED(buffer);
  UNUSED(bufferSize);

  return 0;
}

static int attWriteCallback(
    hci_con_handle_t connectionHandle,
    uint16_t attHandle,
    uint16_t transactionMode,
    uint16_t offset,
    uint8_t *buffer,
    uint16_t bufferSize) {
  UNUSED(connectionHandle);
  UNUSED(transactionMode);
  UNUSED(offset);

  if (attHandle != ROBOT_COMMAND_VALUE_HANDLE) {
    return 0;
  }

  if (bufferSize == 0) {
    return 0;
  }

  handleCommands((const char *)buffer, bufferSize);

  return 0;
}

static void attPacketHandler(
    uint8_t packetType,
    uint16_t channel,
    uint8_t *packet,
    uint16_t size) {
  UNUSED(channel);
  UNUSED(size);

  if (packetType != HCI_EVENT_PACKET) {
    return;
  }

  switch (hci_event_packet_get_type(packet)) {
    case ATT_EVENT_CONNECTED:
      setRosConnected(true);
      break;
    case ATT_EVENT_DISCONNECTED:
      setRosConnected(false);
      break;
    default:
      break;
  }
}

static void startAttServer(void *parameter) {
  UNUSED(parameter);

  att_server_init(profile_data, attReadCallback, attWriteCallback);
  att_server_register_packet_handler(attPacketHandler);

  bd_addr_t nullAddress;
  memset(nullAddress, 0, sizeof(nullAddress));

  gap_advertisements_set_params(
      0x0030,
      0x0030,
      0,
      0,
      nullAddress,
      0x07,
      0x00);

  gap_advertisements_set_data(
      (uint8_t)sizeof(advertisingData),
      (uint8_t *)advertisingData);

  gap_scan_response_set_data(scanResponseLength, scanResponseData);
  gap_advertisements_enable(1);
}

static void bleSetupTask(void *parameter) {
  UNUSED(parameter);

  btstackMainThreadCallback.callback = &startAttServer;
  btstack_run_loop_execute_on_main_thread(&btstackMainThreadCallback);

  vTaskDelete(bleSetupTaskHandle);
}

void setupBleService() {
  buildScanResponseData();

  xTaskCreatePinnedToCore(
      bleSetupTask,
      "ble_setup",
      10000,
      NULL,
      0,
      &bleSetupTaskHandle,
      0);
}
