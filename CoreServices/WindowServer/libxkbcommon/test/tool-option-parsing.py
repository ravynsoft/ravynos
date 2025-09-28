#!/usr/bin/env python3
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

import itertools
import os
import resource
import sys
import subprocess
import logging
import tempfile
import unittest


try:
    top_builddir = os.environ["top_builddir"]
    top_srcdir = os.environ["top_srcdir"]
except KeyError:
    print(
        "Required environment variables not found: top_srcdir/top_builddir",
        file=sys.stderr,
    )
    from pathlib import Path

    top_srcdir = "."
    try:
        top_builddir = next(Path(".").glob("**/meson-logs/")).parent
    except StopIteration:
        sys.exit(1)
    print(
        'Using srcdir "{}", builddir "{}"'.format(top_srcdir, top_builddir),
        file=sys.stderr,
    )


logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger("test")
logger.setLevel(logging.DEBUG)

# Permutation of RMLVO that we use in multiple tests
rmlvos = [
    list(x)
    for x in itertools.permutations(
        ["--rules=evdev", "--model=pc104", "--layout=ch", "--options=eurosign:5"]
    )
]


def _disable_coredump():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def run_command(args):
    logger.debug("run command: {}".format(" ".join(args)))

    try:
        p = subprocess.run(
            args,
            preexec_fn=_disable_coredump,
            capture_output=True,
            text=True,
            timeout=0.7,
        )
        return p.returncode, p.stdout, p.stderr
    except subprocess.TimeoutExpired as e:
        return 0, e.stdout, e.stderr


class XkbcliTool:
    xkbcli_tool = "xkbcli"
    subtool = None

    def __init__(self, subtool=None, skipIf=(), skipError=()):
        self.tool_path = top_builddir
        self.subtool = subtool
        self.skipIf = skipIf
        self.skipError = skipError

    def run_command(self, args):
        for condition, reason in self.skipIf:
            if condition:
                raise unittest.SkipTest(reason)
        if self.subtool is not None:
            tool = "{}-{}".format(self.xkbcli_tool, self.subtool)
        else:
            tool = self.xkbcli_tool
        args = [os.path.join(self.tool_path, tool)] + args

        return run_command(args)

    def run_command_success(self, args):
        rc, stdout, stderr = self.run_command(args)
        if rc != 0:
            for testfunc, reason in self.skipError:
                if testfunc(rc, stdout, stderr):
                    raise unittest.SkipTest(reason)
        assert rc == 0, (rc, stdout, stderr)
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

    def __str__(self):
        return str(self.subtool)


