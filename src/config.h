#pragma once

#ifndef ROBOT_ID
#define ROBOT_ID 1
#endif

#define BLE_NAME_PREFIX     "Robogame"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define MOTOR_SLEEP_PIN 4

// First motor connector (J2), the front wheels
#define MOTOR_LEFT_FORWARD_PIN   14
#define MOTOR_LEFT_BACKWARD_PIN  13
#define MOTOR_RIGHT_FORWARD_PIN  18
#define MOTOR_RIGHT_BACKWARD_PIN 19

// Second motor connector (J3), the rear wheels on a four motor robot. It
// always follows the first, so a robot with a second pair of wheels drives
// all four. Unconnected pins do no harm. Channel D (GPIO 32, 33) drives the
// left wheel and channel C (GPIO 16, 17) the right.
#define MOTOR_SECOND_LEFT_FORWARD_PIN   33
#define MOTOR_SECOND_LEFT_BACKWARD_PIN  32
#define MOTOR_SECOND_RIGHT_FORWARD_PIN  16
#define MOTOR_SECOND_RIGHT_BACKWARD_PIN 17

// Set to true if F drives the robot backwards in the serial monitor. It
// flips every drive command. If F is right but L turns the robot right,
// swap the left and right pins instead. A single robot can be flipped with
// -D DRIVE_REVERSED=true in its platformio.ini build_flags.
#ifndef DRIVE_REVERSED
#define DRIVE_REVERSED false
#endif

#define MOTOR_PWM_FREQUENCY  20000
#define MOTOR_PWM_RESOLUTION 8

// Slowest speed as a percentage of full power. Below this the motors stall,
// so small stick movements are raised to it.
#define MOTOR_MIN_DUTY_PERCENT 35

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

#define SERVO_SHOULDER_CHANNEL 1
#define SERVO_ELBOW_CHANNEL    2
#define SERVO_GRIPPER_CHANNEL  3

#define SERVO_PULSE_MIN 120
#define SERVO_PULSE_MAX 520

#define SHOULDER_START_ANGLE 90
#define ELBOW_START_ANGLE    90
#define ARM_ANGLE_STEP       5

#define GRIPPER_OPEN_ANGLE   105
#define GRIPPER_CLOSED_ANGLE 40
#define GRIPPER_TRAVEL_MS    400

#define DRIVE_COMMAND_TIMEOUT_MS 750

// Lets a PS4 controller pair straight to the robot, for driving without
// ROS2. Off by default, because a robot that is switched on grabs any
// controller put into pairing mode nearby before the ROS2 host can see it.
// Turn it on with -D ALLOW_DIRECT_GAMEPAD_PAIRING=true in build_flags.
#ifndef ALLOW_DIRECT_GAMEPAD_PAIRING
#define ALLOW_DIRECT_GAMEPAD_PAIRING false
#endif

#define GAMEPAD_DEADZONE         120
#define GAMEPAD_ARM_REPEAT_MS    150

#define SERIAL_STATUS_INTERVAL_MS 2000

#define COMMAND_DRIVE_FORWARD  'F'
#define COMMAND_DRIVE_BACKWARD 'B'
#define COMMAND_TURN_LEFT      'L'
#define COMMAND_TURN_RIGHT     'R'
#define COMMAND_STOP           'S'
#define COMMAND_SPEED          'V'
#define COMMAND_SHOULDER_UP    '+'
#define COMMAND_SHOULDER_DOWN  '-'
#define COMMAND_ELBOW_UP       'X'
#define COMMAND_ELBOW_DOWN     'H'
#define COMMAND_GRIPPER_OPEN   'o'
#define COMMAND_GRIPPER_CLOSE  'c'
