#!/usr/bin/env python3
# vim: set expandtab shiftwidth=4:
# -*- Mode: python; coding: utf-8; indent-tabs-mode: nil -*- */
#
# Copyright © 2020 Red Hat, Inc.
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


import sys
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


class DeviceError(Exception):
    pass


class Point:
    def __init__(self, x=None, y=None):
        self.x = x
        self.y = y


class Touchpad(object):
    def __init__(self, evdev):
        x = evdev.absinfo[libevdev.EV_ABS.ABS_X]
        y = evdev.absinfo[libevdev.EV_ABS.ABS_Y]
        if not x or not y:
            raise DeviceError("Device does not have an x or axis")

        if not x.resolution or not y.resolution:
            print("Device does not have resolutions.", file=sys.stderr)
            x.resolution = 1
            y.resolution = 1

        self.xrange = x.maximum - x.minimum
        self.yrange = y.maximum - y.minimum
        self.width = self.xrange / x.resolution
        self.height = self.yrange / y.resolution

        self._x = x
        self._y = y

        # We try to make the touchpad at least look proportional. The
        # terminal character space is (guesswork) ca 2.3 times as high as
        # wide.
        self.columns = 30
        self.rows = int(
            self.columns
            * (self.yrange // y.resolution)
            // (self.xrange // x.resolution)
            / 2.3
        )
        self.pos = Point(0, 0)
        self.min = Point()
        self.max = Point()

    @property
    def x(self):
        return self._x

    @property
    def y(self):
        return self._y

    @x.setter
    def x(self, x):
        self._x.minimum = min(self.x.minimum, x)
        self._x.maximum = max(self.x.maximum, x)
        self.min.x = min(x, self.min.x or 0xFFFFFFFF)
        self.max.x = max(x, self.max.x or -0xFFFFFFFF)
        # we calculate the position based on the original range.
        # this means on devices with a narrower range than advertised, not
        # all corners may be reachable in the touchpad drawing.
        self.pos.x = min(0.99, (x - self._x.minimum) / self.xrange)

    @y.setter
    def y(self, y):
        self._y.minimum = min(self.y.minimum, y)
        self._y.maximum = max(self.y.maximum, y)
        self.min.y = min(y, self.min.y or 0xFFFFFFFF)
        self.max.y = max(y, self.max.y or -0xFFFFFFFF)
        # we calculate the position based on the original range.
        # this means on devices with a narrower range than advertised, not
        # all corners may be reachable in the touchpad drawing.
        self.pos.y = min(0.99, (y - self._y.minimum) / self.yrange)

    def update_from_data(self):
        if None in [self.min.x, self.min.y, self.max.x, self.max.y]:
            raise DeviceError("Insufficient data to continue")
        self._x.minimum = self.min.x
        self._x.maximum = self.max.x
        self._y.minimum = self.min.y
        self._y.maximum = self.max.y

    def draw(self):
        print(
            "Detected axis range: x [{:4d}..{:4d}], y [{:4d}..{:4d}]".format(
                self.min.x if self.min.x is not None else 0,
                self.max.x if self.max.x is not None else 0,
                self.min.y if self.min.y is not None else 0,
                self.max.y if self.max.y is not None else 0,
            )
        )

        print()
        print("Move one finger along all edges of the touchpad".center(self.columns))
        print("until the detected axis range stops changing.".center(self.columns))

        top = int(self.pos.y * self.rows)

        print("+{}+".format("".ljust(self.columns, "-")))
        for row in range(0, top):
            print("|{}|".format("".ljust(self.columns)))

        left = int(self.pos.x * self.columns)
        right = max(0, self.columns - 1 - left)
        print("|{}{}{}|".format("".ljust(left), "O", "".ljust(right)))

        for row in range(top + 1, self.rows):
            print("|{}|".format("".ljust(self.columns)))

        print("+{}+".format("".ljust(self.columns, "-")))

        print("Press Ctrl+C to stop".center(self.columns))

        print("\033[{}A".format(self.rows + 8), flush=True)

        self.rows_printed = self.rows + 8

    def erase(self):
        # Erase all previous lines so we're not left with rubbish
        for row in range(self.rows_printed):
            print("\033[K")
        print("\033[{}A".format(self.rows_printed))


def dimension(string):
    try:
        ts = string.split("x")
        t = tuple([int(x) for x in ts])
        if len(t) == 2:
            return t
    except:  # noqa
        pass

    msg = "{} is not in format WxH".format(string)
    raise argparse.ArgumentTypeError(msg)


def between(v1, v2, deviation):
    return v1 - deviation < v2 < v1 + deviation


def dmi_modalias_match(modalias):
    modalias = modalias.split(":")
    dmi = {"svn": None, "pvr": None, "pn": None}
    for m in modalias:
        for key in dmi:
            if m.startswith(key):
                dmi[key] = m[len(key) :]

    # Based on the current 60-evdev.hwdb, Lenovo uses pvr and everyone else
    # uses pn to provide a human-identifiable match
    if dmi["svn"] == "LENOVO":
        return "dmi:*svn{}:*pvr{}*".format(dmi["svn"], dmi["pvr"])
    else:
        return "dmi:*svn{}:*pn{}*".format(dmi["svn"], dmi["pn"])


def main(args):
    parser = argparse.ArgumentParser(description="Measure the touchpad size")
    parser.add_argument(
        "size",
        metavar="WxH",
        type=dimension,
        help="Touchpad size (width by height) in mm",
    )
    parser.add_argument(
        "path",
        metavar="/dev/input/event0",
        nargs="?",
        type=str,
        help="Path to device (optional)",
    )
    context = pyudev.Context()

    args = parser.parse_args()
    if not args.path:
        for device in context.list_devices(subsystem="input"):
            if device.get("ID_INPUT_TOUCHPAD", 0) and (
                device.device_node or ""
            ).startswith("/dev/input/event"):
                args.path = device.device_node
                name = "unknown"
                parent = device
                while parent is not None:
                    n = parent.get("NAME", None)
                    if n:
                        name = n
                        break
                    parent = parent.parent

                print("Using {}: {}".format(name, device.device_node))
                break
        else:
            print("Unable to find a touchpad device.", file=sys.stderr)
            return 1

    dev = pyudev.Devices.from_device_file(context, args.path)
    overrides = [p for p in dev.properties if p.startswith("EVDEV_ABS")]
    if overrides:
        print()
        print("********************************************************************")
        print("WARNING: axis overrides already in place for this device:")
        for prop in overrides:
            print("  {}={}".format(prop, dev.properties[prop]))
        print("The systemd hwdb already overrides the axis ranges and/or resolution.")
        print("This tool is not needed unless you want to verify the axis overrides.")
        print("********************************************************************")
        print()

    try:
        fd = open(args.path, "rb")
        evdev = libevdev.Device(fd)
        touchpad = Touchpad(evdev)
        print(
            "Kernel specified touchpad size: {:.1f}x{:.1f}mm".format(
                touchpad.width, touchpad.height
            )
        )
        print("User specified touchpad size:   {:.1f}x{:.1f}mm".format(*args.size))

        print()
        print(
            "Kernel axis range:   x [{:4d}..{:4d}], y [{:4d}..{:4d}]".format(
                touchpad.x.minimum,
                touchpad.x.maximum,
                touchpad.y.minimum,
                touchpad.y.maximum,
            )
        )

        print("Put your finger on the touchpad to start\033[1A")

        try:
            touchpad.draw()
            while True:
                for event in evdev.events():
                    if event.matches(libevdev.EV_ABS.ABS_X):
                        touchpad.x = event.value
                    elif event.matches(libevdev.EV_ABS.ABS_Y):
                        touchpad.y = event.value
                    elif event.matches(libevdev.EV_SYN.SYN_REPORT):
                        touchpad.draw()
        except KeyboardInterrupt:
            touchpad.erase()
            touchpad.update_from_data()

        print(
            "Detected axis range: x [{:4d}..{:4d}], y [{:4d}..{:4d}]".format(
                touchpad.x.minimum,
                touchpad.x.maximum,
                touchpad.y.minimum,
                touchpad.y.maximum,
            )
        )

        touchpad.x.resolution = round(
            (touchpad.x.maximum - touchpad.x.minimum) / args.size[0]
        )
        touchpad.y.resolution = round(
            (touchpad.y.maximum - touchpad.y.minimum) / args.size[1]
        )

        print(
            "Resolutions calculated based on user-specified size: x {}, y {} units/mm".format(
                touchpad.x.resolution, touchpad.y.resolution
            )
        )

        # If both x/y are within some acceptable deviation, we skip the axis
        # overrides and only override the resolution
        xorig = evdev.absinfo[libevdev.EV_ABS.ABS_X]
        yorig = evdev.absinfo[libevdev.EV_ABS.ABS_Y]
        deviation = 1.5 * touchpad.x.resolution  # 1.5 mm rounding on each side
        skip = between(xorig.minimum, touchpad.x.minimum, deviation)
        skip = skip and between(xorig.maximum, touchpad.x.maximum, deviation)
        deviation = 1.5 * touchpad.y.resolution  # 1.5 mm rounding on each side
        skip = skip and between(yorig.minimum, touchpad.y.minimum, deviation)
        skip = skip and between(yorig.maximum, touchpad.y.maximum, deviation)

        if skip:
            print()
            print(
                "Note: Axis ranges within acceptable deviation, skipping min/max override"
            )
            print()

        print()
        print("Suggested hwdb entry:")

        use_dmi = evdev.id["bustype"] not in [0x03, 0x05]  # USB, Bluetooth
        if use_dmi:
            modalias = open("/sys/class/dmi/id/modalias").read().strip()
            print(
                "Note: the dmi modalias match is a guess based on your machine's modalias:"
            )
            print(" ", modalias)
            print(
                "Please verify that this is the most sensible match and adjust if necessary."
            )

        print("-8<--------------------------")
        print("# Laptop model description (e.g. Lenovo X1 Carbon 5th)")
        if use_dmi:
            print("evdev:name:{}:{}*".format(evdev.name, dmi_modalias_match(modalias)))
        else:
            print(
                "evdev:input:b{:04X}v{:04X}p{:04X}*".format(
                    evdev.id["bustype"], evdev.id["vendor"], evdev.id["product"]
                )
            )
        print(
            " EVDEV_ABS_00={}:{}:{}".format(
                touchpad.x.minimum if not skip else "",
                touchpad.x.maximum if not skip else "",
                touchpad.x.resolution,
            )
        )
        print(
            " EVDEV_ABS_01={}:{}:{}".format(
                touchpad.y.minimum if not skip else "",
                touchpad.y.maximum if not skip else "",
                touchpad.y.resolution,
            )
        )
        if evdev.absinfo[libevdev.EV_ABS.ABS_MT_POSITION_X]:
            print(
                " EVDEV_ABS_35={}:{}:{}".format(
                    touchpad.x.minimum if not skip else "",
                    touchpad.x.maximum if not skip else "",
                    touchpad.x.resolution,
                )
            )
            print(
                " EVDEV_ABS_36={}:{}:{}".format(
                    touchpad.y.minimum if not skip else "",
                    touchpad.y.maximum if not skip else "",
                    touchpad.y.resolution,
                )
            )
        print("-8<--------------------------")
        print(
            "Instructions on what to do with this snippet are in /usr/lib/udev/hwdb.d/60-evdev.hwdb"
        )
    except DeviceError as e:
        print("Error: {}".format(e), file=sys.stderr)
        return 1
    except PermissionError:
        print("Unable to open device. Please run me as root", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
