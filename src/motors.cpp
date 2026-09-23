#include <Arduino.h>

#include "config.h"
#include "motors.h"

static bool motorsRunning = false;

static void setMotorPins(
    int leftForward,
    int leftBackward,
    int rightForward,
    int rightBackward) {
  digitalWrite(MOTOR_LEFT_FORWARD_PIN, leftForward);
  digitalWrite(MOTOR_LEFT_BACKWARD_PIN, leftBackward);
  digitalWrite(MOTOR_RIGHT_FORWARD_PIN, rightForward);
  digitalWrite(MOTOR_RIGHT_BACKWARD_PIN, rightBackward);

  motorsRunning = leftForward == HIGH || leftBackward == HIGH
      || rightForward == HIGH || rightBackward == HIGH;
}

void setupMotors() {
  pinMode(MOTOR_SLEEP_PIN, OUTPUT);
  digitalWrite(MOTOR_SLEEP_PIN, LOW);

  pinMode(MOTOR_LEFT_FORWARD_PIN, OUTPUT);
  pinMode(MOTOR_LEFT_BACKWARD_PIN, OUTPUT);
  pinMode(MOTOR_RIGHT_FORWARD_PIN, OUTPUT);
  pinMode(MOTOR_RIGHT_BACKWARD_PIN, OUTPUT);

  stopMotors();

  digitalWrite(MOTOR_SLEEP_PIN, HIGH);
}

void driveForward() {
  setMotorPins(HIGH, LOW, HIGH, LOW);
}

void driveBackward() {
  setMotorPins(LOW, HIGH, LOW, HIGH);
}

void turnLeft() {
  setMotorPins(LOW, HIGH, HIGH, LOW);
}

void turnRight() {
  setMotorPins(HIGH, LOW, LOW, HIGH);
}

void stopMotors() {
  setMotorPins(LOW, LOW, LOW, LOW);
}

bool motorsAreRunning() {
  return motorsRunning;
}
