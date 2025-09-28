#!/usr/bin/env python3
# vim: set expandtab shiftwidth=4:
# -*- Mode: python; coding: utf-8; indent-tabs-mode: nil -*- */
#
# Copyright © 2017 Red Hat, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#

import sys
import subprocess
import argparse

try:
    import libevdev
    import pyudev
except ModuleNotFoundError as e:
    print("Error: {}".format(str(e)), file=sys.stderr)
    print(
        "One or more python modules are missing. Please install those "
        "modules and re-run this tool."
    )
    sys.exit(1)


class TableFormatter(object):
    ALIGNMENT = 3

    def __init__(self):
        self.colwidths = []

    @property
    def width(self):
        return sum(self.colwidths) + 1

    def headers(self, args):
        s = "|"
        align = self.ALIGNMENT - 1  # account for |

        for arg in args:
            # +2 because we want space left/right of text
            w = ((len(arg) + 2 + align) // align) * align
            self.colwidths.append(w + 1)
            s += " {:^{width}s} |".format(arg, width=w - 2)

        return s

    def values(self, args):
        s = "|"
        for w, arg in zip(self.colwidths, args):
            w -= 1  # width includes | separator
            if type(arg) == str:
                # We want space margins for strings
                s += " {:{width}s} |".format(arg, width=w - 2)
            elif type(arg) == bool:
                s += "{:^{width}s}|".format("x" if arg else " ", width=w)
            else:
                s += "{:^{width}d}|".format(arg, width=w)

        if len(args) < len(self.colwidths):
            s += "|".rjust(self.width - len(s), " ")
        return s

    def separator(self):
        return "+" + "-" * (self.width - 2) + "+"


fmt = TableFormatter()


class Range(object):
    """Class to keep a min/max of a value around"""

    def __init__(self):
        self.min = float("inf")
        self.max = float("-inf")

    def update(self, value):
        self.min = min(self.min, value)
        self.max = max(self.max, value)


class Touch(object):
    """A single data point of a sequence (i.e. one event frame)"""

    def __init__(self, pressure=None):
        self.pressure = pressure


class TouchSequence(object):
    """A touch sequence from beginning to end"""

    def __init__(self, device, tracking_id):
        self.device = device
        self.tracking_id = tracking_id
        self.points = []

        self.is_active = True

        self.is_down = False
        self.was_down = False
        self.is_palm = False
        self.was_palm = False
        self.is_thumb = False
        self.was_thumb = False

        self.prange = Range()

    def append(self, touch):
        """Add a Touch to the sequence"""
        self.points.append(touch)
        self.prange.update(touch.pressure)

        if touch.pressure < self.device.up:
            self.is_down = False
        elif touch.pressure > self.device.down:
            self.is_down = True
            self.was_down = True

        self.is_palm = touch.pressure > self.device.palm
        if self.is_palm:
            self.was_palm = True

        self.is_thumb = touch.pressure > self.device.thumb
        if self.is_thumb:
            self.was_thumb = True

    def finalize(self):
        """Mark the TouchSequence as complete (finger is up)"""
        self.is_active = False

    def avg(self):
        """Average pressure value of this sequence"""
        return int(sum([p.pressure for p in self.points]) / len(self.points))

    def median(self):
        """Median pressure value of this sequence"""
        ps = sorted([p.pressure for p in self.points])
        idx = int(len(self.points) / 2)
        return ps[idx]

    def __str__(self):
        return self._str_state() if self.is_active else self._str_summary()

    def _str_summary(self):
        if not self.points:
            return fmt.values(
                [
                    self.tracking_id,
                    False,
                    False,
                    False,
                    False,
                    "No pressure values recorded",
                ]
            )

        s = fmt.values(
            [
                self.tracking_id,
                self.was_down,
                True,
                self.was_palm,
                self.was_thumb,
                self.prange.min,
                self.prange.max,
                0,
                self.avg(),
                self.median(),
            ]
        )

        return s

    def _str_state(self):
        s = fmt.values(
            [
                self.tracking_id,
                self.is_down,
                not self.is_down,
                self.is_palm,
                self.is_thumb,
                self.prange.min,
                self.prange.max,
                self.points[-1].pressure,
            ]
        )
        return s


class InvalidDeviceError(Exception):
    pass


class Device(libevdev.Device):
    def __init__(self, path):
        if path is None:
            self.path = self.find_touchpad_device()
        else:
            self.path = path

        fd = open(self.path, "rb")
        super().__init__(fd)

        print("Using {}: {}\n".format(self.name, self.path))

        self.has_mt_pressure = True
        absinfo = self.absinfo[libevdev.EV_ABS.ABS_MT_PRESSURE]
        if absinfo is None:
            absinfo = self.absinfo[libevdev.EV_ABS.ABS_PRESSURE]
            self.has_mt_pressure = False
            if absinfo is None:
                raise InvalidDeviceError(
                    "Device does not have ABS_PRESSURE or ABS_MT_PRESSURE"
                )

        prange = absinfo.maximum - absinfo.minimum

        # libinput defaults
        self.down = int(absinfo.minimum + 0.12 * prange)
        self.up = int(absinfo.minimum + 0.10 * prange)
        self.palm = 130  # the libinput default
        self.thumb = absinfo.maximum

        self._init_thresholds_from_quirks()
        self.sequences = []

    def find_touchpad_device(self):
        context = pyudev.Context()
        for device in context.list_devices(subsystem="input"):
            if not device.get("ID_INPUT_TOUCHPAD", 0):
                continue

            if not device.device_node or not device.device_node.startswith(
                "/dev/input/event"
            ):
                continue

            return device.device_node
        print("Unable to find a touchpad device.", file=sys.stderr)
        sys.exit(1)

    def _init_thresholds_from_quirks(self):
        command = ["libinput", "quirks", "list", self.path]
        cmd = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if cmd.returncode != 0:
            print(
                "Error querying quirks: {}".format(cmd.stderr.decode("utf-8")),
                file=sys.stderr,
            )
            return

        stdout = cmd.stdout.decode("utf-8")
        quirks = [q.split("=") for q in stdout.split("\n")]

        for q in quirks:
            if q[0] == "AttrPalmPressureThreshold":
                self.palm = int(q[1])
            elif q[0] == "AttrPressureRange":
                self.down, self.up = colon_tuple(q[1])
            elif q[0] == "AttrThumbPressureThreshold":
                self.thumb = int(q[1])

    def start_new_sequence(self, tracking_id):
        self.sequences.append(TouchSequence(self, tracking_id))

    def current_sequence(self):
        return self.sequences[-1]


def handle_key(device, event):
    tapcodes = [
        libevdev.EV_KEY.BTN_TOOL_DOUBLETAP,
        libevdev.EV_KEY.BTN_TOOL_TRIPLETAP,
        libevdev.EV_KEY.BTN_TOOL_QUADTAP,
        libevdev.EV_KEY.BTN_TOOL_QUINTTAP,
    ]
    if event.code in tapcodes and event.value > 0:
        print(
            "\r\033[2KThis tool cannot handle multiple fingers, "
            "output will be invalid"
        )


def handle_abs(device, event):
    if event.matches(libevdev.EV_ABS.ABS_MT_TRACKING_ID):
        if event.value > -1:
            device.start_new_sequence(event.value)
        else:
            try:
                s = device.current_sequence()
                s.finalize()
                print("\r\033[2K{}".format(s))
            except IndexError:
                # If the finger was down at startup
                pass
    elif event.matches(libevdev.EV_ABS.ABS_MT_PRESSURE) or (
        event.matches(libevdev.EV_ABS.ABS_PRESSURE) and not device.has_mt_pressure
    ):
        try:
            s = device.current_sequence()
            s.append(Touch(pressure=event.value))
            print("\r\033[2K{}".format(s), end="")
        except IndexError:
            # If the finger was down at startup
            pass


def handle_event(device, event):
    if event.matches(libevdev.EV_ABS):
        handle_abs(device, event)
    elif event.matches(libevdev.EV_KEY):
        handle_key(device, event)


def loop(device):
    print("This is an interactive tool")
    print()
    print("Place a single finger on the touchpad to measure pressure values.")
    print("Check that:")
    print("- touches subjectively perceived as down are tagged as down")
    print("- touches with a thumb are tagged as thumb")
    print("- touches with a palm are tagged as palm")
    print()
    print("If the touch states do not match the interaction, re-run")
    print("with --touch-thresholds=down:up using observed pressure values.")
    print("See --help for more options.")
    print()
    print("Press Ctrl+C to exit")
    print()

    headers = fmt.headers(
        ["Touch", "down", "up", "palm", "thumb", "min", "max", "p", "avg", "median"]
    )
    print(fmt.separator())
    print(fmt.values(["Thresh", device.down, device.up, device.palm, device.thumb]))
    print(fmt.separator())
    print(headers)
    print(fmt.separator())

    while True:
        for event in device.events():
            handle_event(device, event)


def colon_tuple(string):
    try:
        ts = string.split(":")
        t = tuple([int(x) for x in ts])
        if len(t) == 2 and t[0] >= t[1]:
            return t
    except:  # noqa
        pass

    msg = "{} is not in format N:M (N >= M)".format(string)
    raise argparse.ArgumentTypeError(msg)


def main(args):
    parser = argparse.ArgumentParser(description="Measure touchpad pressure values")
    parser.add_argument(
        "path",
        metavar="/dev/input/event0",
        nargs="?",
        type=str,
        help="Path to device (optional)",
    )
    parser.add_argument(
        "--touch-thresholds",
        metavar="down:up",
        type=colon_tuple,
        help="Thresholds when a touch is logically down or up",
    )
    parser.add_argument(
        "--palm-threshold",
        metavar="t",
        type=int,
        help="Threshold when a touch is a palm",
    )
    parser.add_argument(
        "--thumb-threshold",
        metavar="t",
        type=int,
        help="Threshold when a touch is a thumb",
    )
    args = parser.parse_args()

    try:
        device = Device(args.path)

        if args.touch_thresholds is not None:
            device.down, device.up = args.touch_thresholds

        if args.palm_threshold is not None:
            device.palm = args.palm_threshold

        if args.thumb_threshold is not None:
            device.thumb = args.thumb_threshold

        loop(device)
    except KeyboardInterrupt:
        print("\r\033[2K{}".format(fmt.separator()))
        print()

    except (PermissionError, OSError):
        print("Error: failed to open device")
    except InvalidDeviceError as e:
        print(
            "This device does not have the capabilities for pressure-based touch detection."
        )
        print("Details: {}".format(e))


if __name__ == "__main__":
    main(sys.argv)
