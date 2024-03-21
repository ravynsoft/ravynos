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
import argparse

try:
    import libevdev
    import textwrap
    import pyudev
except ModuleNotFoundError as e:
    print("Error: {}".format(e), file=sys.stderr)
    print(
        "One or more python modules are missing. Please install those "
        "modules and re-run this tool."
    )
    sys.exit(1)

print_dest = sys.stdout


def error(msg, **kwargs):
    print(msg, **kwargs, file=sys.stderr)


def msg(msg, **kwargs):
    print(msg, **kwargs, file=print_dest, flush=True)


def tv2us(sec, usec):
    return sec * 1000000 + usec


def us2ms(us):
    return int(us / 1000)


class Touch(object):
    def __init__(self, down):
        self._down = down
        self._up = down

    @property
    def up(self):
        return us2ms(self._up)

    @up.setter
    def up(self, up):
        assert up > self.down
        self._up = up

    @property
    def down(self):
        return us2ms(self._down)

    @property
    def tdelta(self):
        return self.up - self.down


class InvalidDeviceError(Exception):
    pass


class Device(libevdev.Device):
    def __init__(self, path):
        if path is None:
            self.path = self._find_touch_device()
        else:
            self.path = path
        fd = open(self.path, "rb")
        super().__init__(fd)

        print("Using {}: {}\n".format(self.name, self.path))

        if not self.has(libevdev.EV_KEY.BTN_TOUCH):
            raise InvalidDeviceError("device does not have BTN_TOUCH")

        self.touches = []

    def _find_touch_device(self):
        context = pyudev.Context()
        device_node = None
        for device in context.list_devices(subsystem="input"):
            if not device.device_node or not device.device_node.startswith(
                "/dev/input/event"
            ):
                continue

            # pick the touchpad by default, fallback to the first
            # touchscreen only when there is no touchpad
            if device.get("ID_INPUT_TOUCHPAD", 0):
                device_node = device.device_node
                break

            if device.get("ID_INPUT_TOUCHSCREEN", 0) and device_node is None:
                device_node = device.device_node

        if device_node is not None:
            return device_node

        error("Unable to find a touch device.")
        sys.exit(1)

    def handle_btn_touch(self, event):
        if event.value != 0:
            t = Touch(tv2us(event.sec, event.usec))
            self.touches.append(t)
        else:
            self.touches[-1].up = tv2us(event.sec, event.usec)
            msg("\rTouch sequences detected: {}".format(len(self.touches)), end="")

    def handle_key(self, event):
        tapcodes = [
            libevdev.EV_KEY.BTN_TOOL_DOUBLETAP,
            libevdev.EV_KEY.BTN_TOOL_TRIPLETAP,
            libevdev.EV_KEY.BTN_TOOL_QUADTAP,
            libevdev.EV_KEY.BTN_TOOL_QUINTTAP,
        ]
        if event.code in tapcodes and event.value > 0:
            error(
                "\rThis tool cannot handle multiple fingers, " "output will be invalid"
            )
            return

        if event.matches(libevdev.EV_KEY.BTN_TOUCH):
            self.handle_btn_touch(event)

    def handle_syn(self, event):
        if self.touch.dirty:
            self.current_sequence().append(self.touch)
            self.touch = Touch(
                major=self.touch.major,
                minor=self.touch.minor,
                orientation=self.touch.orientation,
            )

    def handle_event(self, event):
        if event.matches(libevdev.EV_KEY):
            self.handle_key(event)

    def read_events(self):
        while True:
            for event in self.events():
                self.handle_event(event)

    def print_summary(self):
        deltas = sorted(t.tdelta for t in self.touches)

        dmax = max(deltas)
        dmin = min(deltas)

        ndeltas = len(deltas)

        davg = sum(deltas) / ndeltas
        dmedian = deltas[int(ndeltas / 2)]
        d95pc = deltas[int(ndeltas * 0.95)]
        d90pc = deltas[int(ndeltas * 0.90)]

        print("Time: ")
        print("  Max delta: {}ms".format(int(dmax)))
        print("  Min delta: {}ms".format(int(dmin)))
        print("  Average delta: {}ms".format(int(davg)))
        print("  Median delta: {}ms".format(int(dmedian)))
        print("  90th percentile: {}ms".format(int(d90pc)))
        print("  95th percentile: {}ms".format(int(d95pc)))

    def print_dat(self):
        print("# libinput-measure-touchpad-tap")
        print(
            textwrap.dedent(
                """\
              # File contents:
              #    This file contains multiple prints of the data in
              #    different sort order. Row number is index of touch
              #    point within each group. Comparing data across groups
              #    will result in invalid analysis.
              # Columns (1-indexed):
              # Group 1, sorted by time of occurrence
              #  1: touch down time in ms, offset by first event
              #  2: touch up time in ms, offset by first event
              #  3: time delta in ms);
              # Group 2, sorted by touch down-up delta time (ascending)
              #  4: touch down time in ms, offset by first event
              #  5: touch up time in ms, offset by first event
              #  6: time delta in ms
              """
            )
        )

        deltas = [t for t in self.touches]
        deltas_sorted = sorted(deltas, key=lambda t: t.tdelta)

        offset = deltas[0].down

        for t1, t2 in zip(deltas, deltas_sorted):
            print(
                t1.down - offset,
                t1.up - offset,
                t1.tdelta,
                t2.down - offset,
                t2.up - offset,
                t2.tdelta,
            )

    def print(self, format):
        if not self.touches:
            error("No tap data available")
            return

        if format == "summary":
            self.print_summary()
        elif format == "dat":
            self.print_dat()


def main(args):
    parser = argparse.ArgumentParser(
        description="Measure tap-to-click properties of devices"
    )
    parser.add_argument(
        "path",
        metavar="/dev/input/event0",
        nargs="?",
        type=str,
        help="Path to device (optional)",
    )
    parser.add_argument(
        "--format",
        metavar="format",
        choices=["summary", "dat"],
        default="summary",
        help='data format to print ("summary" or "dat")',
    )
    args = parser.parse_args()

    if not sys.stdout.isatty():
        global print_dest
        print_dest = sys.stderr

    try:
        device = Device(args.path)
        error(
            "Ready for recording data.\n"
            "Tap the touchpad multiple times with a single finger only.\n"
            "For useful data we recommend at least 20 taps.\n"
            "Ctrl+C to exit"
        )
        device.read_events()
    except KeyboardInterrupt:
        msg("")
        device.print(args.format)
    except (PermissionError, OSError) as e:
        error("Error: failed to open device. {}".format(e))
    except InvalidDeviceError as e:
        error("Error: {}".format(e))


if __name__ == "__main__":
    main(sys.argv)
