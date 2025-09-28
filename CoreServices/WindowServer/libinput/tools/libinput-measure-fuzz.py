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
#

import os
import sys
import argparse
import subprocess

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


DEFAULT_HWDB_FILE = "/usr/lib/udev/hwdb.d/60-evdev.hwdb"
OVERRIDE_HWDB_FILE = "/etc/udev/hwdb.d/99-touchpad-fuzz-override.hwdb"


class tcolors:
    GREEN = "\033[92m"
    RED = "\033[91m"
    YELLOW = "\033[93m"
    BOLD = "\033[1m"
    NORMAL = "\033[0m"


def print_bold(msg, **kwargs):
    print(tcolors.BOLD + msg + tcolors.NORMAL, **kwargs)


def print_green(msg, **kwargs):
    print(tcolors.BOLD + tcolors.GREEN + msg + tcolors.NORMAL, **kwargs)


def print_yellow(msg, **kwargs):
    print(tcolors.BOLD + tcolors.YELLOW + msg + tcolors.NORMAL, **kwargs)


def print_red(msg, **kwargs):
    print(tcolors.BOLD + tcolors.RED + msg + tcolors.NORMAL, **kwargs)


class InvalidConfigurationError(Exception):
    pass


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
        context = pyudev.Context()
        self.udev_device = pyudev.Devices.from_device_file(context, self.path)

    def find_touch_device(self):
        context = pyudev.Context()
        for device in context.list_devices(subsystem="input"):
            if not device.get("ID_INPUT_TOUCHPAD", 0):
                continue

            if not device.device_node or not device.device_node.startswith(
                "/dev/input/event"
            ):
                continue

            return device.device_node

        print("Unable to find a touch device.", file=sys.stderr)
        sys.exit(1)

    def check_property(self):
        """Return a tuple of (xfuzz, yfuzz) with the fuzz as set in the libinput
        property. Returns None if the property doesn't exist"""

        axes = {
            0x00: self.udev_device.get("LIBINPUT_FUZZ_00"),
            0x01: self.udev_device.get("LIBINPUT_FUZZ_01"),
            0x35: self.udev_device.get("LIBINPUT_FUZZ_35"),
            0x36: self.udev_device.get("LIBINPUT_FUZZ_36"),
        }

        if axes[0x35] is not None:
            if axes[0x35] != axes[0x00]:
                print_bold(
                    "WARNING: fuzz mismatch ABS_X: {}, ABS_MT_POSITION_X: {}".format(
                        axes[0x00], axes[0x35]
                    )
                )

        if axes[0x36] is not None:
            if axes[0x36] != axes[0x01]:
                print_bold(
                    "WARNING: fuzz mismatch ABS_Y: {}, ABS_MT_POSITION_Y: {}".format(
                        axes[0x01], axes[0x36]
                    )
                )

        xfuzz = axes[0x35] or axes[0x00]
        yfuzz = axes[0x36] or axes[0x01]

        if xfuzz is None and yfuzz is None:
            return None

        if (xfuzz is not None and yfuzz is None) or (
            xfuzz is None and yfuzz is not None
        ):
            raise InvalidConfigurationError("fuzz should be set for both axes")

        return (int(xfuzz), int(yfuzz))

    def check_axes(self):
        """
        Returns a tuple of (xfuzz, yfuzz) with the fuzz as set on the device
        axis. Returns None if no fuzz is set.
        """
        if not self.has(libevdev.EV_ABS.ABS_X) or not self.has(libevdev.EV_ABS.ABS_Y):
            raise InvalidDeviceError("device does not have x/y axes")

        if self.has(libevdev.EV_ABS.ABS_MT_POSITION_X) != self.has(
            libevdev.EV_ABS.ABS_MT_POSITION_Y
        ):
            raise InvalidDeviceError("device does not have both multitouch axes")

        xfuzz = (
            self.absinfo[libevdev.EV_ABS.ABS_X].fuzz
            or self.absinfo[libevdev.EV_ABS.ABS_MT_POSITION_X].fuzz
        )
        yfuzz = (
            self.absinfo[libevdev.EV_ABS.ABS_Y].fuzz
            or self.absinfo[libevdev.EV_ABS.ABS_MT_POSITION_Y].fuzz
        )

        if xfuzz == 0 and yfuzz == 0:
            return None

        return (xfuzz, yfuzz)


def print_fuzz(what, fuzz):
    print("  Checking {}... ".format(what), end="")
    if fuzz is None:
        print("not set")
    elif fuzz == (0, 0):
        print("is zero")
    else:
        print("x={} y={}".format(*fuzz))


