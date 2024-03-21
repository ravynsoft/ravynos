#!/bin/sh

# keep_text_section_prefix.sh -- test

# Copyright (C) 2018-2023 Free Software Foundation, Inc.
# Written by Sriraman Tallam <tmsriram@google.com>.

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

# The goal of this program is to verify if .text sections are separated
# according to their prefix. .text.hot, .text.unlikely, .text.startup and
# .text.exit must be separated in the final binary.

set -e

check()
{
    awk "
BEGIN { saw1 = 0; saw2 = 0; err = 0; }
/.*$2\$/ { saw1 = 1; }
/.*$3\$/ {
     saw2 = 1;
     if (!saw1)
       {
	  printf \"layout of $2 and $3 is not right\\n\";
	  err = 1;
	  exit 1;
       }
    }
END {
      if (!saw1 && !err)
        {
	  printf \"did not see $2\\n\";
	  exit 1;
	}
      if (!saw2 && !err)
	{
	  printf \"did not see $3\\n\";
	  exit 1;
	}
    }" $1
}

check_str()
{
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected output in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check_str keep_text_section_prefix_readelf.stdout ".text.hot .text .text.startup .text.exit .text.unlikely"

check keep_text_section_prefix_nm.stdout "hot_foo" "regular_foo"
check keep_text_section_prefix_nm.stdout "regular_foo" "startup_foo"
check keep_text_section_prefix_nm.stdout "startup_foo" "exit_foo"
check keep_text_section_prefix_nm.stdout "exit_foo" "unlikely_foo"
