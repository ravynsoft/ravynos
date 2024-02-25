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
import resource
import sys
import subprocess
import logging

try:
    import pytest
except ImportError:
    print("Failed to import pytest. Skipping.", file=sys.stderr)
    sys.exit(77)


logger = logging.getLogger("test")
logger.setLevel(logging.DEBUG)

if "@DISABLE_WARNING@" != "yes":
    print("This is the source file, run the one in the meson builddir instead")
    sys.exit(1)


def _disable_coredump():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def run_command(args):
    logger.debug("run command: {}".format(" ".join(args)))
    with subprocess.Popen(
        args,
        preexec_fn=_disable_coredump,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    ) as p:
        try:
            p.wait(0.7)
        except subprocess.TimeoutExpired:
            p.send_signal(3)  # SIGQUIT
        stdout, stderr = p.communicate(timeout=5)
        if p.returncode == -3:
            p.returncode = 0
        return p.returncode, stdout.decode("UTF-8"), stderr.decode("UTF-8")


class LibinputTool(object):
    libinput_tool = "libinput"
    subtool = None

    def __init__(self, subtool=None):
        self.libinput_tool = "@TOOL_PATH@"
        self.subtool = subtool

    def run_command(self, args):
        args = [self.libinput_tool] + args
        if self.subtool is not None:
            args.insert(1, self.subtool)

        return run_command(args)

    def run_command_success(self, args):
        rc, stdout, stderr = self.run_command(args)
        # if we're running as user, we might fail the command but we should
        # never get rc 2 (invalid usage)
        assert rc in [0, 1], (stdout, stderr)
        return stdout, stderr

    def run_command_invalid(self, args):
        rc, stdout, stderr = self.run_command(args)
        assert rc == 2, (rc, stdout, stderr)
        return rc, stdout, stderr

    def run_command_unrecognized_option(self, args):
        rc, stdout, stderr = self.run_command(args)
        assert rc == 2, (rc, stdout, stderr)
        assert stdout.startswith("Usage") or stdout == ""
        assert "unrecognized option" in stderr

    def run_command_missing_arg(self, args):
        rc, stdout, stderr = self.run_command(args)
        assert rc == 2, (rc, stdout, stderr)
        assert stdout.startswith("Usage") or stdout == ""
        assert "requires an argument" in stderr

    def run_command_unrecognized_tool(self, args):
        rc, stdout, stderr = self.run_command(args)
        assert rc == 2, (rc, stdout, stderr)
        assert stdout.startswith("Usage") or stdout == ""
        assert "is not installed" in stderr


class LibinputDebugGui(LibinputTool):
    def __init__(self, subtool="debug-gui"):
        assert subtool == "debug-gui"
        super().__init__(subtool)

        debug_gui_enabled = "@MESON_ENABLED_DEBUG_GUI@" == "True"
        if not debug_gui_enabled:
            pytest.skip()

        if not os.getenv("DISPLAY") and not os.getenv("WAYLAND_DISPLAY"):
            pytest.skip()

        # 77 means gtk_init() failed, which is probably because you can't
        # connect to the display server.
        rc, _, _ = self.run_command(["--help"])
        if rc == 77:
            pytest.skip()


def get_tool(subtool=None):
    if subtool == "debug-gui":
        return LibinputDebugGui()
    else:
        return LibinputTool(subtool)


@pytest.fixture
def libinput():
    return get_tool()


@pytest.fixture(params=["debug-events", "debug-gui"])
def libinput_debug_tool(request):
    yield get_tool(request.param)


@pytest.fixture
def libinput_debug_events():
    return get_tool("debug-events")


@pytest.fixture
def libinput_debug_gui():
    return get_tool("debug-gui")


@pytest.fixture
def libinput_record():
    return get_tool("record")


def test_help(libinput):
    stdout, stderr = libinput.run_command_success(["--help"])
    assert stdout.startswith("Usage:")
    assert stderr == ""


