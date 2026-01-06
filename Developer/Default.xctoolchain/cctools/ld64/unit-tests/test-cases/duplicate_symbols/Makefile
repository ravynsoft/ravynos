##
# Copyright (c) 2007-2010 Apple Inc. All rights reserved.
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
# Check that multiple duplicate symbols are reported.
#

all:
	${CC} -arch ${ARCH} -c main_extern.c
	${CC} -arch ${ARCH} -c main_no_extern.c
	${CC} -arch ${ARCH} -c duplicates.c
	${CC} -arch ${ARCH} -dynamiclib -o duplicates.dylib duplicates.o 
	libtool -static -o duplicates.a duplicates.o

	# simple case of directly linking duplicates should fail
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main_extern.o duplicates.o 2>&1 | grep "2 duplicate symbols" | ${FAIL_IF_EMPTY}


	# duplicates in a dylib succeed
	${FAIL_IF_ERROR} ${CC} ${CCFLAGS} main_extern.o duplicates.dylib 2>&1 | grep "2 duplicate symbols" | ${FAIL_IF_STDIN}

	# static lib - only fail if module containing duplicate symbols is actually referenced
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main_extern.o duplicates.a 2>&1 | grep "2 duplicate symbols" | ${FAIL_IF_EMPTY}
	${FAIL_IF_ERROR} ${CC} ${CCFLAGS} main_no_extern.o duplicates.a 2>&1 | grep "2 duplicate symbols" | ${FAIL_IF_STDIN}

	# simple case should succeed if we dead strip because none of the duplicates are referenced
	${FAIL_IF_ERROR} ${CC} ${CCFLAGS} -dead_strip main_extern.o duplicates.o 

	rm -f a.out *.o *.a *.dylib
	${PASS_IFF} true
