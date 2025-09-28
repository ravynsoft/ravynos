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

    def __init__(self, major=None, minor=None, orientation=None):
        self._major = major
        self._minor = minor
        self._orientation = orientation
        self.dirty = False

    @property
    def major(self):
        return self._major

    @major.setter
    def major(self, major):
        self._major = major
        self.dirty = True

    @property
    def minor(self):
        return self._minor

    @minor.setter
    def minor(self, minor):
        self._minor = minor
        self.dirty = True

    @property
    def orientation(self):
        return self._orientation

    @orientation.setter
    def orientation(self, orientation):
        self._orientation = orientation
        self.dirty = True

    def __str__(self):
        s = "Touch: major {:3d}".format(self.major)
        if self.minor is not None:
            s += ", minor {:3d}".format(self.minor)
        if self.orientation is not None:
            s += ", orientation {:+3d}".format(self.orientation)
        return s


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

        self.major_range = Range()
        self.minor_range = Range()

    def append(self, touch):
        """Add a Touch to the sequence"""
        self.points.append(touch)
        self.major_range.update(touch.major)
        self.minor_range.update(touch.minor)

        if touch.major < self.device.up or touch.minor < self.device.up:
            self.is_down = False
        elif touch.major > self.device.down or touch.minor > self.device.down:
            self.is_down = True
            self.was_down = True

        self.is_palm = touch.major > self.device.palm
        if self.is_palm:
            self.was_palm = True

        self.is_thumb = self.device.thumb != 0 and touch.major > self.device.thumb
        if self.is_thumb:
            self.was_thumb = True

    def finalize(self):
        """Mark the TouchSequence as complete (finger is up)"""
        self.is_active = False

    def __str__(self):
        return self._str_state() if self.is_active else self._str_summary()

    def _str_summary(self):
        if not self.points:
            return "{:78s}".format("Sequence: no major/minor values recorded")

        s = "Sequence: major: [{:3d}..{:3d}] ".format(
            self.major_range.min, self.major_range.max
        )
        if self.device.has_minor:
            s += "minor: [{:3d}..{:3d}] ".format(
                self.minor_range.min, self.minor_range.max
            )
        if self.was_down:
            s += " down"
        if self.was_palm:
            s += " palm"
        if self.was_thumb:
            s += " thumb"

        return s

    def _str_state(self):
        touch = self.points[-1]
        s = "{}, tags: {} {} {}".format(
            touch,
            "down" if self.is_down else "    ",
            "palm" if self.is_palm else "    ",
            "thumb" if self.is_thumb else "     ",
        )
        return s


class InvalidDeviceError(Exception):
    pass


class Device(libevdev.Device):
    def __init__(self, path):
        if path is None:
            self.path = self.find_touch_device()
        else:
            self.path = path

        fd = open(self.path, "rb")
        super().__init__(fd)

        print("Using {}: {}\n".format(self.name, self.path))

        if not self.has(libevdev.EV_ABS.ABS_MT_TOUCH_MAJOR):
            raise InvalidDeviceError("Device does not have ABS_MT_TOUCH_MAJOR")

        self.has_minor = self.has(libevdev.EV_ABS.ABS_MT_TOUCH_MINOR)
        self.has_orientation = self.has(libevdev.EV_ABS.ABS_MT_ORIENTATION)

        self.up = 0
        self.down = 0
        self.palm = 0
        self.thumb = 0

        self._init_thresholds_from_quirks()
        self.sequences = []
        self.touch = Touch(0, 0)

    def find_touch_device(self):
        context = pyudev.Context()
        for device in context.list_devices(subsystem="input"):
            if not device.get("ID_INPUT_TOUCHPAD", 0) and not device.get(
                "ID_INPUT_TOUCHSCREEN", 0
            ):
                continue

            if not device.device_node or not device.device_node.startswith(
                "/dev/input/event"
            ):
                continue

            return device.device_node

        print("Unable to find a touch device.", file=sys.stderr)
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
            if q[0] == "AttrPalmSizeThreshold":
                self.palm = int(q[1])
            elif q[0] == "AttrTouchSizeRange":
                self.down, self.up = colon_tuple(q[1])
            elif q[0] == "AttrThumbSizeThreshold":
                self.thumb = int(q[1])

    def start_new_sequence(self, tracking_id):
        self.sequences.append(TouchSequence(self, tracking_id))

    def current_sequence(self):
        return self.sequences[-1]

    def handle_key(self, event):
        tapcodes = [
            libevdev.EV_KEY.BTN_TOOL_DOUBLETAP,
            libevdev.EV_KEY.BTN_TOOL_TRIPLETAP,
            libevdev.EV_KEY.BTN_TOOL_QUADTAP,
            libevdev.EV_KEY.BTN_TOOL_QUINTTAP,
        ]
        if event.code in tapcodes and event.value > 0:
            print(
                "\rThis tool cannot handle multiple fingers, " "output will be invalid",
                file=sys.stderr,
            )

    def handle_abs(self, event):
        if event.matches(libevdev.EV_ABS.ABS_MT_TRACKING_ID):
            if event.value > -1:
                self.start_new_sequence(event.value)
            else:
                try:
                    s = self.current_sequence()
                    s.finalize()
                    print("\r{}".format(s))
                except IndexError:
                    # If the finger was down during start
                    pass
        elif event.matches(libevdev.EV_ABS.ABS_MT_TOUCH_MAJOR):
            self.touch.major = event.value
        elif event.matches(libevdev.EV_ABS.ABS_MT_TOUCH_MINOR):
            self.touch.minor = event.value
        elif event.matches(libevdev.EV_ABS.ABS_MT_ORIENTATION):
            self.touch.orientation = event.value

    def handle_syn(self, event):
        if self.touch.dirty:
            try:
                self.current_sequence().append(self.touch)
                print("\r{}".format(self.current_sequence()), end="")
                self.touch = Touch(
                    major=self.touch.major,
                    minor=self.touch.minor,
                    orientation=self.touch.orientation,
                )
            except IndexError:
                pass

    def handle_event(self, event):
        if event.matches(libevdev.EV_ABS):
            self.handle_abs(event)
        elif event.matches(libevdev.EV_KEY):
            self.handle_key(event)
        elif event.matches(libevdev.EV_SYN):
            self.handle_syn(event)

    def read_events(self):
        print("Ready for recording data.")
        print("Touch sizes used: {}:{}".format(self.down, self.up))
        print("Palm size used: {}".format(self.palm))
        print("Thumb size used: {}".format(self.thumb))
        print(
            "Place a single finger on the device to measure touch size.\n"
            "Ctrl+C to exit\n"
        )

        while True:
            for event in self.events():
                self.handle_event(event)


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
    parser = argparse.ArgumentParser(description="Measure touch size and orientation")
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
    args = parser.parse_args()

    try:
        device = Device(args.path)

        if args.touch_thresholds is not None:
            device.down, device.up = args.touch_thresholds

        if args.palm_threshold is not None:
            device.palm = args.palm_threshold

        device.read_events()
    except KeyboardInterrupt:
        pass
    except (PermissionError, OSError):
        print("Error: failed to open device")
    except InvalidDeviceError as e:
        print(
            "This device does not have the capabilities for size-based touch detection."
        )
        print("Details: {}".format(e))


if __name__ == "__main__":
    main(sys.argv)
