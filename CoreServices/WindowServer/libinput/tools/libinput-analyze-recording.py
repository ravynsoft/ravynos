#!/usr/bin/env python3
# -*- coding: utf-8
# vim: set expandtab shiftwidth=4:
# -*- Mode: python; coding: utf-8; indent-tabs-mode: nil -*- */
#
# Copyright © 2021 Red Hat, Inc.
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
#
# Prints the data from a libinput recording in a table format to ease
# debugging.
#
# Input is a libinput record yaml file

import argparse
import os
import sys
import yaml
import libevdev

# minimum width of a field in the table
MIN_FIELD_WIDTH = 6


# Default is to just return the value of an axis, but some axes want special
# formatting.
def format_value(code, value):
    if code in (libevdev.EV_ABS.ABS_MISC, libevdev.EV_MSC.MSC_SERIAL):
        return f"{value & 0xFFFFFFFF:#x}"

    # Rel axes we always print the sign
    if code.type == libevdev.EV_REL:
        return f"{value:+d}"

    return f"{value}"


# The list of axes we want to track
def is_tracked_axis(code, allowlist, denylist):
    if code.type in (libevdev.EV_KEY, libevdev.EV_SW, libevdev.EV_SYN):
        return False

    # We don't do slots in this tool
    if code.type == libevdev.EV_ABS:
        if libevdev.EV_ABS.ABS_MT_SLOT <= code <= libevdev.EV_ABS.ABS_MAX:
            return False

    if allowlist:
        return code in allowlist
    else:
        return code not in denylist


def main(argv):
    parser = argparse.ArgumentParser(
        description="Display a recording in a tabular format"
    )
    parser.add_argument(
        "path", metavar="recording", nargs=1, help="Path to libinput-record YAML file"
    )
    parser.add_argument(
        "--ignore",
        metavar="ABS_X,ABS_Y,...",
        default="",
        help="A comma-separated list of axis names to ignore",
    )
    parser.add_argument(
        "--only",
        metavar="ABS_X,ABS_Y,...",
        default="",
        help="A comma-separated list of axis names to print, ignoring all others",
    )
    parser.add_argument(
        "--print-state",
        action="store_true",
        default=False,
        help="Always print all axis values, even unchanged ones",
    )

    args = parser.parse_args()
    if args.ignore and args.only:
        print("Only one of --ignore and --only may be given", file=sys.stderr)
        sys.exit(2)

    ignored_axes = [libevdev.evbit(axis) for axis in args.ignore.split(",") if axis]
    only_axes = [libevdev.evbit(axis) for axis in args.only.split(",") if axis]

    isatty = os.isatty(sys.stdout.fileno())

    yml = yaml.safe_load(open(args.path[0]))
    if yml["ndevices"] > 1:
        print(f"WARNING: Using only first {yml['ndevices']} devices in recording")
    device = yml["devices"][0]

    if not device["events"]:
        print(f"No events found in recording")
        sys.exit(1)

    def events():
        """
        Yields the next event in the recording
        """
        for event in device["events"]:
            for evdev in event.get("evdev", []):
                yield libevdev.InputEvent(
                    code=libevdev.evbit(evdev[2], evdev[3]),
                    value=evdev[4],
                    sec=evdev[0],
                    usec=evdev[1],
                )

    def interesting_axes(events):
        """
        Yields the libevdev codes with the axes in this recording
        """
        used_axes = []
        for e in events:
            if e.code not in used_axes and is_tracked_axis(
                e.code, only_axes, ignored_axes
            ):
                yield e.code
                used_axes.append(e.code)

    # Compile all axes that we want to print first
    axes = sorted(
        interesting_axes(events()), key=lambda x: x.type.value * 1000 + x.value
    )
    # Strip the REL_/ABS_ prefix for the headers
    headers = [a.name[4:].rjust(MIN_FIELD_WIDTH) for a in axes]
    # for easier formatting later, we keep the header field width in a dict
    axes = {a: len(h) for a, h in zip(axes, headers)}

    # Time is a special case, always the first entry
    # Format uses ms only, we rarely ever care about µs
    headers = [f"{'Time':<7s}"] + headers + ["Keys"]
    header_line = f"{' | '.join(headers)}"
    print(header_line)
    print("-" * len(header_line))

    current_codes = []
    current_frame = {}  # {evdev-code: value}
    axes_in_use = {}  # to print axes never sending events
    last_fields = []  # to skip duplicate lines
    continuation_count = 0

    keystate = {}
    keystate_changed = False

    for e in events():
        axes_in_use[e.code] = True

        if e.code.type == libevdev.EV_KEY:
            keystate[e.code] = e.value
            keystate_changed = True
        elif is_tracked_axis(e.code, only_axes, ignored_axes):
            current_frame[e.code] = e.value
            current_codes.append(e.code)
        elif e.code == libevdev.EV_SYN.SYN_REPORT:
            fields = []
            for a in axes:
                if args.print_state or a in current_codes:
                    s = format_value(a, current_frame.get(a, 0))
                else:
                    s = ""
                fields.append(s.rjust(max(MIN_FIELD_WIDTH, axes[a])))
            current_codes = []

            if last_fields != fields or keystate_changed:
                last_fields = fields.copy()
                keystate_changed = False

                if continuation_count:
                    if not isatty:
                        print(f" ... +{continuation_count}", end="")
                    print("")
                    continuation_count = 0

                fields.insert(0, f"{e.sec: 3d}.{e.usec//1000:03d}")
                keys_down = [k.name for k, v in keystate.items() if v]
                fields.append(", ".join(keys_down))
                print(" | ".join(fields))
            else:
                continuation_count += 1
                if isatty:
                    print(f"\r ... +{continuation_count}", end="", flush=True)

    # Print out any rel/abs axes that not generate events in
    # this recording
    unused_axes = []
    for evtype, evcodes in device["evdev"]["codes"].items():
        for c in evcodes:
            code = libevdev.evbit(int(evtype), int(c))
            if (
                is_tracked_axis(code, only_axes, ignored_axes)
                and code not in axes_in_use
            ):
                unused_axes.append(code)

    if unused_axes:
        print(
            f"Axes present but without events: {', '.join([a.name for a in unused_axes])}"
        )

    for e in events():
        if libevdev.EV_ABS.ABS_MT_SLOT <= code <= libevdev.EV_ABS.ABS_MAX:
            print(
                "WARNING: This recording contains multitouch data that is not supported by this tool."
            )
            break


if __name__ == "__main__":
    try:
        main(sys.argv)
    except BrokenPipeError:
        pass
