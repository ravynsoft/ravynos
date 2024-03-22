#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

# Copyright © 2021 Intel Corporation

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

"""Helper script for manipulating the release calendar."""

from __future__ import annotations
import argparse
import csv
import contextlib
import datetime
import pathlib
import subprocess
import typing

if typing.TYPE_CHECKING:
    import _csv
    from typing_extensions import Protocol

    class RCArguments(Protocol):
        """Typing information for release-candidate command arguments."""

        manager: str

    class FinalArguments(Protocol):
        """Typing information for release command arguments."""

        series: str
        manager: str
        zero_released: bool

    class ExtendArguments(Protocol):
        """Typing information for extend command arguments."""

        series: str
        count: int


    CalendarRowType = typing.Tuple[typing.Optional[str], str, str, str, typing.Optional[str]]


_ROOT = pathlib.Path(__file__).parent.parent
CALENDAR_CSV = _ROOT / 'docs' / 'release-calendar.csv'
VERSION = _ROOT / 'VERSION'
LAST_RELEASE = 'This is the last planned release of the {}.x series.'
OR_FINAL = 'Or {}.0 final.'


def read_calendar() -> typing.List[CalendarRowType]:
    """Read the calendar and return a list of it's rows."""
    with CALENDAR_CSV.open('r') as f:
        return [typing.cast('CalendarRowType', tuple(r)) for r in csv.reader(f)]


def commit(message: str) -> None:
    """Commit the changes the the release-calendar.csv file."""
    subprocess.run(['git', 'commit', str(CALENDAR_CSV), '--message', message])



def _calculate_release_start(major: str, minor: str) -> datetime.date:
    """Calculate the start of the release for release candidates.

    This is quarterly, on the second wednesday, in January, April, July, and October.
    """
    quarter = datetime.date.fromisoformat(f'20{major}-0{[1, 4, 7, 10][int(minor)]}-01')

    # Wednesday is 3
    day = quarter.isoweekday()
    if day > 3:
        # this will walk back into the previous month, it's much simpler to
        # duplicate the 14 than handle the calculations for the month and year
        # changing.
        return quarter.replace(day=quarter.day - day + 3 + 14)
    elif day < 3:
        quarter = quarter.replace(day=quarter.day + 3 - day)
    return quarter.replace(day=quarter.day + 14)


