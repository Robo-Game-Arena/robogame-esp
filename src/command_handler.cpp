#include <Arduino.h>

#include "arm.h"
#include "command_handler.h"
#include "config.h"
#include "motors.h"

static unsigned long lastDriveCommandMs = 0;
static char currentDriveCommand = COMMAND_STOP;
static bool rosConnected = false;

char getCurrentDriveCommand() {
  return currentDriveCommand;
}

void setRosConnected(bool connected) {
  if (rosConnected == connected) {
    return;
  }

  rosConnected = connected;

  if (connected) {
    Serial.println("ROS2 host connected and has control");
    return;
  }

  stopMotors();
  Serial.println("ROS2 host disconnected, gamepad has control");
}

bool rosHasControl() {
  return rosConnected;
}

static void handleCommand(char command) {
  switch (command) {
    case COMMAND_DRIVE_FORWARD:
      driveForward();
      currentDriveCommand = command;
      lastDriveCommandMs = millis();
      break;
    case COMMAND_DRIVE_BACKWARD:
      driveBackward();
      currentDriveCommand = command;
      lastDriveCommandMs = millis();
      break;
    case COMMAND_TURN_LEFT:
      turnLeft();
      currentDriveCommand = command;
      lastDriveCommandMs = millis();
      break;
    case COMMAND_TURN_RIGHT:
      turnRight();
      currentDriveCommand = command;
      lastDriveCommandMs = millis();
      break;
    case COMMAND_STOP:
      stopMotors();
      currentDriveCommand = command;
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

static void handleSpeedCommand(int8_t forwardPercent, int8_t turnPercent) {
  driveWithSpeed(forwardPercent, turnPercent);
  currentDriveCommand = COMMAND_SPEED;
  lastDriveCommandMs = millis();
}

// A speed command is V followed by two signed bytes, the forward speed and
// the turn, so those bytes are consumed here rather than read as commands.
void handleCommands(const char *commands, size_t length) {
  for (size_t index = 0; index < length; index++) {
    if (commands[index] != COMMAND_SPEED) {
      handleCommand(commands[index]);
      continue;
    }

    if (index + 2 >= length) {
      Serial.println("Speed command is missing its speed bytes");
      return;
    }

    handleSpeedCommand(
        (int8_t)commands[index + 1],
        (int8_t)commands[index + 2]);
    index += 2;
  }
}

void updateDriveTimeout() {
  if (rosConnected) {
    return;
  }

  if (!motorsAreRunning()) {
    return;
  }

  if (millis() - lastDriveCommandMs < DRIVE_COMMAND_TIMEOUT_MS) {
    return;
  }

  stopMotors();
  Serial.println("Drive command timed out, motors stopped");
}