def handle_existing_entry(device, fuzz):
    # This is getting messy because we don't really know where the entry
    # could be or how the match rule looks like. So we just check the
    # default location only.
    # For the match comparison, we search for the property value in the
    # file. If there is more than one entry that uses the same
    # overrides this will generate false positives.
    # If the lines aren't in the same order in the file, it'll be a false
    # negative.
    overrides = {
        0x00: device.udev_device.get("EVDEV_ABS_00"),
        0x01: device.udev_device.get("EVDEV_ABS_01"),
        0x35: device.udev_device.get("EVDEV_ABS_35"),
        0x36: device.udev_device.get("EVDEV_ABS_36"),
    }

    has_existing_rules = False
    for key, value in overrides.items():
        if value is not None:
            has_existing_rules = True
            break
    if not has_existing_rules:
        return False

    print_red("Error! ", end="")
    print("This device already has axis overrides defined")
    print("")
    print_bold("Searching for existing override...")

    # Construct a template that looks like a hwdb entry (values only) from
    # the udev property values
    template = [
        " EVDEV_ABS_00={}".format(overrides[0x00]),
        " EVDEV_ABS_01={}".format(overrides[0x01]),
    ]
    if overrides[0x35] is not None:
        template += [
            " EVDEV_ABS_35={}".format(overrides[0x35]),
            " EVDEV_ABS_36={}".format(overrides[0x36]),
        ]

    print("Checking in {}... ".format(OVERRIDE_HWDB_FILE), end="")
    entry, prefix, lineno = check_file_for_lines(OVERRIDE_HWDB_FILE, template)
    if entry is not None:
        print_green("found")
        print("The existing hwdb entry can be overwritten")
        return False
    else:
        print_red("not found")
        print("Checking in {}... ".format(DEFAULT_HWDB_FILE), end="")
        entry, prefix, lineno = check_file_for_lines(DEFAULT_HWDB_FILE, template)
        if entry is not None:
            print_green("found")
        else:
            print_red("not found")
            print(
                "The device has a hwdb override defined but it's not where I expected it to be."
            )
            print("Please look at the libinput documentation for more details.")
            print("Exiting now.")
            return True

    print_bold("Probable entry for this device found in line {}:".format(lineno))
    print("\n".join(prefix + entry))
    print("")

    print_bold("Suggested new entry for this device:")
    new_entry = []
    for i in range(0, len(template)):
        parts = entry[i].split(":")
        while len(parts) < 4:
            parts.append("")
        parts[3] = str(fuzz)
        new_entry.append(":".join(parts))
    print("\n".join(prefix + new_entry))
    print("")

    # Not going to overwrite the 60-evdev.hwdb entry with this program, too
    # risky. And it may not be our device match anyway.
    print_bold("You must now:")
    print(
        "\n".join(
            (
                "1. Check the above suggestion for sanity. Does it match your device?",
                "2. Open {} and amend the existing entry".format(DEFAULT_HWDB_FILE),
                "   as recommended above",
                "",
                "   The property format is:",
                "    EVDEV_ABS_00=min:max:resolution:fuzz",
                "",
                "   Leave the entry as-is and only add or amend the fuzz value.",
                "   A non-existent value can be skipped, e.g. this entry sets the ",
                "   resolution to 32 and the fuzz to 8",
                "    EVDEV_ABS_00=::32:8",
                "",
                "3. Save the edited file",
                "4. Say Y to the next prompt",
            )
        )
    )

    cont = input("Continue? [Y/n] ")
    if cont == "n":
        raise KeyboardInterrupt

    if test_hwdb_entry(device, fuzz):
        print_bold("Please test the new fuzz setting by restarting libinput")
        print_bold(
            "Then submit a pull request for this hwdb entry change to "
            "to systemd at http://github.com/systemd/systemd"
        )
    else:
        print_bold("The new fuzz setting did not take effect.")
        print_bold("Did you edit the correct file?")
        print("Please look at the libinput documentation for more details.")
        print("Exiting now.")

    return True


def reload_and_trigger_udev(device):
    import time

    print("Running systemd-hwdb update")
    subprocess.run(["systemd-hwdb", "update"], check=True)
    syspath = device.path.replace("/dev/input/", "/sys/class/input/")
    time.sleep(2)
    print("Running udevadm trigger {}".format(syspath))
    subprocess.run(["udevadm", "trigger", syspath], check=True)
    time.sleep(2)


def test_hwdb_entry(device, fuzz):
    reload_and_trigger_udev(device)
    print_bold("Testing... ", end="")

    d = Device(device.path)
    f = d.check_axes()
    if f is not None:
        if f == (fuzz, fuzz):
            print_yellow("Warning")
            print_bold(
                "The hwdb applied to the device but libinput's udev "
                "rules have not picked it up. This should only happen"
                "if libinput is not installed"
            )
            return True
        else:
            print_red("Error")
            return False
    else:
        f = d.check_property()
        if f is not None and f == (fuzz, fuzz):
            print_green("Success")
            return True
        else:
            print_red("Error")
            return False


