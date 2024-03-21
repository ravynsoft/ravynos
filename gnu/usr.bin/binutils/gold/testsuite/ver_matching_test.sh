#!/bin/sh

# ver_matching_test.sh -- a test case for version script matching

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

# This file goes with ver_matching_def.cc, a C++ source file
# constructed with several symbols mapped via version_script.map.  We
# run readelf on the resulting shared object and check that each
# symbol has the correct version.

check()
{
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected symbol in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check_missing()
{
    if grep -q "$2" "$1"
    then
	echo "Found unexpected symbol in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check ver_matching_test.stdout "V1  *sizeof_headers$"
check ver_matching_test.stdout "Base  *globaoeufostuff$"
check ver_matching_test.stdout "V1  *globaoeufxstuff$"
check ver_matching_test.stdout "V2  *otherns::stuff$"
check ver_matching_test.stdout "Base  *otherns::biz$"
check ver_matching_test.stdout "V1  *foo$"
check ver_matching_test.stdout "V1  *bar()$"
check ver_matching_test.stdout "Base  *bar1()$"
check ver_matching_test.stdout "V1  *bar2$"
check ver_matching_test.stdout "V1  *myns::blah()$"
check ver_matching_test.stdout "V1  *myns::bip()$"
check ver_matching_test.stdout "V1  *myns::Stuff::Stuff()$"
check ver_matching_test.stdout "Base  *Biz::Biz()$"
check ver_matching_test.stdout "V2  *blaza1$"
check ver_matching_test.stdout "V2  *blaza2$"
check ver_matching_test.stdout "V2  *blaza$"
check ver_matching_test.stdout "Base  *bla$"
check ver_matching_test.stdout "V2  *blaz$"
check ver_matching_test.stdout "V2  *blazb$"

# Stuff inside quotes is matched literally, so "baz(int*, char)" should
# not match the "baz(int *)" entry in the version table.
check ver_matching_test.stdout "V1   *baz(int\\*)$"
check_missing ver_matching_test.stdout "V1   *baz(int\\*, char)$"
check_missing ver_matching_test.stdout "V1   *baz(char\\*, int)$"

check_missing ver_matching_test.stdout "foo1"

# This symbols is injected by the linker itself, but should still
# follow local:
check_missing ver_matching_test.stdout "__bss_start"

exit 0
