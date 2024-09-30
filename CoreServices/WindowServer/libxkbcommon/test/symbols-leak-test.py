#!/usr/bin/env python3
"""Check that all exported symbols are specified in the symbol version scripts.

If this fails, please update the appropriate .map file (adding new version
nodes as needed).
"""
import os
import pathlib
import re
import sys


top_srcdir = pathlib.Path(os.environ["top_srcdir"])


def symbols_from_map(path):
    return re.findall(r"^\s+(xkb_.*);", path.read_text("utf-8"), re.MULTILINE)


def symbols_from_src(path):
    return re.findall(r"XKB_EXPORT.*\n(xkb_.*)\(", path.read_text("utf-8"))


def diff(map_path, src_paths):
    map_symbols = set(symbols_from_map(map_path))
    src_symbols = set.union(set(), *(symbols_from_src(path) for path in src_paths))
    return sorted(map_symbols - src_symbols), sorted(src_symbols - map_symbols)


exit = 0

# xkbcommon symbols
left, right = diff(
    top_srcdir / "xkbcommon.map",
    [
        *(top_srcdir / "src").glob("*.c"),
        *(top_srcdir / "src" / "xkbcomp").glob("*.c"),
        *(top_srcdir / "src" / "compose").glob("*.c"),
    ],
)
if left:
    print("xkbcommon map has extra symbols:", " ".join(left))
    exit = 1
if right:
    print("xkbcommon src has extra symbols:", " ".join(right))
    exit = 1

# xkbcommon-x11 symbols
left, right = diff(
    top_srcdir / "xkbcommon-x11.map",
    [
        *(top_srcdir / "src" / "x11").glob("*.c"),
    ],
)
if left:
    print("xkbcommon-x11 map has extra symbols:", " ".join(left))
    exit = 1
if right:
    print("xkbcommon-x11 src has extra symbols:", " ".join(right))
    exit = 1

sys.exit(exit)
