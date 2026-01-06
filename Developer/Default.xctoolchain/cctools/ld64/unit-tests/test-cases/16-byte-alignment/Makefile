##
# Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
# Test that -O2 optimization fails when trying to make a long 16-byte aligned.
#

run: all

all:
	${FAIL_IF_ERROR} ${CC} ${CCFLAGS} -arch ${ARCH} -c -O2 tl_test2.c -o tl_test2-${ARCH}.o

	# verify that the alignment is correct in the .o
	${OBJECTDUMP} -only _ai -align -no_content tl_test2-${ARCH}.o|${FAIL_IF_ERROR} grep '0 mod 16' >/dev/null

	# now verify the executable
	${FAIL_IF_ERROR} ${CC} ${CCFLAGS} -arch ${ARCH} -O2 tl_test2-${ARCH}.o -o tl_test2-${ARCH}
	${FAIL_IF_ERROR} sh -c "nm tl_test2-${ARCH}|grep '0 D _ai' >/dev/null"
	${PASS_IFF_GOOD_MACHO} tl_test2-${ARCH}

clean:
	rm -rf tl_test2-*
