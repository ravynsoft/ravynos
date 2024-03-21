#!/bin/sh

# relro_test.sh -- test -z relro

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
# Written by Cary Coutant <ccoutant@google.com>.

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

# This test checks that the PT_GNU_RELRO segment is properly
# aligned and is coincident with the beginning of the data segment.


# Cleans a hexadecimal number for input to dc.
clean_hex()
{
  echo "$1" | sed -e 's/0x//' -e 'y/abcdef/ABCDEF/'
}

check()
{
  # Get the address and length of the PT_GNU_RELRO segment.
  RELRO_START=`grep GNU_RELRO "$1" | awk '{ print $3; }'`
  RELRO_LEN=`grep GNU_RELRO "$1" | awk '{ print $6; }'`

  if test -z "$RELRO_START"
  then
    echo "Did not find a PT_GNU_RELRO segment."
    exit 1
  fi

  # Get the address and alignment of the PT_LOAD segment whose address
  # matches the PT_GNU_RELRO segment.
  LOAD_ALIGN=`grep LOAD "$1" | awk -v A=$RELRO_START '$3 == A { print $NF; }'`
  LOAD_LEN=`grep LOAD "$1" | awk -v A=$RELRO_START '$3 == A { print $6; }'`

  if test -z "$LOAD_LEN"
  then
    echo "Did not find a PT_LOAD segment matching the PT_GNU_RELRO segment."
    exit 1
  fi

  # Compute the address of the end of the PT_GNU_RELRO segment,
  # modulo the alignment of the PT_LOAD segment.
  RELRO_START=`clean_hex "$RELRO_START"`
  RELRO_LEN=`clean_hex "$RELRO_LEN"`
  LOAD_ALIGN=`clean_hex "$LOAD_ALIGN"`
  RELRO_END=`echo "16o 16i $RELRO_START $RELRO_LEN + p" | dc`
  REM=`echo "16i $RELRO_END $LOAD_ALIGN % p" | dc`

  if test "$REM" -eq 0; then
    :
  else
    echo "PT_GNU_RELRO segment does not end at page boundary."
    exit 1
  fi
}

check relro_test.stdout
