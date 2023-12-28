#!/bin/sh

# debug_msg.sh -- a test case for printing debug info for missing symbols.

# Copyright (C) 2006-2023 Free Software Foundation, Inc.
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

# This file goes with debug_msg.cc, a C++ source file constructed to
# have undefined references.  We compile that file with debug
# information and then try to link it, and make sure the proper errors
# are displayed.  The errors will be found in debug_msg.err.

check()
{
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected error in $1:"
	echo "   $2"
	echo ""
	echo "Actual error output below:"
	cat "$1"
	exit 1
    fi
}

check_missing()
{
    if grep -q "$2" "$1"
    then
	echo "Found unexpected error in $1:"
	echo "   $2"
	echo ""
	echo "Actual error output below:"
	cat "$1"
	exit 1
    fi
}

# We don't know how the compiler might order these variables, so we
# can't test for the actual offset from .data, hence the regexp.
check debug_msg.err "debug_msg.o:debug_msg.cc:fn_array:(.*): error: undefined reference to 'undef_fn1()'"
check debug_msg.err "debug_msg.o:debug_msg.cc:fn_array:(.*): error: undefined reference to 'undef_fn2()'"
check debug_msg.err "debug_msg.o:debug_msg.cc:badref1:(.*): error: undefined reference to 'undef_int'"

# These tests check only for the source file's file name (not the complete
# path) because use of -fdebug-prefix-map may change the path to the source
# file recorded in the objects.
check debug_msg.err ".*/debug_msg.cc:50: error: undefined reference to 'undef_fn1()'"
check debug_msg.err ".*/debug_msg.cc:55: error: undefined reference to 'undef_fn2()'"
check debug_msg.err ".*/debug_msg.cc:4[356]: error: undefined reference to 'undef_fn1()'"
check debug_msg.err ".*/debug_msg.cc:4[456]: error: undefined reference to 'undef_fn2()'"
if test "$DEFAULT_TARGET" != "powerpc"
then
  check debug_msg.err ".*/debug_msg.cc:.*: error: undefined reference to 'undef_int'"
fi

# Check we detected the ODR (One Definition Rule) violation.
check debug_msg.err ": symbol 'Ordering::operator()(int, int)' defined in multiple places (possible ODR violation):"
check debug_msg.err "odr_violation1.cc:6"
check debug_msg.err "odr_violation2.cc:1[256]"

# Check we don't have ODR false positives:
check_missing debug_msg.err "OdrDerived::~OdrDerived()"
check_missing debug_msg.err "__adjust_heap"
# We block ODR detection for combinations of C weak and strong
# symbols, to allow people to use the linker to override things.  We
# still flag it for C++ symbols since those are more likely to be
# unintentional.
check_missing debug_msg.err ": symbol 'OverriddenCFunction' defined in multiple places (possible ODR violation):"
check_missing debug_msg.err "odr_violation1.cc:1[6-8]"
check_missing debug_msg.err "odr_violation2.cc:2[3-5]"
check debug_msg.err ": symbol 'SometimesInlineFunction(int)' defined in multiple places (possible ODR violation):"
check debug_msg.err "debug_msg.cc:6[89]"
check debug_msg.err "odr_violation2.cc:3[0-7]"

# Check for the same error messages when using --compressed-debug-sections.
if test -r debug_msg_cdebug.err
then
  check debug_msg_cdebug.err "debug_msg_cdebug.o:debug_msg.cc:fn_array:(.*): error: undefined reference to 'undef_fn1()'"
  check debug_msg_cdebug.err "debug_msg_cdebug.o:debug_msg.cc:fn_array:(.*): error: undefined reference to 'undef_fn2()'"
  check debug_msg_cdebug.err "debug_msg_cdebug.o:debug_msg.cc:badref1:(.*): error: undefined reference to 'undef_int'"
  check debug_msg_cdebug.err ".*/debug_msg.cc:50: error: undefined reference to 'undef_fn1()'"
  check debug_msg_cdebug.err ".*/debug_msg.cc:55: error: undefined reference to 'undef_fn2()'"
  check debug_msg_cdebug.err ".*/debug_msg.cc:4[356]: error: undefined reference to 'undef_fn1()'"
  check debug_msg_cdebug.err ".*/debug_msg.cc:4[456]: error: undefined reference to 'undef_fn2()'"
  if test "$DEFAULT_TARGET" != "powerpc"
  then
    check debug_msg_cdebug.err ".*/debug_msg.cc:.*: error: undefined reference to 'undef_int'"
  fi
  check debug_msg_cdebug.err ": symbol 'Ordering::operator()(int, int)' defined in multiple places (possible ODR violation):"
  check debug_msg_cdebug.err "odr_violation1.cc:6"
  check debug_msg_cdebug.err "odr_violation2.cc:1[256]"
  check_missing debug_msg_cdebug.err "OdrDerived::~OdrDerived()"
  check_missing debug_msg_cdebug.err "__adjust_heap"
  check_missing debug_msg_cdebug.err ": symbol 'OverriddenCFunction' defined in multiple places (possible ODR violation):"
  check_missing debug_msg_cdebug.err "odr_violation1.cc:1[6-8]"
  check_missing debug_msg_cdebug.err "odr_violation2.cc:2[3-5]"
  check debug_msg_cdebug.err ": symbol 'SometimesInlineFunction(int)' defined in multiple places (possible ODR violation):"
  check debug_msg_cdebug.err "debug_msg.cc:6[89]"
  check debug_msg_cdebug.err "odr_violation2.cc:3[0-7]"
fi

# When linking together .so's, we don't catch the line numbers, but we
# still find all the undefined variables, and the ODR violation.
check debug_msg_so.err "debug_msg.so: error: undefined reference to 'undef_fn1()'"
check debug_msg_so.err "debug_msg.so: error: undefined reference to 'undef_fn2()'"
check debug_msg_so.err "debug_msg.so: error: undefined reference to 'undef_int'"
check debug_msg_so.err ": symbol 'Ordering::operator()(int, int)' defined in multiple places (possible ODR violation):"
check debug_msg_so.err "odr_violation1.cc:6"
check debug_msg_so.err "odr_violation2.cc:1[256]"
check_missing debug_msg_so.err "OdrDerived::~OdrDerived()"
check_missing debug_msg_so.err "__adjust_heap"
check_missing debug_msg_so.err ": symbol 'OverriddenCFunction' defined in multiple places (possible ODR violation):"
check_missing debug_msg_so.err "odr_violation1.cc:1[6-8]"
check_missing debug_msg_so.err "odr_violation2.cc:2[3-5]"
check debug_msg_so.err ": symbol 'SometimesInlineFunction(int)' defined in multiple places (possible ODR violation):"
check debug_msg_so.err "debug_msg.cc:6[89]"
check debug_msg_so.err "odr_violation2.cc:3[0-7]"

# These messages shouldn't need any debug info to detect:
check debug_msg_ndebug.err "debug_msg_ndebug.so: error: undefined reference to 'undef_fn1()'"
check debug_msg_ndebug.err "debug_msg_ndebug.so: error: undefined reference to 'undef_fn2()'"
check debug_msg_ndebug.err "debug_msg_ndebug.so: error: undefined reference to 'undef_int'"
# However, we shouldn't detect or declare any ODR violation
check_missing debug_msg_ndebug.err "(possible ODR violation)"

exit 0