def test_version(libinput):
    stdout, stderr = libinput.run_command_success(["--version"])
    assert stdout.startswith("1")
    assert stderr == ""


@pytest.mark.parametrize("argument", ["--banana", "--foo", "--quiet", "--verbose"])
def test_invalid_arguments(libinput, argument):
    libinput.run_command_unrecognized_option([argument])


@pytest.mark.parametrize("tool", [["foo"], ["debug"], ["foo", "--quiet"]])
def test_invalid_tool(libinput, tool):
    libinput.run_command_unrecognized_tool(tool)


def test_udev_seat(libinput_debug_tool):
    libinput_debug_tool.run_command_missing_arg(["--udev"])
    libinput_debug_tool.run_command_success(["--udev", "seat0"])
    libinput_debug_tool.run_command_success(["--udev", "seat1"])


@pytest.mark.skipif(os.environ.get("UDEV_NOT_AVAILABLE"), reason="udev required")
def test_device_arg(libinput_debug_tool):
    libinput_debug_tool.run_command_missing_arg(["--device"])
    libinput_debug_tool.run_command_success(["--device", "/dev/input/event0"])
    libinput_debug_tool.run_command_success(["--device", "/dev/input/event1"])
    libinput_debug_tool.run_command_success(["/dev/input/event0"])


options = {
    "pattern": ["sendevents"],
    # enable/disable options
    "enable-disable": [
        "tap",
        "drag",
        "drag-lock",
        "middlebutton",
        "natural-scrolling",
        "left-handed",
        "dwt",
        "dwtp",
    ],
    # options with distinct values
    "enums": {
        "set-click-method": ["none", "clickfinger", "buttonareas"],
        "set-scroll-method": ["none", "twofinger", "edge", "button"],
        "set-profile": ["adaptive", "flat"],
        "set-tap-map": ["lrm", "lmr"],
    },
    # options with a range (and increment)
    "ranges": {
        "set-speed": (-1.0, +1.0, 0.1),
        "set-rotation": (0, 360, 10),
    },
}


# Options that allow for glob patterns
@pytest.mark.parametrize("option", options["pattern"])
def test_options_pattern(libinput_debug_tool, option):
    libinput_debug_tool.run_command_success(["--disable-{}".format(option), "*"])
    libinput_debug_tool.run_command_success(["--disable-{}".format(option), "abc*"])


@pytest.mark.parametrize("option", options["enable-disable"])
def test_options_enable_disable(libinput_debug_tool, option):
    libinput_debug_tool.run_command_success(["--enable-{}".format(option)])
    libinput_debug_tool.run_command_success(["--disable-{}".format(option)])


@pytest.mark.parametrize("option", options["enums"].items())
def test_options_enums(libinput_debug_tool, option):
    name, values = option
    for v in values:
        libinput_debug_tool.run_command_success(["--{}".format(name), v])
        libinput_debug_tool.run_command_success(["--{}={}".format(name, v)])


@pytest.mark.parametrize("option", options["ranges"].items())
def test_options_ranges(libinput_debug_tool, option):
    name, values = option
    minimum, maximum, step = values
    value = minimum
    while value < maximum:
        libinput_debug_tool.run_command_success(["--{}".format(name), str(value)])
        libinput_debug_tool.run_command_success(["--{}={}".format(name, value)])
        value += step
    libinput_debug_tool.run_command_success(["--{}".format(name), str(maximum)])
    libinput_debug_tool.run_command_success(["--{}={}".format(name, maximum)])


def test_apply_to(libinput_debug_tool):
    libinput_debug_tool.run_command_missing_arg(["--apply-to"])
    libinput_debug_tool.run_command_success(["--apply-to", "*foo*"])
    libinput_debug_tool.run_command_success(["--apply-to", "foobar"])
    libinput_debug_tool.run_command_success(["--apply-to", "any"])


