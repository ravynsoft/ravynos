##
# Copyright (c) 2011 Apple Inc. All rights reserved.
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
# Check compiler emitted compact unwind info
#

all: all-${FILEARCH}

all-i386: all-cu
all-x86_64: all-cu
all-arm: all-good

all-cu:
	${CC} ${CCFLAGS} test.s -c -o test.o
	${OBJECTDUMP} test.o > test.o.dump
	${LD} -r -arch ${ARCH} test.o -o test-r.o
	${OBJECTDUMP} test-r.o > test-r.o.dump
	${PASS_IFF} diff test.o.dump test-r.o.dump

all-good:
	${PASS_IFF} true

clean:
	rm -f test.o test-r.o test.o.dump test-r.o.dump
