# Motor test (Arduino)

First step of the build: taking control of the plush's original DC motor with an Arduino, before any AI is involved.

The toy uses a single DC motor with a cam gearbox to animate the head, ears, eyelids and arm. This sketch drives that motor through an L293D H-bridge and lets you control it from the Serial Monitor.

## What you need

| Part | Notes |
| --- | --- |
| Arduino Mega 2560 | Any Arduino with PWM pins works; adjust pin numbers if needed |
| L293D | Dual H-bridge motor driver (only channel 1 is used) |
| Breadboard power module | Set to 5 V, powered by a 9 V adapter |
| Breadboard and jumper wires | |
| 100 nF ceramic capacitor (104) | Across the motor wires, to reduce electrical noise |
| USB cable | To upload the sketch and use the Serial Monitor |

## Preparing the toy

Unplug the 2-wire motor connector (red and black) from the original board behind the eyes. Nothing needs to be cut.

Do not open the gearbox itself: the cams are timed together and are hard to realign.

## Wiring

With the L293D notch facing up, pin 1 is top left. Pins 1 to 8 run down the left side, pins 9 to 16 run up the right side. Place the chip across the center gap of the breadboard.

| L293D pin | Connect to |
| --- | --- |
| 1 (EN1) | Arduino D5 |
| 2 (IN1) | Arduino D7 |
| 3 (OUT1) | Motor wire |
| 4, 5 | GND |
| 6 (OUT2) | Other motor wire |
| 7 (IN2) | Arduino D8 |
| 8 (VCC2, motor supply) | 5 V from the power module |
| 12, 13 | GND |
| 16 (VCC1, logic supply) | 5 V |

Connect an Arduino GND pin to the breadboard GND rail. Without a common ground, nothing works.

Do not power the motor from the Arduino 5 V pin: current spikes can reset the board.

Motor polarity does not matter. Swapping the two wires only reverses the direction.

## Usage

1. Open `motor.ino` in the Arduino IDE.
2. Select **Tools > Board > Arduino Mega or Mega 2560** and the matching port.
3. Upload the sketch.
4. Open the Serial Monitor at **9600 baud**.
5. Type a command and press Enter:

| Command | Action |
| --- | --- |
| `f` | Run forward |
| `b` | Run backward |
| `s` | Stop |
| `+` | Speed up |
| `-` | Slow down |

The motor stops automatically after 3 seconds, so a stalled mechanism never overheats the motor or the L293D.

## Troubleshooting

- **Nothing moves**: check the common ground, then the 5 V on L293D pins 8 and 16.
- **The motor hums but barely moves**: increase the speed with `+`. The L293D drops about 2 V, so the motor gets less than 5 V.
- **The board does not show up in the IDE**: try another USB cable; some cables only carry power.
- **The motor sounds like it is forcing**: type `s` immediately.

## Next step

The L293D is fine for testing, but the final build uses an ESP32-S3 with a DRV8833 driver, which loses much less voltage.

