#!/usr/bin/env python3
# -*- coding: utf-8
# vim: set expandtab shiftwidth=4:
# -*- Mode: python; coding: utf-8; indent-tabs-mode: nil -*- */
#
# Copyright © 2020 Red Hat, Inc.
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
#
# Prints the down/up state of each touch slot
#
# Input is a libinput record yaml file

import argparse
import enum
import sys
import yaml
import libevdev


class Slot:
    class State(enum.Enum):
        NONE = "NONE"
        BEGIN = "BEGIN"
        UPDATE = "UPDATE"
        END = "END"

    def __init__(self, index):
        self._state = Slot.State.NONE
        self.index = index
        self.used = False

    def begin(self):
        assert self.state == Slot.State.NONE
        self.state = Slot.State.BEGIN

    def end(self):
        assert self.state in (Slot.State.BEGIN, Slot.State.UPDATE)
        self.state = Slot.State.END

    def sync(self):
        if self.state == Slot.State.BEGIN:
            self.state = Slot.State.UPDATE
        elif self.state == Slot.State.END:
            self.state = Slot.State.NONE

    @property
    def state(self):
        return self._state

    @state.setter
    def state(self, newstate):
        assert newstate in Slot.State

        if newstate != Slot.State.NONE:
            self.used = True
        self._state = newstate

    @property
    def is_active(self):
        return self.state in (Slot.State.BEGIN, Slot.State.UPDATE)

    def __str__(self):
        return "+" if self.state in (Slot.State.BEGIN, Slot.State.UPDATE) else " "


def main(argv):
    parser = argparse.ArgumentParser(description="Print the state of touches over time")
    parser.add_argument(
        "--use-st", action="store_true", help="Ignore slots, use the BTN_TOOL bits"
    )
    parser.add_argument(
        "path", metavar="recording", nargs=1, help="Path to libinput-record YAML file"
    )
    args = parser.parse_args()

    yml = yaml.safe_load(open(args.path[0]))
    device = yml["devices"][0]
    absinfo = device["evdev"]["absinfo"]
    try:
        nslots = absinfo[libevdev.EV_ABS.ABS_MT_SLOT.value][1] + 1
    except KeyError:
        args.use_st = True

    tool_slot_map = {
        libevdev.EV_KEY.BTN_TOOL_FINGER: 0,
        libevdev.EV_KEY.BTN_TOOL_PEN: 0,
        libevdev.EV_KEY.BTN_TOOL_DOUBLETAP: 1,
        libevdev.EV_KEY.BTN_TOOL_TRIPLETAP: 2,
        libevdev.EV_KEY.BTN_TOOL_QUADTAP: 3,
        libevdev.EV_KEY.BTN_TOOL_QUINTTAP: 4,
    }
    if args.use_st:
        for bit in tool_slot_map:
            if bit.value in device["evdev"]["codes"][libevdev.EV_KEY.value]:
                nslots = max(nslots, tool_slot_map[bit])

    slots = [Slot(i) for i in range(0, nslots)]
    # We claim the first slots are used just to make the formatting
    # more consistent
    for i in range(min(5, len(slots))):
        slots[i].used = True

    slot = 0
    last_time = None
    last_slot_state = None
    header = "Timestamp | Rel time |     Slots     |"
    print(header)
    print("-" * len(header))

    def events():
        for event in device["events"]:
            for evdev in event["evdev"]:
                yield evdev

    for evdev in events():
        e = libevdev.InputEvent(
            code=libevdev.evbit(evdev[2], evdev[3]),
            value=evdev[4],
            sec=evdev[0],
            usec=evdev[1],
        )

        # single-touch formatting is simpler than multitouch, it'll just
        # show the highest finger down rather than the correct output.
        if args.use_st:
            if e.code in tool_slot_map:
                slot = tool_slot_map[e.code]
                s = slots[slot]
                if e.value:
                    s.begin()
                else:
                    s.end()
        else:
            if e.code == libevdev.EV_ABS.ABS_MT_SLOT:
                slot = e.value
                s = slots[slot]
                # bcm5974 cycles through slot numbers, so let's say all below
                # our current slot number was used
                for sl in slots[: slot + 1]:
                    sl.used = True
            else:
                s = slots[slot]
                if e.code == libevdev.EV_ABS.ABS_MT_TRACKING_ID:
                    if e.value == -1:
                        s.end()
                    else:
                        s.begin()
                elif e.code in (
                    libevdev.EV_ABS.ABS_MT_POSITION_X,
                    libevdev.EV_ABS.ABS_MT_POSITION_Y,
                    libevdev.EV_ABS.ABS_MT_PRESSURE,
                    libevdev.EV_ABS.ABS_MT_TOUCH_MAJOR,
                    libevdev.EV_ABS.ABS_MT_TOUCH_MINOR,
                ):
                    # If recording started after touch down
                    if s.state == Slot.State.NONE:
                        s.begin()

        if e.code == libevdev.EV_SYN.SYN_REPORT:
            current_slot_state = tuple(s.is_active for s in slots)

            if current_slot_state != last_slot_state:
                if last_time is None:
                    last_time = e.sec * 1000000 + e.usec
                    tdelta = 0
                else:
                    t = e.sec * 1000000 + e.usec
                    tdelta = int((t - last_time) / 1000) / 1000
                    last_time = t

                fmt = " | ".join([str(s) for s in slots if s.used])
                print(
                    "{:2d}.{:06d} | {:+7.3f}s | {}".format(e.sec, e.usec, tdelta, fmt)
                )

                last_slot_state = current_slot_state

            for s in slots:
                s.sync()


if __name__ == "__main__":
    main(sys.argv)
