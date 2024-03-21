#!/bin/sh

# incremental_test.sh -- test that incremental linking information is correct.

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Rafael Avila de Espindola <espindola@google.com>
# and Cary Coutant <ccoutant@google.com>

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

check_cmp()
{
    if ! cmp -s "$1" "$2"
    then
	echo "Actual output differs from expected:"
	echo "diff $1 $2"
	diff $1 $2
	exit 1
    fi
}

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

# Extract actual command line from linker's -v output.
cat incremental_test.cmdline |
  grep "gcctestdir/\(collect-\)\?ld " |
  sed "s/--incremental[-a-z]* //g" |
  cut -d ' ' -f 2- > actual

# Extract recorded command line from dump of the output file.
cat incremental_test.stdout |
  grep "Link command line" |
  cut -d : -f 2- |
  cut -d ' ' -f 3- |
  sed "s/'//g" > recorded

# Verify that the command line was recorded correctly.
check_cmp actual recorded

rm -f actual recorded

# Filter the incremental-dump output into a format that can be grepped
# more easily.

awk '
    /^[A-Za-z][A-Za-z ]+:$/ { section = $0; }
    /^[[]/ { subsection = $0; }
    /^ / { print section, subsection, $0; }
' < incremental_test.stdout > incremental_test.dump

check incremental_test.dump "Input sections: .* incremental_test_1.o  *1 "
check incremental_test.dump "Input sections: .* incremental_test_2.o  *1 "
check incremental_test.dump "Global symbol table: .* main  .* relocation type "
check incremental_test.dump "Global symbol table: .* a  *incremental_test_1.o "
check incremental_test.dump "Global symbol table: .* a .* relocation type "
check incremental_test.dump "Global symbol table: .* b  *incremental_test_2.o "
check incremental_test.dump "Global symbol table: .* b .* relocation type "
check incremental_test.dump "Global symbol table: .* t1  *incremental_test_2.o "
check incremental_test.dump "Global symbol table: .* t1 .* relocation type "

rm -f incremental_test.dump

exit 0
