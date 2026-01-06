##
# Copyright (c) 2013 Apple Inc. All rights reserved.
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
# test thumb2 branch ranges
#

run: all

all:	all-${ARCH}

all-i386:
	${PASS_IFF} true

all-x86_64:
	${PASS_IFF} true

all-armv7:
	${CC} ${CCFLAGS} -static foo.c -c 
	${CC} ${CCFLAGS} bar.s -c 
	# verify islands are created and used
	${CC} ${CCFLAGS} foo.o bar.o -e _foo -o foobar -nostdlib -Wl,-preload -segaddr __MY 0x3000000
	${OTOOL} -s __MY __text -V foobar | grep '_myweak1.island:' | ${FAIL_IF_EMPTY}
	${OTOOL} -s __MY __text -V foobar | grep '_foo2.island:' | ${FAIL_IF_EMPTY}
	${OTOOL} -s __MY __text -V foobar | grep '_foo.island:' | ${FAIL_IF_EMPTY}
	${OTOOL} -s __MY __text -V foobar | grep "bl\t_foo.island" | ${FAIL_IF_EMPTY}
	# verify that is close enough islands are bypassed and target called directly
	${CC} ${CCFLAGS} foo.o bar.o -e _foo -o foobar2 -nostdlib -Wl,-preload -segaddr __MY 0x30000
	${OTOOL} -s __MY __text -V foobar2 | grep 'bl\t_foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -s __MY __text -V foobar2 | grep 'bl\t_foo2' | ${FAIL_IF_EMPTY}
	${OTOOL} -s __MY __text -V foobar2 | grep 'bl\t_myweak1' | ${FAIL_IF_EMPTY}
	${PASS_IFF} true

clean:
	rm  *.o foobar*
