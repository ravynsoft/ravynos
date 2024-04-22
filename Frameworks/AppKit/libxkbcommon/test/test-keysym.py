#!/usr/bin/env python3
#
# This script creates a custom layout, overriding the TDLE key with the first
# argument given.

import argparse
import tempfile
from pathlib import Path
import subprocess
import os
import re
import sys

# Template to force our key to TLDE
template = """
default
xkb_symbols "basic" {{
    include "us(basic)"
    replace key <TLDE> {{ [ {} ] }};
}};
"""

parser = argparse.ArgumentParser(
    description="Tool to verify whether a keysym is resolved"
)
parser.add_argument("keysym", type=str, help="XKB keysym")
parser.add_argument(
    "--tool",
    type=str,
    nargs=1,
    default=["xkbcli", "compile-keymap"],
    help="Full path to the xkbcli-compile-keymap tool",
)
args = parser.parse_args()

with tempfile.TemporaryDirectory() as tmpdir:
    symfile = Path(tmpdir) / "symbols" / "keytest"
    symfile.parent.mkdir()
    with symfile.open(mode="w") as f:
        f.write(template.format(args.keysym))

    try:
        cmd = [
            *args.tool,
            "--layout",
            "keytest",
        ]

        env = os.environ.copy()
        env["XKB_CONFIG_EXTRA_PATH"] = tmpdir

        result = subprocess.run(
            cmd, env=env, capture_output=True, universal_newlines=True
        )
        if result.returncode != 0:
            print("ERROR: Failed to compile:")
            print(result.stderr)
            sys.exit(1)

        # grep for TLDE actually being remapped
        for l in result.stdout.split("\n"):
            match = re.match(r"\s+key \<TLDE\>\s+{\s+\[\s+(?P<keysym>\w+)\s+\]\s+}", l)
            if match:
                if args.keysym == match.group("keysym"):
                    sys.exit(0)
                elif match.group("keysym") == "NoSymbol":
                    print("ERROR: key {} not resolved:".format(args.keysym), l)
                else:
                    print("ERROR: key {} mapped to wrong key:".format(args.keysym), l)
                sys.exit(1)

        print(result.stdout)
        print("ERROR: above keymap is missing key mapping for {}".format(args.keysym))
        sys.exit(1)
    except FileNotFoundError as err:
        print("ERROR: invalid or missing tool: {}".format(err))
        sys.exit(1)
