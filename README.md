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

| Character | Action |
| --- | --- |
| `F` `B` `L` `R` `S` | Forward, back, left, right, stop |
| `+` `-` | Shoulder up, shoulder down |
| `X` `H` | Elbow up, elbow down |
| `o` `c` | Gripper open, gripper close |

## Build and flash

Every robot has its own environment so that each board advertises a unique
BLE name (`Robogame-1`, `Robogame-2`, and so on).

```
pio run -e robot_1 -t upload
pio run -e robot_2 -t upload
```

The BLE profile header `src/att_profile.h` is generated from
`src/att_profile.gatt` by a pre-build script, so it is not checked in.

## Layout

| File | Purpose |
| --- | --- |
| `src/config.h` | Pins, BLE identifiers, limits and timeouts |
| `src/motors.cpp` | Motor driver outputs |
| `src/arm.cpp` | PCA9685 shoulder, elbow and gripper servos |
| `src/command_handler.cpp` | Command dispatch and control arbitration |
| `src/ble_service.cpp` | BTstack ATT server used by the ROS2 bridge |
| `src/gamepad_control.cpp` | Bluepad32 PS4 controller fallback |

## Wiring

Motor pins are GPIO 25, 26, 27 and 14. The PCA9685 uses the default I2C pins,
GPIO 21 (SDA) and GPIO 22 (SCL). Adjust these in `src/config.h` to match the
board.
