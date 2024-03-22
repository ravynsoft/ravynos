#!/usr/bin/env python3
# Copyright © 2019 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""This script reads a meson build directory and gives back the command line it
was configured with.

This only works for meson 0.49.0 and newer.
"""

import argparse
import ast
import configparser
import pathlib
import sys


def parse_args() -> argparse.Namespace:
    """Parse arguments."""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'build_dir',
        help='Path the meson build directory')
    args = parser.parse_args()
    return args


def load_config(path: pathlib.Path) -> configparser.ConfigParser:
    """Load config file."""
    conf = configparser.ConfigParser()
    with path.open() as f:
        conf.read_file(f)
    return conf


def build_cmd(conf: configparser.ConfigParser) -> str:
    """Rebuild the command line."""
    args = []
    for k, v in conf['options'].items():
        if ' ' in v:
            args.append(f'-D{k}="{v}"')
        else:
            args.append(f'-D{k}={v}')

    cf = conf['properties'].get('cross_file')
    if cf:
        args.append('--cross-file={}'.format(cf))
    nf = conf['properties'].get('native_file')
    if nf:
        # this will be in the form "['str', 'str']", so use ast.literal_eval to
        # convert it to a list of strings.
        nf = ast.literal_eval(nf)
        args.extend(['--native-file={}'.format(f) for f in nf])
    return ' '.join(args)


def main():
    args = parse_args()
    path = pathlib.Path(args.build_dir, 'meson-private', 'cmd_line.txt')
    if not path.exists():
        print('Cannot find the necessary file to rebuild command line. '
              'Is your meson version >= 0.49.0?', file=sys.stderr)
        sys.exit(1)

    conf = load_config(path)
    cmd = build_cmd(conf)
    print(cmd)


if __name__ == '__main__':
    main()