def check_file_for_lines(path, template):
    """
    Checks file at path for the lines given in template. If found, the
    return value is a tuple of the matching lines and the prefix (i.e. the
    two lines before the matching lines)
    """
    try:
        lines = [l[:-1] for l in open(path).readlines()]
        idx = -1
        try:
            while idx < len(lines) - 1:
                idx += 1
                line = lines[idx]
                if not line.startswith(" EVDEV_ABS_00"):
                    continue
                if lines[idx : idx + len(template)] != template:
                    continue

                return (lines[idx : idx + len(template)], lines[idx - 2 : idx], idx)

        except IndexError:
            pass
    except FileNotFoundError:
        pass

    return (None, None, None)


def write_udev_rule(device, fuzz):
    """Write out a udev rule that may match the device, run udevadm trigger and
    check if the udev rule worked. Of course, there's plenty to go wrong...
    """
    print("")
    print_bold("Guessing a udev rule to overwrite the fuzz")

    # Some devices match better on pvr, others on pn, so we get to try both. yay
    modalias = open("/sys/class/dmi/id/modalias").readlines()[0]
    ms = modalias.split(":")
    svn, pn, pvr = None, None, None
    for m in ms:
        if m.startswith("svn"):
            svn = m
        elif m.startswith("pn"):
            pn = m
        elif m.startswith("pvr"):
            pvr = m

    # Let's print out both to inform and/or confuse the user
    template = "\n".join(
        (
            "# {} {}",
            "evdev:name:{}:dmi:*:{}*:{}*:",
            " EVDEV_ABS_00=:::{}",
            " EVDEV_ABS_01=:::{}",
            " EVDEV_ABS_35=:::{}",
            " EVDEV_ABS_36=:::{}",
            "",
        )
    )
    rule1 = template.format(
        svn[3:], device.name, device.name, svn, pvr, fuzz, fuzz, fuzz, fuzz
    )
    rule2 = template.format(
        svn[3:], device.name, device.name, svn, pn, fuzz, fuzz, fuzz, fuzz
    )

    print("Full modalias is: {}".format(modalias))
    print()
    print_bold("Suggested udev rule, option 1:")
    print(rule1)
    print()
    print_bold("Suggested udev rule, option 2:")
    print(rule2)
    print("")

    # The weird hwdb matching behavior means we match on the least specific
    # rule (i.e. most wildcards) first although that was supposed to be fixed in
    # systemd 3a04b789c6f1.
    # Our rule uses dmi strings and will be more specific than what 60-evdev.hwdb
    # already has. So we basically throw up our hands because we can't do anything
    # then.
    if handle_existing_entry(device, fuzz):
        return

    while True:
        print_bold("Wich rule do you want to to test? 1 or 2? ", end="")
        yesno = input("Ctrl+C to exit ")

        if yesno == "1":
            rule = rule1
            break
        elif yesno == "2":
            rule = rule2
            break

    fname = OVERRIDE_HWDB_FILE
    try:
        fd = open(fname, "x")
    except FileExistsError:
        yesno = input("File {} exists, overwrite? [Y/n] ".format(fname))
        if yesno.lower == "n":
            return

        fd = open(fname, "w")

    fd.write("# File generated by libinput measure fuzz\n\n")
    fd.write(rule)
    fd.close()

    if test_hwdb_entry(device, fuzz):
        print("Your hwdb override file is in {}".format(fname))
        print_bold("Please test the new fuzz setting by restarting libinput")
        print_bold(
            "Then submit a pull request for this hwdb entry to "
            "systemd at http://github.com/systemd/systemd"
        )
    else:
        print("The hwdb entry failed to apply to the device.")
        print("Removing hwdb file again.")
        os.remove(fname)
        reload_and_trigger_udev(device)
        print_bold("What now?")
        print(
            "1. Re-run this program and try the other suggested udev rule. If that fails,"
        )
        print(
            "2. File a bug with the suggested udev rule at http://github.com/systemd/systemd"
        )


def main(args):
    parser = argparse.ArgumentParser(
        description="Print fuzz settings and/or suggest udev rules for the fuzz to be adjusted."
    )
    parser.add_argument(
        "path",
        metavar="/dev/input/event0",
        nargs="?",
        type=str,
        help="Path to device (optional)",
    )
    parser.add_argument("--fuzz", type=int, help="Suggested fuzz")
    args = parser.parse_args()

    try:
        device = Device(args.path)
        print_bold("Using {}: {}".format(device.name, device.path))

        fuzz = device.check_property()
        print_fuzz("udev property", fuzz)

        fuzz = device.check_axes()
        print_fuzz("axes", fuzz)

        userfuzz = args.fuzz
        if userfuzz is not None:
            write_udev_rule(device, userfuzz)

    except PermissionError:
        print("Permission denied, please re-run as root")
    except InvalidConfigurationError as e:
        print("Error: {}".format(e))
    except InvalidDeviceError as e:
        print("Error: {}".format(e))
    except KeyboardInterrupt:
        print("Exited on user request")


if __name__ == "__main__":
    main(sys.argv)
