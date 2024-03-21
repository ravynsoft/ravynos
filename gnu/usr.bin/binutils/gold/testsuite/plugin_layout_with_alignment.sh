#!/bin/sh

# plugin_layout_with_alignment.sh -- test

# Copyright (C) 2016-2023 Free Software Foundation, Inc.
# Written by Than McIntosh <thanm@google.com>.

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

# The goal of this program is to verify plugin alignment and size
# interfaces and working correctly in combination with section ordering.
# intended.  File plugin_layout_with_alignment.cc is in this test.


set -e

check()
{
    awk "
BEGIN { saw1 = 0; saw2 = 0; saw3 = 0; saw4 = 0; counter = 1; err = 0; ord = \"\"; }
/.*$2\$/ { saw1 = counter; counter = counter + 1; ord = ord \" $2\";  }
/.*$3\$/ { saw2 = counter; counter = counter + 1; ord = ord \" $3\";  }
/.*$4\$/ { saw3 = counter; counter = counter + 1; ord = ord \" $4\";  }
/.*$5\$/ { saw4 = counter; counter = counter + 1; ord = ord \" $5\";  }
END {
      if (!saw1) {
	  printf \"did not see $2\\n\";
	  exit 1;
	}
      if (!saw2) {
	  printf \"did not see $3\\n\";
	  exit 1;
	}
      if (!saw3) {
	  printf \"did not see $4\\n\";
	  exit 1;
	}
      if (!saw4) {
	  printf \"did not see $5\\n\";
	  exit 1;
	}
      if (saw1 != 1 || saw2 != 2 || saw3 != 3 || saw4 != 4) {
	  printf \"incorrect ordering:\\nwas:%s\\nshould have been: $2 $3 $4 $5\\n\", ord;
	  exit 1;
	}
    }" $1
}

check plugin_layout_with_alignment.stdout "bss_item3" "bss_item1" "bss_item4" "bss_item2"
check plugin_layout_with_alignment.stdout "rwdata_item2" "rwdata_item4" "rwdata_item1" "rwdata_item3"
check plugin_layout_with_alignment.stdout "rodata_item3" "rodata_item1" "rodata_item2" "rodata_item4"
