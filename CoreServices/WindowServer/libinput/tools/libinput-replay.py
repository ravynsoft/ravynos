#!/usr/bin/env python3
# vim: set expandtab shiftwidth=4:
# -*- Mode: python; coding: utf-8; indent-tabs-mode: nil -*- */
#
# Copyright © 2018 Red Hat, Inc.
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

import os
import sys
import time
import math
import multiprocessing
import argparse
from pathlib import Path

try:
    import libevdev
    import yaml
    import pyudev
except ModuleNotFoundError as e:
    print("Error: {}".format(e), file=sys.stderr)
    print(
        "One or more python modules are missing. Please install those "
        "modules and re-run this tool."
    )
    sys.exit(1)


SUPPORTED_FILE_VERSION = 1


def error(msg, **kwargs):
    print(msg, **kwargs, file=sys.stderr)


class YamlException(Exception):
    pass


def fetch(yaml, key):
    """Helper function to avoid confusing a YAML error with a
    normal KeyError bug"""
    try:
        return yaml[key]
    except KeyError:
        raise YamlException("Failed to get '{}' from recording.".format(key))


def check_udev_properties(yaml_data, uinput):
    """
    Compare the properties our new uinput device has with the ones from the
    recording and ring the alarm bell if one of them is off.
    """
    yaml_udev_section = fetch(yaml_data, "udev")
    yaml_udev_props = fetch(yaml_udev_section, "properties")
    yaml_props = {
        k: v for (k, v) in [prop.split("=", maxsplit=1) for prop in yaml_udev_props]
    }
    try:
        # We don't assign this one to virtual devices
        del yaml_props["LIBINPUT_DEVICE_GROUP"]
    except KeyError:
        pass

    # give udev some time to catch up
    time.sleep(0.2)
    context = pyudev.Context()
    udev_device = pyudev.Devices.from_device_file(context, uinput.devnode)
    for name, value in udev_device.properties.items():
        if name in yaml_props:
            if yaml_props[name] != value:
                error(
                    f"Warning: udev property mismatch: recording has {name}={yaml_props[name]}, device has {name}={value}"
                )
            del yaml_props[name]
        else:
            # The list of properties we add to the recording, see libinput-record.c
            prefixes = (
                "ID_INPUT",
                "LIBINPUT",
                "EVDEV_ABS",
                "MOUSE_DPI",
                "POINTINGSTICK_",
            )
            for prefix in prefixes:
                if name.startswith(prefix):
                    error(f"Warning: unexpected property: {name}={value}")

    # the ones we found above were removed from the dict
    for name, value in yaml_props.items():
        error(f"Warning: device is missing recorded udev property: {name}={value}")


def create(device):
    evdev = fetch(device, "evdev")

    d = libevdev.Device()
    d.name = fetch(evdev, "name")

    ids = fetch(evdev, "id")
    if len(ids) != 4:
        raise YamlException("Invalid ID format: {}".format(ids))
    d.id = dict(zip(["bustype", "vendor", "product", "version"], ids))

    codes = fetch(evdev, "codes")
    for evtype, evcodes in codes.items():
        for code in evcodes:
            data = None
            if evtype == libevdev.EV_ABS.value:
                values = fetch(evdev, "absinfo")[code]
                absinfo = libevdev.InputAbsInfo(
                    minimum=values[0],
                    maximum=values[1],
                    fuzz=values[2],
                    flat=values[3],
                    resolution=values[4],
                )
                data = absinfo
            elif evtype == libevdev.EV_REP.value:
                if code == libevdev.EV_REP.REP_DELAY.value:
                    data = 500
                elif code == libevdev.EV_REP.REP_PERIOD.value:
                    data = 20
            d.enable(libevdev.evbit(evtype, code), data=data)

    properties = fetch(evdev, "properties")
    for prop in properties:
        d.enable(libevdev.propbit(prop))

    uinput = d.create_uinput_device()

    check_udev_properties(device, uinput)

    return uinput


def print_events(devnode, indent, evs):
    devnode = os.path.basename(devnode)
    for e in evs:
        print(
            "{}: {}{:06d}.{:06d} {} / {:<20s} {:4d}".format(
                devnode,
                " " * (indent * 8),
                e.sec,
                e.usec,
                e.type.name,
                e.code.name,
                e.value,
            )
        )


def collect_events(frame):
    evs = []
    events_skipped = False
    for (sec, usec, evtype, evcode, value) in frame:
        if evtype == libevdev.EV_KEY.value and value == 2:  # key repeat
            events_skipped = True
            continue

        e = libevdev.InputEvent(
            libevdev.evbit(evtype, evcode), value=value, sec=sec, usec=usec
        )
        evs.append(e)

    # If we skipped some events and now all we have left is the
    # SYN_REPORTs, we drop the SYN_REPORTs as well.
    if events_skipped and all(e for e in evs if e.matches(libevdev.EV_SYN.SYN_REPORT)):
        return []
    else:
        return evs