def release_candidate(args: RCArguments) -> None:
    """Add release candidate entries."""
    with VERSION.open('r') as f:
        version = f.read().rstrip('-devel')
    major, minor, _ = version.split('.')
    date = _calculate_release_start(major, minor)

    data = read_calendar()

    with CALENDAR_CSV.open('w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(data)

        writer.writerow([f'{major}.{minor}', date.isoformat(), f'{major}.{minor}.0-rc1', args.manager])
        for row in range(2, 4):
            date = date + datetime.timedelta(days=7)
            writer.writerow([None, date.isoformat(), f'{major}.{minor}.0-rc{row}', args.manager])
        date = date + datetime.timedelta(days=7)
        writer.writerow([None, date.isoformat(), f'{major}.{minor}.0-rc4', args.manager, OR_FINAL.format(f'{major}.{minor}')])

    commit(f'docs: Add calendar entries for {major}.{minor} release candidates.')


def _calculate_next_release_date(next_is_zero: bool) -> datetime.date:
    """Calculate the date of the next release.

    If the next is .0, we have the release in seven days, if the next is .1,
    then it's in 14
    """
    date = datetime.date.today()
    day = date.isoweekday()
    if day < 3:
        delta = 3 - day
    elif day > 3:
        # this will walk back into the previous month, it's much simpler to
        # duplicate the 14 than handle the calculations for the month and year
        # changing.
        delta = (3 - day)
    else:
        delta = 0
    delta += 7
    if not next_is_zero:
        delta += 7
    return date + datetime.timedelta(days=delta)


def final_release(args: FinalArguments) -> None:
    """Add final release entries."""
    data = read_calendar()
    date = _calculate_next_release_date(not args.zero_released)

    with CALENDAR_CSV.open('w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(data)

        base = 1 if args.zero_released else 0

        writer.writerow([args.series, date.isoformat(), f'{args.series}.{base}', args.manager])
        for row in range(base + 1, 3):
            date = date + datetime.timedelta(days=14)
            writer.writerow([None, date.isoformat(), f'{args.series}.{row}', args.manager])
        date = date + datetime.timedelta(days=14)
        writer.writerow([None, date.isoformat(), f'{args.series}.3', args.manager, LAST_RELEASE.format(args.series)])

    commit(f'docs: Add calendar entries for {args.series} release.')


def extend(args: ExtendArguments) -> None:
    """Extend a release."""
    @contextlib.contextmanager
    def write_existing(writer: _csv._writer, current: typing.List[CalendarRowType]) -> typing.Iterator[CalendarRowType]:
        """Write the orinal file, yield to insert new entries.

        This is a bit clever, basically what happens it writes out the
        original csv file until it reaches the start of the release after the
        one we're appending, then it yields the last row. When control is
        returned it writes out the rest of the original calendar data.
        """
        last_row: typing.Optional[CalendarRowType] = None
        in_wanted = False
        for row in current:
            if in_wanted and row[0]:
                in_wanted = False
                assert last_row is not None
                yield last_row
            if row[0] == args.series:
                in_wanted = True
            if in_wanted and len(row) >= 5 and row[4] in {LAST_RELEASE.format(args.series), OR_FINAL.format(args.series)}:
                # If this was the last planned release and we're adding more,
                # then we need to remove that message and add it elsewhere
                r = list(row)
                r[4] = None
                # Mypy can't figure this out…
                row = typing.cast('CalendarRowType', tuple(r))
            last_row = row
            writer.writerow(row)
        # If this is the only entry we can hit a case where the contextmanager
        # hasn't yielded
        if in_wanted:
            yield row

    current = read_calendar()

    with CALENDAR_CSV.open('w', newline='') as f:
        writer = csv.writer(f)
        with write_existing(writer, current) as row:
            # Get rid of -rcX as well
            if '-rc' in row[2]:
                first_point = int(row[2].split('rc')[-1]) + 1
                template = '{}.0-rc{}'
                days = 7
            else:
                first_point = int(row[2].split('-')[0].split('.')[-1]) + 1
                template = '{}.{}'
                days = 14

            date = datetime.date.fromisoformat(row[1])
            for i in range(first_point, first_point + args.count):
                date = date + datetime.timedelta(days=days)
                r = [None, date.isoformat(), template.format(args.series, i), row[3], None]
                if i == first_point + args.count - 1:
                    if days == 14:
                        r[4] = LAST_RELEASE.format(args.series)
                    else:
                        r[4] = OR_FINAL.format(args.series)
                writer.writerow(r)

    commit(f'docs: Extend calendar entries for {args.series} by {args.count} releases.')


def main() -> None:
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers()

    rc = sub.add_parser('release-candidate', aliases=['rc'], help='Generate calendar entries for a release candidate.')
    rc.add_argument('manager', help="the name of the person managing the release.")
    rc.set_defaults(func=release_candidate)

    fr = sub.add_parser('release', help='Generate calendar entries for a final release.')
    fr.add_argument('manager', help="the name of the person managing the release.")
    fr.add_argument('series', help='The series to extend, such as "29.3" or "30.0".')
    fr.add_argument('--zero-released', action='store_true', help='The .0 release was today, the next release is .1')
    fr.set_defaults(func=final_release)

    ex = sub.add_parser('extend', help='Generate additional entries for a release.')
    ex.add_argument('series', help='The series to extend, such as "29.3" or "30.0".')
    ex.add_argument('count', type=int, help='The number of new entries to add.')
    ex.set_defaults(func=extend)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
