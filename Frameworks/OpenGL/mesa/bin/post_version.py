#!/usr/bin/env python3
# Copyright © 2019-2020 Intel Corporation

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

"""Update the main page, release notes, and calendar."""

import argparse
import csv
import pathlib
import subprocess


def update_calendar(version: str) -> None:
    p = pathlib.Path('docs') / 'release-calendar.csv'

    with p.open('r') as f:
        calendar = list(csv.reader(f))

    branch = None
    for i, line in enumerate(calendar):
        if line[2] == version:
            if line[0]:
                branch = line[0]
            break
    if branch is not None:
        calendar[i + 1][0] = branch
    del calendar[i]

    with p.open('w') as f:
        writer = csv.writer(f)
        writer.writerows(calendar)

    subprocess.run(['git', 'add', p])


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('version', help="The released version.")
    args = parser.parse_args()

    update_calendar(args.version)

    subprocess.run(['git', 'commit', '-m',
                    f'docs: update calendar for {args.version}'])


if __name__ == "__main__":
    main()
