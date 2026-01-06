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
# can link a program with a large zero-fill section
#

all:	test-${FILEARCH}

# i386 catches the problem in the assembler phase
test-i386:
	${PASS_IFF} true

test-x86_64:
	${CC} ${CCFLAGS} -DSHRINK=1 test.c --save-temps -o test-${ARCH}
	${PASS_IFF_GOOD_MACHO} test-${ARCH}

test-ppc:
	${PASS_IFF_ERROR} ${CC} ${CCFLAGS} -DSHRINK=4 test.c --save-temps -o test-${ARCH} 2>/dev/null

test-arm:
	${PASS_IFF_ERROR} ${CC} ${CCFLAGS} -DSHRINK=4 test.c --save-temps -o test-${ARCH} 2>/dev/null

clean:
	rm -rf test-* *.o *.s *.i *.bc
