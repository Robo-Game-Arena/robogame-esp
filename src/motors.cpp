#include <Arduino.h>

#include "config.h"
#include "motors.h"

static bool motorsRunning = false;

static void setMotorPins(int in1, int in2, int in3, int in4) {
  digitalWrite(MOTOR_IN1_PIN, in1);
  digitalWrite(MOTOR_IN2_PIN, in2);
  digitalWrite(MOTOR_IN3_PIN, in3);
  digitalWrite(MOTOR_IN4_PIN, in4);

  motorsRunning = in1 == HIGH || in2 == HIGH || in3 == HIGH || in4 == HIGH;
}

void setupMotors() {
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);
  pinMode(MOTOR_IN3_PIN, OUTPUT);
  pinMode(MOTOR_IN4_PIN, OUTPUT);

  stopMotors();
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