def replay(device, verbose):
    events = fetch(device, "events")
    if events is None:
        return
    uinput = device["__uinput"]

    # The first event may have a nonzero offset but we want to replay
    # immediately regardless. When replaying multiple devices, the first
    # offset is the offset from the first event on any device.
    offset = time.time() - device["__first_event_offset"]

    if offset < 0:
        error("WARNING: event time offset is in the future, refusing to replay")
        return

    # each 'evdev' set contains one SYN_REPORT so we only need to check for
    # the time offset once per event
    for event in events:
        try:
            evdev = fetch(event, "evdev")
        except YamlException:
            continue

        evs = collect_events(evdev)
        if not evs:
            continue

        evtime = evs[0].sec + evs[0].usec / 1e6 + offset
        now = time.time()

        if evtime - now > 150 / 1e6:  # 150 µs error margin
            time.sleep(evtime - now - 150 / 1e6)

        uinput.send_events(evs)
        if verbose:
            print_events(uinput.devnode, device["__index"], evs)


def first_timestamp(device):
    events = fetch(device, "events")
    for e in events or []:
        try:
            evdev = fetch(e, "evdev")
            (sec, usec, *_) = evdev[0]
            return sec + usec / 1.0e6
        except YamlException:
            pass

    return None


def wrap(func, *args):
    try:
        func(*args)
    except KeyboardInterrupt:
        pass


def loop(args, recording):
    devices = fetch(recording, "devices")

    first_timestamps = tuple(
        filter(lambda x: x is not None, [first_timestamp(d) for d in devices])
    )
    # All devices need to start replaying at the same time, so let's find
    # the very first event and offset everything by that timestamp.
    toffset = min(first_timestamps or [math.inf])

    for idx, d in enumerate(devices):
        uinput = create(d)
        print("{}: {}".format(uinput.devnode, uinput.name))
        d["__uinput"] = uinput  # cheaper to hide it in the dict then work around it
        d["__index"] = idx
        d["__first_event_offset"] = toffset

    if not first_timestamps:
        input("No events in recording. Hit enter to quit")
        return

    while True:
        if args.replay_after >= 0:
            time.sleep(args.replay_after)
        else:
            input("Hit enter to start replaying")

        processes = []
        for d in devices:
            p = multiprocessing.Process(target=wrap, args=(replay, d, args.verbose))
            processes.append(p)

        for p in processes:
            p.start()

        for p in processes:
            p.join()

        del processes

        if args.once:
            break


def create_device_quirk(device):
    try:
        quirks = fetch(device, "quirks")
        if not quirks:
            return None
    except YamlException:
        return None
    # Where the device has a quirk, we match on name, vendor and product.
    # That's the best match we can assemble here from the info we have.
    evdev = fetch(device, "evdev")
    name = fetch(evdev, "name")
    id = fetch(evdev, "id")
    quirk = (
        "[libinput-replay {name}]\n"
        "MatchName={name}\n"
        "MatchVendor=0x{id[1]:04X}\n"
        "MatchProduct=0x{id[2]:04X}\n"
    ).format(name=name, id=id)
    quirk += "\n".join(quirks)
    return quirk


def setup_quirks(recording):
    devices = fetch(recording, "devices")
    overrides = None
    quirks = []
    for d in devices:
        if "quirks" in d:
            quirk = create_device_quirk(d)
            if quirk:
                quirks.append(quirk)
    if not quirks:
        return None

    overrides = Path("/etc/libinput/local-overrides.quirks")
    if overrides.exists():
        print(
            "{} exists, please move it out of the way first".format(overrides),
            file=sys.stderr,
        )
        sys.exit(1)

    overrides.parent.mkdir(exist_ok=True)
    with overrides.open("w+") as fd:
        fd.write("# This file was generated by libinput replay\n")
        fd.write("# Unless libinput replay is running right now, remove this file.\n")
        fd.write("\n\n".join(quirks))

    return overrides


def check_file(recording):
    version = fetch(recording, "version")
    if version != SUPPORTED_FILE_VERSION:
        raise YamlException(
            "Invalid file format: {}, expected {}".format(
                version, SUPPORTED_FILE_VERSION
            )
        )

    ndevices = fetch(recording, "ndevices")
    devices = fetch(recording, "devices")
    if ndevices != len(devices):
        error(
            "WARNING: truncated file, expected {} devices, got {}".format(
                ndevices, len(devices)
            )
        )


def main():
    parser = argparse.ArgumentParser(description="Replay a device recording")
    parser.add_argument(
        "recording",
        metavar="recorded-file.yaml",
        type=str,
        help="Path to device recording",
    )
    parser.add_argument(
        "--replay-after",
        type=int,
        default=-1,
        help="Automatically replay once after N seconds",
    )
    parser.add_argument(
        "--once",
        action="store_true",
        default=False,
        help="Stop and exit after one replay",
    )
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    quirks_file = None

    try:
        with open(args.recording) as f:
            y = yaml.safe_load(f)
            check_file(y)
            quirks_file = setup_quirks(y)
            loop(args, y)
    except KeyboardInterrupt:
        pass
    except (PermissionError, OSError) as e:
        error("Error: failed to open device: {}".format(e))
    except YamlException as e:
        error("Error: failed to parse recording: {}".format(e))
    finally:
        if quirks_file:
            quirks_file.unlink()


if __name__ == "__main__":
    main()
