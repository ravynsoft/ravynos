#!/bin/sh

# arm_exidx_test.sh -- a test case for .ARM.exidx section.

# Copyright (C) 2011-2023 Free Software Foundation, Inc.
# Written by Doug Kwan <dougkwan@google.com>.

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

# This file goes with arm_exidx_test.s, an ARM assembly source file constructed
# to test handling of .ARM.exidx and .ARM.extab sections.

check()
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

check_not()
{
    if grep -q "$2" "$1"
    then
	echo "Found unexpected output in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

# Check that SHF_LINK_ORDER is set.
check arm_exidx_test.stdout ".* .ARM.exidx .* ARM_EXIDX .* AL .*"
check arm_exidx_test.stdout ".* .ARM.extab .* PROGBITS .* A .*"
check_not arm_exidx_test.stdout ".* .* R_ARM_GLOB_DAT .* __exidx_start"
check_not arm_exidx_test.stdout ".* .* R_ARM_GLOB_DAT .* __exidx_end"

exit 0
