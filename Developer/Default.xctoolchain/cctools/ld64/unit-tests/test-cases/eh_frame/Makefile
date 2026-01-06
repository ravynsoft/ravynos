##
# Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
# strips .eh symbols out of final linked images,
# even when an intermediate ld -r was used.
#

run: all

all:
	${CXX} ${CCXXFLAGS} foo.cxx -dynamiclib -o libfoo.dylib
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	nm libfoo.dylib | grep '.eh' | ${FAIL_IF_STDIN}
	${CXX} ${CCXXFLAGS} foo.cxx -c -o foo.o
	${FAIL_IF_BAD_OBJ} foo.o
	${LD} -r foo.o -o foo2.o
	${FAIL_IF_BAD_OBJ} foo2.o
	${CXX} ${CCXXFLAGS} foo2.o bar.cxx -dynamiclib -o libfoobar.dylib
	${FAIL_IF_BAD_MACHO} libfoobar.dylib
	dwarfdump --eh-frame --verify libfoobar.dylib > /dev/null
	nm libfoobar.dylib | grep '.eh' | ${PASS_IFF_EMPTY}

clean:
	rm  *.dylib *.o
