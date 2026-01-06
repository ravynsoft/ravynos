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
# The point of this test is to check that a non-lazy-pointer
# in foo.o to a private-extern symbol in bar.o will
# properly survive ld -r
#

run: all

all:
	${CC} ${CCFLAGS} -c foo.c -o foo.o
	${FAIL_IF_BAD_OBJ} foo.o

	${CC} ${CCFLAGS} -c bar.c -o bar.o
	${FAIL_IF_BAD_OBJ} bar.o

	${LD} -r foo.o bar.o -o foobar.o -arch ${ARCH}
	${FAIL_IF_BAD_OBJ} foobar.o

	${CC} ${CCFLAGS} hello.c foobar.o -o hello
	${FAIL_IF_BAD_MACHO} hello

	${LD} -r foo.o bar.o -o foobar2.o -arch ${ARCH} -keep_private_externs
	${FAIL_IF_BAD_OBJ} foobar2.o

	${CC} ${CCFLAGS} hello.c foobar2.o -o hello2
	${PASS_IFF_GOOD_MACHO} hello2

clean:
	rm -rf *.o hello hello2
