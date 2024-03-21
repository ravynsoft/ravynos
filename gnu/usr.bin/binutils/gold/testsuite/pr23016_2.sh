#!/bin/sh

# pr23016_2.sh -- check that relocations get written to separate sections
# when data sections of the same name are not merged.

# Copyright (C) 2018-2023 Free Software Foundation, Inc.
# Written by Cary Coutant <ccoutant@gmail.com>.

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

set -e

awk '
BEGIN {
    sect = 0;
    relocx = 0;
    relocy = 0;
  }
/^Relocation section .\.relaone/ {
    sect += 1;
  }
/R_X86_64_64.*x \+ 0/ {
    relocx += sect;
  }
/R_X86_64_64.*y \+ 0/ {
    relocy = sect;
  }
END {
    if (relocx != 1)
      {
	printf "Relocation for x should be in first relocation section.\n";
	exit 1;
      }
    if (relocy != 2)
      {
	printf "Relocation for y should be in second relocation section.\n";
	exit 1;
      }
  }
' pr23016_2.stdout

exit 0
