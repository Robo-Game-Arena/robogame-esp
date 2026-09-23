# Robogame ESP32 Firmware

[![Platform](https://img.shields.io/badge/platform-ESP32%20WROOM-blue)](https://www.espressif.com/en/products/socs/esp32)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D)](https://docs.platformio.org/en/latest/frameworks/arduino.html)
[![Build](https://img.shields.io/badge/build-PlatformIO-FF7F00)](https://platformio.org/)
[![Bluetooth](https://img.shields.io/badge/bluetooth-BLE%20%2B%20Bluepad32-0082FC)](https://bluepad32.readthedocs.io/)

Firmware for the Robogame arena robots. Each board drives two motors and a
three servo arm, and accepts single character commands over BLE.

Commands normally arrive from the ROS2 bridge. If no ROS2 host has sent a
command for 1.5 seconds, control falls back to a PS4 controller paired
directly to the board through Bluepad32.

## Commands

Commands are single ASCII characters sent to one BLE characteristic. See
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
source, the active drive command, the arm angles and whether a gamepad is
connected. Typing a command character drives the robot straight from the
monitor, which is useful for checking wiring without any Bluetooth. Press
`?` for help and `!` to silence the status line.

## Wiring

| Signal | Header | GPIO |
| --- | --- | --- |
| Left motor forward (A1) | J2 | 13 |
| Left motor backward (A2) | J2 | 14 |
| Right motor forward (B1) | J2 | 18 |
| Right motor backward (B2) | J2 | 19 |
| Motor driver sleep | | 4 |
| Servo driver SDA | J4 | 21 |
| Servo driver SCL | J4 | 22 |

The motor driver only runs while the sleep pin is high. The firmware holds it
low until the direction pins are set, then raises it at the end of
`setupMotors()`.

The J3 motor channels (C1 GPIO16, C2 GPIO17, D1 GPIO32, D2 GPIO33) and the
J5 attachment pins (GPIO 25 and 26) are not used.

If a motor spins the wrong way, swap its two pins in `src/config.h`.
