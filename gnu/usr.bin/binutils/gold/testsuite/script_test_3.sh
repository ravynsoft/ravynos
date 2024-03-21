#!/bin/sh

# script_test_3.sh -- test PHDRS

# Copyright (C) 2008-2023 Free Software Foundation, Inc.
# Written by Ian Lance Taylor <iant@google.com>.

# This file is part of gold.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
# MA 02110-1301, USA.

# This file goes with script_test_3.t, which is a linker script which
# uses a PHDRS clause.  We run objdump -p on a program linked with
# that linker script.

check()
{
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected segment in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check_count()
{
    if test "`grep -c "$2" "$1"`" != "$3"
    then
	echo "Did not find expected segment in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check_count script_test_3.stdout "^  INTERP" 1
check_count script_test_3.stdout "^  LOAD" 3
check_count script_test_3.stdout "^  DYNAMIC" 1

# Make sure that the size of the INTERP segment is the same as the
# size of the .interp section.
section=`fgrep .interp script_test_3.stdout | grep PROGBITS`
if test "$section" = ""; then
  echo "Did not find .interp section"
  echo ""
  echo "Actual output below:"
  cat script_test_3.stdout
  exit 1
fi
# Remove the brackets around the section number, since they can give
# an unpredictable number of fields.
section=`echo "$section" | sed -e 's/[][]*//g'`
section_size=`echo "$section" | awk '{ print $6; }'`

segment=`grep '^  INTERP' script_test_3.stdout`
# We already checked above that we have an INTERP segment.
segment_size=`echo "$segment" | awk '{ print $5; }'`

# Now $section_size looks like 000013 and $segment_size looks like
# 0x00013.  Both numbers are in hex.
section_size=`echo "$section_size" | sed -e 's/^0*//'`
segment_size=`echo "$segment_size" | sed -e 's/^0x//' -e 's/^0*//'`

if test "$section_size" != "$segment_size"; then
  echo ".interp size $section_size != PT_INTERP size $segment_size"
  exit 1
fi

# At least one PT_LOAD segment should have an alignment >= 0x100000.
found=no
for a in `grep LOAD script_test_3.stdout | sed -e 's/^.* 0x/0x/'`; do
  script="BEGIN { if ($a >= 0x100000) { print \"true\" } else { print \"false\" } }"
  x=`awk "$script" < /dev/null`
  if test "$x" = "true"; then
    found=yes
  fi
done
if test "$found" = "no"; then
  echo "no LOAD segment has required alignment"
  exit 1
fi

exit 0
