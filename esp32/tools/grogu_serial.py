#!/usr/bin/env python3
"""Send bring-up commands to the ESP32 and print what it answers.

Examples:
    python3 grogu_serial.py --reset          # reset the board, print the boot log and help menu
    python3 grogu_serial.py h                # print the help menu
    python3 grogu_serial.py f --wait 4       # run the motor and wait for the auto-stop
    python3 grogu_serial.py m --wait 5 m     # mic meter on for 5 s, then off

Each command is one key from the bring-up firmware (see ../README.md).
"""

import argparse
import sys
import time

import serial
from serial.tools import list_ports

BAUD = 115200

# USB-serial bridges used on ESP32 boards (CP210x, CH340/CH343/CH9102) and the
# native ESP32-S3 USB port.
BRIDGE_VIDS = {0x10C4, 0x1A86, 0x303A}

# Last line of the help menu, printed at the end of setup().
HELP_END = "this help"
# The firmware waits 1.5 s, then calibrates touch before printing the help.
BOOT_TIMEOUT = 5.0


def find_port():
    ports = [p for p in list_ports.comports() if p.vid in BRIDGE_VIDS]
    if not ports:
        sys.exit("No ESP32 serial port found. Check the cable (data, not charge-only) "
                 "and the USB port, or pass --port.")
    if len(ports) > 1:
        names = ", ".join(p.device for p in ports)
        sys.exit(f"Several serial ports found ({names}). Pass --port.")
    return ports[0].device


def reset_board(ser):
    # Same sequence as esptool: RTS drives EN, DTR drives GPIO0 (kept high to boot normally).
    ser.dtr = False
    ser.rts = True
    time.sleep(0.1)
    ser.rts = False


def read_for(ser, seconds, until=None):
    """Print incoming data for `seconds`, or until the `until` text shows up.

    Returns everything read."""
    seen = ""
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        data = ser.read(ser.in_waiting or 1)
        if data:
            text = data.decode("utf-8", errors="replace")
            sys.stdout.write(text)
            sys.stdout.flush()
            seen += text
            if until and until in seen:
                break
    return seen


def wait_for_boot(ser, seen=""):
    """Wait for the help menu the firmware prints once it has booted."""
    if HELP_END not in seen:
        read_for(ser, BOOT_TIMEOUT, until=HELP_END)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("commands", nargs="*", help="command keys to send, in order")
    parser.add_argument("--port", help="serial port (auto-detected by default)")
    parser.add_argument("--reset", action="store_true",
                        help="reset the board first and print the boot output")
    parser.add_argument("--wait", type=float, default=1.0,
                        help="seconds to read after each command (default: 1)")
    args = parser.parse_args()

    port = args.port or find_port()
    # Open with DTR/RTS released so opening the port does not hold the chip in reset.
    ser = serial.Serial()
    ser.port = port
    ser.baudrate = BAUD
    ser.timeout = 0.1
    ser.dtr = False
    ser.rts = False
    ser.open()

    with ser:
        if args.reset:
            reset_board(ser)
            wait_for_boot(ser)
        else:
            # Opening the port can reset the board on macOS (the bridge toggles
            # DTR/RTS). If boot output shows up, let the boot finish first.
            seen = read_for(ser, 0.5, until=HELP_END)
            if seen:
                wait_for_boot(ser, seen)
        for command in args.commands:
            ser.reset_input_buffer()
            ser.write(command.encode())
            read_for(ser, args.wait)


if __name__ == "__main__":
    main()
