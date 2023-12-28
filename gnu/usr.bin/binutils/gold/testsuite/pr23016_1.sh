#!/bin/sh

# pr23016_1.sh -- check that .eh_frame sections and their relocations
# are merged together even when mixing SHT_PROGBITS and SHT_X86_64_UNWIND.

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

check() {
  awk -v "FILE=$1" '
  BEGIN {
      progbits = 0;
      unwind = 0;
      ehframe_rel = 0;
      relocx = 0;
      relocy = 0;
    }
  /\.eh_frame *PROGBITS/ {
      progbits++;
    }
  /\.eh_frame *X86_64_UNWIND/ {
      unwind++;
    }
  /^Relocation section .\.rela\.eh_frame/ {
      ehframe_rel++;
    }
  /R_X86_64_64.*x \+ 0/ {
      relocx++;
    }
  /R_X86_64_64.*y \+ 0/ {
      relocy++;
    }
  END {
      errs = 0;
      if (progbits != 0)
	{
	  printf "%s: There should be no .eh_frame sections of type PROGBITS.\n", FILE;
	  errs++;
	}
      if (unwind != 1)
	{
	  printf "%s: There should be exactly one .eh_frame section of type X86_64_UNWIND.\n", FILE;
	  errs++;
	}
      if (ehframe_rel != 1)
	{
	  printf "%s: There should be exactly one .rela.eh_frame relocation section.\n", FILE;
	  errs++;
	}
      if (relocx != 1)
	{
	  printf "%s: There should be exactly one relocation for x.\n", FILE;
	  errs++;
	}
      if (relocy != 1)
	{
	  printf "%s: There should be exactly one relocation for y.\n", FILE;
	  errs++;
	}
      exit errs;
    }
  ' $1
}

check pr23016_1.stdout
check pr23016_1r.stdout

exit 0
