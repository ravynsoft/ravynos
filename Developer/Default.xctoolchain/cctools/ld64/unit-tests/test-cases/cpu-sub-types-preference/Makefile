##
# Copyright (c) 2008 Apple Inc. All rights reserved.
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
# Validate cpu subtype preference
#

test: test-${ARCH}

test-ppc64:
	${PASS_IFF} true

test-i386:
	${PASS_IFF} true

test-x86_64:
	${PASS_IFF} true

test-armv6: test-arm
test-armv7: test-arm

test-arm:
	clang foo.c -arch armv4t -c -DAAA=1 -o foo4.o
	clang foo.c -arch armv5 -c -DBBB=1 -o foo5.o
	clang foo.c -arch armv6 -c -DCCC=1 -o foo6.o
	clang foo.c -arch armv7 -c -DDDD=1 -o foo7.o
	lipo foo4.o foo5.o foo6.o foo7.o -create -output foo.o
	
	# check -arch armv4t pulls out V4 slice
	${LD} -r -arch armv4t foo.o -o fooa.o
	otool -hv fooa.o | grep V4T | ${FAIL_IF_EMPTY}
	nm fooa.o | grep _aaa | ${FAIL_IF_EMPTY}

	# check -arch armv5 pulls out V5 slice
	${LD} -r -arch armv5 foo.o -o foob.o
	otool -hv foob.o | grep V5 | ${FAIL_IF_EMPTY}
	nm foob.o | grep _bbb | ${FAIL_IF_EMPTY}

	# check -arch armv6 pulls out V6 slice
	${LD} -r -arch armv6 foo.o -o fooc.o
	otool -hv fooc.o | grep V6 | ${FAIL_IF_EMPTY}
	nm fooc.o | grep _ccc | ${FAIL_IF_EMPTY}

	# check -arch armv7 pulls out V7 slice
	${LD} -r -arch armv7 foo.o -o fooc.o
	otool -hv fooc.o | grep V7 | ${FAIL_IF_EMPTY}
	nm fooc.o | grep _ddd | ${FAIL_IF_EMPTY}

	${PASS_IFF} true


test-ppc:
	gcc foo.c -arch ppc750 -c -DAAA=1 -o foog3.o
	gcc foo.c -arch ppc7400 -c -DBBB=1 -o foog4.o
	gcc foo.c -arch ppc970 -c -DCCC=1 -o foog5.o
	lipo foog3.o foog4.o foog5.o -create -output foo.o
	
	# check -arch ppc750 pulls out g3 slice
	${LD} -r -arch ppc750 foo.o -o fooa.o
	otool -hv fooa.o | grep ppc750 | ${FAIL_IF_EMPTY}
	nm fooa.o | grep _aaa | ${FAIL_IF_EMPTY}

	# check -arch ppc7400 pulls out g4 slice
	${LD} -r -arch ppc7400 foo.o -o foob.o
	otool -hv foob.o | grep ppc7400 | ${FAIL_IF_EMPTY}
	nm foob.o | grep _bbb | ${FAIL_IF_EMPTY}

	# check -arch ppc970 pulls out g5 slice
	${LD} -r -arch ppc970 foo.o -o fooc.o
	otool -hv fooc.o | grep ppc970 | ${FAIL_IF_EMPTY}
	nm fooc.o | grep _ccc | ${FAIL_IF_EMPTY}

	${PASS_IFF} true


clean:
	rm -f *.o
