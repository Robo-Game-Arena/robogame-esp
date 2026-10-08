#include <Arduino.h>

#include "config.h"
#include "motors.h"

struct Motor {
  int forwardPin;
  int backwardPin;
  bool onLeft;
};

static const Motor motors[] = {
    {MOTOR_LEFT_FORWARD_PIN, MOTOR_LEFT_BACKWARD_PIN, true},
    {MOTOR_RIGHT_FORWARD_PIN, MOTOR_RIGHT_BACKWARD_PIN, false},
    {MOTOR_SECOND_LEFT_FORWARD_PIN, MOTOR_SECOND_LEFT_BACKWARD_PIN, true},
    {MOTOR_SECOND_RIGHT_FORWARD_PIN, MOTOR_SECOND_RIGHT_BACKWARD_PIN, false},
};

static const int MOTOR_COUNT = sizeof(motors) / sizeof(motors[0]);
static const int MAX_DUTY = (1 << MOTOR_PWM_RESOLUTION) - 1;

static int leftSpeed = 0;
static int rightSpeed = 0;

// Each motor pin has its own PWM channel: motor N uses channels 2N and 2N+1.
static int forwardChannel(int motor) {
  return motor * 2;
}

static int backwardChannel(int motor) {
  return motor * 2 + 1;
}

static int dutyForSpeed(int speedPercent) {
  int magnitude = min(abs(speedPercent), 100);

  if (magnitude == 0) {
    return 0;
  }

  int dutyPercent = MOTOR_MIN_DUTY_PERCENT
      + (100 - MOTOR_MIN_DUTY_PERCENT) * magnitude / 100;

  return MAX_DUTY * dutyPercent / 100;
}

static void setMotorSpeed(int motor, int speedPercent) {
  int duty = dutyForSpeed(speedPercent);

  ledcWrite(forwardChannel(motor), speedPercent > 0 ? duty : 0);
  ledcWrite(backwardChannel(motor), speedPercent < 0 ? duty : 0);
}

static void setSideSpeeds(int left, int right) {
  leftSpeed = constrain(left, -100, 100);
  rightSpeed = constrain(right, -100, 100);

  int direction = DRIVE_REVERSED ? -1 : 1;

  for (int motor = 0; motor < MOTOR_COUNT; motor++) {
    bool drivesLeft = motors[motor].onLeft != SWAP_LEFT_RIGHT;
    int speed = drivesLeft ? leftSpeed : rightSpeed;
    setMotorSpeed(motor, speed * direction);
  }
}

void setupMotors() {
  pinMode(MOTOR_SLEEP_PIN, OUTPUT);
  digitalWrite(MOTOR_SLEEP_PIN, LOW);

  for (int motor = 0; motor < MOTOR_COUNT; motor++) {
    ledcSetup(forwardChannel(motor), MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);
    ledcSetup(backwardChannel(motor), MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);
    ledcAttachPin(motors[motor].forwardPin, forwardChannel(motor));
    ledcAttachPin(motors[motor].backwardPin, backwardChannel(motor));
  }

  stopMotors();

  digitalWrite(MOTOR_SLEEP_PIN, HIGH);
}

void driveForward() {
  setSideSpeeds(100, 100);
}

void driveBackward() {
  setSideSpeeds(-100, -100);
}

void turnLeft() {
  setSideSpeeds(-100, 100);
}

void turnRight() {
  setSideSpeeds(100, -100);
}

void stopMotors() {
  setSideSpeeds(0, 0);
}

// Mixes a forward speed and a turn into left and right wheel speeds. Both
// are percentages from -100 to 100, and a positive turn turns left. When the
// mix asks a wheel for more than full speed, both wheels are scaled down
// together so the robot keeps the same curve.
void driveWithSpeed(int forwardPercent, int turnPercent) {
  int forward = constrain(forwardPercent, -100, 100);
  int turn = constrain(turnPercent, -100, 100);

  int left = forward - turn;
  int right = forward + turn;
  int largest = max(abs(left), abs(right));

  if (largest > 100) {
    left = left * 100 / largest;
    right = right * 100 / largest;
  }

  setSideSpeeds(left, right);
}

bool motorsAreRunning() {
  return leftSpeed != 0 || rightSpeed != 0;
}

int getLeftSpeed() {
  return leftSpeed;
}

int getRightSpeed() {
  return rightSpeed;
}
