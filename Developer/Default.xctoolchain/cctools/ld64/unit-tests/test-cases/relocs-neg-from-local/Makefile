##
# Copyright (c) 2009-2010 Apple Inc. All rights reserved.
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
# <rdar://problem/6583757> i386 relocation error with negative offsets from local labels
#

ifeq (${ARCH},i386)
	TARGET =  run-i386
else 
	TARGET =  run-other
endif

run: ${TARGET}

run-other:
	${PASS_IFF} /usr/bin/true
	

run-i386:
	${CC} ${ASMFLAGS} test.s -c -o test.o
	${OBJECTDUMP} test.o | grep "direct(anon@__data+0x00000000) + 0xFFFFFFFFFFFFFFE2" | ${FAIL_IF_EMPTY}

	${LD} -arch ${ARCH} -r -keep_private_externs test.o -o test-r.o
	${OBJECTDUMP} test-r.o | grep "direct(anon@__data+0x00000000) + 0xFFFFFFFFFFFFFFE2" | ${PASS_IFF_STDIN}

	
clean:
	rm -rf *.o
