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

import argparse
import os
import sys
import unittest
import yaml
import re

from pkg_resources import parse_version


class TestYaml(unittest.TestCase):
    filename = ""

    @classmethod
    def setUpClass(cls):
        with open(cls.filename) as f:
            cls.yaml = yaml.safe_load(f)

    def dict_key_crosscheck(self, d, keys):
        """Check that each key in d is in keys, and that each key is in d"""
        self.assertEqual(sorted(d.keys()), sorted(keys))

    def libinput_events(self, filter=None):
        """Returns all libinput events in the recording, regardless of the
        device"""
        devices = self.yaml["devices"]
        for d in devices:
            events = d["events"]
            if not events:
                raise unittest.SkipTest()
            for e in events:
                try:
                    libinput = e["libinput"]
                except KeyError:
                    continue

                for ev in libinput:
                    if (
                        filter is None
                        or ev["type"] == filter
                        or isinstance(filter, list)
                        and ev["type"] in filter
                    ):
                        yield ev

    def test_sections_exist(self):
        sections = ["version", "ndevices", "libinput", "system", "devices"]
        for section in sections:
            self.assertIn(section, self.yaml)

    def test_version(self):
        version = self.yaml["version"]
        self.assertTrue(isinstance(version, int))
        self.assertEqual(version, 1)

    def test_ndevices(self):
        ndevices = self.yaml["ndevices"]
        self.assertTrue(isinstance(ndevices, int))
        self.assertGreaterEqual(ndevices, 1)
        self.assertEqual(ndevices, len(self.yaml["devices"]))

    def test_libinput(self):
        libinput = self.yaml["libinput"]
        version = libinput["version"]
        self.assertTrue(isinstance(version, str))
        self.assertGreaterEqual(parse_version(version), parse_version("1.10.0"))
        git = libinput["git"]
        self.assertTrue(isinstance(git, str))
        self.assertNotEqual(git, "unknown")

    def test_system(self):
        system = self.yaml["system"]
        kernel = system["kernel"]
        self.assertTrue(isinstance(kernel, str))
        self.assertEqual(kernel, os.uname().release)

        dmi = system["dmi"]
        self.assertTrue(isinstance(dmi, str))
        with open("/sys/class/dmi/id/modalias") as f:
            sys_dmi = f.read()[:-1]  # trailing newline
            self.assertEqual(dmi, sys_dmi)

    def test_devices_sections_exist(self):
        devices = self.yaml["devices"]
        for d in devices:
            self.assertIn("node", d)
            self.assertIn("evdev", d)
            self.assertIn("udev", d)

    def test_evdev_sections_exist(self):
        sections = ["name", "id", "codes", "properties"]
        devices = self.yaml["devices"]
        for d in devices:
            evdev = d["evdev"]
            for s in sections:
                self.assertIn(s, evdev)

    def test_evdev_name(self):
        devices = self.yaml["devices"]
        for d in devices:
            evdev = d["evdev"]
            name = evdev["name"]
            self.assertTrue(isinstance(name, str))
            self.assertGreaterEqual(len(name), 5)

    def test_evdev_id(self):
        devices = self.yaml["devices"]
        for d in devices:
            evdev = d["evdev"]
            id = evdev["id"]
            self.assertTrue(isinstance(id, list))
            self.assertEqual(len(id), 4)
            self.assertGreater(id[0], 0)
            self.assertGreater(id[1], 0)

    def test_evdev_properties(self):
        devices = self.yaml["devices"]
        for d in devices:
            evdev = d["evdev"]
            properties = evdev["properties"]
            self.assertTrue(isinstance(properties, list))

    def test_hid(self):
        devices = self.yaml["devices"]
        for d in devices:
            hid = d["hid"]
            self.assertTrue(isinstance(hid, list))
            for byte in hid:
                self.assertGreaterEqual(byte, 0)
                self.assertLessEqual(byte, 255)

    def test_udev_sections_exist(self):
        sections = ["properties"]
        devices = self.yaml["devices"]
        for d in devices:
            udev = d["udev"]
            for s in sections:
                self.assertIn(s, udev)

    def test_udev_properties(self):
        devices = self.yaml["devices"]
        for d in devices:
            udev = d["udev"]
            properties = udev["properties"]
            self.assertTrue(isinstance(properties, list))
            self.assertGreater(len(properties), 0)

            self.assertIn("ID_INPUT=1", properties)
            for p in properties:
                self.assertTrue(re.match("[A-Z0-9_]+=.+", p))

    def test_udev_id_inputs(self):
        devices = self.yaml["devices"]
        for d in devices:
            udev = d["udev"]
            properties = udev["properties"]
            id_inputs = [p for p in properties if p.startswith("ID_INPUT")]
            # We expect ID_INPUT and ID_INPUT_something, but might get more
            # than one of the latter
            self.assertGreaterEqual(len(id_inputs), 2)

    def test_events_have_section(self):
        devices = self.yaml["devices"]
        for d in devices:
            events = d["events"]
            if not events:
                raise unittest.SkipTest()
            for e in events:
                self.assertTrue("evdev" in e or "libinput" in e)

    def test_events_evdev(self):
        devices = self.yaml["devices"]
        for d in devices:
            events = d["events"]
            if not events:
                raise unittest.SkipTest()
            for e in events:
                try:
                    evdev = e["evdev"]
                except KeyError:
                    continue

                for ev in evdev:
                    self.assertEqual(len(ev), 5)

                # Last event in each frame is SYN_REPORT
                ev_syn = evdev[-1]
                self.assertEqual(ev_syn[2], 0)
                self.assertEqual(ev_syn[3], 0)
                # SYN_REPORT value is 1 in case of some key repeats
                self.assertLessEqual(ev_syn[4], 1)

    def test_events_evdev_syn_report(self):
        devices = self.yaml["devices"]
        for d in devices:
            events = d["events"]
            if not events:
                raise unittest.SkipTest()
            for e in events:
                try:
                    evdev = e["evdev"]
                except KeyError:
                    continue
                for ev in evdev[:-1]:
                    self.assertFalse(ev[2] == 0 and ev[3] == 0)

    def test_events_libinput(self):
        devices = self.yaml["devices"]
        for d in devices:
            events = d["events"]
            if not events:
                raise unittest.SkipTest()
            for e in events:
                try:
                    libinput = e["libinput"]
                except KeyError:
                    continue

                self.assertTrue(isinstance(libinput, list))
                for ev in libinput:
                    self.assertTrue(isinstance(ev, dict))

    def test_events_libinput_type(self):
        types = [
            "POINTER_MOTION",
            "POINTER_MOTION_ABSOLUTE",
            "POINTER_AXIS",
            "POINTER_BUTTON",
            "DEVICE_ADDED",
            "KEYBOARD_KEY",
            "TOUCH_DOWN",
            "TOUCH_MOTION",
            "TOUCH_UP",
            "TOUCH_FRAME",
            "GESTURE_SWIPE_BEGIN",
            "GESTURE_SWIPE_UPDATE",
            "GESTURE_SWIPE_END",
            "GESTURE_PINCH_BEGIN",
            "GESTURE_PINCH_UPDATE",
            "GESTURE_PINCH_END",
            "TABLET_TOOL_AXIS",
            "TABLET_TOOL_PROXIMITY",
            "TABLET_TOOL_BUTTON",
            "TABLET_TOOL_TIP",
            "TABLET_PAD_STRIP",
            "TABLET_PAD_RING",
            "TABLET_PAD_BUTTON",
            "SWITCH_TOGGLE",
        ]
        for e in self.libinput_events():
            self.assertIn("type", e)
            self.assertIn(e["type"], types)

    def test_events_libinput_time(self):
        # DEVICE_ADDED has no time
        # first event may have 0.0 time if the first frame generates a
        # libinput event.
        try:
            for e in list(self.libinput_events())[2:]:
                self.assertIn("time", e)
                self.assertGreater(e["time"], 0.0)
                self.assertLess(e["time"], 60.0)
        except IndexError:
            pass

    def test_events_libinput_device_added(self):
        keys = ["type", "seat", "logical_seat"]
        for e in self.libinput_events("DEVICE_ADDED"):
            self.dict_key_crosscheck(e, keys)
            self.assertEqual(e["seat"], "seat0")
            self.assertEqual(e["logical_seat"], "default")

    def test_events_libinput_pointer_motion(self):
        keys = ["type", "time", "delta", "unaccel"]
        for e in self.libinput_events("POINTER_MOTION"):
            self.dict_key_crosscheck(e, keys)
            delta = e["delta"]
            self.assertTrue(isinstance(delta, list))
            self.assertEqual(len(delta), 2)
            for d in delta:
                self.assertTrue(isinstance(d, float))
            unaccel = e["unaccel"]
            self.assertTrue(isinstance(unaccel, list))
            self.assertEqual(len(unaccel), 2)
            for d in unaccel:
                self.assertTrue(isinstance(d, float))

    def test_events_libinput_pointer_button(self):
        keys = ["type", "time", "button", "state", "seat_count"]
        for e in self.libinput_events("POINTER_BUTTON"):
            self.dict_key_crosscheck(e, keys)
            button = e["button"]
            self.assertGreater(button, 0x100)  # BTN_0
            self.assertLess(button, 0x160)  # KEY_OK
            state = e["state"]
            self.assertIn(state, ["pressed", "released"])
            scount = e["seat_count"]
            self.assertGreaterEqual(scount, 0)

    def test_events_libinput_pointer_absolute(self):
        keys = ["type", "time", "point", "transformed"]
        for e in self.libinput_events("POINTER_MOTION_ABSOLUTE"):
            self.dict_key_crosscheck(e, keys)
            point = e["point"]
            self.assertTrue(isinstance(point, list))
            self.assertEqual(len(point), 2)
            for p in point:
                self.assertTrue(isinstance(p, float))
                self.assertGreater(p, 0.0)
                self.assertLess(p, 300.0)

            transformed = e["transformed"]
            self.assertTrue(isinstance(transformed, list))
            self.assertEqual(len(transformed), 2)
            for t in transformed:
                self.assertTrue(isinstance(t, float))
                self.assertGreater(t, 0.0)
                self.assertLess(t, 100.0)

    def test_events_libinput_touch(self):
        keys = ["type", "time", "slot", "seat_slot"]
        for e in self.libinput_events():
            if not e["type"].startswith("TOUCH_") or e["type"] == "TOUCH_FRAME":
                continue

            for k in keys:
                self.assertIn(k, e.keys())
            slot = e["slot"]
            seat_slot = e["seat_slot"]

            self.assertGreaterEqual(slot, 0)
            self.assertGreaterEqual(seat_slot, 0)

    def test_events_libinput_touch_down(self):
        keys = ["type", "time", "slot", "seat_slot", "point", "transformed"]
        for e in self.libinput_events("TOUCH_DOWN"):
            self.dict_key_crosscheck(e, keys)
            point = e["point"]
            self.assertTrue(isinstance(point, list))
            self.assertEqual(len(point), 2)
            for p in point:
                self.assertTrue(isinstance(p, float))
                self.assertGreater(p, 0.0)
                self.assertLess(p, 300.0)

            transformed = e["transformed"]
            self.assertTrue(isinstance(transformed, list))
            self.assertEqual(len(transformed), 2)
            for t in transformed:
                self.assertTrue(isinstance(t, float))
                self.assertGreater(t, 0.0)
                self.assertLess(t, 100.0)

    def test_events_libinput_touch_motion(self):
        keys = ["type", "time", "slot", "seat_slot", "point", "transformed"]
        for e in self.libinput_events("TOUCH_MOTION"):
            self.dict_key_crosscheck(e, keys)
            point = e["point"]
            self.assertTrue(isinstance(point, list))
            self.assertEqual(len(point), 2)
            for p in point:
                self.assertTrue(isinstance(p, float))
                self.assertGreater(p, 0.0)
                self.assertLess(p, 300.0)

            transformed = e["transformed"]
            self.assertTrue(isinstance(transformed, list))
            self.assertEqual(len(transformed), 2)
            for t in transformed:
                self.assertTrue(isinstance(t, float))
                self.assertGreater(t, 0.0)
                self.assertLess(t, 100.0)

    def test_events_libinput_touch_frame(self):
        devices = self.yaml["devices"]
        for d in devices:
            events = d["events"]
            if not events:
                raise unittest.SkipTest()
            for e in events:
                try:
                    evdev = e["libinput"]
                except KeyError:
                    continue

                need_frame = False
                for ev in evdev:
                    t = ev["type"]
                    if not t.startswith("TOUCH_"):
                        self.assertFalse(need_frame)
                        continue

                    if t == "TOUCH_FRAME":
                        self.assertTrue(need_frame)
                        need_frame = False
                    else:
                        need_frame = True

                self.assertFalse(need_frame)

    def test_events_libinput_gesture_pinch(self):
        keys = ["type", "time", "nfingers", "delta", "unaccel", "angle_delta", "scale"]
        for e in self.libinput_events(
            ["GESTURE_PINCH_BEGIN", "GESTURE_PINCH_UPDATE", "GESTURE_PINCH_END"]
        ):
            self.dict_key_crosscheck(e, keys)
            delta = e["delta"]
            self.assertTrue(isinstance(delta, list))
            self.assertEqual(len(delta), 2)
            for d in delta:
                self.assertTrue(isinstance(d, float))
            unaccel = e["unaccel"]
            self.assertTrue(isinstance(unaccel, list))
            self.assertEqual(len(unaccel), 2)
            for d in unaccel:
                self.assertTrue(isinstance(d, float))

            adelta = e["angle_delta"]
            self.assertTrue(isinstance(adelta, list))
            self.assertEqual(len(adelta), 2)
            for d in adelta:
                self.assertTrue(isinstance(d, float))

            scale = e["scale"]
            self.assertTrue(isinstance(scale, list))
            self.assertEqual(len(scale), 2)
            for d in scale:
                self.assertTrue(isinstance(d, float))

    def test_events_libinput_gesture_swipe(self):
        keys = ["type", "time", "nfingers", "delta", "unaccel"]
        for e in self.libinput_events(
            ["GESTURE_SWIPE_BEGIN", "GESTURE_SWIPE_UPDATE", "GESTURE_SWIPE_END"]
        ):
            self.dict_key_crosscheck(e, keys)
            delta = e["delta"]
            self.assertTrue(isinstance(delta, list))
            self.assertEqual(len(delta), 2)
            for d in delta:
                self.assertTrue(isinstance(d, float))
            unaccel = e["unaccel"]
            self.assertTrue(isinstance(unaccel, list))
            self.assertEqual(len(unaccel), 2)
            for d in unaccel:
                self.assertTrue(isinstance(d, float))

    def test_events_libinput_tablet_pad_button(self):
        keys = ["type", "time", "button", "state", "mode", "is-toggle"]

        for e in self.libinput_events("TABLET_PAD_BUTTON"):
            self.dict_key_crosscheck(e, keys)

            b = e["button"]
            self.assertTrue(isinstance(b, int))
            self.assertGreaterEqual(b, 0)
            self.assertLessEqual(b, 16)

            state = e["state"]
            self.assertIn(state, ["pressed", "released"])

            m = e["mode"]
            self.assertTrue(isinstance(m, int))
            self.assertGreaterEqual(m, 0)
            self.assertLessEqual(m, 3)

            t = e["is-toggle"]
            self.assertTrue(isinstance(t, bool))

    def test_events_libinput_tablet_pad_ring(self):
        keys = ["type", "time", "number", "position", "source", "mode"]

        for e in self.libinput_events("TABLET_PAD_RING"):
            self.dict_key_crosscheck(e, keys)

            n = e["number"]
            self.assertTrue(isinstance(n, int))
            self.assertGreaterEqual(n, 0)
            self.assertLessEqual(n, 4)

            p = e["position"]
            self.assertTrue(isinstance(p, float))
            if p != -1.0:  # special 'end' case
                self.assertGreaterEqual(p, 0.0)
                self.assertLess(p, 360.0)

            m = e["mode"]
            self.assertTrue(isinstance(m, int))
            self.assertGreaterEqual(m, 0)
            self.assertLessEqual(m, 3)

            s = e["source"]
            self.assertIn(s, ["finger", "unknown"])

    def test_events_libinput_tablet_pad_strip(self):
        keys = ["type", "time", "number", "position", "source", "mode"]

        for e in self.libinput_events("TABLET_PAD_STRIP"):
            self.dict_key_crosscheck(e, keys)

            n = e["number"]
            self.assertTrue(isinstance(n, int))
            self.assertGreaterEqual(n, 0)
            self.assertLessEqual(n, 4)

            p = e["position"]
            self.assertTrue(isinstance(p, float))
            if p != -1.0:  # special 'end' case
                self.assertGreaterEqual(p, 0.0)
                self.assertLessEqual(p, 1.0)

            m = e["mode"]
            self.assertTrue(isinstance(m, int))
            self.assertGreaterEqual(m, 0)
            self.assertLessEqual(m, 3)

            s = e["source"]
            self.assertIn(s, ["finger", "unknown"])

    def test_events_libinput_tablet_tool_proximity(self):
        keys = ["type", "time", "proximity", "tool-type", "serial", "axes"]

        for e in self.libinput_events("TABLET_TOOL_PROXIMITY"):
            for k in keys:
                self.assertIn(k, e)

            p = e["proximity"]
            self.assertIn(p, ["in", "out"])

            p = e["tool-type"]
            self.assertIn(
                p, ["pen", "eraser", "brush", "airbrush", "mouse", "lens", "unknown"]
            )

            s = e["serial"]
            self.assertTrue(isinstance(s, int))
            self.assertGreaterEqual(s, 0)

            a = e["axes"]
            for ax in e["axes"]:
                self.assertIn(a, "pdtrsw")

    def test_events_libinput_tablet_tool(self):
        keys = ["type", "time", "tip"]

        for e in self.libinput_events(["TABLET_TOOL_AXIS", "TABLET_TOOL_TIP"]):
            for k in keys:
                self.assertIn(k, e)

            t = e["tip"]
            self.assertIn(t, ["down", "up"])

    def test_events_libinput_tablet_tool_button(self):
        keys = ["type", "time", "button", "state"]

        for e in self.libinput_events("TABLET_TOOL_BUTTON"):
            self.dict_key_crosscheck(e, keys)

            b = e["button"]
            # STYLUS, STYLUS2, STYLUS3
            self.assertIn(b, [0x14B, 0x14C, 0x139])

            s = e["state"]
            self.assertIn(s, ["pressed", "released"])

    def test_events_libinput_tablet_tool_axes(self):
        for e in self.libinput_events(
            ["TABLET_TOOL_PROXIMITY", "TABLET_TOOL_AXIS", "TABLET_TOOL_TIP"]
        ):

            point = e["point"]
            self.assertTrue(isinstance(point, list))
            self.assertEqual(len(point), 2)
            for p in point:
                self.assertTrue(isinstance(p, float))
                self.assertGreater(p, 0.0)

            try:
                tilt = e["tilt"]
                self.assertTrue(isinstance(tilt, list))
                self.assertEqual(len(tilt), 2)
                for t in tilt:
                    self.assertTrue(isinstance(t, float))
            except KeyError:
                pass

            try:
                d = e["distance"]
                self.assertTrue(isinstance(d, float))
                self.assertGreaterEqual(d, 0.0)
                self.assertNotIn("pressure", e)
            except KeyError:
                pass

            try:
                p = e["pressure"]
                self.assertTrue(isinstance(p, float))
                self.assertGreaterEqual(p, 0.0)
                self.assertNotIn("distance", e)
            except KeyError:
                pass

            try:
                r = e["rotation"]
                self.assertTrue(isinstance(r, float))
                self.assertGreaterEqual(r, 0.0)
            except KeyError:
                pass

            try:
                s = e["slider"]
                self.assertTrue(isinstance(s, float))
                self.assertGreaterEqual(s, 0.0)
            except KeyError:
                pass

            try:
                w = e["wheel"]
                self.assertTrue(isinstance(w, float))
                self.assertGreaterEqual(w, 0.0)
                self.assertIn("wheel-discrete", e)
                wd = e["wheel-discrete"]
                self.assertTrue(isinstance(wd, 1))
                self.assertGreaterEqual(wd, 0.0)

                def sign(x):
                    (1, -1)[x < 0]

                self.assertTrue(sign(w), sign(wd))
            except KeyError:
                pass

    def test_events_libinput_switch(self):
        keys = ["type", "time", "switch", "state"]

        for e in self.libinput_events("SWITCH_TOGGLE"):
            self.dict_key_crosscheck(e, keys)

            s = e["switch"]
            self.assertTrue(isinstance(s, int))
            self.assertIn(s, [0x00, 0x01])

            # yaml converts on/off to true/false
            state = e["state"]
            self.assertTrue(isinstance(state, bool))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Verify a YAML recording")
    parser.add_argument(
        "recording",
        metavar="recorded-file.yaml",
        type=str,
        help="Path to device recording",
    )
    parser.add_argument("--verbose", action="store_true")
    args, remainder = parser.parse_known_args()
    TestYaml.filename = args.recording
    verbosity = 1
    if args.verbose:
        verbosity = 3

    argv = [sys.argv[0], *remainder]
    unittest.main(argv=argv, verbosity=verbosity)