@pytest.mark.parametrize(
    "args",
    [["--verbose"], ["--quiet"], ["--verbose", "--quiet"], ["--quiet", "--verbose"]],
)
def test_debug_events_verbose_quiet(libinput_debug_events, args):
    libinput_debug_events.run_command_success(args)


@pytest.mark.parametrize("arg", ["--banana", "--foo", "--version"])
def test_invalid_args(libinput_debug_tool, arg):
    libinput_debug_tool.run_command_unrecognized_option([arg])


def test_libinput_debug_events_multiple_devices(libinput_debug_events):
    libinput_debug_events.run_command_success(
        ["--device", "/dev/input/event0", "/dev/input/event1"]
    )
    # same event path multiple times? meh, your problem
    libinput_debug_events.run_command_success(
        ["--device", "/dev/input/event0", "/dev/input/event0"]
    )
    libinput_debug_events.run_command_success(
        ["/dev/input/event0", "/dev/input/event1"]
    )


def test_libinput_debug_events_too_many_devices(libinput_debug_events):
    # Too many arguments just bails with the usage message
    rc, stdout, stderr = libinput_debug_events.run_command(["/dev/input/event0"] * 61)
    assert rc == 2, (stdout, stderr)


@pytest.mark.parametrize("arg", ["--quiet"])
def test_libinput_debug_gui_invalid_arg(libinput_debug_gui, arg):
    libinput_debug_gui.run_command_unrecognized_option([arg])


def test_libinput_debug_gui_verbose(libinput_debug_gui):
    libinput_debug_gui.run_command_success(["--verbose"])


@pytest.mark.parametrize(
    "arg", ["--help", "--show-keycodes", "--with-libinput", "--with-hidraw"]
)
def test_libinput_record_args(libinput_record, arg):
    libinput_record.run_command_success([arg])


def test_libinput_record_multiple_arg(libinput_record):
    # this arg is deprecated and a noop
    libinput_record.run_command_success(["--multiple"])


@pytest.fixture
def recording(tmp_path):
    return str((tmp_path / "record.out").resolve())


def test_libinput_record_all(libinput_record, recording):
    libinput_record.run_command_success(["--all", "-o", recording])
    libinput_record.run_command_success(["--all", recording])


def test_libinput_record_outfile(libinput_record, recording):
    libinput_record.run_command_success(["-o", recording])
    libinput_record.run_command_success(["--output-file", recording])
    libinput_record.run_command_success(["--output-file={}".format(recording)])


def test_libinput_record_single(libinput_record, recording):
    libinput_record.run_command_success(["/dev/input/event0"])
    libinput_record.run_command_success(["-o", recording, "/dev/input/event0"])
    libinput_record.run_command_success(["/dev/input/event0", recording])
    libinput_record.run_command_success([recording, "/dev/input/event0"])


def test_libinput_record_multiple(libinput_record, recording):
    libinput_record.run_command_success(
        ["-o", recording, "/dev/input/event0", "/dev/input/event1"]
    )
    libinput_record.run_command_success(
        [recording, "/dev/input/event0", "/dev/input/event1"]
    )
    libinput_record.run_command_success(
        ["/dev/input/event0", "/dev/input/event1", recording]
    )


def test_libinput_record_autorestart(libinput_record, recording):
    libinput_record.run_command_invalid(["--autorestart"])
    libinput_record.run_command_invalid(["--autorestart=2"])
    libinput_record.run_command_success(["-o", recording, "--autorestart=2"])


def main():
    args = ["-m", "pytest"]
    try:
        import xdist  # noqa

        ncores = os.environ.get("FDO_CI_CONCURRENT", "auto")
        args += ["-n", ncores]
    except ImportError:
        logger.info("python-xdist missing, this test will be slow")
        pass

    args += ["@MESON_BUILD_ROOT@"]

    os.environ["LIBINPUT_RUNNING_TEST_SUITE"] = "1"

    return subprocess.run([sys.executable] + args).returncode


if __name__ == "__main__":
    raise SystemExit(main())
