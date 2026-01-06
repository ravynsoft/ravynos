##
# Copyright (c) 2005-2010 Apple Inc. All rights reserved.
#
# @APPLE_LICENSE_HEADER_START@
# 
# This file contains Original Code and/or Modifications of Original Code
# as defined in and that are subject to the Apple Public Source License
# Version 2.0 (the 'License'). You may not use this file except in
# compliance with the License. Please obtain a copy of the License at
# http://www.opensource.apple.com/apsl/ and read it before using this
# file.
# 
# The Original Code and all software distributed under the License are
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
# Please see the License for the specific language governing rights and
# limitations under the License.
# 
# @APPLE_LICENSE_HEADER_END@
##
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# The point of this test is a sanity check that ld
# produces good "debug notes" stabs from dwarf .o files
# Running nm through stabs-filter.pl produces connonical stabs
# that can be diffed against a checked in know good set of stabs
#

run: all

all:
	${CXX} ${CCXXFLAGS} -flto -gdwarf-2 hello.cxx -c -o hello.o
	${CXX} ${CCXXFLAGS} -flto -gdwarf-2 other.cxx -c -o other.o
	${CXX} ${CCXXFLAGS} -flto  -gdwarf-2 hello.o other.o -o hello -Wl,-object_path_lto,lto.o
	${FAIL_IF_BAD_MACHO} hello
	# Check that we have a non zero timestamp in the debug note
	nm -ap hello | grep '^[0-9a-z]*[1-9a-z][0-9a-z]* .*OSO.*lto.o$$' | ${FAIL_IF_EMPTY}

    # Same for Incremental LTO now
	${CXX} ${CCXXFLAGS} -flto=thin -gdwarf-2 hello.cxx -c -o hello.o
	${CXX} ${CCXXFLAGS} -flto=thin -gdwarf-2 other.cxx -c -o other.o
	${CXX} ${CCXXFLAGS} -flto=thin  -gdwarf-2 hello.o other.o -o hello.thin -Wl,-object_path_lto,thinlto.o
	${FAIL_IF_BAD_MACHO} hello.thin
	# Check that we have a non zero timestamp in the debug note
	nm -ap hello.thin | grep '^[0-9a-z]*[1-9a-z][0-9a-z]* .*OSO.*thinlto.o/0.thinlto.o$$' | ${FAIL_IF_EMPTY}
	nm -ap hello.thin | grep '^[0-9a-z]*[1-9a-z][0-9a-z]* .*OSO.*thinlto.o/1.thinlto.o$$' | ${FAIL_IF_EMPTY}

	${PASS_IFF} true

clean:
	rm -rf hello hello.thin hello.o other.o lto.o thinlto.o
