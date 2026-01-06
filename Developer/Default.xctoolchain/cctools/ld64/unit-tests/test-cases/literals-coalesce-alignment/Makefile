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
# The point of this test is to verify that when two cstrings
# are coalesced that the one with greater alignment is used.
#

run: all

all:
	${CC} ${ASMFLAGS} cstring-align-0.s -c -o cstring-align-0-${ARCH}.o
	${FAIL_IF_BAD_OBJ} cstring-align-0-${ARCH}.o

	${CC} ${ASMFLAGS} cstring-align-3.s -c -o cstring-align-3-${ARCH}.o
	${FAIL_IF_ERROR} ${OBJECTDUMP} cstring-align-3-${ARCH}.o | grep 'align:' > align-3

	${LD} -arch ${ARCH} -r cstring-align-0-${ARCH}.o cstring-align-3-${ARCH}.o -o cstring-r-${ARCH}.o
	${FAIL_IF_ERROR} ${OBJECTDUMP} cstring-r-${ARCH}.o | grep 'align:' > align-r

	${PASS_IFF} diff align-3 align-r

clean:
	rm -rf *.o align-3 align-r
