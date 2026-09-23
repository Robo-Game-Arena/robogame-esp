#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#include "arm.h"
#include "config.h"

static Adafruit_PWMServoDriver servoDriver = Adafruit_PWMServoDriver();

static int shoulderAngle = SHOULDER_START_ANGLE;
static int elbowAngle = ELBOW_START_ANGLE;

static bool gripperIsPowered = false;
static unsigned long gripperPoweredAtMs = 0;

static uint16_t angleToPulse(int angle) {
  return map(angle, 0, 180, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
}

static void setServoAngle(int channel, int angle) {
  servoDriver.setPWM(channel, 0, angleToPulse(angle));
}

static void cutServoPower(int channel) {
  servoDriver.setPWM(channel, 0, 0);
}

static void startGripperMove(int angle) {
  setServoAngle(SERVO_GRIPPER_CHANNEL, angle);
  gripperIsPowered = true;
  gripperPoweredAtMs = millis();
}

void setupArm() {
  Wire.end();
  delay(50);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  servoDriver.begin();
  servoDriver.setPWMFreq(50);

  setServoAngle(SERVO_SHOULDER_CHANNEL, shoulderAngle);
  setServoAngle(SERVO_ELBOW_CHANNEL, elbowAngle);

  startGripperMove(GRIPPER_OPEN_ANGLE);
}

void updateArm() {
  if (!gripperIsPowered) {
    return;
  }

  if (millis() - gripperPoweredAtMs < GRIPPER_TRAVEL_MS) {
    return;
  }

  cutServoPower(SERVO_GRIPPER_CHANNEL);
  gripperIsPowered = false;
}

void moveShoulder(int angleStep) {
  shoulderAngle = constrain(shoulderAngle + angleStep, 0, 180);
  setServoAngle(SERVO_SHOULDER_CHANNEL, shoulderAngle);
}

void moveElbow(int angleStep) {
  elbowAngle = constrain(elbowAngle + angleStep, 0, 180);
  setServoAngle(SERVO_ELBOW_CHANNEL, elbowAngle);
}

void openGripper() {
  startGripperMove(GRIPPER_OPEN_ANGLE);
}

void closeGripper() {
  startGripperMove(GRIPPER_CLOSED_ANGLE);
}
