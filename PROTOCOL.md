# Robot Command Protocol

The wire format between the ROS2 bridge (`microcontroller_node`) and the
robot firmware. Both sides read these characters from a single shared
definition: `src/config.h` on the firmware and `robot_commands.py` in the
ROS2 package.

## Transport

| Item | Value |
| --- | --- |
| Service UUID | `4fafc201-1fb5-459e-8fcc-c5c9c331914b` |
| Characteristic UUID | `beb5483e-36e1-4688-b7f5-ea07361b26a8` |
| Properties | Read, write, write without response |
| Write type used by ROS2 | Write without response |
| Payload | One or more ASCII characters, no terminator |

The robot advertises the service UUID, and its name (`Robogame-1`,
`Robogame-2`, and so on) in the scan response. The bridge scans for the
service UUID rather than a fixed name, so any robot on the arena is found
without configuration.

Each write may carry several characters. The firmware applies them in order,
so `Fo` drives forward and opens the gripper in one write.

## Commands

| Character | Action | Effect on the robot |
| --- | --- | --- |
| `F` | Drive forward | Both motors forward until another drive command |
| `B` | Drive backward | Both motors backward |
| `L` | Turn left | Left motor back, right motor forward |
| `R` | Turn right | Left motor forward, right motor back |
| `S` | Stop | Both motors off |
| `+` | Shoulder up | Shoulder servo moves 5 degrees, clamped to 0 - 180 |
| `-` | Shoulder down | Shoulder servo moves 5 degrees down |
| `X` | Elbow up | Elbow servo moves 5 degrees |
| `H` | Elbow down | Elbow servo moves 5 degrees down |
| `o` | Gripper open | Gripper to 105 degrees, then power cut after 400 ms |
| `c` | Gripper close | Gripper to 40 degrees, then power cut after 400 ms |

Unknown characters are logged over serial and ignored.

Gripper power is cut once the claw has travelled, which keeps the servo from
stalling and overheating while holding a token.

## When ROS2 sends each command

Drive commands come from `/robot_<id>/cmd_vel`. The bridge converts a Twist
into one character, checking linear motion first:

| Condition | Command |
| --- | --- |
| `linear.x` greater than `linear_threshold` | `F` |
| `linear.x` less than negative `linear_threshold` | `B` |
| `angular.z` greater than `angular_threshold` | `L` |
| `angular.z` less than negative `angular_threshold` | `R` |
| Otherwise | `S` |

Both thresholds default to 0.1. Diagonal motion is not mixed, linear motion
wins over turning.

Arm commands come from `/robot_<id>/arm_command` as a `std_msgs/String`
holding one character. Anything outside the arm command set is rejected by
the bridge and never reaches the robot.

## Timing and failsafes

The bridge resends the current drive command every 250 ms, including `S`.
This serves two purposes.

| Timeout | Value | Behaviour |
| --- | --- | --- |
| `DRIVE_COMMAND_TIMEOUT_MS` | 750 ms | Motors stop if no drive command arrives |
| `ROS_CONTROL_TIMEOUT_MS` | 1500 ms | Control falls back to the PS4 gamepad |

So a dropped BLE link stops the motors within 750 ms, and a ROS2 host that
goes away hands control to a directly paired gamepad after 1.5 seconds. When
the bridge starts sending again it immediately reclaims control.

The firmware also stops the motors when a BLE central disconnects and when a
paired gamepad disconnects.
