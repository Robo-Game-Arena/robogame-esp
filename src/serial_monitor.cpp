#include <Arduino.h>

#include "arm.h"
#include "ble_service.h"
#include "command_handler.h"
#include "config.h"
#include "gamepad_control.h"
#include "motors.h"
#include "serial_monitor.h"

static unsigned long lastStatusMs = 0;
static bool statusEnabled = true;

static const char *controlSourceName() {
  if (rosHasControl()) {
    return "ros2";
  }

  if (gamepadIsConnected()) {
    return "gamepad";
  }

  return "none";
}

static void printHelp() {
  Serial.println();
  Serial.println("Serial debug console");
  Serial.println("  F B L R S   drive forward, back, left, right, stop");
  Serial.println("  + -         shoulder up, shoulder down");
  Serial.println("  X H         elbow up, elbow down");
  Serial.println("  o c         gripper open, gripper close");
  Serial.println("  ?           show this help");
  Serial.println("  !           toggle the periodic status line");
  Serial.println();
  Serial.println("Typed commands run immediately. While a ROS2 host has");
  Serial.println("control its keepalive overwrites drive commands.");
  Serial.println();
}

static void printStatus() {
  Serial.print("[");
  Serial.print(millis() / 1000.0, 1);
  Serial.print("s] name=");
  Serial.print(getBleDeviceName());
  Serial.print(" control=");
  Serial.print(controlSourceName());
  Serial.print(" drive=");
  Serial.print(getCurrentDriveCommand());
  Serial.print(" motors=");
  Serial.print(motorsAreRunning() ? "on" : "off");
  Serial.print(" gamepad=");
  Serial.print(gamepadIsConnected() ? "yes" : "no");
  Serial.print(" shoulder=");
  Serial.print(getShoulderAngle());
  Serial.print(" elbow=");
  Serial.print(getElbowAngle());
  Serial.print(" gripper=");
  Serial.println(gripperIsPowered() ? "moving" : "idle");
}

static void handleTypedCharacter(char character) {
  if (character == '\n' || character == '\r' || character == ' ') {
    return;
  }

  if (character == '?') {
    printHelp();
    return;
  }

  if (character == '!') {
    statusEnabled = !statusEnabled;
    Serial.print("Status line ");
    Serial.println(statusEnabled ? "enabled" : "disabled");
    return;
  }

  Serial.print("Serial command: ");
  Serial.println(character);

  handleCommands(&character, 1);
}

void setupSerialMonitor() {
  printHelp();
  lastStatusMs = millis();
}

void updateSerialMonitor() {
  while (Serial.available() > 0) {
    handleTypedCharacter((char)Serial.read());
  }

  if (!statusEnabled) {
    return;
  }

  if (millis() - lastStatusMs < SERIAL_STATUS_INTERVAL_MS) {
    return;
  }

  lastStatusMs = millis();
  printStatus();
}
