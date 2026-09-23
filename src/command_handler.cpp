#include <Arduino.h>

#include "arm.h"
#include "command_handler.h"
#include "config.h"
#include "motors.h"

static unsigned long lastDriveCommandMs = 0;
static unsigned long lastRosCommandMs = 0;
static bool rosWasInControl = false;

void noteRosActivity() {
  lastRosCommandMs = millis();

  if (!rosWasInControl) {
    rosWasInControl = true;
    Serial.println("ROS2 host has control");
  }
}

bool rosHasControl() {
  if (lastRosCommandMs == 0) {
    return false;
  }

  if (millis() - lastRosCommandMs < ROS_CONTROL_TIMEOUT_MS) {
    return true;
  }

  if (rosWasInControl) {
    rosWasInControl = false;
    Serial.println("ROS2 host went quiet, gamepad has control");
  }

  return false;
}

static void handleCommand(char command) {
  switch (command) {
    case COMMAND_DRIVE_FORWARD:
      driveForward();
      lastDriveCommandMs = millis();
      break;
    case COMMAND_DRIVE_BACKWARD:
      driveBackward();
      lastDriveCommandMs = millis();
      break;
    case COMMAND_TURN_LEFT:
      turnLeft();
      lastDriveCommandMs = millis();
      break;
    case COMMAND_TURN_RIGHT:
      turnRight();
      lastDriveCommandMs = millis();
      break;
    case COMMAND_STOP:
      stopMotors();
      lastDriveCommandMs = millis();
      break;
    case COMMAND_SHOULDER_UP:
      moveShoulder(ARM_ANGLE_STEP);
      break;
    case COMMAND_SHOULDER_DOWN:
      moveShoulder(-ARM_ANGLE_STEP);
      break;
    case COMMAND_ELBOW_UP:
      moveElbow(ARM_ANGLE_STEP);
      break;
    case COMMAND_ELBOW_DOWN:
      moveElbow(-ARM_ANGLE_STEP);
      break;
    case COMMAND_GRIPPER_OPEN:
      openGripper();
      break;
    case COMMAND_GRIPPER_CLOSE:
      closeGripper();
      break;
    default:
      Serial.print("Unknown command: ");
      Serial.println(command);
      break;
  }
}

void handleCommands(const char *commands, size_t length) {
  for (size_t index = 0; index < length; index++) {
    handleCommand(commands[index]);
  }
}

void updateDriveTimeout() {
  if (!motorsAreRunning()) {
    return;
  }

  if (millis() - lastDriveCommandMs < DRIVE_COMMAND_TIMEOUT_MS) {
    return;
  }

  stopMotors();
  Serial.println("Drive command timed out, motors stopped");
}
