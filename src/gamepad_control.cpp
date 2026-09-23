#include <Arduino.h>
#include <Bluepad32.h>

#include "command_handler.h"
#include "config.h"
#include "gamepad_control.h"
#include "motors.h"

static ControllerPtr connectedController = nullptr;

static unsigned long lastArmCommandMs = 0;
static bool gripperOpenWasPressed = false;
static bool gripperCloseWasPressed = false;
static char lastDriveCommand = COMMAND_STOP;

static void onControllerConnected(ControllerPtr controller) {
  if (connectedController != nullptr) {
    Serial.println("Ignoring extra gamepad, one is already paired");
    return;
  }

  connectedController = controller;
  Serial.println("Gamepad connected");
}

static void onControllerDisconnected(ControllerPtr controller) {
  if (connectedController != controller) {
    return;
  }

  connectedController = nullptr;
  lastDriveCommand = COMMAND_STOP;
  stopMotors();

  Serial.println("Gamepad disconnected, motors stopped");
}

static char driveCommandFromSticks(int forwardAxis, int turnAxis) {
  if (forwardAxis < -GAMEPAD_DEADZONE) {
    return COMMAND_DRIVE_FORWARD;
  }

  if (forwardAxis > GAMEPAD_DEADZONE) {
    return COMMAND_DRIVE_BACKWARD;
  }

  if (turnAxis < -GAMEPAD_DEADZONE) {
    return COMMAND_TURN_LEFT;
  }

  if (turnAxis > GAMEPAD_DEADZONE) {
    return COMMAND_TURN_RIGHT;
  }

  return COMMAND_STOP;
}

static void sendCommand(char command) {
  handleCommands(&command, 1);
}

static void updateDriving(ControllerPtr controller) {
  char command = driveCommandFromSticks(
      controller->axisY(),
      controller->axisRX());

  if (command == lastDriveCommand) {
    if (command != COMMAND_STOP) {
      sendCommand(command);
    }
    return;
  }

  lastDriveCommand = command;
  sendCommand(command);
}

static void updateArmSteps(ControllerPtr controller) {
  if (millis() - lastArmCommandMs < GAMEPAD_ARM_REPEAT_MS) {
    return;
  }

  char command = 0;

  if (controller->y()) {
    command = COMMAND_SHOULDER_UP;
  } else if (controller->a()) {
    command = COMMAND_SHOULDER_DOWN;
  } else if (controller->b()) {
    command = COMMAND_ELBOW_UP;
  } else if (controller->x()) {
    command = COMMAND_ELBOW_DOWN;
  }

  if (command == 0) {
    return;
  }

  lastArmCommandMs = millis();
  sendCommand(command);
}

static void updateGripper(ControllerPtr controller) {
  bool openPressed = controller->r1();
  bool closePressed = controller->l1();

  if (openPressed && !gripperOpenWasPressed) {
    sendCommand(COMMAND_GRIPPER_OPEN);
  }

  if (closePressed && !gripperCloseWasPressed) {
    sendCommand(COMMAND_GRIPPER_CLOSE);
  }

  gripperOpenWasPressed = openPressed;
  gripperCloseWasPressed = closePressed;
}

void setupGamepadControl() {
  BP32.setup(&onControllerConnected, &onControllerDisconnected);
  BP32.enableVirtualDevice(false);
}

bool gamepadIsConnected() {
  return connectedController != nullptr && connectedController->isConnected();
}

void updateGamepadControl() {
  BP32.update();

  if (rosHasControl()) {
    lastDriveCommand = COMMAND_STOP;
    return;
  }

  if (!gamepadIsConnected()) {
    return;
  }

  updateDriving(connectedController);
  updateArmSteps(connectedController);
  updateGripper(connectedController);
}
