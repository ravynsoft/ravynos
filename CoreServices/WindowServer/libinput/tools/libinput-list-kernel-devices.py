#!/usr/bin/env python3
# vim: set expandtab shiftwidth=4:
# -*- Mode: python; coding: utf-8; indent-tabs-mode: nil -*- */
#
# Copyright © 2018 Red Hat, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the 'Software'),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

import argparse
import sys

try:
    import pyudev
except ModuleNotFoundError as e:
    print("Error: {}".format(str(e)), file=sys.stderr)
    print(
        "One or more python modules are missing. Please install those "
        "modules and re-run this tool."
    )
    sys.exit(1)


def list_devices():
    devices = {}
    context = pyudev.Context()
    for device in context.list_devices(subsystem="input"):
        if (device.device_node or "").startswith("/dev/input/event"):
            parent = device.parent
            if parent is not None:
                name = parent.properties["NAME"] or ""
                # The udev name includes enclosing quotes
                devices[device.device_node] = name[1:-1]

    def versionsort(key):
        return int(key[len("/dev/input/event") :])

    for k in sorted(devices, key=versionsort):
        print(f"{k}:\t{devices[k]}")


class HidDevice:
    def __init__(self, name, driver, vendor, product, devpath):
        self.name = name
        self.driver = driver
        self.vendor = vendor
        self.product = product
        self.devpath = devpath
        self.hidraws = []
        self.evdevs = []


def list_hid_devices():
    devices = []
    context = pyudev.Context()
    for device in context.list_devices(subsystem="hid"):
        name = device.properties.get("HID_NAME")
        driver = device.properties.get("DRIVER")
        devpath = device.properties.get("DEVPATH")
        id = device.properties.get("HID_ID") or "0:0:0"
        _, vendor, product = (int(x, 16) for x in id.split(":"))
        devices.append(HidDevice(name, driver, vendor, product, devpath))

    for device in context.list_devices(subsystem="hidraw"):
        devpath = device.properties["DEVPATH"]

        for hid in devices:
            if devpath.startswith(hid.devpath):
                hid.hidraws.append(f"'{device.device_node}'")

    for device in context.list_devices(subsystem="input"):
        if (device.device_node or "").startswith("/dev/input/event"):
            devpath = device.properties["DEVPATH"]

            for hid in devices:
                if devpath.startswith(hid.devpath):
                    hid.evdevs.append(f"'{device.device_node}'")

    print("hid:")
    for d in devices:
        print(f"- name:   '{d.name}'")
        print(f"  id:     '{d.vendor:04x}:{d.product:04x}'")
        print(f"  driver: '{d.driver}'")
        print(f"  hidraw: [{', '.join(h for h in d.hidraws)}]")
        print(f"  evdev:  [{', '.join(h for h in d.evdevs)}]")
        print("")


def main():
    parser = argparse.ArgumentParser(description="List kernel devices")
    parser.add_argument("--hid", action="store_true", default=False)
    args = parser.parse_args()

    if args.hid:
        list_hid_devices()
    else:
        list_devices()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Exited on user request")