class TestXkbcli(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.xkbcli = XkbcliTool()
        cls.xkbcli_list = XkbcliTool(
            "list",
            skipIf=(
                (
                    not int(os.getenv("HAVE_XKBCLI_LIST", "1")),
                    "xkbregistory not enabled",
                ),
            ),
        )
        cls.xkbcli_how_to_type = XkbcliTool("how-to-type")
        cls.xkbcli_compile_keymap = XkbcliTool("compile-keymap")
        cls.xkbcli_interactive_evdev = XkbcliTool(
            "interactive-evdev",
            skipIf=(
                (
                    not int(os.getenv("HAVE_XKBCLI_INTERACTIVE_EVDEV", "1")),
                    "evdev not enabled",
                ),
                (not os.path.exists("/dev/input/event0"), "event node required"),
                (
                    not os.access("/dev/input/event0", os.R_OK),
                    "insufficient permissions",
                ),
            ),
            skipError=(
                (
                    lambda rc, stdout, stderr: "Couldn't find any keyboards" in stderr,
                    "No keyboards available",
                ),
            ),
        )
        cls.xkbcli_interactive_x11 = XkbcliTool(
            "interactive-x11",
            skipIf=(
                (
                    not int(os.getenv("HAVE_XKBCLI_INTERACTIVE_X11", "1")),
                    "x11 not enabled",
                ),
                (not os.getenv("DISPLAY"), "DISPLAY not set"),
            ),
        )
        cls.xkbcli_interactive_wayland = XkbcliTool(
            "interactive-wayland",
            skipIf=(
                (
                    not int(os.getenv("HAVE_XKBCLI_INTERACTIVE_WAYLAND", "1")),
                    "wayland not enabled",
                ),
                (not os.getenv("WAYLAND_DISPLAY"), "WAYLAND_DISPLAY not set"),
            ),
        )
        cls.all_tools = [
            cls.xkbcli,
            cls.xkbcli_list,
            cls.xkbcli_how_to_type,
            cls.xkbcli_compile_keymap,
            cls.xkbcli_interactive_evdev,
            cls.xkbcli_interactive_x11,
            cls.xkbcli_interactive_wayland,
        ]

    def test_help(self):
        # --help is supported by all tools
        for tool in self.all_tools:
            with self.subTest(tool=tool):
                stdout, stderr = tool.run_command_success(["--help"])
                assert stdout.startswith("Usage:")
                assert stderr == ""

    def test_invalid_option(self):
        # --foobar generates "Usage:" for all tools
        for tool in self.all_tools:
            with self.subTest(tool=tool):
                tool.run_command_unrecognized_option(["--foobar"])

    def test_xkbcli_version(self):
        # xkbcli --version
        stdout, stderr = self.xkbcli.run_command_success(["--version"])
        assert stdout.startswith("1")
        assert stderr == ""

    def test_xkbcli_too_many_args(self):
        self.xkbcli.run_command_invalid(["a"] * 64)

    def test_compile_keymap_args(self):
        for args in (
            ["--verbose"],
            ["--rmlvo"],
            # ['--kccgst'],
            ["--verbose", "--rmlvo"],
            # ['--verbose', '--kccgst'],
        ):
            with self.subTest(args=args):
                self.xkbcli_compile_keymap.run_command_success(args)

    def test_compile_keymap_rmlvo(self):
        for rmlvo in rmlvos:
            with self.subTest(rmlvo=rmlvo):
                self.xkbcli_compile_keymap.run_command_success(rmlvo)

    def test_compile_keymap_include(self):
        for args in (
            ["--include", ".", "--include-defaults"],
            ["--include", "/tmp", "--include-defaults"],
        ):
            with self.subTest(args=args):
                # Succeeds thanks to include-defaults
                self.xkbcli_compile_keymap.run_command_success(args)

    def test_compile_keymap_include_invalid(self):
        # A non-directory is rejected by default
        args = ["--include", "/proc/version"]
        rc, stdout, stderr = self.xkbcli_compile_keymap.run_command(args)
        assert rc == 1, (stdout, stderr)
        assert "There are no include paths to search" in stderr

        # A non-existing directory is rejected by default
        args = ["--include", "/tmp/does/not/exist"]
        rc, stdout, stderr = self.xkbcli_compile_keymap.run_command(args)
        assert rc == 1, (stdout, stderr)
        assert "There are no include paths to search" in stderr

        # Valid dir, but missing files
        args = ["--include", "/tmp"]
        rc, stdout, stderr = self.xkbcli_compile_keymap.run_command(args)
        assert rc == 1, (stdout, stderr)
        assert "Couldn't look up rules" in stderr

    def test_how_to_type(self):
        # Unicode codepoint conversions, we support whatever strtol does
        for args in (["123"], ["0x123"], ["0123"]):
            with self.subTest(args=args):
                self.xkbcli_how_to_type.run_command_success(args)

    def test_how_to_type_rmlvo(self):
        for rmlvo in rmlvos:
            with self.subTest(rmlvo=rmlvo):
                args = rmlvo + ["0x1234"]
                self.xkbcli_how_to_type.run_command_success(args)

    def test_list_rmlvo(self):
        for args in (
            ["--verbose"],
            ["-v"],
            ["--verbose", "--load-exotic"],
            ["--load-exotic"],
            ["--ruleset=evdev"],
            ["--ruleset=base"],
        ):
            with self.subTest(args=args):
                self.xkbcli_list.run_command_success(args)

    def test_list_rmlvo_includes(self):
        args = ["/tmp/"]
        self.xkbcli_list.run_command_success(args)

    def test_list_rmlvo_includes_invalid(self):
        args = ["/proc/version"]
        rc, stdout, stderr = self.xkbcli_list.run_command(args)
        assert rc == 1
        assert "Failed to append include path" in stderr

    def test_list_rmlvo_includes_no_defaults(self):
        args = ["--skip-default-paths", "/tmp"]
        rc, stdout, stderr = self.xkbcli_list.run_command(args)
        assert rc == 1
        assert "Failed to parse XKB description" in stderr

    def test_interactive_evdev_rmlvo(self):
        for rmlvo in rmlvos:
            with self.subTest(rmlvo=rmlvo):
                self.xkbcli_interactive_evdev.run_command_success(rmlvo)

    def test_interactive_evdev(self):
        # Note: --enable-compose fails if $prefix doesn't have the compose tables
        # installed
        for args in (
            ["--report-state-changes"],
            ["--enable-compose"],
            ["--consumed-mode=xkb"],
            ["--consumed-mode=gtk"],
            ["--without-x11-offset"],
        ):
            with self.subTest(args=args):
                self.xkbcli_interactive_evdev.run_command_success(args)

    def test_interactive_x11(self):
        # To be filled in if we handle something other than --help
        pass

    def test_interactive_wayland(self):
        # To be filled in if we handle something other than --help
        pass


if __name__ == "__main__":
    with tempfile.TemporaryDirectory() as tmpdir:
        # Use our own test xkeyboard-config copy.
        os.environ["XKB_CONFIG_ROOT"] = top_srcdir + "/test/data"
        # Use our own X11 locale copy.
        os.environ["XLOCALEDIR"] = top_srcdir + "/test/data/locale"
        # Use our own locale.
        os.environ["LC_CTYPE"] = "en_US.UTF-8"
        # libxkbcommon has fallbacks when XDG_CONFIG_HOME isn't set so we need
        # to override it with a known (empty) directory. Otherwise our test
        # behavior depends on the system the test is run on.
        os.environ["XDG_CONFIG_HOME"] = tmpdir
        # Prevent the legacy $HOME/.xkb from kicking in.
        del os.environ["HOME"]
        # This needs to be separated if we do specific extra path testing
        os.environ["XKB_CONFIG_EXTRA_PATH"] = tmpdir

        unittest.main()
