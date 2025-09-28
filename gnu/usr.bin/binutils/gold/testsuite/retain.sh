#!/bin/sh

# retain.sh -- Tests for SHF_GNU_RETAIN.

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
    number_of_occurrence=`$EGREP "$2" ./$1 -o | wc -l`
    if [ $number_of_occurrence != $3 ]
    then
	echo "$1: \"$2\" $3: Failed"
	status=1
    fi
}

status=0
check retain_1.out " T fnretain1" 1
check retain_1.out " b lsretain0.2" 1
check retain_1.out " b lsretain1.1" 1
check retain_1.out " d lsretain2.0" 1
check retain_1.out " B retain0" 1
check retain_1.out " B retain1" 1
check retain_1.out " D retain2" 1
check retain_1.out " b sretain0" 1
check retain_1.out " b sretain1" 1
check retain_1.out " d sretain2" 1
if grep discard retain_1.out
then
  echo "retain_1.out: Garbage collection failed"
  status=1
fi

check retain_2.out " \(PREINIT_ARRAY\)" 1
check retain_2.out " \(PREINIT_ARRAYSZ\)" 1
check retain_2.out " \(INIT_ARRAY\)" 1
check retain_2.out " \(INIT_ARRAYSZ\)" 1
check retain_2.out " \(FINI_ARRAY\)" 1
check retain_2.out " \(FINI_ARRAYSZ\)" 1

exit $status
