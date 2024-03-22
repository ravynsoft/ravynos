#!/usr/bin/python3

import sys
import serial

mode = sys.argv[1]
relay = sys.argv[2]

# our relays are "off" means "board is powered".
mode_swap = {
    "on": "off",
    "off": "on",
}
mode = mode_swap[mode]

ser = serial.Serial('/dev/ttyACM0', 115200, timeout=2)
command = "relay {} {}\n\r".format(mode, relay)
ser.write(command.encode())
ser.close()
