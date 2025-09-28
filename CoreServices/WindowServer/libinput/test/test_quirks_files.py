#!/usr/bin/env python3
#
# This file is formatted with Python Black
#
# Run with pytest

from pathlib import Path

import configparser
import os
import pytest
import re

# see the IDs from
# https://github.com/torvalds/linux/blob/master/drivers/hid/hid-ids.h#L772
# https://github.com/torvalds/linux/blob/master/drivers/hid/hid-logitech-dj.c#L1826
logitech_receivers = [
    0xC50C,  # USB_DEVICE_ID_S510_RECEIVER
    0xC517,  # USB_DEVICE_ID_S510_RECEIVER_2
    0xC512,  # USB_DEVICE_ID_LOGITECH_CORDLESS_DESKTOP_LX500
    0xC513,  # USB_DEVICE_ID_MX3000_RECEIVER
    0xC51B,  # USB_DEVICE_ID_LOGITECH_27MHZ_MOUSE_RECEIVER
    0xC52B,  # USB_DEVICE_ID_LOGITECH_UNIFYING_RECEIVER
    0xC52F,  # USB_DEVICE_ID_LOGITECH_NANO_RECEIVER
    0xC532,  # USB_DEVICE_ID_LOGITECH_UNIFYING_RECEIVER_2
    0xC534,  # USB_DEVICE_ID_LOGITECH_NANO_RECEIVER_2
    0xC539,  # USB_DEVICE_ID_LOGITECH_NANO_RECEIVER_LIGHTSPEED_1
    0xC53F,  # USB_DEVICE_ID_LOGITECH_NANO_RECEIVER_LIGHTSPEED_1_1
    0xC53A,  # USB_DEVICE_ID_LOGITECH_NANO_RECEIVER_POWERPLAY
    0xC545,  # Bolt receiver, not listed in the kernel (yet)
    0xC547,  # Bolt receiver, not listed in the kernel (yet)
    0xC548,  # Bolt receiver, not listed in the kernel (yet)
]


def quirksdir():
    return Path(os.getenv("MESON_SOURCE_ROOT") or ".") / "quirks"


def pytest_generate_tests(metafunc):
    # for any function that takes a "quirksfile" argument return the path to
    # a quirks file
    if "quirksfile" in metafunc.fixturenames:
        metafunc.parametrize("quirksfile", [f for f in quirksdir().glob("*.quirks")])


def test_matches_are_valid(quirksfile):
    quirks = configparser.ConfigParser(strict=True)
    # Don't convert to lowercase
    quirks.optionxform = lambda option: option  # type: ignore
    quirks.read(quirksfile)

    for name, section in filter(lambda n: n != "DEFAULT", quirks.items()):
        bus = section.get("MatchBus")
        if bus is not None:
            assert bus in ("ps2", "usb", "bluetooth", "i2c", "spi")

        vid = section.get("MatchVendor")
        if vid is not None:
            assert re.match(
                "0x[0-9A-F]{4}", vid
            ), f"{quirksfile}: {name}: {vid} must be uppercase hex (0xAB12)"

        pid = section.get("MatchProduct")
        if pid is not None:
            assert re.match(
                "0x[0-9A-F]{4}", pid
            ), f"{quirksfile}: {name}: {pid} must be uppercase hex (0xAB12)"


def test_match_product_is_not_a_logitech_receiver(quirksfile):
    quirks = configparser.ConfigParser(strict=True)
    # Don't convert to lowercase
    quirks.optionxform = lambda option: option  # type: ignore
    quirks.read(quirksfile)

    for name, section in filter(lambda n: n != "DEFAULT", quirks.items()):
        vid = int(section.get("MatchVendor", "0x0"), 16)
        if vid == 0x046D:
            pid = int(section.get("MatchProduct", "0x0"), 16)
            assert (
                pid not in logitech_receivers
            ), f"{quirksfile}: {name}: applies to a Logitech Receiver"


def main():
    args = [__file__]
    try:
        import xdist  # noqa

        ncores = os.environ.get("FDO_CI_CONCURRENT", "auto")
        args += ["-n", ncores]
    except ImportError:
        pass

    return pytest.main(args)


if __name__ == "__main__":
    raise SystemExit(main())
