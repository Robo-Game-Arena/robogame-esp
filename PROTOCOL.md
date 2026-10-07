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
| `V` + 2 bytes | Drive at a speed | See below |
| `F` | Drive forward | Both sides forward at full speed until another drive command |
| `B` | Drive backward | Both sides backward at full speed |
| `L` | Turn left | Left side back, right side forward |
| `R` | Turn right | Left side forward, right side back |
| `S` | Stop | All motors off |
| `+` | Shoulder up | Shoulder servo moves 5 degrees, clamped to 0 - 180 |
| `-` | Shoulder down | Shoulder servo moves 5 degrees down |
| `X` | Elbow up | Elbow servo moves 5 degrees |
| `H` | Elbow down | Elbow servo moves 5 degrees down |
| `o` | Gripper open | Gripper to 105 degrees, then power cut after 400 ms |
| `c` | Gripper close | Gripper to 40 degrees, then power cut after 400 ms |

Unknown characters are logged over serial and ignored.

Gripper power is cut once the claw has travelled, which keeps the servo from
stalling and overheating while holding a token.

### Speed command

`V` is followed by two signed bytes: the forward speed, then the turn, each a
percentage from -100 to 100. A positive turn turns left (counterclockwise
from above), and values outside the range are clamped. The robot mixes them
into wheel speeds:

```
left  = forward - turn
right = forward + turn
```

If either side comes out above 100, both are scaled down by the same factor,
so the robot keeps the same curve at the highest speed it can manage. For
example `V 50 0` drives straight at half speed, `V 0 100` spins left in
place, and `V 100 30` drives forward while curving left.

The speed bytes are binary, so `V` cannot be typed in the serial monitor.
Commands after the two bytes in the same write still run, so a speed and an
arm command can share one write.

Every drive command, `V` and the letters alike, drives both motor
connectors, and is flipped when the firmware is built with `DRIVE_REVERSED`.

## When ROS2 sends each command

Drive commands come from `/robot_<id>/cmd_vel`. By default the bridge sends
a speed command: `linear.x` divided by `full_linear_speed` (0.5) is the
forward speed and `angular.z` divided by `full_angular_speed` (1.5) is the
turn, both as percentages.

Launched with `drive_mode:=letters`, the bridge instead converts a Twist
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

The bridge writes to each robot at most 20 times a second. A new drive
command replaces one that has not been sent yet, while arm commands are kept
in order and sent in the same write as the drive command.

## Control handover

The robot takes commands from the ROS2 bridge whenever one is connected, and
from a directly paired PS4 gamepad otherwise. This is decided by the BLE
connection itself, not by a timeout.

The firmware registers an ATT packet handler, so `ATT_EVENT_CONNECTED` and
`ATT_EVENT_DISCONNECTED` mark the bridge as present or gone. Only centrals
connecting to the robot raise these events, so a BLE gamepad that the robot
connects out to is never mistaken for the bridge.

| State | Who drives |
| --- | --- |
| Bridge connected | ROS2, gamepad input ignored |
| Bridge not connected, gamepad paired | Gamepad |
| Neither | Nobody, motors stay stopped |

Gamepads can only pair when the firmware is built with
`ALLOW_DIRECT_GAMEPAD_PAIRING`. By default the robot forgets any paired
gamepad at startup and accepts no new ones, so only the bridge drives it.

Motors stop the moment the bridge disconnects, and again when a paired
gamepad disconnects.

Characters typed into the serial console run immediately whatever the state
is. While the bridge is connected its keepalive overwrites typed drive
commands within 250 ms, so stop the bridge to drive from the monitor.

## Timing and failsafes

| Setting | Value | Behaviour |
| --- | --- | --- |
| Bridge keepalive | 250 ms | Resends the current drive command, including `S` |
| `DRIVE_COMMAND_TIMEOUT_MS` | 750 ms | Motors stop if no drive command arrives |

The drive timeout applies only while the bridge is disconnected, so it
guards the gamepad path. A drive command from the bridge holds until the
next one arrives.

The keepalive is redundancy rather than a heartbeat. Writes are sent without
a response, so they are never acknowledged, and resending means a single
lost write cannot leave the robot driving in the wrong direction.

A bridge that stops responding without dropping its BLE connection leaves
the motors running, because nothing on either side is watching a clock. A
crash, an unplugged adapter or a robot going out of range all drop the
connection and stop the motors.
