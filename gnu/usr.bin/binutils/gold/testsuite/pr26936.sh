#!/bin/sh

# pr26936.sh -- Tests for relocations in debug sections against linkonce
# and comdat sections.

# Copyright (C) 2020-2023 Free Software Foundation, Inc.

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

check()
{
    number_of_occurrence=`$EGREP "$2" ./$1 -o| wc -l`
    if [ $number_of_occurrence != $3 ]
    then
	echo "$1: \"$2\" $3: Failed"
	status=1
    fi
}

status=0
check pr26936a.stdout "^pr26936a.s +6 +0x10108 +x" 1
check pr26936a.stdout "^pr26936b.s +5 +0x10109 +x" 1
check pr26936a.stdout "^pr26936b.s +11 +0x10108 +x" 1
check pr26936a.stdout "^pr26936c.s +6 +0x10108 +x" 1
check pr26936a.stdout "^ +0+10108 0+1" 3
check pr26936a.stdout "^ +0+10109 0+1" 1
check pr26936a.stdout "^ +0+ 0+10109 0+1010a" 1
check pr26936a.stdout "^ +0+ 0+10108 0+10109" 1
check pr26936b.stdout "^pr26936d.s +6 +0x10108 +x" 1
check pr26936b.stdout "^pr26936b.s +5 +0x10109 +x" 1
check pr26936b.stdout "^pr26936b.s +11 +0x10108 +x" 1
check pr26936b.stdout "^pr26936c.s +6 +0x10108 +x" 1
check pr26936b.stdout "^ +0+10108 0+1" 3
check pr26936b.stdout "^ +0+10109 0+1" 1
check pr26936b.stdout "^ +0+ 0+10109 0+1010a" 1
check pr26936b.stdout "^ +0+ 0+10108 0+10109" 1

exit $status
