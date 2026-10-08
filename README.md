# Robogame ESP32 Firmware

[![Platform](https://img.shields.io/badge/platform-ESP32%20WROOM-blue)](https://www.espressif.com/en/products/socs/esp32)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D)](https://docs.platformio.org/en/latest/frameworks/arduino.html)
[![Build](https://img.shields.io/badge/build-PlatformIO-FF7F00)](https://platformio.org/)
[![Bluetooth](https://img.shields.io/badge/bluetooth-BLE%20%2B%20Bluepad32-0082FC)](https://bluepad32.readthedocs.io/)

Firmware for the Robogame arena robots. Each board drives two or four
motors at proportional speeds and a three servo arm, and accepts commands
over BLE.

Commands normally arrive from the ROS2 bridge. A PS4 controller can also
pair directly to the board through Bluepad32 and drive it while no ROS2 host
is connected. Direct pairing is off by default, because a robot that is
switched on grabs any controller put into pairing mode nearby before the
ROS2 host can see it. Add `-D ALLOW_DIRECT_GAMEPAD_PAIRING=true` to a
robot's `build_flags` to turn it on.

## Commands

A directly paired gamepad drives with the left stick and turns with the
right stick, one direction at a time at full speed. Through ROS2 the same
sticks drive proportionally and the left stick also steers.

Commands are ASCII characters, plus a speed command carrying two binary
bytes, written to one BLE characteristic. See
[PROTOCOL.md](PROTOCOL.md) for the full command set, the transport details
and the failsafe timings.

## Build and flash

Every robot has its own environment so that each board advertises a unique
BLE name (`Robogame-1`, `Robogame-2`, and so on).

```
pio run -e robot_1 -t upload
pio run -e robot_2 -t upload
```

The BLE profile header `src/att_profile.h` is generated from
`src/att_profile.gatt` before each build by `tools/compile_gatt.py`, so it is
not checked in. That tool comes from BTstack and needs either OpenSSL or the
`pycryptodome` package on the build machine.

## Layout

| File | Purpose |
| --- | --- |
| `src/config.h` | Pins, BLE identifiers, limits and timeouts |
| `src/motors.cpp` | Motor driver outputs |
| `src/arm.cpp` | PCA9685 shoulder, elbow and gripper servos |
| `src/command_handler.cpp` | Command dispatch and control arbitration |
| `src/ble_service.cpp` | BTstack ATT server used by the ROS2 bridge |
| `src/gamepad_control.cpp` | Bluepad32 PS4 controller fallback |
| `src/serial_monitor.cpp` | Serial status line and debug console |

## Debugging

```
pio device monitor -e robot_1
```

The firmware prints a status line every two seconds showing the control
source, the active drive command, the left and right wheel speeds, the arm
angles and whether a gamepad is connected. Typing a command character drives
the robot straight from the monitor at full speed, which is useful for
checking wiring without any Bluetooth. Press `?` for help and `!` to silence
the status line.

## Checking directions

Lift the robot so the wheels spin freely, open the serial monitor, then:

1. Type `F`. Every driven wheel should roll the robot forward. If they all
   roll backwards, set `DRIVE_REVERSED` to `true` in `src/config.h`, or add
   `-D DRIVE_REVERSED=true` to that robot's `build_flags` to flip one robot.
2. Type `L`. The robot should turn left (counterclockwise from above): left
   wheels backwards, right wheels forwards. If it turns right, add
   `-D SWAP_LEFT_RIGHT=true` to that robot's `build_flags`.
3. Type `S` to stop.

A single wheel spinning the wrong way is a wiring difference on that motor.
Swap its two pins in `src/config.h`, or swap the motor's two wires.

## Wiring

| Signal | Header | GPIO |
| --- | --- | --- |
| Front left motor forward (A2) | J2 | 14 |
| Front left motor backward (A1) | J2 | 13 |
| Front right motor forward (B1) | J2 | 18 |
| Front right motor backward (B2) | J2 | 19 |
| Rear left motor forward (D2) | J3 | 33 |
| Rear left motor backward (D1) | J3 | 32 |
| Rear right motor forward (C1) | J3 | 16 |
| Rear right motor backward (C2) | J3 | 17 |
| Motor driver sleep | | 4 |
| Servo driver SDA | J4 | 21 |
| Servo driver SCL | J4 | 22 |

The motor driver only runs while the sleep pin is high. The firmware holds it
low until the direction pins are set, then raises it at the end of
`setupMotors()`.

Every motor pin is driven with PWM at 20 kHz, so speeds are proportional.
`MOTOR_MIN_DUTY_PERCENT` (35) is the slowest power the motors get, because
below it they stall rather than turn.

The J3 connector always follows J2, so a robot with a second pair of wheels
drives all four, and a robot without one is unaffected. The J5 attachment
pins (GPIO 25 and 26) are not used.
