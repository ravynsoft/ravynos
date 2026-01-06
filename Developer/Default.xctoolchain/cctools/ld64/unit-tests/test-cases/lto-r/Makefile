##
# Copyright (c) 2014 Apple Inc. All rights reserved.
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
# Check that -r mode preserves symbols with LTO
#


run: all

all:
	${CC} ${CCFLAGS} -flto foo.c -c -o foo.o
	${CC} ${CCFLAGS} bar.c -c -o bar.o
	${LD} -arch ${ARCH} -r foo.o bar.o -o foobar.o 
	nm -nm foobar.o | grep _foo_hidden | grep non-external | ${FAIL_IF_EMPTY}
	nm -nm foobar.o | grep _foo_static | grep non-external | ${FAIL_IF_EMPTY}
	nm -nm foobar.o | grep _foo_weak_hidden | grep non-external | ${FAIL_IF_EMPTY}
	nm -nm foobar.o | grep _foo_weak_global | grep "weak external " | ${FAIL_IF_EMPTY}
	nm -nm foobar.o | grep _foo_global | grep " external " | ${FAIL_IF_EMPTY}
	nm -nm foobar.o | grep _bar | grep " external " | ${FAIL_IF_EMPTY}
	${LD} -arch ${ARCH} -r -keep_private_externs foo.o bar.o -o foobar2.o 
	nm -nm foobar2.o | grep _foo_hidden | grep "private external" | ${FAIL_IF_EMPTY}
	nm -nm foobar2.o | grep _foo_static | grep non-external | ${FAIL_IF_EMPTY}
	nm -nm foobar2.o | grep _foo_weak_hidden | grep "weak private external" | ${FAIL_IF_EMPTY}
	nm -nm foobar2.o | grep _foo_weak_global | grep "weak external " | ${FAIL_IF_EMPTY}
	nm -nm foobar2.o | grep _foo_global | grep " external " | ${FAIL_IF_EMPTY}
	nm -nm foobar2.o | grep _bar | grep " external " | ${FAIL_IF_EMPTY}
	${PASS_IFF} true

clean:
	rm -f foo.o bar.o foobar.o foobar2.o
