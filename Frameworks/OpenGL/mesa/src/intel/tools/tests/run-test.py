#!/usr/bin/env python3

import argparse
import difflib
import errno
import os
import pathlib
import subprocess
import sys

# The meson version handles windows paths better, but if it's not available
# fall back to shlex
try:
    from meson.mesonlib import split_args
except ImportError:
    from shlex import split as split_args

parser = argparse.ArgumentParser()
parser.add_argument('--i965_asm',
                    help='path to i965_asm binary')
parser.add_argument('--gen_name',
                    help='name of the hardware generation (as understood by i965_asm)')
parser.add_argument('--gen_folder',
                    type=pathlib.Path,
                    help='name of the folder for the generation')
args = parser.parse_args()

wrapper = os.environ.get('MESON_EXE_WRAPPER')
if wrapper is not None:
    i965_asm = split_args(wrapper) + [args.i965_asm]
else:
    i965_asm = [args.i965_asm]

if not args.gen_folder.is_dir():
    print('Test files path does not exist or is not a directory.',
          file=sys.stderr)
    exit(99)

success = True

for asm_file in args.gen_folder.glob('*.asm'):
    expected_file = asm_file.stem + '.expected'
    expected_path = args.gen_folder / expected_file

    try:
        command = i965_asm + [
            '--type', 'hex',
            '--gen', args.gen_name,
            asm_file
        ]
        with subprocess.Popen(command,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.DEVNULL) as cmd:
            lines_after = [line.decode('ascii') for line in cmd.stdout.readlines()]
    except OSError as e:
        if e.errno == errno.ENOEXEC:
            print('Skipping due to inability to run host binaries.',
                  file=sys.stderr)
            exit(77)
        raise

    with expected_path.open() as f:
        lines_before = f.readlines()

    diff = ''.join(difflib.unified_diff(lines_before, lines_after,
                                        expected_file, asm_file.stem + '.out'))

    if diff:
        print('Output comparison for {}:'.format(asm_file.name))
        print(diff)
        success = False
    else:
        print('{} : PASS'.format(asm_file.name))

if not success:
    exit(1)
