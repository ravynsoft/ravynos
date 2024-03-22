# encoding=utf-8
# Copyright © 2018 Intel Corporation

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

"""Script to generate and run glsl optimization tests."""

import argparse
import difflib
import errno
import os
import subprocess
import sys

import sexps
import lower_jump_cases

# The meson version handles windows paths better, but if it's not available
# fall back to shlex
try:
    from meson.mesonlib import split_args
except ImportError:
    from shlex import split as split_args


def arg_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--test-runner',
        required=True,
        help='The glsl_test binary.')
    return parser.parse_args()


def compare(actual, expected):
    """Compare the s-expresions and return a diff if they are different."""
    actual = sexps.sort_decls(sexps.parse_sexp(actual))
    expected = sexps.sort_decls(sexps.parse_sexp(expected))

    if actual == expected:
        return None

    actual = sexps.sexp_to_string(actual)
    expected = sexps.sexp_to_string(expected)

    return difflib.unified_diff(expected.splitlines(), actual.splitlines())


def get_test_runner(runner):
    """Wrap the test runner in the exe wrapper if necessary."""
    wrapper = os.environ.get('MESON_EXE_WRAPPER', None)
    if wrapper is None:
        return [runner]
    return split_args(wrapper) + [runner]


def main():
    """Generate each test and report pass or fail."""
    args = arg_parser()

    total = 0
    passes = 0

    runner = get_test_runner(args.test_runner)

    for gen in lower_jump_cases.CASES:
        for name, opt, source, expected in gen():
            total += 1
            print('{}: '.format(name), end='')
            proc = subprocess.Popen(
                runner + ['optpass', '--quiet', '--input-ir', opt],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                stdin=subprocess.PIPE)
            out, err = proc.communicate(source.encode('utf-8'))
            out = out.decode('utf-8')
            err = err.decode('utf-8')

            if proc.returncode == 255:
                print("Test returned general error, possibly missing linker")
                sys.exit(77)

            if err:
                print('FAIL')
                print('Unexpected output on stderr: {}'.format(err),
                      file=sys.stdout)
                continue

            result = compare(out, expected)
            if result is not None:
                print('FAIL')
                for l in result:
                    print(l, file=sys.stderr)
            else:
                print('PASS')
                passes += 1

    print('{}/{} tests returned correct results'.format(passes, total))
    exit(0 if passes == total else 1)


if __name__ == '__main__':
    try:
        main()
    except OSError as e:
        if e.errno == errno.ENOEXEC:
            print('Skipping due to inability to run host binaries', file=sys.stderr)
            sys.exit(77)
        raise
